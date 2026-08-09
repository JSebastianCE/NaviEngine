#include "BaseApp.h"
#include "ResourceManager.h"
#include <fstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

// Necesario para que Win32 reenvíe los inputs a ImGui
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

BaseApp::BaseApp(HINSTANCE hInst, int nCmdShow) {
  // Constructor vacío
}

HRESULT
BaseApp::awake() {
  HRESULT hr = S_OK;

  m_sceneGraph.init();

  MESSAGE("Main", "Awake", "Application awake successfully.");
  return hr;
}

int
BaseApp::run(HINSTANCE hInst, int nCmdShow) {
  if (FAILED(m_window.init(hInst, nCmdShow, WndProc, this))) {
    ERROR("Main", "Run", "Failed to initialize window.");
    return 0;
  }

  if (FAILED(awake())) {
    ERROR("Main", "Run", "Failed to awake application.");
    return 0;
  }

  if (FAILED(init())) {
    ERROR("Main", "Run", "Failed to initialize device and device context.");
    return 0;
  }

  m_gui.init(m_window, m_device, m_deviceContext);
  m_guiInitialized = true;

  MSG msg = {};
  LARGE_INTEGER freq, prev;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&prev);

  while (WM_QUIT != msg.message)
  {
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    else
    {
      LARGE_INTEGER curr;
      QueryPerformanceCounter(&curr);
      float deltaTime = static_cast<float>(curr.QuadPart - prev.QuadPart) / freq.QuadPart;

      // LIMITADOR DE TIEMPO (120 FPS MAX) ---
      const float targetFPS = 120.0f;
      const float targetFrameTime = 1.0f / targetFPS; // Tiempo requerido por frame (~0.0083s)

      if (deltaTime < targetFrameTime)
      {
        // Calculam cuántos milisegundos nos sobran
        float sleepTimeMs = (targetFrameTime - deltaTime) * 1000.0f;

        // Si nos sobra más de 1 milisegundo, dormimos el hilo para ahorrar recursos
        if (sleepTimeMs > 1.0f) {
          Sleep(static_cast<DWORD>(sleepTimeMs - 1.0f));
        }
        // Usa 'continue' para volver al inicio del while SIN hacer update ni render aún
        continue;
      }

      // Solo actualiza "prev" cuando el tiempo haya superado el targetFrameTime (120 FPS)
      prev = curr;

      update(deltaTime);
      render();
    }
  }

  return (int)msg.wParam;
}

