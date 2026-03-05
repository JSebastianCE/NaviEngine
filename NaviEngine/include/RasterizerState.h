#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;


class
RasterizerState {
public:
  
  RasterizerState() = default;
 
  ~RasterizerState() = default;

  
  HRESULT
  init(Device device);

  HRESULT
  init(Device& device,
      D3D11_FILL_MODE fill,
      D3D11_CULL_MODE cull,
      bool frontCCW,
      bool depthClip);


  void
  update();

  void
  render(DeviceContext& deviceContext);


  void
  destroy();

private:
  
  ID3D11RasterizerState* m_rasterizerState = nullptr;
};