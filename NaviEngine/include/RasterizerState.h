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
    init(Device device, unsigned int FillMode, unsigned int CullMode);

  void
    update();

  void
    render(DeviceContext& deviceContext);

  void
    destroy();

private:
  ID3D11RasterizerState* m_rasterizerState = nullptr;




};