HRESULT
BaseApp::init() {
  HRESULT hr = S_OK;

  // 1. SwapChain
  hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize SwapChain. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // 2. Render Target View
  hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize RenderTargetView. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // 3. Depth Stencil Texture
  hr = m_depthStencil.init(
    m_device,
    m_window.m_width,
    m_window.m_height,
    DXGI_FORMAT_D24_UNORM_S8_UINT,
    D3D11_BIND_DEPTH_STENCIL,
    4,
    0
  );
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize DepthStencil. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // 4. Depth Stencil View
  hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize DepthStencilView. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // 5. Viewport
  hr = m_viewport.init(m_window);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize Viewport. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  m_d3dReady = true;

  // --------------------------------------------------------------------------
  // SKYBOX
  // --------------------------------------------------------------------------
  std::array<std::string, 6> faces = {
    "Skybox/cubemap_0.png",
    "Skybox/cubemap_1.png",
    "Skybox/cubemap_2.png",
    "Skybox/cubemap_3.png",
    "Skybox/cubemap_4.png",
    "Skybox/cubemap_5.png"
  };
  m_skyboxTex.CreateCubemap(m_device, m_deviceContext, faces, false);

  // Icono 2D para marcar las luces en el viewport (NO afecta la escena 3D).
  HRESULT lightIconHr = m_lightIconTexture.init(m_device, "Assets/Icons/light_icon", ExtensionType::PNG);
  if (FAILED(lightIconHr)) {
    MESSAGE("Main", "InitDevice", "No se encontro el icono de luz. Se usara un marcador por defecto.");
  }


  // --------------------------------------------------------------------------
  // ACTOR PRINCIPAL:  HACHA
  // --------------------------------------------------------------------------
  m_cyberGun = EU::MakeShared<Actor>(m_device);

  if (!m_cyberGun.isNull()) {
    m_model = new Model3D("Assets/SF_Golden_double_axe_.fbx", ModelType::FBX);
    if (!m_model || !m_model->load("Assets/SF_Golden_double_axe_.fbx")) {
      ERROR("Main", "InitDevice", "Failed to load SF_Golden_double_axe_ model.");
      return E_FAIL;
    }

    hr = m_AlbedoSRV.init(m_device, "Assets/T_Golden_double_Axe_D", ExtensionType::JPG);
    if (FAILED(hr)) {
      ERROR("Main", "InitDevice",
        ("Failed to initialize Albedo texture. HRESULT: " + std::to_string(hr)).c_str());
      return hr;
    }

    hr = m_MetallicSRV.init(m_device, "Assets/T_Golden_double_Axe_M", ExtensionType::JPG);
    if (FAILED(hr)) {
      ERROR("Main", "InitDevice",
        ("Failed to initialize Metallic texture. HRESULT: " + std::to_string(hr)).c_str());
      return hr;
    }

    hr = m_RoughnessSRV.init(m_device, "Assets/T_Golden_double_Axe_R", ExtensionType::JPG);
    if (FAILED(hr)) {
      ERROR("Main", "InitDevice",
        ("Failed to initialize Roughness texture. HRESULT: " + std::to_string(hr)).c_str());
      return hr;
    }

    hr = m_AOSRV.init(m_device, "Assets/T_Golden_double_Axe_AO", ExtensionType::JPG);
    if (FAILED(hr)) {
      ERROR("Main", "InitDevice",
        ("Failed to initialize AO texture. HRESULT: " + std::to_string(hr)).c_str());
      return hr;
    }

    hr = m_NormalSRV.init(m_device, "Assets/T_Golden_double_Axe_N", ExtensionType::JPG);
    if (FAILED(hr)) {
      ERROR("Main", "InitDevice",
        ("Failed to initialize Normal texture. HRESULT: " + std::to_string(hr)).c_str());
      return hr;
    }

    m_cyberGun->setName("CyberGun");
    m_actors.push_back(m_cyberGun);

    EU::TSharedPointer<Transform> transform = m_cyberGun->getComponent<Transform>();
    if (transform) {
      transform->setTransform(
        EU::Vector3(-6.5f, 1.0f, 36.0f),
        EU::Vector3(-90.0f, 0.0f, 0.0f),
        EU::Vector3(0.30f, 0.30f, 0.30f)
      );
      transform->rebuildMatrixFromVectors();
    }
  }
  else {
    ERROR("Main", "InitDevice", "Failed to create CyberGun Actor.");
    return E_FAIL;
  }

  // --------------------------------------------------------------------------
  // SCENE GRAPH
  // --------------------------------------------------------------------------
  for (auto& actor : m_actors) {
    m_sceneGraph.addEntity(actor.get());
  }

  // --------------------------------------------------------------------------
  // SHADER BASE
  // --------------------------------------------------------------------------
  LayoutBuilder builder;
  builder.Add("POSITION", DXGI_FORMAT_R32G32B32_FLOAT)
    .Add("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT)
    .Add("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
    .Add("BITANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
    .Add("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);

  hr = m_shaderProgram.init(m_device, "PBRShader.hlsl", builder);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize ShaderProgram. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  hr = m_constantBuffer.init(m_device, sizeof(CBMain));
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize m_constantBuffer Buffer. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // --------------------------------------------------------------------------
  // CAMERA / LIGHT
  // --------------------------------------------------------------------------
  m_camera.setLens(XM_PIDIV4, m_window.m_width / (float)m_window.m_height, 0.01f, 100.0f);
  m_camera.setPosition(0.0f, 3.0f, -6.0f);

  m_constantBufferStruct.LightColor = EU::Vector3(1.0f, 1.0f, 1.0f);
  m_constantBufferStruct.LightDir = EU::Vector3(-0.20f, -1.0f, 1.0f);

  // --------------------------------------------------------------------------
  // SKYBOX
  // --------------------------------------------------------------------------
  m_skybox.init(m_device, &m_deviceContext, m_skyboxTex);

  // --------------------------------------------------------------------------
  // ESTADOS POR DEFECTO
  // --------------------------------------------------------------------------
  hr = m_defaultRasterizer.init(m_device, D3D11_FILL_SOLID, D3D11_CULL_BACK, false, true);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize default Rasterizer. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  hr = m_defaultDepthStencil.init(m_device, true, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize default DepthStencilState. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  hr = m_defaultSampler.init(m_device);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize default SamplerState. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // --------------------------------------------------------------------------
  // MATERIALES
  // --------------------------------------------------------------------------
  m_pbrMaterial.setShader(&m_shaderProgram);
  m_pbrMaterial.setRasterizerState(&m_defaultRasterizer);
  m_pbrMaterial.setDepthStencilState(&m_defaultDepthStencil);
  m_pbrMaterial.setSamplerState(&m_defaultSampler);
  m_pbrMaterial.setDomain(MaterialDomain::Opaque);
  m_pbrMaterial.setBlendMode(BlendMode::Opaque);

  m_transparentPbrMaterial.setShader(&m_shaderProgram);
  m_transparentPbrMaterial.setRasterizerState(&m_defaultRasterizer);
  m_transparentPbrMaterial.setDepthStencilState(&m_defaultDepthStencil);
  m_transparentPbrMaterial.setSamplerState(&m_defaultSampler);
  m_transparentPbrMaterial.setDomain(MaterialDomain::Transparent);
  m_transparentPbrMaterial.setBlendMode(BlendMode::Alpha);

  m_cyberGunMaterial.setMaterial(&m_pbrMaterial);
  m_cyberGunMaterial.setAlbedo(&m_AlbedoSRV);
  m_cyberGunMaterial.setNormal(&m_NormalSRV);
  m_cyberGunMaterial.setMetallic(&m_MetallicSRV);
  m_cyberGunMaterial.setRoughness(&m_RoughnessSRV);
  m_cyberGunMaterial.setAO(&m_AOSRV);

  m_cyberGunMaterial.getParams().baseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
  m_cyberGunMaterial.getParams().metallic = 1.0f;
  m_cyberGunMaterial.getParams().roughness = 1.0f;
  m_cyberGunMaterial.getParams().ao = 1.0f;
  m_cyberGunMaterial.getParams().normalScale = 1.0f;
  m_cyberGunMaterial.getParams().alphaCutoff = 0.5f;

  // --------------------------------------------------------------------------
  // MESH RENDERIZABLE
  // --------------------------------------------------------------------------
  m_cyberGunRenderMesh.destroy();

  for (const MeshComponent& meshComponent : m_model->GetMeshes()) {
    Submesh submesh{};

    hr = submesh.vertexBuffer.init(m_device, meshComponent, D3D11_BIND_VERTEX_BUFFER);
    if (FAILED(hr)) {
      ERROR("Main", "InitDevice",
        ("Failed to initialize CyberGun vertex buffer. HRESULT: " + std::to_string(hr)).c_str());
      return hr;
    }

    hr = submesh.indexBuffer.init(m_device, meshComponent, D3D11_BIND_INDEX_BUFFER);
    if (FAILED(hr)) {
      ERROR("Main", "InitDevice",
        ("Failed to initialize CyberGun index buffer. HRESULT: " + std::to_string(hr)).c_str());
      return hr;
    }

    submesh.indexCount = meshComponent.m_numIndex;
    submesh.localTransform = meshComponent.m_localTransform;
    submesh.materialSlot = 0;

    m_cyberGunRenderMesh.getSubmeshes().push_back(std::move(submesh));
  }

  // --------------------------------------------------------------------------
  // MESH RENDERER COMPONENT
  // --------------------------------------------------------------------------
  EU::TSharedPointer<MeshRendererComponent> meshRenderer =
    m_cyberGun->getComponent<MeshRendererComponent>();

  if (!meshRenderer) {
    meshRenderer = EU::MakeShared<MeshRendererComponent>();
    m_cyberGun->addComponent(meshRenderer);
  }

  meshRenderer->setMesh(&m_cyberGunRenderMesh);
  meshRenderer->setMaterialInstance(&m_cyberGunMaterial);
  meshRenderer->setVisible(true);
  meshRenderer->setCastShadow(true);

  // --------------------------------------------------------------------------
  // LUZ DIRECCIONAL COMO ACTOR
  // --------------------------------------------------------------------------
  m_directionalLightActor = EU::MakeShared<Actor>(m_device);
  if (!m_directionalLightActor.isNull()) {
    m_directionalLightActor->setName("DirectionalLight");

    EU::TSharedPointer<LightComponent> lightComponent =
      m_directionalLightActor->getComponent<LightComponent>();

    if (!lightComponent) {
      lightComponent = EU::MakeShared<LightComponent>();
      m_directionalLightActor->addComponent(lightComponent);
    }

    lightComponent->getLightData().type = LightType::Directional;
    lightComponent->getLightData().direction = m_constantBufferStruct.LightDir;
    lightComponent->getLightData().color = m_constantBufferStruct.LightColor;
    lightComponent->getLightData().intensity = 1.0f;
    lightComponent->setCastShadow(false);

    m_sceneGraph.addEntity(m_directionalLightActor.get());
  }

  loadScene(getDefaultScenePath());

  // --------------------------------------------------------------------------
  // VIEWPORT PASS / RENDER PIPELINE (DEFERRED)
  // --------------------------------------------------------------------------
  hr = m_editorViewportPass.init(m_device, 1280, 720);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize EditorViewportPass. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  hr = m_renderPipeline.init(m_device, RendererType::Deferred);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize RenderPipeline. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  return S_OK;
}

void
BaseApp::update(float deltaTime) {

  // Aplica el resize diferido del viewport ANTES de construir la UI,
  // para que ImGui capture los SRVs nuevos (evita punteros colgantes al redimensionar).
  handleEditorViewportResize();

  static float t = 0.0f;
  if (m_swapChain.m_driverType == D3D_DRIVER_TYPE_REFERENCE)
  {
    t += (float)XM_PI * 0.0125f;
  }
  else
  {
    static DWORD dwTimeStart = 0;
    DWORD dwTimeCur = GetTickCount();
    if (dwTimeStart == 0)
      dwTimeStart = dwTimeCur;
    t = (dwTimeCur - dwTimeStart) / 1000.0f;
  }

  m_gui.update(m_viewport, m_window);

  // Crear luz desde el boton del ribbon
  if (m_gui.consumeCreateLightActorRequest()) {
    EU::TSharedPointer<Actor> lightActor = createLightActor();
    if (!lightActor.isNull()) {
      m_gui.selectedActorIndex = static_cast<int>(m_actors.size()) - 1;
    }
  }

  // Crear partículas desde el botón del ribbon
  if (m_gui.consumeCreateParticleActorRequest()) {
    EU::TSharedPointer<Actor> particleActor = createParticleActor();
    if (!particleActor.isNull()) {
      m_gui.selectedActorIndex = static_cast<int>(m_actors.size()) - 1;
    }
  }

  m_gui.drawViewportPanel(m_editorViewportPass.getSRV());
  m_gui.drawViewportLightIcons(m_actors, m_camera, m_lightIconTexture.m_textureFromImg);
 
  /*m_gui.drawRenderDebugPanel(
    m_renderPipeline.getPreShadowSRV(),
    m_editorViewportPass.getSRV(),
    m_renderPipeline.getShadowMapSRV()
  );*/

  m_gui.outliner(m_actors);

  EU::TSharedPointer<Actor> selectedActor;
  if (m_gui.selectedActorIndex >= 0 &&
    m_gui.selectedActorIndex < static_cast<int>(m_actors.size())) {
    selectedActor = m_actors[m_gui.selectedActorIndex];
  }

  m_gui.inspectorGeneral(selectedActor);

  m_gui.drawStatsPanel();

  // Panel del G-Buffer (deferred) + conexion de modos de debug al pipeline
  m_gui.drawGBufferDebugPanel(
    m_renderPipeline.getGBufferAlbedoMetallicSRV(),
    m_renderPipeline.getGBufferNormalRoughnessSRV(),
    m_renderPipeline.getGBufferWorldAoSRV(),
    m_renderPipeline.getGBufferEmissiveAlphaSRV(),
    selectedActor
  );

  m_renderPipeline.setShadowFactorDebugEnabled(m_gui.m_visualizeDeferredShadowFactor);
  m_renderPipeline.setDeferredDebugViewMode(m_gui.m_deferredDebugViewMode);

  m_gui.editTransform(m_camera, m_window, selectedActor);

  if (m_gui.consumeSaveSceneRequest()) {
    saveScene(getDefaultScenePath());
  }

  unsigned int desiredW = static_cast<unsigned int>(m_gui.m_viewportSize.x);
  unsigned int desiredH = static_cast<unsigned int>(m_gui.m_viewportSize.y);

  const unsigned int kMinViewportSize = 64;
  if (desiredW < kMinViewportSize) desiredW = kMinViewportSize;
  if (desiredH < kMinViewportSize) desiredH = kMinViewportSize;

  if (desiredW != m_lastRequestedViewportWidth || desiredH != m_lastRequestedViewportHeight)
  {
    m_lastRequestedViewportWidth = desiredW;
    m_lastRequestedViewportHeight = desiredH;
    m_viewportResizeStableFrames = 0;
  }
  else
  {
    m_viewportResizeStableFrames++;
  }

  const int kStableFramesRequired = 2;
  if (m_viewportResizeStableFrames >= kStableFramesRequired)
  {
    if (desiredW != m_editorViewportPass.getWidth() ||
      desiredH != m_editorViewportPass.getHeight())
    {
      m_editorViewportResizePending = true;
      m_pendingViewportWidth = desiredW;
      m_pendingViewportHeight = desiredH;
    }
  }

  // Aspect ratio de la camara = aspecto del render target del viewport del editor.
// Evita que el modelo se deforme cuando el panel cambia de forma.
  const unsigned int vpW = m_editorViewportPass.getWidth();
  const unsigned int vpH = m_editorViewportPass.getHeight();
  if (vpW > 0 && vpH > 0) {
    m_camera.setLens(XM_PIDIV4, static_cast<float>(vpW) / static_cast<float>(vpH), 0.01f, 100.0f);
  }

  // ----------------------------------------------------------------------
  // NAVEGACIÓN DE CÁMARA DCC 
  // ----------------------------------------------------------------------
  ImGuiIO& io = ImGui::GetIO();



    // SHORTCUTS DEL EDITOR (COPIAR, PEGAR, DUPLICAR) 
  bool isCtrlDown = io.KeyCtrl; // Detecta si Control está presionado

  // Asegurarnos de que no estamos escribiendo texto en un input de ImGui
  if (!io.WantTextInput) {

    // COPIAR (Ctrl + C)
    if (isCtrlDown && ImGui::IsKeyPressed(ImGuiKey_C)) {
      if (selectedActor) {
        m_clipboardActor = selectedActor;
        MESSAGE("Editor", "Shortcuts", "Actor copiado al portapapeles.");
      }
    }

    // PEGAR (Ctrl + V)
    if (isCtrlDown && ImGui::IsKeyPressed(ImGuiKey_V)) {
      if (!m_clipboardActor.isNull()) {
        auto newActor = cloneActor(m_clipboardActor);

        // Desfasa un poco la posición para que no aparezca exactamente dentro del original
        auto trans = newActor->getComponent<Transform>();
        EU::Vector3 pos = trans->getPosition();
        trans->setPosition(EU::Vector3(pos.x + 1.0f, pos.y, pos.z + 1.0f));
        trans->rebuildMatrixFromVectors();

        // Seleccionar automáticamente el nuevo actor pegado
        m_gui.selectedActorIndex = static_cast<int>(m_actors.size()) - 1;
        MESSAGE("Editor", "Shortcuts", "Actor pegado en la escena.");
      }
    }

    // DUPLICAR (Ctrl + D) = Copiar + Pegar al instante
    if (isCtrlDown && ImGui::IsKeyPressed(ImGuiKey_D)) {
      if (selectedActor) {
        auto newActor = cloneActor(selectedActor);
        m_gui.selectedActorIndex = static_cast<int>(m_actors.size()) - 1;
        MESSAGE("Editor", "Shortcuts", "Actor duplicado.");
      }
    }

    if (ImGui::IsKeyPressed(ImGuiKey_1)) m_gui.m_currentGizmoTool = 0; // Seleccionar
    if (ImGui::IsKeyPressed(ImGuiKey_2)) m_gui.m_currentGizmoTool = 1; // Traslación
    if (ImGui::IsKeyPressed(ImGuiKey_3)) m_gui.m_currentGizmoTool = 2; // Rotación
    if (ImGui::IsKeyPressed(ImGuiKey_4)) m_gui.m_currentGizmoTool = 3; // Escala
  }

  // mover la cámara si el mouse está dentro del Viewport 3D y se presiona ALT
  if (m_gui.m_viewportHovered && io.KeyAlt)
  {
    float deltaX = io.MouseDelta.x;
    float deltaY = io.MouseDelta.y;

    // Definir el punto de pivote para orbitar (Target)
    // Si hay un actor seleccionado, orbitamos a su alrededor; si no, al origen (0,0,0)
    EU::Vector3 pivotTarget(0.0f, 0.0f, 0.0f);
    if (selectedActor) {
      // pivotTarget = selectedActor->getComponent<TransformComponent>()->getPosition();
    }

    // 1. ALT + CLIC IZQUIERDO = ORBITAR (Orbit)
    if (io.MouseDown[0])
    {
      float sensitivity = 0.005f;
      float angleX = deltaX * sensitivity;
      float angleY = deltaY * sensitivity;

      // Obtener vector desde el objetivo hasta la cámara
      XMVECTOR camPos = XMVectorSet(m_camera.getPosition().x, m_camera.getPosition().y, m_camera.getPosition().z, 1.0f);
      XMVECTOR targetPos = XMVectorSet(pivotTarget.x, pivotTarget.y, pivotTarget.z, 1.0f);
      XMVECTOR dir = XMVectorSubtract(camPos, targetPos);

      // Rotar horizontalmente alrededor del eje Y global (Yaw)
      dir = XMVector3TransformCoord(dir, XMMatrixRotationY(angleX));

      // Rotar verticalmente alrededor del eje Right local de la cámara (Pitch)
      XMVECTOR right = XMVectorSet(m_camera.GetRight().x, m_camera.GetRight().y, m_camera.GetRight().z, 0.0f);
      dir = XMVector3TransformCoord(dir, XMMatrixRotationAxis(right, angleY));

      // Calcular la nueva posición de la cámara sumando el vector rotado al pivote
      XMVECTOR newCamPos = XMVectorAdd(targetPos, dir);
      XMFLOAT3 finalPos;
      XMStoreFloat3(&finalPos, newCamPos);

      m_camera.setPosition(finalPos.x, finalPos.y, finalPos.z);
      m_camera.lookAt(m_camera.getPosition(), pivotTarget);
    }
    // 2. ALT + CLIC MEDIO (RUEDA) = PANEO (Pan)
    else if (io.MouseDown[2])
    {
      float panSpeed = 0.015f;
      // Moverse lateralmente
      m_camera.strafe(-deltaX * panSpeed);

      // Moverse verticalmente en el eje Up local de la cámara
      EU::Vector3 upVector = m_camera.GetUp();
      m_camera.setPosition(m_camera.getPosition() + upVector * (deltaY * panSpeed));
    }
    // 3. ALT + CLIC DERECHO = ZOOM (Zoom suave)
    else if (io.MouseDown[1])
    {
      float zoomSpeed = 0.04f;
      // Caminar hacia adelante o atrás usando tu método walk
      m_camera.walk(-deltaY * zoomSpeed);
    }
  }

  m_camera.updateViewMatrix();

  XMStoreFloat4x4(&m_constantBufferStruct.View, XMMatrixTranspose(m_camera.getView()));
  XMStoreFloat4x4(&m_constantBufferStruct.Projection, XMMatrixTranspose(m_camera.getProj()));
  m_constantBufferStruct.CameraPos = m_camera.getPosition();

  m_gui.vec3Control("Light Direction", &m_constantBufferStruct.LightDir.x, 0.1f);
  m_gui.vec3Control("Light Color", &m_constantBufferStruct.LightColor.x, 0.1f);

  if (!m_directionalLightActor.isNull()) {
    EU::TSharedPointer<LightComponent> lightComponent =
      m_directionalLightActor->getComponent<LightComponent>();

    if (lightComponent) {
      lightComponent->getLightData().direction = m_constantBufferStruct.LightDir;
      lightComponent->getLightData().color = m_constantBufferStruct.LightColor;
    }
  }

  m_skybox.update(m_deviceContext, m_camera);

  m_sceneGraph.update(deltaTime, m_deviceContext);

  m_gui.drawLogConsole();
}

void
BaseApp::render() {
  //handleEditorViewportResize();

  float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

  m_renderScene.clear();
  m_sceneGraph.gatherRenderScene(m_renderScene, m_camera);

  for (auto& actor : m_actors) {
    if (!actor.isNull()) {
      auto particleComp = actor->getComponent<ParticleEmitterComponent>();
      if (particleComp) {
        m_renderScene.particleEmitters.push_back(particleComp.get());
      }
    }
  }

  m_renderScene.skybox = &m_skybox;

  m_renderPipeline.render(
    m_deviceContext,
    m_camera,
    m_renderScene,
    m_editorViewportPass
  );

  m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, ClearColor);
  m_viewport.render(m_deviceContext);
  m_depthStencilView.render(m_deviceContext);

  m_gui.render();

  m_swapChain.present();
}

void
BaseApp::destroy() {
  if (m_deviceContext.m_deviceContext)
    m_deviceContext.m_deviceContext->ClearState();

  m_sceneGraph.destroy();
  m_editorViewportPass.destroy();
  m_renderPipeline.destroy();
  m_cyberGunRenderMesh.destroy();

  m_AlbedoSRV.destroy();
  m_MetallicSRV.destroy();
  m_NormalSRV.destroy();
  m_RoughnessSRV.destroy();
  m_AOSRV.destroy();

  m_defaultRasterizer.destroy();
  m_defaultDepthStencil.destroy();
  m_defaultSampler.destroy();

  m_shaderProgram.destroy();
  m_depthStencil.destroy();
  m_depthStencilView.destroy();
  m_renderTargetView.destroy();
  m_swapChain.destroy();
  m_backBuffer.destroy();

  m_lightIconTexture.destroy();


  if (m_guiInitialized) {
    m_gui.destroy();
    m_guiInitialized = false;
  }

  delete m_model;
  m_model = nullptr;

  m_deviceContext.destroy();
  m_device.destroy();
}

LRESULT
BaseApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam)) {
    return true;
  }

  switch (message) {
  case WM_CREATE:
  {
    CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pCreate->lpCreateParams);
  }
  return 0;

  case WM_PAINT:
  {
    PAINTSTRUCT ps;
    BeginPaint(hWnd, &ps);
    EndPaint(hWnd, &ps);
  }
  return 0;

  case WM_SIZE:
  {
    if (wParam == SIZE_MINIMIZED) return 0;

    unsigned int newW = LOWORD(lParam);
    unsigned int newH = HIWORD(lParam);
    if (newW == 0 || newH == 0) return 0;

    BaseApp* app = reinterpret_cast<BaseApp*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (app) app->onResize(newW, newH);
    return 0;
  }

  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  }

  return DefWindowProc(hWnd, message, wParam, lParam);
}

