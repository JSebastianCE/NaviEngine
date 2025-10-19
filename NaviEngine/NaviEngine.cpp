//--------------------------------------------------------------------------------------
// File: NaviEngine.cpp
//
// This application demonstrates texturing
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "Prerequisites.h"
#include "Window.h"
#include "Device.h"
#include "DeviceContext.h"
#include "SwapChain.h"
#include "Texture.h"
#include "RenderTargetView.h"
#include "DepthStencilView.h"
#include "Viewport.h"
//#include "InputLayout.h"
#include "ShaderProgram.h"
#include "MeshComponent.h"
#include "Buffer.h"

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
//HINSTANCE                           g_hInst = NULL;
//HWND                                g_hWnd = NULL;
Window                              g_window;
Device                              g_device;
DeviceContext                       g_deviceContext;
SwapChain                           g_swapChain;
Texture                             g_backBuffer;
RenderTargetView                    g_renderTargetView;
Texture                             g_depthStencil;
DepthStencilView                    g_depthStencilView;
Viewport                            g_viewport;
ShaderProgram                       g_shaderProgram;
MeshComponent												g_mesh;
Buffer															g_vertexBuffer;
Buffer															g_indexBuffer;
Buffer															g_cbNeverChanges;
Buffer															g_cbChangeOnResize;
Buffer															g_cbChangesEveryFrame;
Texture 														g_textureCube;

ID3D11SamplerState*                 g_pSamplerLinear = NULL;
XMMATRIX                            g_World;
XMMATRIX                            g_View;
XMMATRIX                            g_Projection;
XMFLOAT4                            g_vMeshColor( 0.7f, 0.7f, 0.7f, 1.0f );

CBChangeOnResize cbChangesOnResize;
CBNeverChanges cbNeverChanges;
CBChangesEveryFrame cb;

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
void update();
void Render();


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

    if( FAILED(g_window.init( hInstance, nCmdShow, WndProc) ) )
        return 0;

    if( FAILED( InitDevice() ) )
    {
        CleanupDevice();
        return 0;
    }

    // Main message loop
    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
          update();
          Render();
        }
    }

    CleanupDevice();

    return ( int )msg.wParam;
}



