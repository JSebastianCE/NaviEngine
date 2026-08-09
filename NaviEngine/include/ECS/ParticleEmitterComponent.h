#pragma once

/**
 * @file ParticleEmitterComponent.h
 * @brief Definición de las estructuras y la clase principal para el sistema de partículas.
 */

#include "Prerequisites.h"
#include "ECS/Component.h"
#include "EngineUtilities/Vectors/Vector3.h"
#include "ShaderProgram.h"
#include "ECS/Transform.h"
#include "EngineUtilities/Utilities/Camera.h"

class DeviceContext;
class Device;

/**
 * @enum EmitterShape
 * @brief Define el volumen espacial desde el cual se emitirán las partículas.
 */
enum 
class EmitterShape {
  Point,   ///< Emisión desde un único punto.
  Sphere,  ///< Emisión volumétrica dentro de una esfera.
  Box,     ///< Emisión volumétrica dentro de una caja.
  Circle,  ///< Emisión plana dentro de un círculo.
  Ring,    ///< Emisión en los bordes de un anillo.
  Cone,    ///< Emisión en forma cónica.
  Line     ///< Emisión a lo largo de una línea.
};

/**
 * @enum ParticleShape
 * @brief Define la forma procedimental (SDF) que tendrá la partícula al renderizarse.
 */
enum
class ParticleShape {
  Circle = 0,   ///< Forma circular.
  Square = 1,   ///< Forma cuadrada.
  Triangle = 2  ///< Forma triangular.
};


// 1. EL STRUCT DE LA PARTÍCULA
/**
 * @struct Particle
 * @brief Estructura que representa el estado atómico e individual de una partícula.
 */
struct Particle {
  EU::Vector3 position; ///< Posición actual en el espacio 3D.
  EU::Vector3 velocity; ///< Velocidad y dirección actual.
  EU::Vector3 color;    ///< Color actual de la partícula (O XMFLOAT4 si necesitas canal Alpha).

  float life = 0.0f;    ///< Tiempo de vida transcurrido (edad de la partícula).
  float maxLife = 1.0f; ///< Tiempo de vida total máximo antes de reciclarse.
  float size = 1.0f;    ///< Escala actual de la partícula.

  bool active = false;  ///< Si es false, la partícula está "muerta" y disponible para reciclarse.

  ParticleShape shapeType = ParticleShape::Circle; ///< Forma geométrica de la partícula.
};

// 2. LOS PARÁMETROS DEL EMISOR (Lo que modificaremos desde ImGui)
/**
 * @struct EmitterParams
 * @brief Parámetros de configuración general del emisor que determinan el comportamiento del efecto visual.
 */
struct
EmitterParams {
  float spawnRate = 0.1f;        ///< Segundos de espera entre la generación de cada nueva partícula.
  float gravityMultiplier = 1.0f; ///< Multiplicador de la fuerza de gravedad.

  float minLife = 1.0f;          ///< Tiempo de vida mínimo aleatorio al nacer.
  float maxLife = 3.0f;          ///< Tiempo de vida máximo aleatorio al nacer.

  float startSize = 1.0f;        ///< Tamaño inicial de la partícula al generarse.
  float endSize = 0.1f;          ///< Tamaño final de la partícula justo antes de morir.

  // Nace de color Morado (mezcla de rojo y azul)
  EU::Vector3 startColor = { 0.6f, 0.0f, 1.0f }; ///< Color inicial de la partícula.

  // Muere de color Cyan (mezcla de verde y azul)
  EU::Vector3 endColor = { 0.0f, 1.0f, 1.0f };   ///< Color de la partícula al finalizar su ciclo de vida.

  // Contorno Blanco por defecto (para cuando lo conectemos al shader)
  XMFLOAT4 outlineColor = { 1.0f, 1.0f, 1.0f, 1.0f }; ///< Color global del contorno suave (SDF).

  EU::Vector3 emitterVelocity = { 0.0f, 2.0f, 0.0f }; ///< Velocidad base agregada a las partículas al nacer.

  // Filtros de forma (pueden estar varios encendidos a la vez)
  bool allowCircles = true;     ///< Permite la instanciación de partículas circulares.
  bool allowSquares = false;    ///< Permite la instanciación de partículas cuadradas.
  bool allowTriangles = false;  ///< Permite la instanciación de partículas triangulares.
};