void
BaseApp::onResize(unsigned int newW, unsigned int newH)
{
  if (!m_d3dReady) {
    m_window.m_width = (int)newW;
    m_window.m_height = (int)newH;
    return;
  }

  if (!m_deviceContext.m_deviceContext || !m_swapChain.m_swapChain) return;
  if (newW == 0 || newH == 0) return;

  m_window.m_width = (int)newW;
  m_window.m_height = (int)newH;

  ID3D11RenderTargetView* nullRTV = nullptr;
  m_deviceContext.m_deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);

  m_renderTargetView.destroy();
  m_depthStencilView.destroy();
  m_depthStencil.destroy();
  m_backBuffer.destroy();

  HRESULT hr = m_swapChain.resizeBuffers(newW, newH);
  if (FAILED(hr)) return;

  hr = m_swapChain.getBackBuffer(m_backBuffer);
  if (FAILED(hr)) return;

  hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
  if (FAILED(hr)) return;

  hr = m_depthStencil.init(
    m_device,
    newW,
    newH,
    DXGI_FORMAT_D24_UNORM_S8_UINT,
    D3D11_BIND_DEPTH_STENCIL,
    4,
    0
  );
  if (FAILED(hr)) return;

  hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
  if (FAILED(hr)) return;

  m_viewport.init(m_window);

  m_camera.setLens(XM_PIDIV4, newW / (float)newH, 0.01f, 100.0f);
}

