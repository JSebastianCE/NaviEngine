#include "BaseApp.h"
#include "ResourceManager.h"

// Necesario para que Win32 reenvíe los inputs a ImGui
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

BaseApp::BaseApp(HINSTANCE hInst, int nCmdShow) {
  // Constructor vacío
}

HRESULT BaseApp::awake() {
  HRESULT hr = S_OK;

  m_sceneGraph.init();

  // Inicializacion de dlls y elementos externos al motor. 
  MESSAGE("Main", "Awake", "Application awake successfully.");
  return hr;
}

int BaseApp::run(HINSTANCE hInst, int nCmdShow) {
  // 1) Initialize Window
  if (FAILED(m_window.init(hInst, nCmdShow, WndProc, this))) {
    ERROR("Main", "Run", "Failed to initialize window.");
    return 0;
  }

  // 2) Awake application
  if (FAILED(awake())) {
    ERROR("Main", "Run", "Failed to awake application.");
    return 0;
  }

  // 3) Initialize Device and Device Context
  if (FAILED(init())) {
    ERROR("Main", "Run", "Failed to initialize device and device context.");
    return 0;
  }

  // 4) Initialize GUI (Igual que el profesor, antes del loop)
  m_gui.init(m_window.m_hWnd, m_device.m_device, m_deviceContext.m_deviceContext);

  // Main message loop
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
    ERROR("Main", "InitDevice", ("Failed to initialize SwapChain. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // 2. Render Target View
  hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice", ("Failed to initialize RenderTargetView. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // 3. Depth Stencil Texture
  hr = m_depthStencil.init(m_device, m_window.m_width, m_window.m_height, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, 4, 0);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice", ("Failed to initialize DepthStencil. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // 4. Depth Stencil View
  hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice", ("Failed to initialize DepthStencilView. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // 5. Viewport
  hr = m_viewport.init(m_window);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice", ("Failed to initialize Viewport. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  m_d3dReady = true;

  // --- Load Resources (Modelos y Texturas) ---

  std::array<std::string, 6> faces = {
  "Skybox/cubemap_0.png",
  "Skybox/cubemap_1.png",
  "Skybox/cubemap_2.png",
  "Skybox/cubemap_3.png",
  "Skybox/cubemap_4.png",
  "Skybox/cubemap_5.png"
  };
  m_skyboxTex.CreateCubemap(m_device, m_deviceContext, faces, false);


  m_cyberGun = EU::MakeShared<Actor>(m_device);

  if (!m_cyberGun.isNull()) {
    // Crear vertex buffer y index buffer para el modelo
    std::vector<MeshComponent> cyberGunMeshes;

    // RUTA DE MODELO (Tu ruta)
    m_model = new Model3D("Assets/SF_Golden_double_axe_.fbx", ModelType::FBX);
    cyberGunMeshes = m_model->GetMeshes();

    std::vector<Texture> cyberGunTextures;

    // RUTA DE TEXTURA (Tu ruta)
    hr = m_cyberGunAlbedo.init(m_device, "Assets/T_Golden_double_Axe_D", ExtensionType::JPG);

    if (FAILED(hr)) {
      ERROR("Main", "InitDevice", ("Failed to initialize cyberGunAlbedo. HRESULT: " + std::to_string(hr)).c_str());
      return hr;
    }
    cyberGunTextures.push_back(m_cyberGunAlbedo);

    m_cyberGun->setMesh(m_device, cyberGunMeshes);
    m_cyberGun->setTextures(cyberGunTextures);
    m_cyberGun->setName("CyberGun");

    // Añadir a la lista global de actores
    m_actors.push_back(m_cyberGun);

    // Transform inicial
    auto t = m_cyberGun->getComponent<Transform>();
    if (t) {
      t->setTransform(
        EU::Vector3(-6.5f, 1.0f, 36.0f), // Y = -15.8 (Muy abajo)
        EU::Vector3(-90.0f, 0.0f, 0.0f),
        EU::Vector3(0.30f, 0.30f, 0.3f)      // Escala 0.3 (Pequeña)
      );
    }
  }
  else {
    ERROR("Main", "InitDevice", "Failed to create CyberGun Actor.");
    return E_FAIL;
  }

  // Store the Actors in the Scene Graph
  for (auto& actor : m_actors) {
    m_sceneGraph.addEntity(actor.get());
  }

  LayoutBuilder builder;

  builder.Add("POSITION", DXGI_FORMAT_R32G32B32_FLOAT)
         //.Add("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT)
         //.Add("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
         //.Add("BITANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
         .Add("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
 

  // Create the Shader Program
  hr = m_shaderProgram.init(m_device, "NaviEngine.fx", builder);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize ShaderProgram. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // Create the constant buffers
  hr = m_cbNeverChanges.init(m_device, sizeof(CBNeverChanges));
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize NeverChanges Buffer. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  hr = m_cbChangeOnResize.init(m_device, sizeof(CBChangeOnResize));
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize ChangeOnResize Buffer. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  hr = m_cbChangesEveryFrame.init(m_device, sizeof(CBChangesEveryFrame));
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice", "Failed to init ChangesEveryFrame Buffer.");
    return hr;
  }

  // Initialize the Camera
  m_camera.setLens(XM_PIDIV4, m_window.m_width / (float)m_window.m_height, 0.01f, 100.0f);
  m_camera.setPosition(0.0f, 3.0f, -6.0f);

  cbNeverChanges.mView = XMMatrixTranspose(m_camera.getView());
  cbChangesOnResize.mProjection = XMMatrixTranspose(m_camera.getProj());

  //Initialize Skybox
  m_skybox.init(m_device, &m_deviceContext, m_skyboxTex);


  //Initialize default states (Rasterizer, DepthStencil)
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


  return S_OK;
}

void BaseApp::update(float deltaTime) {
  // 1. Update Time logic
  static float t = 0.0f;
  if (m_swapChain.m_driverType == D3D_DRIVER_TYPE_REFERENCE) {
    t += (float)XM_PI * 0.0125f;
  }
  else {
    static DWORD dwTimeStart = 0;
    DWORD dwTimeCur = GetTickCount();
    if (dwTimeStart == 0) dwTimeStart = dwTimeCur;
    t = (dwTimeCur - dwTimeStart) / 1000.0f;
  }

  // 2. UI Updates (Exactamente como el profesor)
  m_gui.update();

  // Panel de Jerarquía
  m_gui.outliner(m_actors);


  // Shot cubemap on imgui image
  static ID3D11ShaderResourceView* faceSRV[6] = { nullptr };

  if (!faceSRV[0]) {
    for (UINT i = 0; i < 6; ++i) {
      faceSRV[i] = m_skyboxTex.CreateCubemapFaceSRV(m_device.m_device, m_skyboxTex.m_texture,
        DXGI_FORMAT_R8G8B8A8_UNORM, i, 1);
    }
  }

  ImGui::Text("Cubemap Faces:");
  const float thumb = 128.0f;

  for (int i = 0; i < 6; ++i) {
    ImGui::Image((ImTextureID)faceSRV[i], ImVec2(thumb, thumb));
    if ((i % 3) != 2) ImGui::SameLine();
  }
  ImGui::Begin("Cubemap");
  ImGui::Text("Skybox Cubemap");
  ImGui::Image((void*)m_skyboxTex.m_textureFromImg,
    ImVec2(256, 256),
    ImVec2(0, 0),
    ImVec2(1, 1));
  ImGui::End();

  // Validar si hay un actor seleccionado antes de mostrar inspector o gizmos
 

// Validar si hay un actor seleccionado
  if (m_gui.selectedActorIndex >= 0 &&
    m_gui.selectedActorIndex < m_actors.size()) {

    auto& selectedActor = m_actors[m_gui.selectedActorIndex];

    // 1. Muestra los valores numéricos en la ventana gris
    m_gui.inspectorGeneral(selectedActor);

    // 2. Dibuja las FLECHAS 3D sobre el objeto en la escena principal
    // (Solo una llamada es necesaria)
    m_gui.editTransform(m_camera.getView(), m_camera.getProj(), selectedActor);
  }


  // 3. Update Camera & Projection Matrices
  m_camera.updateViewMatrix();
  cbNeverChanges.mView = XMMatrixTranspose(m_camera.getView());
  m_cbNeverChanges.update(m_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);
  m_cbChangeOnResize.update(m_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);
  //cbChangesOnResize.mProjection = XMMatrixTranspose(m_camera.getProj());


  // 4. Update Actors logic
  m_sceneGraph.update(deltaTime, m_deviceContext);

}



void 
BaseApp::render() {
  // Clear Targets
  float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
  m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, ClearColor);

  m_viewport.render(m_deviceContext);
  m_depthStencilView.render(m_deviceContext);

  // 1. Skybox Pass
  m_skybox.render(m_deviceContext, m_camera);

  // 2. Restaurar estados + pipeline de la escena
  m_defaultRasterizer.render(m_deviceContext);
  m_defaultDepthStencil.render(m_deviceContext, 0, false);

  // limpiar SRVs por seguridad
  ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
  m_deviceContext.m_deviceContext->PSSetShaderResources(10, 1, nullSRV);
  m_deviceContext.m_deviceContext->PSSetShaderResources(0, 1, nullSRV);

  // Re-binda shader/layout de escena
  m_shaderProgram.render(m_deviceContext);

  //CBs para VS (view/proj)
  m_cbNeverChanges.render(m_deviceContext, 0, 1); // slot 0 para never changes
  m_cbChangeOnResize.render(m_deviceContext, 1, 1); // slot 1 para resize


  // 3. Scene pass
  m_sceneGraph.render(m_deviceContext);


  // 4. Render UI (Always last before present)
  m_gui.render();

  // 5. Present
  m_swapChain.present();
}

void
BaseApp::destroy() {
  if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();

  m_sceneGraph.destroy();
  m_cbNeverChanges.destroy();
  m_cbChangeOnResize.destroy();
  m_shaderProgram.destroy();
  m_depthStencil.destroy();
  m_depthStencilView.destroy();
  m_renderTargetView.destroy();
  m_swapChain.destroy();
  m_backBuffer.destroy();
  m_gui.destroy();
  m_deviceContext.destroy();
  m_device.destroy();
}

LRESULT BaseApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  // Reenviar mensajes a ImGui
  if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
    return true;

  switch (message)
  {
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
    // Evita recrear cuando está minimizada
    if (wParam == SIZE_MINIMIZED) return 0;

    UINT newW = LOWORD(lParam);
    UINT newH = HIWORD(lParam);
    if (newW == 0 || newH == 0) return 0;

    // Recupera tu instancia BaseApp (lo más común es guardarla en GWLP_USERDATA en WM_CREATE)
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
BaseApp::onResize(UINT newW, UINT newH)
{
  // 1) Actualiza window size 
  if (!m_d3dReady) {
    // Aun así puedes actualizar el tamaño lógico de la ventana
    m_window.m_width = (int)newW;
    m_window.m_height = (int)newH;
    return;
  }

  if (!m_deviceContext.m_deviceContext || !m_swapChain.m_swapChain) return;
  if (newW == 0 || newH == 0) return;

  m_window.m_width = (int)newW;
  m_window.m_height = (int)newH;
  // 2) Desbindea targets actuales (clave antes de destruir)
  ID3D11RenderTargetView* nullRTV = nullptr;
  m_deviceContext.m_deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);

  // 3) Libera recursos dependientes del tamaño (RTV/DSV/Depth/BackBuffer)
  m_renderTargetView.destroy();
  m_depthStencilView.destroy();
  m_depthStencil.destroy();
  m_backBuffer.destroy();

  // 4) Resize swapchain
  HRESULT hr = m_swapChain.resizeBuffers(newW, newH);
  if (FAILED(hr)) return;

  // 5) Re-obtén backbuffer
  hr = m_swapChain.getBackBuffer(m_backBuffer);
  if (FAILED(hr)) return;

  // 6) Re-crea RTV
  hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
  if (FAILED(hr)) return;

  // 7) Re-crea Depth/DSV 
  hr = m_depthStencil.init(m_device, newW, newH, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, 4, 0);
  if (FAILED(hr)) return;

  hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
  if (FAILED(hr)) return;

  // 8) Viewport
  m_viewport.init(m_window);

  // 9) Cámara (aspect ratio)
  m_camera.setLens(XM_PIDIV4, newW / (float)newH, 0.01f, 100.0f);

}

