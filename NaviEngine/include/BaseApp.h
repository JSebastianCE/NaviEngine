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

/**
 * @class BaseApp
 * @brief Clase principal de la aplicación base del motor gráfico.
 *        Se encarga de inicializar, actualizar, renderizar y destruir
 *        todos los componentes necesarios para ejecutar una aplicación DirectX.
 */
class
BaseApp {
public:

  /**
   * @brief Constructor de BaseApp.
   * @param hInst Instancia de la aplicación.
   * @param nCmdShow Parámetro que indica cómo debe mostrarse la ventana.
   */
  BaseApp(HINSTANCE hInst, int nCmdShow);

  /**
   * @brief Destructor de BaseApp.
   *        Llama automáticamente al método destroy().
   */
  ~BaseApp() { destroy(); }

  /**
   * @brief Ejecuta el ciclo principal de la aplicación.
   * @param hInst Instancia de la aplicación.
   * @param nCmdShow Modo de visualización de la ventana.
   * @return Código de salida de la aplicación.
   */
  int
  run(HINSTANCE hInst, int nCmdShow);

  /**
   * @brief Inicializa todos los componentes principales del motor gráfico.
   * @return HRESULT que indica el resultado de la inicialización.
   */
  HRESULT
  init();

  /**
   * @brief Actualiza el estado lógico de la aplicación.
   * @param deltaTime Tiempo transcurrido desde el último frame (en segundos).
   */
  void
  update(float deltaTime);

  /**
   * @brief Renderiza la escena en pantalla.
   */
  void
  render();

  /**
   * @brief Libera todos los recursos y objetos utilizados por la aplicación.
   */
  void
  destroy();

private:

  /**
   * @brief Función de ventana (Window Procedure) que maneja los mensajes del sistema.
   * @param hWnd Handle de la ventana.
   * @param message Mensaje recibido.
   * @param wParam Parámetro adicional del mensaje.
   * @param lParam Parámetro adicional del mensaje.
   * @return LRESULT con el resultado del procesamiento del mensaje.
   */
  static LRESULT CALLBACK
  WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


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
  MeshComponent												m_mesh;
  Buffer															m_vertexBuffer;
  Buffer															m_indexBuffer;
  Buffer															m_cbNeverChanges;
  Buffer															m_cbChangeOnResize;
  Buffer															m_cbChangesEveryFrame;
  Texture 														m_textureCube;
  SamplerState                        m_samplerState;

  ModelLoader                         m_modelLoader;
  LoadData                            LD;

  XMMATRIX                            m_World;
  XMMATRIX                            m_View;
  XMMATRIX                            m_Projection;
  XMFLOAT4                            m_vMeshColor; // (0.7f, 0.7f, 0.7f, 1.0f);

  CBChangeOnResize cbChangesOnResize;
  CBNeverChanges cbNeverChanges;
  CBChangesEveryFrame cb;
};