void
BaseApp::handleEditorViewportResize()
{
  if (!m_editorViewportResizePending)
    return;

  m_deviceContext.m_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);

  ID3D11ShaderResourceView* nullSRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
  m_deviceContext.m_deviceContext->PSSetShaderResources(
    0,
    D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT,
    nullSRVs
  );

  EditorViewportPass newPass;
  HRESULT hr = newPass.init(m_device, m_pendingViewportWidth, m_pendingViewportHeight);
  if (FAILED(hr))
  {
    m_editorViewportResizePending = false;
    return;
  }

  m_editorViewportPass.swap(newPass);
  m_renderPipeline.resize(m_device, m_pendingViewportWidth, m_pendingViewportHeight);

  m_editorViewportResizePending = false;
}

std::string
BaseApp::getDefaultScenePath() const
{
  CreateDirectoryA("Saved", nullptr);
  return "Saved/DefaultScene.wvscene";
}

EU::TSharedPointer<Actor>
BaseApp::createLightActor(const std::string& name)
{
  EU::TSharedPointer<Actor> lightActor = EU::MakeShared<Actor>(m_device);
  if (lightActor.isNull()) {
    ERROR("Main", "createLightActor", "Failed to create Light Actor.");
    return lightActor;
  }

  // Cuenta cuantas luces hay (para nombrar y desplazar la nueva)
  size_t lightActorCount = 0;
  for (const auto& actor : m_actors) {
    if (!actor.isNull() && !actor->getComponent<LightComponent>().isNull()) {
      ++lightActorCount;
    }
  }

  lightActor->setName(name.empty()
    ? "Light Actor " + std::to_string(lightActorCount + 1)
    : name);

  EU::TSharedPointer<LightComponent> lightComponent = lightActor->getComponent<LightComponent>();
  if (!lightComponent) {
    lightComponent = EU::MakeShared<LightComponent>();
    lightActor->addComponent(lightComponent);
  }

  // Luz puntual configurable por defecto
  lightComponent->getLightData().type = LightType::Point;
  lightComponent->getLightData().direction = EU::Vector3(-0.20f, -1.0f, 1.0f);
  lightComponent->getLightData().color = EU::Vector3(1.0f, 1.0f, 1.0f);
  lightComponent->getLightData().intensity = 1.0f;
  lightComponent->getLightData().range = 12.0f;
  lightComponent->setCastShadow(false);

  // Transform inicial (se puede mover luego con el gizmo / inspector)
  EU::TSharedPointer<Transform> transform = lightActor->getComponent<Transform>();
  if (transform) {
    const float lightOffset = static_cast<float>(lightActorCount) * 2.0f;
    transform->setTransform(EU::Vector3(lightOffset, 3.0f, 0.0f),
      EU::Vector3(0.0f, 0.0f, 0.0f),
      EU::Vector3(1.0f, 1.0f, 1.0f));
  }

  m_actors.push_back(lightActor);
  m_sceneGraph.addEntity(lightActor.get());
  return lightActor;
}


