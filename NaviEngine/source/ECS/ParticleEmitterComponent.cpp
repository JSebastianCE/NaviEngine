#include "ECS/ParticleEmitterComponent.h"
#include "DeviceContext.h"
#include "EngineUtilities/Utilities/LayoutBuilder.h"
#include <d3d11.h>
#include "Device.h" 
#include <cmath>
#include <cstdlib> // Necesario para generar números aleatorios con rand()

void ParticleEmitterComponent::init() {
  // --- 1. CONFIGURACIÓN DEL SHADER Y SU ESTRUCTURA ---
  // Le decimos a la tarjeta gráfica cómo va a leer la información de cada vértice de nuestras partículas.
  // En este caso, cada vértice tiene una posición, un color, un tamaño y una forma.
  LayoutBuilder layout;
  layout
    .Add("POSITION", DXGI_FORMAT_R32G32B32_FLOAT)
    .Add("COLOR", DXGI_FORMAT_R32G32B32A32_FLOAT)
    .Add("SIZE", DXGI_FORMAT_R32_FLOAT)
    .Add("SHAPE", DXGI_FORMAT_R32_UINT);

  m_particleShader.init(*m_device, "ParticleShader.hlsl", layout);

  // --- 2. CREACIÓN DEL BUFFER DE MEMORIA (CPU -> GPU) ---
  // Este buffer es un espacio de memoria dinámico. Lo usamos para mandar la información 
  // actualizada de las partículas desde el procesador (CPU) a la tarjeta gráfica (GPU) en cada frame.
  D3D11_BUFFER_DESC vbd = {};
  vbd.Usage = D3D11_USAGE_DYNAMIC;
  // Cada partícula se dibuja como un cuadrado (compuesto por 2 triángulos = 6 vértices)
  vbd.ByteWidth = sizeof(ParticleVertex) * m_maxParticles * 6;
  vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // Permite que la CPU escriba datos aquí
  vbd.MiscFlags = 0;

  m_device->m_device->CreateBuffer(&vbd, nullptr, &m_dynamicVertexBuffer);

  // --- 3. CONFIGURACIÓN DEL EFECTO VISUAL (MEZCLA ADITIVA) ---
  // La mezcla aditiva hace que los colores se sumen al superponerse. 
  // Es ideal para efectos brillantes como fuego, magia, humo iluminado o chispas.
  D3D11_BLEND_DESC blendDesc = {};
  blendDesc.RenderTarget[0].BlendEnable = TRUE;
  blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
  blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
  blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
  blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
  blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
  blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
  blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

  m_device->m_device->CreateBlendState(&blendDesc, &m_additiveBlendState);

  // --- 4. RENDERIZADO POR AMBAS CARAS (CULLING APAGADO) ---
  // Normalmente los motores gráficos no dibujan la parte de atrás de los polígonos para ahorrar recursos.
  // Aquí apagamos esa optimización para asegurarnos de que la partícula se vea desde cualquier ángulo.
  D3D11_RASTERIZER_DESC rasterDesc = {};
  rasterDesc.FillMode = D3D11_FILL_SOLID;
  rasterDesc.CullMode = D3D11_CULL_NONE;
  rasterDesc.FrontCounterClockwise = false;

  m_device->m_device->CreateRasterizerState(&rasterDesc, &m_noCullState);

  // --- 5. DIBUJADO SIEMPRE AL FRENTE (PROFUNDIDAD APAGADA) ---
  // Al apagar la prueba Z (profundidad), evitamos que las partículas se tapen entre sí 
  // de manera extraña o que geometría lejana corte el efecto visual.
  D3D11_DEPTH_STENCIL_DESC depthDesc = {};
  depthDesc.DepthEnable = false;
  depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
  depthDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;

  m_device->m_device->CreateDepthStencilState(&depthDesc, &m_noDepthState);

  // Pre-inicializamos un pequeño lote de partículas de prueba para evitar que el arreglo empiece vacío
  for (int i = 0; i < 10; ++i) {
    m_particles[i].active = true;
    m_particles[i].life = 0.1f;
    m_particles[i].maxLife = 1000.0f;
    m_particles[i].position = EU::Vector3(0.0f, 2.0f, 0.0f);
    m_particles[i].color = EU::Vector3(1.0f, 0.0f, 0.0f);
    m_particles[i].size = 5.0f;
  }
}

