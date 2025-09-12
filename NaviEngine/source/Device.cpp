#include "Device.h"

void
Device::destroy() {
  SAFE_RELEASE(m_device);
}

HRESULT
Device::CreateRenderTargetView(ID3D11Resource* pResource,
  const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
  ID3D11RenderTargetView** ppRTView) {
  //Validae paramentros de entrada
  if (!pResource) {
    ERROR("Device", "CreateRenderTargetView", "pResource is nullptr");
    return E_INVALIDARG;
  }
  if (!ppRTView) {
    ERROR("Device", "CreateRenderTargetView", "ppRTView is nullptr");
    return E_POINTER;
  }

  //Create el Render Target View
  HRESULT hr = m_device->CreateRenderTargetView(pResource, pDesc, ppRTView);

  if (SUCCEEDED(hr)) {
    MESSAGE("Device", "CreateRenderTargetView",
      "Render Target View created successfully");
  }
  else {
    ERROR("Device", "CreateRenderTargetView",
      ("Failed to create RenderTargetView. HRESULT: " + std::to_string(hr)).c_str());
  }

  return hr;
}

HRESULT
Device::CreateTexture2D(const D3D11_TEXTURE2D_DESC* pDesc,
  const D3D11_SUBRESOURCE_DATA* pInitialData,
  ID3D11Texture2D** ppTexture2D) {

  if (!pInitialData) {
    ERROR("Device", "CreateTexture2D", "pInitialData is nullptr");
    return E_INVALIDARG;
  }
  if (!ppTexture2D) {
    ERROR("Device", "CreateTexture2D", "ppTexture2D is nullptr");
    return E_POINTER;
  }

  //Create Texture 2D
  HRESULT hr = m_device->CreateTexture2D(pDesc, pInitialData, ppTexture2D);


  if (SUCCEEDED(hr)) {
    MESSAGE("Device", "CreateTexture2D",
      "Texture 2D created successfully");
  }
  else {
    ERROR("Device", "CreateTexture2D",
      ("Failed to create CreateTexture2D. HRESULT: " + std::to_string(hr)).c_str());
  }
 
  return hr;
}

HRESULT
Device::CreateDepthStencilView(ID3D11Resource* pResource,
  const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc,
  ID3D11DepthStencilView** ppDepthStencilView) {

  if (!pResource) {
    ERROR("Device", "CreateDepthStencilView", "pResource is nullptr");
    return E_INVALIDARG;
  }
  if (!ppDepthStencilView) {
    ERROR("Device", "CreateDepthStencilView", "ppDepthStencilView is nullptr");
    return E_POINTER;
  }

  //Create CreateDepthStencilView
  HRESULT hr = m_device->CreateDepthStencilView(pResource, pDesc, ppDepthStencilView);


  if (SUCCEEDED(hr)) {
    MESSAGE("Device", "CreateDepthStencilView",
      "Create Depth Stencil View created successfully");
  }
  else {
    ERROR("Device", "CreateDepthStencilView",
      ("Failed to create CreateDepthStencilView. HRESULT: " + std::to_string(hr)).c_str());
  }

  return hr;
}

HRESULT
Device::CreateVertexShader(const void* pShaderBytecode,
  unsigned int BytecodeLength, //unsigned
  ID3D11ClassLinkage* pClassLinkage,
  ID3D11VertexShader** ppVertexShader) {

  if (!pShaderBytecode) {
    ERROR("Device", "CreateVertexShader", "pShaderBytecode is nullptr");
    return E_INVALIDARG;
  }
  if (!BytecodeLength) {
    ERROR("Device", "CreateVertexShader", "BytecodeLength is nullptr");
    return E_POINTER;
  }
  if (!pClassLinkage) {
    ERROR("Device", "CreateVertexShader", "pClassLinkage is nullptr");
    return E_POINTER;
  }
  if (!ppVertexShader) {
    ERROR("Device", "CreateVertexShader", "ppVertexShader is nullptr");
    return E_POINTER;
  }

  //Create CreateVertexShader
  HRESULT hr = m_device->CreateVertexShader(pShaderBytecode, BytecodeLength,
                                            pClassLinkage, ppVertexShader);


  if (SUCCEEDED(hr)) {
    MESSAGE("Device", "CreateVertexShader",
      "Create Vertex Shader created successfully");
  }
  else {
    ERROR("Device", "CreateVertexShader",
      ("Failed to create CreateVertexShader. HRESULT: " + std::to_string(hr)).c_str());
  }


  return hr;
}