bool
BaseApp::saveScene(const std::string& path)
{
  std::ofstream stream(path, std::ios::trunc);
  if (!stream.is_open()) {
    ERROR("Main", "saveScene", ("Failed to open scene file for writing: " + path).c_str());
    return false;
  }

  stream << "WVSCENE 1";
    stream << "ACTOR_COUNT " << m_actors.size() << "";

    for (size_t actorIndex = 0; actorIndex < m_actors.size(); ++actorIndex) {
      const EU::TSharedPointer<Actor>& actor = m_actors[actorIndex];
      if (actor.isNull()) {
        continue;
      }

      stream << "ACTOR " << actorIndex << " " << std::quoted(actor->getName()) << " ";

        EU::TSharedPointer<Transform> transform = actor->getComponent<Transform>();
      if (transform) {
        const EU::Vector3& position = transform->getPosition();
        const EU::Vector3& rotation = transform->getRotation();
        const EU::Vector3& scale = transform->getScale();

        stream << "POSITION " << position.x << " " << position.y << " " << position.z << "";
          stream << "ROTATION " << rotation.x << " " << rotation.y << " " << rotation.z << "";
          stream << "SCALE " << scale.x << " " << scale.y << " " << scale.z << " ";
      }

      EU::TSharedPointer<MeshRendererComponent> meshRenderer =
        actor->getComponent<MeshRendererComponent>();

      if (meshRenderer) {
        stream << "VISIBLE " << (meshRenderer->isVisible() ? 1 : 0) << " ";
          stream << "CAST_SHADOW " << (meshRenderer->canCastShadow() ? 1 : 0) << " ";

          const std::vector<MaterialInstance*>&materials = meshRenderer->getMaterialInstances();
        stream << "MATERIAL_COUNT " << materials.size() << " ";

          for (size_t i = 0; i < materials.size(); ++i) {
            MaterialInstance* materialInstance = materials[i];
            if (!materialInstance) {
              stream << "MATERIAL " << i << " 0 0 1 1 1 1 0 1 1 1 0.5  ";
                continue;
            }

            Material* material = materialInstance->getMaterial();
            const MaterialParams& params = materialInstance->getParams();

            const int domain = material ? static_cast<int>(material->getDomain()) : 0;
            const int blendMode = material ? static_cast<int>(material->getBlendMode()) : 0;

            stream << "MATERIAL " << i << " "
              << domain << " "
              << blendMode << " "
              << params.baseColor.x << " "
              << params.baseColor.y << " "
              << params.baseColor.z << " "
              << params.baseColor.w << " "
              << params.metallic << " "
              << params.roughness << " "
              << params.ao << " "
              << params.normalScale << " "
              << params.alphaCutoff << " ";
          }
      }

      stream << "END_ACTOR  ";
    }

  stream << "LIGHT "
    << m_constantBufferStruct.LightDir.x << " "
    << m_constantBufferStruct.LightDir.y << " "
    << m_constantBufferStruct.LightDir.z << " "
    << m_constantBufferStruct.LightColor.x << " "
    << m_constantBufferStruct.LightColor.y << " "
    << m_constantBufferStruct.LightColor.z << "  ";

    stream << "END_SCENE ";
    return true;
}

