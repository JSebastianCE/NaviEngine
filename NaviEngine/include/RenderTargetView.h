#pragma once
#include "Prerequisites.h"

//Forward Declarations
class 
Device;

class 
DeviceContext;

class 
Texture;

class
DepthStencilView;


/**
 * @class RenderTargetView
 * @brief Encapsula un Render Target View (RTV) de DirectX 11.
 *
 * Esta clase administra la creaci�n, uso y destrucci�n de un
 * ID3D11RenderTargetView, el cual se utiliza para renderizar
 * gr�ficos en una textura o en el back buffer.
 */
class
RenderTargetView {
public:

  /**
   * @brief Constructor por defecto.
   */
  RenderTargetView() = default;

  /**
   * @brief Destructor por defecto.
   */
  ~RenderTargetView() = default;

  /**
   * @brief Inicializa el Render Target View usando el back buffer.
   *
   * @param device Referencia al dispositivo de DirectX.
   * @param backBuffer Textura del back buffer.
   * @param format Formato de la textura (DXGI_FORMAT).
   * @return HRESULT C�digo de resultado (S_OK si se inicializ� correctamente).
   */
  HRESULT
  init(Device& device, Texture& backBuffer, DXGI_FORMAT format);

  /**
   * @brief Inicializa el Render Target View con una textura personalizada.
   *
   * @param device Referencia al dispositivo de DirectX.
   * @param inTex Textura de entrada.
   * @param viewDimension Dimensi�n del RTV (por ejemplo, TEXTURE2D, TEXTURE2DARRAY, etc.).
   * @param format Formato de la textura (DXGI_FORMAT).
   * @return HRESULT C�digo de resultado (S_OK si se inicializ� correctamente).
   */
  HRESULT
  init(Device& device,
      Texture& inTex,
      D3D11_RTV_DIMENSION viewDimension,
      DXGI_FORMAT format);

  /**
   * @brief Actualiza el estado del Render Target View.
   *
   * Funci�n placeholder que puede usarse para l�gica de actualizaci�n
   * relacionada al render target.
   */
  void
  update();

  /**
   * @brief Renderiza utilizando este Render Target View y un DepthStencilView.
   *
   * @param deviceContext Contexto del dispositivo para emitir comandos de render.
   * @param depthStencilView Referencia al DepthStencilView asociado.
   * @param numViews N�mero de vistas a aplicar.
   * @param clearColor Color con el que se limpia el render target (RGBA, 4 componentes).
   */
  void
  render(DeviceContext& deviceContext,
        DepthStencilView& depthStencilView,
        unsigned int numViews,
        const float clearColor[4]);

  /**
   * @brief Renderiza utilizando este Render Target View sin un DepthStencilView.
   *
   * @param deviceContext Contexto del dispositivo.
   * @param numViews N�mero de vistas a aplicar.
   */
  void
  render(DeviceContext& deviceContext,
        unsigned int numViews);

  /**
   * @brief Libera los recursos asociados al Render Target View.
   */
  void
  destroy();

private:
  /**
   * @brief Puntero al objeto ID3D11RenderTargetView de DirectX 11.
   */
  ID3D11RenderTargetView* m_renderTargetView = nullptr;
};