void ParticleEmitterComponent::update(float deltaTime) {
  // --- 1. LÓGICA DE NACIMIENTO DE PARTÍCULAS (SPAWN) ---
  m_spawnTimer += deltaTime;

  // Si ya pasó el tiempo suficiente según nuestra tasa de aparición...
  if (m_spawnTimer >= m_params.spawnRate) {
    m_spawnTimer = 0.0f; // Reiniciamos el reloj

    // Implementamos un "Object Pool": en lugar de crear y destruir memoria, 
    // buscamos una partícula que ya haya "muerto" (inactiva) y la reciclamos.
    for (auto& p : m_particles) {
      if (!p.active) {
        p.active = true;
        p.life = 0.0f; // Nace con 0 segundos de vida

        // Le asignamos un tiempo de vida aleatorio entre el mínimo y máximo configurado
        float randomLife = m_params.minLife + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (m_params.maxLife - m_params.minLife)));
        p.maxLife = randomLife;

        // Función auxiliar (Lambda) para no repetir la matemática de los números aleatorios
        auto randF = [](float min, float max) {
          return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
          };

        EU::Vector3 spawnPosition;
        float r, theta, phi;

        // --- DISTRIBUCIÓN ESPACIAL ---
        // Dependiendo de la forma seleccionada para el emisor, calculamos dónde nacerá la partícula
        switch (m_currentShape) {
        case EmitterShape::Point:
          // Nacen todas exactamente en el mismo punto central
          spawnPosition = EU::Vector3(0.0f, 0.0f, 0.0f);
          break;

        case EmitterShape::Sphere:
          // Se distribuyen uniformemente dentro de una esfera usando coordenadas esféricas
          r = m_radius * std::cbrt(randF(0.0f, 1.0f));
          theta = randF(0.0f, 2.0f * 3.14159265f);
          phi = std::acos(randF(-1.0f, 1.0f));
          spawnPosition = EU::Vector3(
            r * std::sin(phi) * std::cos(theta),
            r * std::sin(phi) * std::sin(theta),
            r * std::cos(phi)
          );
          break;

        case EmitterShape::Box:
          // Posición aleatoria dentro de los límites de una caja (ejes X, Y, Z)
          spawnPosition = EU::Vector3(
            randF(-m_boxExtents.x, m_boxExtents.x),
            randF(-m_boxExtents.y, m_boxExtents.y),
            randF(-m_boxExtents.z, m_boxExtents.z)
          );
          break;

        case EmitterShape::Circle:
          // Nacen en cualquier punto dentro de un círculo plano (suelo)
          r = m_radius * std::sqrt(randF(0.0f, 1.0f));
          theta = randF(0.0f, 2.0f * 3.14159265f);
          spawnPosition = EU::Vector3(
            r * std::cos(theta),
            0.0f,
            r * std::sin(theta)
          );
          break;

        case EmitterShape::Ring:
          // Nacen ÚNICAMENTE en el borde exterior del círculo, creando un anillo
          theta = randF(0.0f, 2.0f * 3.14159265f);
          spawnPosition = EU::Vector3(
            m_radius * std::cos(theta),
            0.0f,
            m_radius * std::sin(theta)
          );
          break;

        case EmitterShape::Cone:
          // Simula una erupción o chorro que se expande hacia arriba
          r = randF(0.0f, m_radius); // Altura aleatoria dentro del cono
          theta = randF(0.0f, 2.0f * 3.14159265f);
          phi = r * std::sqrt(randF(0.0f, 1.0f)); // Apertura basada en la altura
          spawnPosition = EU::Vector3(
            phi * std::cos(theta),
            r,
            phi * std::sin(theta)
          );
          break;

        case EmitterShape::Line:
          // Nacen aleatoriamente a lo largo de una línea recta en el eje X
          spawnPosition = EU::Vector3(
            randF(-m_radius, m_radius),
            0.0f,
            0.0f
          );
          break;
        }

        // Aplicamos los valores iniciales configurados en la UI/Parámetros
        p.position = spawnPosition;
        p.velocity = m_params.emitterVelocity;
        p.color = m_params.startColor;
        p.size = m_params.startSize;

        // --- ASIGNACIÓN DE FORMA (GEOMETRÍA) ---
        // Revisamos qué figuras están permitidas por el usuario
        std::vector<ParticleShape> allowedShapes;
        if (m_params.allowCircles) allowedShapes.push_back(ParticleShape::Circle);
        if (m_params.allowSquares) allowedShapes.push_back(ParticleShape::Square);
        if (m_params.allowTriangles) allowedShapes.push_back(ParticleShape::Triangle);

        // Si el usuario apagó todas las opciones, forzamos que al menos sean círculos
        if (allowedShapes.empty()) allowedShapes.push_back(ParticleShape::Circle);

        // Elegimos una figura al azar de entre las permitidas
        int randomShapeIndex = rand() % allowedShapes.size();
        p.shapeType = allowedShapes[randomShapeIndex];

        // Refrescamos posición y velocidad por seguridad
        p.position = spawnPosition;
        p.velocity = m_params.emitterVelocity;

        break; // Rompemos el ciclo porque solo queremos revivir UNA partícula por cada "tick" de spawn
      }
    }
  }

  // --- 2. ACTUALIZACIÓN DE LAS PARTÍCULAS VIVAS (FÍSICA Y VISUALES) ---
  for (auto& p : m_particles) {
    if (p.active) {
      p.life += deltaTime; // Envejecemos la partícula

      // Si ya superó su tiempo de vida, la matamos (la marcamos para ser reciclada luego)
      if (p.life >= p.maxLife) {
        p.active = false;
      }
      else {
        // --- FÍSICA ---
        // Aplicamos la gravedad modificada y actualizamos la posición basada en la velocidad
        p.velocity.y -= 9.81f * m_params.gravityMultiplier * deltaTime;
        p.position.x += p.velocity.x * deltaTime;
        p.position.y += p.velocity.y * deltaTime;
        p.position.z += p.velocity.z * deltaTime;

        // --- TRANSICIONES (INTERPOLACIÓN LÍNEAL / LERP) ---
        // 't' representa el porcentaje de vida de la partícula (va de 0.0 cuando nace a 1.0 cuando muere)
        float t = p.life / p.maxLife;

        // Calculamos su tamaño actual mezclando el inicial y el final basándonos en 't'
        p.size = m_params.startSize + (m_params.endSize - m_params.startSize) * t;

        // Hacemos lo mismo para transformar su color progresivamente a lo largo de su vida
        p.color.x = m_params.startColor.x + (m_params.endColor.x - m_params.startColor.x) * t;
        p.color.y = m_params.startColor.y + (m_params.endColor.y - m_params.startColor.y) * t;
        p.color.z = m_params.startColor.z + (m_params.endColor.z - m_params.startColor.z) * t;
      }
    }
  }
}