bool
BaseApp::loadScene(const std::string& path)
{
  std::ifstream stream(path);
  if (!stream.is_open()) {
    return false;
  }

  std::string token;
  stream >> token;
  if (token != "WVSCENE") {
    return false;
  }

  int version = 0;
  stream >> version;
  if (version != 1) {
    return false;
  }

  EU::TSharedPointer<Actor> currentActor;

  while (stream >> token) {
    if (token == "ACTOR_COUNT") {
      size_t ignoredCount = 0;
      stream >> ignoredCount;
    }
    else if (token == "ACTOR") {
      size_t actorIndex = 0;
      std::string actorName;
      stream >> actorIndex >> std::quoted(actorName);

      currentActor = EU::TSharedPointer<Actor>();
      if (actorIndex < m_actors.size()) {
        currentActor = m_actors[actorIndex];
      }

      if (!currentActor.isNull()) {
        currentActor->setName(actorName);
      }
    }
    else if (token == "POSITION" && !currentActor.isNull()) {
      float x = 0.0f, y = 0.0f, z = 0.0f;
      stream >> x >> y >> z;

      EU::TSharedPointer<Transform> transform = currentActor->getComponent<Transform>();
      if (transform) {
        transform->setPosition(EU::Vector3(x, y, z));
      }
    }
    else if (token == "ROTATION" && !currentActor.isNull()) {
      float x = 0.0f, y = 0.0f, z = 0.0f;
      stream >> x >> y >> z;

      EU::TSharedPointer<Transform> transform = currentActor->getComponent<Transform>();
      if (transform) {
        transform->setRotation(EU::Vector3(x, y, z));
      }
    }
    else if (token == "SCALE" && !currentActor.isNull()) {
      float x = 1.0f, y = 1.0f, z = 1.0f;
      stream >> x >> y >> z;

      EU::TSharedPointer<Transform> transform = currentActor->getComponent<Transform>();
      if (transform) {
        transform->setScale(EU::Vector3(x, y, z));
      }
    }
    else if (token == "VISIBLE" && !currentActor.isNull()) {
      int value = 1;
      stream >> value;

      EU::TSharedPointer<MeshRendererComponent> meshRenderer =
        currentActor->getComponent<MeshRendererComponent>();

      if (meshRenderer) {
        meshRenderer->setVisible(value != 0);
      }
    }
    else if (token == "CAST_SHADOW" && !currentActor.isNull()) {
      int value = 1;
      stream >> value;

      EU::TSharedPointer<MeshRendererComponent> meshRenderer =
        currentActor->getComponent<MeshRendererComponent>();

      if (meshRenderer) {
        meshRenderer->setCastShadow(value != 0);
      }
    }
    else if (token == "MATERIAL_COUNT") {
      size_t ignoredCount = 0;
      stream >> ignoredCount;
    }
    else if (token == "MATERIAL" && !currentActor.isNull()) {
      size_t materialIndex = 0;
      int domain = 0;
      int blendMode = 0;
      MaterialParams params{};

      stream >> materialIndex
        >> domain
        >> blendMode
        >> params.baseColor.x
        >> params.baseColor.y
        >> params.baseColor.z
        >> params.baseColor.w
        >> params.metallic
        >> params.roughness
        >> params.ao
        >> params.normalScale
        >> params.alphaCutoff;

      EU::TSharedPointer<MeshRendererComponent> meshRenderer =
        currentActor->getComponent<MeshRendererComponent>();

      if (meshRenderer) {
        const std::vector<MaterialInstance*>& materials = meshRenderer->getMaterialInstances();
        if (materialIndex < materials.size() && materials[materialIndex]) {
          materials[materialIndex]->getParams() = params;

          Material* material = materials[materialIndex]->getMaterial();
          if (material) {
            material->setDomain(static_cast<MaterialDomain>(domain));
            material->setBlendMode(static_cast<BlendMode>(blendMode));
          }
        }
      }
    }
    else if (token == "LIGHT") {
      stream >> m_constantBufferStruct.LightDir.x
        >> m_constantBufferStruct.LightDir.y
        >> m_constantBufferStruct.LightDir.z
        >> m_constantBufferStruct.LightColor.x
        >> m_constantBufferStruct.LightColor.y
        >> m_constantBufferStruct.LightColor.z;

      if (!m_directionalLightActor.isNull()) {
        EU::TSharedPointer<LightComponent> lightComponent =
          m_directionalLightActor->getComponent<LightComponent>();

        if (lightComponent) {
          lightComponent->getLightData().direction = m_constantBufferStruct.LightDir;
          lightComponent->getLightData().color = m_constantBufferStruct.LightColor;
        }
      }
    }
    else if (token == "END_ACTOR") {
      currentActor = EU::TSharedPointer<Actor>();
    }
    else if (token == "END_SCENE") {
      break;
    }
  }
  return true;
}

