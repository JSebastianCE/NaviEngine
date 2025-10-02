#include "DepthStencilView.h"
#include "Device.h"
#include "DeviceContext.h"
#include "Texture.h"

//
// La función `init` inicializa el DepthStencilView. 
// Aquí se crea la vista de Direct3D que nos permite usar la textura como un buffer de profundidad.
//
HRESULT
DepthStencilView::init(Device& device, Texture& depthStencil, DXGI_FORMAT format) {
  //
  // Verificación de errores: se asegura de que los punteros y el formato sean válidos.
  // Si algo es nulo o desconocido, se devuelve un error para evitar problemas.
  //
  if (!device.m_device) {
    ERROR("DepthStencilView", "init", "Device is null.");
    return E_POINTER;
  }
  if (!depthStencil.m_texture) {
    ERROR("DepthStencilView", "init", "texture is null.");
    return E_POINTER;
  }
  if (format == DXGI_FORMAT_UNKNOWN) {
    ERROR("DepthStencilView", "init", "Format is DXGI_FORMAT_UNKNOWN.");
    return E_INVALIDARG;
  }

  //
  // Se obtiene la descripción de la textura para saber si es una textura multimuestreo.
  // Esto es importante para configurar correctamente el tipo de vista.
  //
  D3D11_TEXTURE2D_DESC texDesc;
  depthStencil.m_texture->GetDesc(&texDesc);

  //
  // Se configura la estructura que describe la vista de profundidad/plantilla.
  // Se usa ZeroMemory para limpiar la estructura antes de llenarla.
  //
  D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
  ZeroMemory(&descDSV, sizeof(descDSV));
  descDSV.Format = format;

  //
  // Si la textura tiene más de una muestra, se configura como una vista multimuestreo.
  // De lo contrario, se usa una vista 2D estándar con el primer mipmap.
  //
  if (texDesc.SampleDesc.Count > 1) {
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
  }
  else {
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
  }

  //
  // Se crea la vista de profundidad/plantilla usando la descripción.
  // Si la creación falla, se devuelve el código de error.
  //
  HRESULT hr = device.m_device->
    CreateDepthStencilView(depthStencil.m_texture,
                           &descDSV,
                           &m_depthStencilView);

  if (FAILED(hr)) {
    ERROR("DepthStencilView", "init",
      ("Failed to create depth stencil view. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  //
  // Mensaje de éxito si la vista se crea correctamente.
  //
  MESSAGE("DepthStencilView", "init", "Depth stencil view created successfully.");
  return S_OK;
}

//
// La función `render` se encarga de limpiar el buffer de profundidad y de plantilla.
// Esto es necesario al comienzo de cada fotograma para que la información del anterior no interfiera.
//
void
DepthStencilView::render(DeviceContext& deviceContext) {
  //
  // Verificación de errores: se asegura de que el contexto del dispositivo y la vista no sean nulos.
  //
  if (!deviceContext.m_deviceContext) {
    ERROR("DepthStencilView", "render", "Device context is null.");
    return;
  }

  if (!m_depthStencilView) {
    ERROR("DepthStencilView", "render", "DepthStencilView is null.");
    return;
  }

  //
  // Se limpia la vista de profundidad y plantilla.
  // Se establecen los valores por defecto: 1.0f para profundidad (el punto más lejano) y 0 para plantilla.
  //
  deviceContext.m_deviceContext->ClearDepthStencilView(m_depthStencilView,
                                                      D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                                      1.0f,
                                                      0);
}

//
// La función `destroy` libera la memoria de la vista de profundidad/plantilla cuando ya no es necesaria.
// Se usa la macro SAFE_RELEASE para asegurar que se libere correctamente.
//
void
DepthStencilView::destroy() {
  SAFE_RELEASE(m_depthStencilView);
}