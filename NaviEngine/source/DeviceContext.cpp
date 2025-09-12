#include "DeviceContext.h"

void
DeviceContext::destroy() {
  SAFE_RELEASE(m_deviceContext);
}

void
DeviceContext::OMSetRenderTargets(unsigned int NumViews,
  ID3D11RenderTargetView* const* ppRenderTargetViews,
  ID3D11DepthStencilView* pDepthStencilView) {

  //Validar los parametros de entrada
  if (!ppRenderTargetViews && pDepthStencilView) {
    ERROR("DeviceContext", "OMSetRenderTargets",
      "Both ppRenderTargetViews and pDepthStencilView are nullptr");
    return;
  }
  if (!NumViews && ppRenderTargetViews) {
    ERROR("DeviceContext", "OMSetRenderTargets",
      "ppRenderTargetViews is nullptr, but NumView > 0");
    return;
  }

  //Asignar los render targets y el depth stencil
  m_deviceContext->OMSetRenderTargets(NumViews, ppRenderTargetViews, pDepthStencilView);
}

void 
DeviceContext::RSSetViewports(unsigned int NumViewports,
                              const D3D11_VIEWPORT* pViewports) {

  //Validar los parametrod se entrada
  if (NumViewports == 0) {
    ERROR("DeviceContext", "RSSetViewports", "NumViewports is 0");
    return;
  }
  if (!pViewports) {
    ERROR("DeviceContext", "RSSetViewports", "pViewports is nullptr");
    return;
  }
  //Asignar los viewports
  m_deviceContext->RSSetViewports(NumViewports, pViewports);
}

void
DeviceContext::IASetInputLayout(ID3D11InputLayout* pInputLayout) {
  //Validar los parametros de entrada
  if (!pInputLayout) {
    ERROR("DeviceContext", "IASetInputLayout", "pInputLayout is nullptr");
    return;
  }

  m_deviceContext->IASetInputLayout(pInputLayout);
}

void
DeviceContext::IASetVertexBuffers(unsigned int StartSlot,
                                  unsigned int NumBuffers,
                                  ID3D11Buffer* const* ppVertexBuffers,
                                  const unsigned int* pStrides,
                                  const unsigned int* pOffsets) {
  if (!ppVertexBuffers || pStrides || pOffsets) {
    ERROR("DeviceContext", "IASetVertexBuffers",
      "Invalid argument: ppVertexBuffers, pStrides, or pOffsets is nullptr");
      return;
  }
  m_deviceContext->IASetVertexBuffers(StartSlot, 
                                      NumBuffers, 
                                      ppVertexBuffers, 
                                      pStrides, 
                                      pOffsets);
  }


  void
  DeviceContext::IASetIndexBuffer(ID3D11Buffer * pIndexBuffer,
                                    DXGI_FORMAT Format,
                                    unsigned int Offset) {
    if (!pIndexBuffer) {
      ERROR("DeviceContext", "IASetIndexBuffer", "pIndexBuffer is nullptr");
      return;
    }
    m_deviceContext->IASetIndexBuffer(pIndexBuffer, Format, Offset);
}

  void
  DeviceContext::UpdateSubresource(ID3D11Resource* pDstResource,
                                    unsigned int DstSubresource,
                                    const D3D11_BOX* pDstBox,
                                    const void* pSrcData,
                                    unsigned int SrcRowPitch,
                                    unsigned int SrcDepthPitch) {
    if (!pDstResource || !pSrcData) {
      ERROR("DeviceContext", "UpdateSubresource",
        "Invalid argument: pDstResource, or pSrcData is nullptr");
      return;
    }
    m_deviceContext->UpdateSubresource(pDstResource,
                                      DstSubresource,
                                      pDstBox,
                                      pSrcData,
                                      SrcRowPitch,
                                      SrcDepthPitch);
  }

  void
  DeviceContext::IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY Topology) {
    //Validar el parametro Topology
    if (!Topology) {
      ERROR("DeviceContext", "IASetPrimitiveTopology", 
            "Topology is D3D11_PRIMITIVE_TOPOLOGY_UNDEGINED");
      return;
    }
    m_deviceContext->IASetPrimitiveTopology(Topology);
  }

  void
    DeviceContext::ClearRenderTargetView(ID3D11RenderTargetView* pRenderTargetView,
      const float ColorRGBA[4]) {
    //Validar parametros
    if (!pRenderTargetView) {
      ERROR("DeviceContext", "ClearRenderTargetView", "pRenderTargetView is nullptr");
      return;
    }
    if (!ColorRGBA) {
      ERROR("DeviceContext", "ClearRenderTargetView", "ColorRGBA is nullptr");
      return;
    }

    //Limpiar el render target
    m_deviceContext->ClearRenderTargetView(pRenderTargetView, ColorRGBA);
  }