EU::TSharedPointer<Actor>
BaseApp::cloneActor(EU::TSharedPointer<Actor> original) {
  if (original.isNull()) return EU::TSharedPointer<Actor>();

  EU::TSharedPointer<Actor> newActor = EU::MakeShared<Actor>(m_device);
  newActor->setName(original->getName() + " (Copy)");

  // 1. Copiar Transform
  auto origTransform = original->getComponent<Transform>();
  auto newTransform = newActor->getComponent<Transform>();
  if (origTransform && newTransform) {
    newTransform->setTransform(origTransform->getPosition(),
      origTransform->getRotation(),
      origTransform->getScale());
    newTransform->rebuildMatrixFromVectors();
  }

  // ---------------------------------------------------------------------
  // 2.Copiar MeshRendererComponent (Para Render Deferred PBR)
  // ---------------------------------------------------------------------
  auto origMeshRenderer = original->getComponent<MeshRendererComponent>();
  if (origMeshRenderer) {
    // Creamos el componente porque el Actor no lo trae por defecto
    EU::TSharedPointer<MeshRendererComponent> newMeshRenderer = EU::MakeShared<MeshRendererComponent>();
    newMeshRenderer->setMesh(origMeshRenderer->getMesh());
    newMeshRenderer->setMaterialInstances(origMeshRenderer->getMaterialInstances());
    newMeshRenderer->setVisible(origMeshRenderer->isVisible());
    newMeshRenderer->setCastShadow(origMeshRenderer->canCastShadow());

    // ¡Lo agregamos al nuevo Actor!
    newActor->addComponent(newMeshRenderer);
  }

  // 3. Copiar las mallas y texturas clásicas (Por si las usas para colisiones o el render del Skybox)
  if (!original->getMeshes().empty()) {
    newActor->setMesh(m_device, original->getMeshes());
  }
  if (!original->getTextures().empty()) {
    newActor->setTextures(original->getTextures());
  }

  // 4. Copiar Luz (si tiene)
  auto origLight = original->getComponent<LightComponent>();
  if (origLight) {
    EU::TSharedPointer<LightComponent> newLight = EU::MakeShared<LightComponent>();
    newLight->getLightData() = origLight->getLightData();
    newLight->setCastShadow(origLight->canCastShadow());
    newActor->addComponent(newLight);
  }

  // Forzar llenado del Constant Buffer para el primer frame
  newActor->update(0.016f, m_deviceContext);

  // 5. Registrar en el motor
  m_actors.push_back(newActor);
  m_sceneGraph.addEntity(newActor.get());

  return newActor;
}

