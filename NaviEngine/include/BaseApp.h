#pragma once
#include "Prerequisites.h"
#include "Window.h"
#include "Device.h"
#include "DeviceContext.h"
#include "SwapChain.h"
#include "Texture.h"
#include "RenderTargetView.h"
#include "DepthStencilView.h"
#include "Viewport.h"
#include "ShaderProgram.h"
#include "MeshComponent.h"
#include "Buffer.h"
#include "SamplerState.h"

// Inclusiones adicionales necesarias  proyecto
#include "ModelLoader.h"
#include "Model3D.h"
#include "GUI.h"
#include "ECS/Actor.h"

#include "SceneGraph\SceneGraph.h"

class
  BaseApp {
public:
  // Tu constructor personalizado 
  BaseApp(HINSTANCE hInst, int nCmdShow);

  // Destructor
  ~BaseApp() { destroy(); }

  HRESULT
    awake();

  int
    run(HINSTANCE hInst, int nCmdShow);

  HRESULT
    init();

  void
    update(float deltaTime);

  void
    render();

  void
    destroy();

private:
  static LRESULT CALLBACK
    WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
  Window                              m_window;
  Device                              m_device;
  DeviceContext                       m_deviceContext;
  SwapChain                           m_swapChain;
  Texture                             m_backBuffer;
  RenderTargetView                    m_renderTargetView;
  Texture                             m_depthStencil;
  DepthStencilView                    m_depthStencilView;
  Viewport                            m_viewport;
  ShaderProgram                       m_shaderProgram;

  // MeshComponent                    m_mesh;
  // Buffer                           m_vertexBuffer;
  // Buffer                           m_indexBuffer;

  Buffer                              m_cbNeverChanges;
  Buffer                              m_cbChangeOnResize;
  Buffer                              m_cbChangesEveryFrame;

  Texture                             m_cyberGunAlbedo;      // Tu textura especifica

  Texture															m_skyboxTex;

  SamplerState                        m_samplerState;

  // Matrices y Variables Globales
  XMMATRIX                            m_World;
  XMMATRIX                            m_View;
  XMMATRIX                            m_Projection;
  XMFLOAT4                            m_vMeshColor;


  SceneGraph													m_sceneGraph;

  // Actores
  std::vector<EU::TSharedPointer<Actor>> m_actors;
  EU::TSharedPointer<Actor>              m_cyberGun;
  EU::TSharedPointer<Actor>              m_Character;

  // Recursos
  Model3D* m_model;
  ModelLoader                         m_modelLoader; // loader
  LoadData                            LD;            // Datos de carga

  // Estructuras de Constant Buffers
  CBChangeOnResize                    cbChangesOnResize;
  CBNeverChanges                      cbNeverChanges;
  CBChangesEveryFrame                 cb;

  // Interfaz de Usuario
  GUI                                m_gui;
};