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


#include "ModelLoader.h"
#include "Model3D.h"
#include "GUI.h"
#include "ECS/Actor.h"

#include "SceneGraph\SceneGraph.h"
#include "EngineUtilities\Utilities\Camera.h"

#include "EngineUtilities\Utilities\Skybox.h"

#include "EngineUtilities\Utilities\LayoutBuilder.h"

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

  void
  onResize(UINT newW, UINT newH);

  void 
  handleEditorViewportResize();

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
  // Buffer                           m_vertexBuffer;
  // Buffer                           m_indexBuffer;

  bool m_d3dReady = false;
  Buffer                              m_constantBuffer;

  //Textures
  //Texture m_AlbedoSRV;
  //Texture m_MetallicSRV;
  //Texture m_RoughnessSRV;
  //Texture m_AOSRV;
  //Texture m_NormalSRV;

  Buffer                              m_cbNeverChanges;
  Buffer                              m_cbChangeOnResize;
  Buffer                              m_cbChangesEveryFrame;

  Texture                             m_cyberGunAlbedo;      // Tu textura especifica

  Texture															m_skyboxTex;

  SamplerState                        m_samplerState;

  // Matrices y Variables Globales
  XMMATRIX                            m_World;


  Camera															m_camera;
  //XMMATRIX                            m_View;
  //XMMATRIX                            m_Projection;

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

  Skybox m_skybox;
  RasterizerState m_defaultRasterizer;
  DepthStencilState m_defaultDepthStencil;
};