EU::TSharedPointer<Actor>
BaseApp::createParticleActor(const std::string& name)
{
  // --- 1. CREACIÓN DEL ACTOR ---
  // Intentamos crear una nueva entidad (Actor) en la memoria pasándole nuestro dispositivo gráfico.
  EU::TSharedPointer<Actor> particleActor = EU::MakeShared<Actor>(m_device);

  // Si por alguna razón nos quedamos sin memoria o falla la creación, 
  // lanzamos un mensaje de error y detenemos el proceso devolviendo un puntero vacío.
  if (particleActor.isNull()) {
    ERROR("Main", "createParticleActor", "Failed to create Particle Actor.");
    return particleActor;
  }

  // --- 2. GENERACIÓN AUTOMÁTICA DE NOMBRE ---
  // Hacemos un recuento rápido de cuántos emisores de partículas ya existen en nuestra lista de actores.
  // Esto nos servirá para darle un número único al nuevo emisor si el usuario no le puso nombre.
  size_t particleCount = 0;
  for (const auto& actor : m_actors) {
    if (!actor.isNull() && !actor->getComponent<ParticleEmitterComponent>().isNull()) {
      ++particleCount;
    }
  }

  // Le asignamos el nombre al actor. 
  // Si el texto 'name' viene vacío, lo bautizamos automáticamente (ej. "Particle Emitter 1", "Particle Emitter 2").
  particleActor->setName(name.empty()
    ? "Particle Emitter " + std::to_string(particleCount + 1)
    : name);

  // --- 3. CONFIGURACIÓN DEL COMPONENTE DE PARTÍCULAS ---
  // Revisamos si el actor ya trae un componente de partículas por defecto.
  EU::TSharedPointer<ParticleEmitterComponent> particleComp = particleActor->getComponent<ParticleEmitterComponent>();

  if (!particleComp) {
    // Si no lo tiene, lo construimos desde cero. 
    // Le pasamos la dirección de memoria de nuestro dispositivo gráfico (&m_device) 
    // para que el componente tenga acceso a la tarjeta gráfica y pueda crear sus recursos visuales.
    particleComp = EU::MakeShared<ParticleEmitterComponent>(&m_device);

    // Arrancamos el componente (esto carga los shaders, buffers y estados gráficos)
    particleComp->init();

    // Finalmente, "pegamos" este componente al actor.
    particleActor->addComponent(particleComp);
  }

  // --- 4. POSICIÓN INICIAL EN EL MUNDO (TRANSFORM) ---
  EU::TSharedPointer<Transform> transform = particleActor->getComponent<Transform>();
  if (transform) {
    // Para evitar que todos los emisores aparezcan exactamente en la misma coordenada (0,0,0) y se superpongan,
    // usamos la cantidad de emisores creados para desplazarlos un poco hacia un lado (offset en el eje X).
    const float offset = static_cast<float>(particleCount) * 2.0f;

    // Lo colocamos en su nueva posición, sin rotación y con una escala normal (1x).
    transform->setTransform(EU::Vector3(offset, 1.0f, 0.0f),
      EU::Vector3(0.0f, 0.0f, 0.0f),
      EU::Vector3(1.0f, 1.0f, 1.0f));
  }

  // --- 5. REGISTRO EN EL MOTOR ---
  // Guardamos nuestro nuevo actor en la lista maestra y lo metemos al grafo de la escena
  // para que el motor empiece a procesarlo (actualizar su lógica y dibujarlo en pantalla) en el próximo frame.
  m_actors.push_back(particleActor);
  m_sceneGraph.addEntity(particleActor.get());

  // Devolvemos el actor recién horneado y listo para usarse.
  return particleActor;
}