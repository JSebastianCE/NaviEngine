#include "Texture.h"
#include "Device.h"
#include "DeviceContext.h"

HRESULT
Texture::init(Device& device,
  const std::string& texturename,
  ExtensionType extensionType) {
  return E_NOTIMPL; 
}

HRESULT
Texture::init(Device& device,
        unsigned int widht,
        unsigned int height,
        DXGI_FORMAT Format,
        unsigned int BindFlags,
        unsigned int sampleCount,
        unsigned int qualityLevels) {
  if (!device.m_device) {
    ERROR("Texture", "init", "Device is null");
    return E_POINTER;
  }
  if (widht == 0 || height == 0) {
    ERROR("Texture", "init", "Width and height must be greater than 0");
    return E_INVALIDARG;
  }

  D3D11_TEXTURE2D_DESC desc = {};
  desc.Width = widht;
  desc.Height = height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = Format;
  desc.SampleDesc.Count = sampleCount;
  desc.SampleDesc.Quality = qualityLevels;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = BindFlags;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;

  HRESULT hr = device.CreateTexture2D(&desc, nullptr, &m_texture);

  if (FAILED(hr)) {
    ERROR("Texture", "init",
      ("Failed to create texture with specified params. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  return S_OK;
}

HRESULT
Texture::init(Device& device, Texture& textureRef, DXGI_FORMAT format) {
  if (!device.m_device) {
    ERROR("Texture", "init", "Device is null.");
    return E_POINTER;
  }
  if (!textureRef.m_texture) {
    ERROR("Texture", "init", "Texture is null");
    return E_POINTER;
  }

  // Create Shader Resource View
  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Format = format;
  srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MipLevels = 1;
  srvDesc.Texture2D.MostDetailedMip = 0;

  HRESULT hr = device.m_device->CreateShaderResourceView(textureRef.m_texture,
    &srvDesc,
    &m_textureFromImg);

  if (FAILED(hr)) {
    ERROR("Texture", "init",
      ("Failed to create shader resource view for texture. HRESULT: " + std::to_string(hr)).c_str());
    return hr;
  }

  return S_OK;
}

void
Texture::update() {
 
}

void
Texture::render(DeviceContext& deviceContext,
  unsigned int StartSlot,
  unsigned int NumView) {
  if (!deviceContext.m_deviceContext) {
    ERROR("Texture", "render", "Device Context is null.");
    return;
  }

  if (m_textureFromImg) {
    deviceContext.PSSetShaderResources(StartSlot, NumView, &m_textureFromImg);
  }
}

void
Texture::destroy() {
  if (m_texture) {
    SAFE_RELEASE(m_texture);
  }
  if (m_textureFromImg) {
    SAFE_RELEASE(m_textureFromImg);
  }
}
