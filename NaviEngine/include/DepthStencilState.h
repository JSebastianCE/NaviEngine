#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;


class
DepthStencilState {
public:
  
  DepthStencilState() = default;

  ~DepthStencilState() = default;

 
  HRESULT
    init(Device& device,
      bool depthEnable,
      D3D11_DEPTH_WRITE_MASK writeMask,
      D3D11_COMPARISON_FUNC depthFunc);

 
  void
    update();

  void
    render(DeviceContext& deviceContext, unsigned int stencilRef = 0, bool reset = false);


  void
    destroy();

private:
  ID3D11DepthStencilState* m_depthStencilState = nullptr;
};