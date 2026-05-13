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

#include "Model3D.h"
#include "GUI.h"
#include "ECS/Actor.h"

#include "SceneGraph/SceneGraph.h"

#include "EngineUtilities/Utilities/Camera.h"
#include "EngineUtilities/Utilities/Skybox.h"
#include "EngineUtilities/Utilities/LayoutBuilder.h"
#include "EngineUtilities/Utilities/EditorViewportPass.h"

#include "ECS/LightComponent.h"
#include "ECS/MeshRendererComponent.h"

#include "Rendering/Material.h"
#include "Rendering/MaterialInstance.h"
#include "Rendering/Mesh.h"
#include "Rendering/ForwardRenderer.h"
#include "Rendering/RenderScene.h"

#include <string>

extern IMGUI_IMPL_API
LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class BaseApp {
public:
  BaseApp(HINSTANCE hInst, int nCmdShow);
  ~BaseApp() { destroy(); }

  HRESULT awake();
  int run(HINSTANCE hInst, int nCmdShow);
  HRESULT init();
  void update(float deltaTime);
  void render();
  void destroy();

  void onResize(UINT newW, UINT newH);
  void handleEditorViewportResize();

  bool saveScene(const std::string& path);
  bool loadScene(const std::string& path);
  std::string getDefaultScenePath() const;

private:
  static LRESULT CALLBACK
    WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
  Window              m_window;
  Device              m_device;
  DeviceContext       m_deviceContext;
  SwapChain           m_swapChain;
  Texture             m_backBuffer;
  RenderTargetView    m_renderTargetView;
  Texture             m_depthStencil;
  DepthStencilView    m_depthStencilView;
  Viewport            m_viewport;
  ShaderProgram       m_shaderProgram;

  bool                m_d3dReady = false;
  bool                m_guiInitialized = false;

  Buffer              m_constantBuffer;
  CBMain              m_constantBufferStruct;

  // Texturas del modelo principal
  Texture             m_AlbedoSRV;
  Texture             m_MetallicSRV;
  Texture             m_RoughnessSRV;
  Texture             m_AOSRV;
  Texture             m_NormalSRV;

  // Texturas Body
  Texture m_BodyAlbedoSRV;
  Texture m_BodyMetallicSRV;
  Texture m_BodyRoughnessSRV;
  Texture m_BodyAOSRV;
  Texture m_BodyNormalSRV;

  // Texturas Glass
  Texture m_GlassAlbedoSRV;
  Texture m_GlassMetallicSRV;
  Texture m_GlassRoughnessSRV;
  Texture m_GlassAOSRV;
  Texture m_GlassNormalSRV;

  // Texturas Head
  Texture m_HeadAlbedoSRV;
  Texture m_HeadMetallicSRV;
  Texture m_HeadRoughnessSRV;
  Texture m_HeadAOSRV;
  Texture m_HeadNormalSRV;
  Texture m_HeadSSSSRV;

  // Recursos generales
  Texture             m_skyboxTex;

  Camera              m_camera;
  SceneGraph          m_sceneGraph;

  // Actores
  std::vector<EU::TSharedPointer<Actor>> m_actors;
  EU::TSharedPointer<Actor> m_cyberGun;
  EU::TSharedPointer<Actor> m_directionalLightActor;

  // Recursos de modelo
  Model3D* m_model = nullptr;

  // GUI
  GUI                 m_gui;

  // Render base
  Skybox              m_skybox;
  RasterizerState     m_defaultRasterizer;
  DepthStencilState   m_defaultDepthStencil;
  SamplerState        m_defaultSampler;

  // Render estilo profe
  Mesh                m_cyberGunRenderMesh;
  Material            m_pbrMaterial;
  Material            m_transparentPbrMaterial;
  MaterialInstance    m_cyberGunMaterial;

  MaterialInstance m_bodyMaterial;
  MaterialInstance m_glassMaterial;
  MaterialInstance m_headMaterial;

  EditorViewportPass  m_editorViewportPass;
  ForwardRenderer     m_forwardRenderer;
  RenderScene         m_renderScene;

  // Resize diferido del viewport editor
  bool                m_editorViewportResizePending = false;
  unsigned int        m_pendingViewportWidth = 1;
  unsigned int        m_pendingViewportHeight = 1;

  unsigned int        m_lastRequestedViewportWidth = 1;
  unsigned int        m_lastRequestedViewportHeight = 1;
  int                 m_viewportResizeStableFrames = 0;
};