HRESULT
Device::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC * pInputElementDescs,
  UINT NumElements,
  const void* pShaderBytecodeWithInputSignature,
  unsigned int BytecodeLength, //unsigned
  ID3D11InputLayout * *ppInputLayout) {

  if (!pInputElementDescs) {
    ERROR("Device", "CreateInputLayout", "pInputElementDescs is nullptr");
    return E_INVALIDARG;
  }
  if (!NumElements) {
    ERROR("Device", "CreateInputLayout", "NumElements is nullptr");
    return E_POINTER;
  }
  if (!pShaderBytecodeWithInputSignature) {
    ERROR("Device", "CreateInputLayout", "pShaderBytecodeWithInputSignature is nullptr");
    return E_POINTER;
  }
  if (!ppInputLayout) {
    ERROR("Device", "CreateInputLayout", "ppInputLayout is nullptr");
    return E_POINTER;
  }

  //Create CreateVertexShader
  HRESULT hr = m_device->CreateInputLayout(pInputElementDescs, NumElements, 
                                           pShaderBytecodeWithInputSignature,
                                           BytecodeLength, ppInputLayout);


  if (SUCCEEDED(hr)) {
    MESSAGE("Device", "CreateInputLayout",
      "Create Input Layout created successfully");
  }
  else {
    ERROR("Device", "CreateInputLayout",
      ("Failed to create CreateInputLayout. HRESULT: " + std::to_string(hr)).c_str());
  }

  return 0;
  }

HRESULT
Device::CreatePixelShader(const void* pShaderBytecode,
  unsigned int BytecodeLength, //unsigned
  ID3D11ClassLinkage* pClassLinkage,
  ID3D11PixelShader** ppPixelShader) {

  if (!pShaderBytecode) {
    ERROR("Device", "CreatePixelShader", "pShaderBytecode is nullptr");
    return E_INVALIDARG;
  }
  if (!BytecodeLength) {
    ERROR("Device", "CreatePixelShader", "BytecodeLength is nullptr");
    return E_POINTER;
  }
  if (!pClassLinkage) {
    ERROR("Device", "CreatePixelShader", "pClassLinkage is nullptr");
    return E_POINTER;
  }
  if (!ppPixelShader) {
    ERROR("Device", "CreatePixelShader", "ppPixelShader is nullptr");
    return E_POINTER;
  }

  //Create CreateVertexShader
  HRESULT hr = m_device->CreatePixelShader(pShaderBytecode, BytecodeLength,
                                            pClassLinkage, ppPixelShader);


  if (SUCCEEDED(hr)) {
    MESSAGE("Device", "CreatePixelShader",
      "Create Pixel Shader created successfully");
  }
  else {
    ERROR("Device", "CreatePixelShader",
      ("Failed to create CreatePixelShader. HRESULT: " + std::to_string(hr)).c_str());
  }

  return 0;
}


HRESULT
Device::CreateBuffer(const D3D11_BUFFER_DESC* pDesc,
  const D3D11_SUBRESOURCE_DATA* pInitialData,
  ID3D11Buffer** ppBuffer) {

  if (!pInitialData) {
    ERROR("Device", "CreateBuffer", "pInitialData is nullptr");
    return E_POINTER;
  }
  if (!ppBuffer) {
    ERROR("Device", "CreateBuffer", "ppBuffer is nullptr");
    return E_POINTER;
  }
 

  //Create CreateVertexShader
  HRESULT hr = m_device->CreateBuffer(pDesc, pInitialData, ppBuffer);


  if (SUCCEEDED(hr)) {
    MESSAGE("Device", "CreateBuffer",
      "Create Buffer created successfully");
  }
  else {
    ERROR("Device", "CreateBuffer",
      ("Failed to create CreateBuffer. HRESULT: " + std::to_string(hr)).c_str());
  }

  return 0;
}


HRESULT
Device::CreateSamplerState(const D3D11_SAMPLER_DESC* pSamplerDesc,
  ID3D11SamplerState** ppSamplerState) {

  if (!pSamplerDesc) {
    ERROR("Device", "CreateSamplerState", "pSamplerDesc is nullptr");
    return E_POINTER;
  }
  if (!ppSamplerState) {
    ERROR("Device", "CreateSamplerState", "ppSamplerState is nullptr");
    return E_POINTER;
  }


  //Create CreateVertexShader
  HRESULT hr = m_device->CreateSamplerState(pSamplerDesc, ppSamplerState);


  if (SUCCEEDED(hr)) {
    MESSAGE("Device", "CreateSamplerState",
      "Create Sample State created successfully");
  }
  else {
    ERROR("Device", "CreateSamplerState",
      ("Failed to create CreateSamplerState. HRESULT: " + std::to_string(hr)).c_str());
  }

  return 0;
}
