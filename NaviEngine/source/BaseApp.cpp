#include "BaseApp.h"
#include "ResourceManager.h"

// Necesario para que Win32 reenvíe los inputs a ImGui
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

BaseApp::BaseApp(HINSTANCE hInst, int nCmdShow) {
  // Constructor vacío
}

HRESULT BaseApp::awake() {
  HRESULT hr = S_OK;
  // Inicializacion de dlls y elementos externos al motor. 
  MESSAGE("Main", "Awake", "Application awake successfully.");
  return hr;
}

int BaseApp::run(HINSTANCE hInst, int nCmdShow) {
  // 1) Initialize Window
  if (FAILED(m_window.init(hInst, nCmdShow, WndProc))) {
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
  UI.init(m_window.m_hWnd, m_device.m_device, m_deviceContext.m_deviceContext);

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

HRESULT BaseApp::init() {
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

  // --- Load Resources (Modelos y Texturas) ---

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
        EU::Vector3(0.19f, -15.80f, 12.86f),
        EU::Vector3(-1.0f, 0.0f, 0.0f),
        EU::Vector3(0.30f, 0.30f, 0.3f)
      );
    }
  }
  else {
    ERROR("Main", "InitDevice", "Failed to create CyberGun Actor.");
    return E_FAIL;
  }

  // Define the input layout
  std::vector<D3D11_INPUT_ELEMENT_DESC> Layout;
  D3D11_INPUT_ELEMENT_DESC position = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
  Layout.push_back(position);

  D3D11_INPUT_ELEMENT_DESC texcoord = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
  Layout.push_back(texcoord);

  D3D11_INPUT_ELEMENT_DESC normal = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
  Layout.push_back(normal);

  // Create the Shader Program
  hr = m_shaderProgram.init(m_device, "NaviEngine.fx", Layout);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice", ("Failed to initialize ShaderProgram. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // Create the constant buffers
  hr = m_cbNeverChanges.init(m_device, sizeof(CBNeverChanges));
  if (FAILED(hr)) return hr;

  hr = m_cbChangeOnResize.init(m_device, sizeof(CBChangeOnResize));
  if (FAILED(hr)) return hr;

  // Initialize the view matrix
  XMVECTOR Eye = XMVectorSet(0.0f, 18.0f, -18.0f, 0.0f);
  XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
  XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
  m_View = XMMatrixLookAtLH(Eye, At, Up);

  // Initialize the projection matrix
  cbNeverChanges.mView = XMMatrixTranspose(m_View);
  m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_window.m_width / (FLOAT)m_window.m_height, 0.01f, 100.0f);
  cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);

  // NOTA: UI.init() se ha movido a run() para coincidir con el profesor.

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
  UI.update();

  // Panel de Jerarquía
  UI.outliner(m_actors);

  // Validar si hay un actor seleccionado antes de mostrar inspector o gizmos
  if (UI.selectedActorIndex >= 0 && UI.selectedActorIndex < m_actors.size()) {
    auto& selectedActor = m_actors[UI.selectedActorIndex];

    // Panel Inspector
    UI.inspectorGeneral(selectedActor);

    // Gizmos en pantalla (Manipulación 3D)
    UI.editTransform(m_View, m_Projection, selectedActor);
  }

  // 3. Update Camera & Projection Matrices
  cbNeverChanges.mView = XMMatrixTranspose(m_View);
  m_cbNeverChanges.update(m_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);

  m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_window.m_width / (FLOAT)m_window.m_height, 0.01f, 100.0f);
  cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);
  m_cbChangeOnResize.update(m_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);

  // 4. Update Actors logic
  for (auto& actor : m_actors) {
    if (!actor.isNull()) {
      actor->update(deltaTime, m_deviceContext);
    }
  }
}

void BaseApp::render() {
  // 1. Clear Targets
  float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
  m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, ClearColor);

  m_viewport.render(m_deviceContext);
  m_depthStencilView.render(m_deviceContext);

  // 2. Set Pipeline State
  m_shaderProgram.render(m_deviceContext);

  m_cbNeverChanges.render(m_deviceContext, 0, 1);
  m_cbChangeOnResize.render(m_deviceContext, 1, 1);

  // 3. Render Scene
  for (auto& actor : m_actors) {
    if (!actor.isNull()) {
      actor->render(m_deviceContext);
    }
  }

  // 4. Render UI (Always last before present)
  UI.render();

  // 5. Present
  m_swapChain.present();
}

void BaseApp::destroy() {
  if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();

  if (m_model) {
    delete m_model;
    m_model = nullptr;
  }

  m_cbNeverChanges.destroy();
  m_cbChangeOnResize.destroy();
  m_shaderProgram.destroy();
  m_depthStencil.destroy();
  m_depthStencilView.destroy();
  m_renderTargetView.destroy();
  m_swapChain.destroy();
  m_backBuffer.destroy();

  m_deviceContext.destroy();
  m_device.destroy();

  UI.destroy();
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
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProc(hWnd, message, wParam, lParam);
}