/**
 * @class ParticleEmitterComponent
 * @brief Componente que genera y simula un sistema de partículas (VFX).
 *
 * Se integra en la arquitectura ECS y se encarga tanto de la simulación en CPU
 * como de preparar los datos para el renderizado en GPU.
 */
class ParticleEmitterComponent : public Component {
public:
  /**
   * @brief Constructor de la clase ParticleEmitterComponent.
   * @param device Puntero al dispositivo lógico de DirectX para inicialización de recursos.
   */
  ParticleEmitterComponent(Device* device)
    : Component(ComponentType::PARTICLE_EMITTER), m_device(device) {
    m_particles.resize(m_maxParticles);
  }

  EmitterShape m_currentShape = EmitterShape::Sphere; ///< Forma actual del volumen de emisión.

  /**
   * @brief Establece el tipo de volumen de emisión.
   * @param shape El nuevo volumen de emisión (ej. Sphere, Box).
   */
  void 
  setShape(EmitterShape shape) { m_currentShape = shape; }

  /**
   * @brief Obtiene el volumen de emisión actual.
   * @return EmitterShape Volumen activo.
   */
  EmitterShape getShape() const { return m_currentShape; }

  /**
   * @brief Obtiene el radio actual de emisión.
   * @return float Radio del volumen.
   */
  float
  getRadius() const { return m_radius; }

  /**
   * @brief Modifica el radio de emisión.
   * @param newRadius Nuevo valor para el radio.
   */
  void 
  setRadius(float newRadius) { m_radius = newRadius; }

  // Parámetros de las formas
  float m_radius = 5.0f; ///< Radio de emisión (usado para formas como Esfera y Círculo).
  EU::Vector3 m_boxExtents = EU::Vector3(5.0f, 5.0f, 5.0f); ///< Dimensiones del volumen de la caja (ancho, alto, profundidad).

  /**
   * @brief Inicializa los recursos internos y buffers gráficos de las partículas.
   */
  void 
  init() override;

  /**
   * @brief Actualiza la cinemática, el ciclo de vida y la generación (spawning) de partículas.
   * @param deltaTime Tiempo transcurrido desde el último fotograma (en segundos).
   */
  void 
  update(float deltaTime) override;

  /**
   * @brief Ejecuta el pase de renderizado de las partículas activas enviando los datos a la GPU.
   * @param deviceContext Contexto del dispositivo para emitir comandos de dibujo.
   */
  void 
  render(DeviceContext& deviceContext) override;

  /**
   * @brief Libera los recursos de memoria y buffers de DirectX utilizados por el emisor.
   */
  void
  destroy() override;

  // Getters y Setters para ImGui
  /**
   * @brief Obtiene una referencia modificable a los parámetros del emisor.
   * @return Referencia a EmitterParams.
   */
  EmitterParams& getParams() { return m_params; }

  /**
   * @brief Obtiene una referencia de solo lectura a los parámetros del emisor.
   * @return Referencia constante a EmitterParams.
   */
  const EmitterParams& getParams() const { return m_params; }

  // Función para obtener las partículas (útil para debug o render)
  /**
   * @brief Devuelve la lista completa de partículas del sistema (Object Pool).
   * @return Referencia al std::vector que contiene las partículas.
   */
  std::vector<Particle>& getParticles() { return m_particles; }

private:
  EmitterParams m_params;                  ///< Parámetros paramétricos del VFX.
  std::vector<Particle> m_particles;       ///< Arreglo contiguo de partículas (Pool pre-alocado).
  int m_maxParticles = 1000;               ///< Límite máximo de partículas para no saturar la RAM.
  float m_spawnTimer = 0.0f;               ///< Temporizador interno para generar nuevas partículas.

private:
  ShaderProgram m_particleShader;                ///< Programa de shaders (Vertex y Pixel) para renderizar SDF y Billboarding.
  ID3D11BlendState* m_additiveBlendState = nullptr; ///< Estado de mezcla (Blending) aditivo para las partículas.
  ID3D11Buffer* m_dynamicVertexBuffer = nullptr;    ///< Vertex Buffer dinámico mapeable entre CPU y GPU.

  ID3D11RasterizerState* m_noCullState = nullptr;   ///< Estado de rasterizador para deshabilitar el Culling.
  ID3D11DepthStencilState* m_noDepthState = nullptr;///< Estado de profundidad para deshabilitar la escritura en Z-Buffer.

  Device* m_device = nullptr;                       ///< Puntero al dispositivo DirectX utilizado para crear recursos gráficos.
};