void ParticleEmitterComponent::render(DeviceContext& deviceContext) {
  // Comprobación de seguridad para evitar cuelgues si los recursos no se crearon
  if (!m_dynamicVertexBuffer || !m_additiveBlendState) return;

  // --- 1. PREPARACIÓN DE DATOS PARA LA GPU (MAPPING) ---
  // "Bloqueamos" el buffer temporalmente para que la CPU pueda escribir los datos actualizados de este frame.
  D3D11_MAPPED_SUBRESOURCE mappedData;
  HRESULT hr = deviceContext.m_deviceContext->Map(m_dynamicVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
  if (FAILED(hr)) return;

  ParticleVertex* vertices = reinterpret_cast<ParticleVertex*>(mappedData.pData);
  int activeParticleCount = 0;

  // Recorremos el pool y mandamos a la GPU solo las partículas vivas
  for (const auto& particle : m_particles) {
    if (particle.active) {
      // Calculamos la transparencia (Alpha). Se desvanecen conforme envejecen.
      float alpha = 1.0f - (particle.life / particle.maxLife);
      if (alpha < 0.0f) alpha = 0.0f;

      // Cada partícula necesita 6 vértices para formar el cuadrado plano donde se dibujará
      for (int i = 0; i < 6; ++i) {
        int index = (activeParticleCount * 6) + i;
        vertices[index].Position.x = particle.position.x;
        vertices[index].Position.y = particle.position.y;
        vertices[index].Position.z = particle.position.z;
        vertices[index].Color = XMFLOAT4(particle.color.x, particle.color.y, particle.color.z, alpha);
        vertices[index].Size = particle.size;
        vertices[index].ShapeType = static_cast<unsigned int>(particle.shapeType);
      }
      activeParticleCount++;
    }
  }
  // Liberamos el buffer para que la GPU ya pueda usarlo
  deviceContext.m_deviceContext->Unmap(m_dynamicVertexBuffer, 0);

  // Si no hay partículas vivas, nos ahorramos el trabajo de llamar al dibujado
  if (activeParticleCount == 0) return;

  // --- 2. CONFIGURACIÓN DEL PIPELINE GRÁFICO ---
  // Le decimos a la tarjeta gráfica dónde están los datos y cómo interpretarlos (como listas de triángulos)
  unsigned int stride = sizeof(ParticleVertex);
  unsigned int offset = 0;
  deviceContext.m_deviceContext->IASetVertexBuffers(0, 1, &m_dynamicVertexBuffer, &stride, &offset);
  deviceContext.m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  m_particleShader.render(deviceContext);

  // --- 3. APLICACIÓN DE ESTADOS Y DIBUJADO ---
  // Activamos nuestros estados especiales (mezcla brillante, sin recorte trasero, siempre al frente)
  float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
  deviceContext.m_deviceContext->OMSetBlendState(m_additiveBlendState, blendFactor, 0xffffffff);

  if (m_noCullState) {
    deviceContext.m_deviceContext->RSSetState(m_noCullState);
  }
  if (m_noDepthState) {
    deviceContext.m_deviceContext->OMSetDepthStencilState(m_noDepthState, 1);
  }

  // ¡La orden final para que la gráfica pinte las partículas en pantalla!
  deviceContext.m_deviceContext->Draw(activeParticleCount * 6, 0);

  // --- 4. LIMPIEZA DE ESTADOS ---
  // Devolvemos los estados a la normalidad para no arruinar el dibujado del resto de objetos del juego
  deviceContext.m_deviceContext->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
  deviceContext.m_deviceContext->RSSetState(nullptr);
  deviceContext.m_deviceContext->OMSetDepthStencilState(nullptr, 0);
}

void ParticleEmitterComponent::destroy() {
  // Limpieza de memoria tradicional. 
  // Soltamos todos los recursos creados en la GPU para evitar fugas de memoria (Memory Leaks).
  m_particles.clear();
  m_particleShader.destroy();

  if (m_additiveBlendState) {
    m_additiveBlendState->Release();
    m_additiveBlendState = nullptr;
  }

  if (m_dynamicVertexBuffer) {
    m_dynamicVertexBuffer->Release();
    m_dynamicVertexBuffer = nullptr;
  }

  if (m_noCullState) {
    m_noCullState->Release();
    m_noCullState = nullptr;
  }

  if (m_noDepthState) {
    m_noDepthState->Release();
    m_noDepthState = nullptr;
  }
}