//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
  HRESULT hr = S_OK;

  //Crear swapChain
  hr = g_swapChain.init(g_device, g_deviceContext, g_backBuffer, g_window);

  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize SwapChain. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  //Creacion render target view
  hr = g_renderTargetView.init(g_device, g_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);

  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize RenderTargetView. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // Crear textura de depth stencil
  hr = g_depthStencil.init(g_device,
    g_window.m_width,
    g_window.m_height,
    DXGI_FORMAT_D24_UNORM_S8_UINT,
    D3D11_BIND_DEPTH_STENCIL,
    4,
    0);

  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize DepthStencil. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }


  // Crear el depth stencil view
  hr = g_depthStencilView.init(g_device,
    g_depthStencil,
    DXGI_FORMAT_D24_UNORM_S8_UINT);

  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize DepthStencilView. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }


  //Crear el m_viewport
  hr = g_viewport.init(g_window);

  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize Viewport. HREULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // --Load Resoources--


  // Define the input layout
  std::vector<D3D11_INPUT_ELEMENT_DESC> Layout;
  D3D11_INPUT_ELEMENT_DESC position;
  position.SemanticName = "POSITION";
  position.SemanticIndex = 0;
  position.Format = DXGI_FORMAT_R32G32B32_FLOAT;
  position.InputSlot = 0;
  position.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT /*0*/;
  position.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  position.InstanceDataStepRate = 0;
  Layout.push_back(position);

  D3D11_INPUT_ELEMENT_DESC texcoord;
  texcoord.SemanticName = "TEXCOORD";
  texcoord.SemanticIndex = 0;
  texcoord.Format = DXGI_FORMAT_R32G32_FLOAT;
  texcoord.InputSlot = 0;
  texcoord.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT /*0*/;
  texcoord.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  texcoord.InstanceDataStepRate = 0;
  Layout.push_back(texcoord);

  //Create the Shader program
  hr = g_shaderProgram.init(g_device, "NaviEngine.fx", Layout);
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize ShaderProgram. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  // Create vertex buffer
  SimpleVertex vertices[] =
  {
      { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
      { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
      { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
      { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

      { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
      { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
      { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
      { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

      { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
      { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
      { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
      { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

      { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
      { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
      { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
      { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

      { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
      { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
      { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
      { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

      { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
      { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
      { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
      { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
  };

  unsigned int indices[] =
  {
      3,1,0,
      2,1,3,

      6,4,5,
      7,4,6,

      11,9,8,
      10,9,11,

      14,12,13,
      15,12,14,

      19,17,16,
      18,17,19,

      22,20,21,
      23,20,22
  };

  // Integrar los vertices a meshcomponent
  for (unsigned int i = 0; i < 24; i++) {
    g_mesh.m_vertex.push_back(vertices[i]);
  }
  g_mesh.m_numVertex = 24;

  // Integrar los indices a meshcomponent
  for (unsigned int i = 0; i < 36; i++) {
    g_mesh.m_index.push_back(indices[i]);
  }
  g_mesh.m_numIndex = 36;

  // Create vertex buffer
  hr = g_vertexBuffer.init(g_device, g_mesh, D3D11_BIND_VERTEX_BUFFER);

  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize VertexBuffer. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  } // <-- LLAVE CORREGIDA

    // Create index buffer
  hr = g_indexBuffer.init(g_device, g_mesh, D3D11_BIND_INDEX_BUFFER);

  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize IndexBuffer. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  } // <-- LLAVE CORREGIDA

  // Set primitive topology
  g_deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  // Create the constant buffers
  hr = g_cbNeverChanges.init(g_device, sizeof(CBNeverChanges));
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize NeverChanges Buffer. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  } // <-- LLAVE CORREGIDA

  hr = g_cbChangeOnResize.init(g_device, sizeof(CBChangeOnResize));
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize ChangeOnResize Buffer. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  } // <-- LLAVE CORREGIDA

  hr = g_cbChangesEveryFrame.init(g_device, sizeof(CBChangesEveryFrame));
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize ChangesEveryFrame Buffer. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  } // <-- LLAVE CORREGIDA

  hr = g_textureCube.init(g_device, "seafloor", ExtensionType::DDS);


  // Load the Texture
  if (FAILED(hr)) {
    ERROR("Main", "InitDevice",
      ("Failed to initialize texture Cube. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  } // <-- LLAVE CORREGIDA

  // Create the sample state
  D3D11_SAMPLER_DESC sampDesc;
  ZeroMemory(&sampDesc, sizeof(sampDesc));
  sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  sampDesc.MinLOD = 0;
  sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
  hr = g_device.m_device->CreateSamplerState(&sampDesc, &g_pSamplerLinear);
  if (FAILED(hr))
    return hr;

  // Initialize the world matrices
  g_World = XMMatrixIdentity();

  // Initialize the view matrix
  XMVECTOR Eye = XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f);
  XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
  XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
  g_View = XMMatrixLookAtLH(Eye, At, Up);


  // Initialize the projection matrix
  cbNeverChanges.mView = XMMatrixTranspose(g_View);
  g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, g_window.m_width / (FLOAT)g_window.m_height, 0.01f, 100.0f);

  cbChangesOnResize.mProjection = XMMatrixTranspose(g_Projection);

  // --- CORRECCIÓN FALTANTE ---
  // Actualizar los buffers en la GPU por primera vez
  g_cbNeverChanges.update(g_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);
  g_cbChangeOnResize.update(g_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);
  // --- FIN DE LA CORRECCIÓN ---

  return S_OK;
}

//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
  if (g_deviceContext.m_deviceContext) g_deviceContext.m_deviceContext->ClearState();

  if (g_pSamplerLinear) g_pSamplerLinear->Release();
  //if (g_pTextureRV) g_pTextureRV->Release();
  g_textureCube.destroy();
  g_cbNeverChanges.destroy();
  g_cbChangeOnResize.destroy();
  g_cbChangesEveryFrame.destroy();
  g_vertexBuffer.destroy();
  g_indexBuffer.destroy();
  
  g_shaderProgram.destroy();
  g_depthStencil.destroy();
  g_depthStencilView.destroy();
  g_renderTargetView.destroy();
  g_swapChain.destroy();
  g_backBuffer.destroy();
  g_deviceContext.destroy();
  g_device.destroy();
}


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )
    {
        case WM_PAINT:
            hdc = BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;

        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}


//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void 
update()
{
    // Update our time
    static float t = 0.0f;
    if (g_swapChain.m_driverType == D3D_DRIVER_TYPE_REFERENCE)
    {
        t += ( float )XM_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount();
        if( dwTimeStart == 0 )
            dwTimeStart = dwTimeCur;
        t = ( dwTimeCur - dwTimeStart ) / 1000.0f;
    }

    // Actualizar la matriz de proyección y vista
    cbNeverChanges.mView = XMMatrixTranspose(g_View);
    g_cbNeverChanges.update(g_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);
    g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, g_window.m_width / (FLOAT)g_window.m_height, 0.01f, 100.0f);
    cbChangesOnResize.mProjection = XMMatrixTranspose(g_Projection);
    g_cbChangeOnResize.update(g_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);

    // Modify the color
    g_vMeshColor.x = ( sinf( t * 1.0f ) + 1.0f ) * 0.5f;
    g_vMeshColor.y = ( cosf( t * 3.0f ) + 1.0f ) * 0.5f;
    g_vMeshColor.z = ( sinf( t * 5.0f ) + 1.0f ) * 0.5f;

    // Rotate cube around the origin
    g_World = XMMatrixRotationY(t);
    cb.mWorld = XMMatrixTranspose(g_World);
    cb.vMeshColor = g_vMeshColor;
    g_cbChangesEveryFrame.update(g_deviceContext, nullptr, 0, nullptr, &cb, 0, 0);
}

//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void Render()
{
    // Set Render Target View
    float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    g_renderTargetView.render(g_deviceContext, g_depthStencilView, 1, ClearColor);

    //
    // Clear the depth buffer to 1.0 (max depth)
    //

    // Set Viewport
    g_viewport.render(g_deviceContext);

    // Set depth stencil view
    g_depthStencilView.render(g_deviceContext);

    //Set shader program
    g_shaderProgram.render(g_deviceContext);

    // Render the cube
   // Asignar buffers Vertex e Index
    g_vertexBuffer.render(g_deviceContext, 0, 1);
    g_indexBuffer.render(g_deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);

    // Asignar buffers constantes
    g_cbNeverChanges.render(g_deviceContext, 0, 1);
    g_cbChangeOnResize.render(g_deviceContext, 1, 1);
    g_cbChangesEveryFrame.render(g_deviceContext, 2, 1);
    g_cbChangesEveryFrame.render(g_deviceContext, 2, 1, true);

    // Asignar textura y sampler
    //g_deviceContext.PSSetShaderResources(0, 1, &g_pTextureRV);
    g_textureCube.render(g_deviceContext, 0, 1);
    g_deviceContext.PSSetSamplers(0, 1, &g_pSamplerLinear);
    g_deviceContext.DrawIndexed(g_mesh.m_numIndex, 0, 0);

    //
    // Present our back buffer to our front buffer
    //
    g_swapChain.present();
}