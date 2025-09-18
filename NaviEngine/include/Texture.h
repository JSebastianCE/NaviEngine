#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;


class
Texture {
public:
  Texture();
  ~Texture();

  HRESULT
  init(Device& device,
      const std::string& textureName,
      ExtensionType extensionType);

  HRESULT
    init(Device& device,
      unsigned int widht,
      unsigned int height,
      DXGI_FORMAT Format,
      unsigned int BindFlags,
      unsigned int sampleCount = 1,
      unsigned int qualityLevels = 0);

  HRESULT
  init(Device& device, Texture& textureRef, DXGI_FORMAT format);


  void
    update();

  void
    render(DeviceContext& deviceContext, unsigned int StartSlot, unsigned int NumView);

  void
    destroy();


public:

  ID3D11Texture2D* m_texture = nullptr;

  ID3D11ShaderResourceView* m_rextureFromImg = nullptr;

  std::string m_rextureName;


};
