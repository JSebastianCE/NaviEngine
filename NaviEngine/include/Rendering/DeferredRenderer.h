#pragma once
#include "Buffer.h"
#include "DepthStencilState.h"
#include "DepthStencilView.h"
#include "RasterizerState.h"
#include "Rendering/ISceneRenderer.h"
#include "Rendering/RenderScene.h"
#include "Rendering/RenderTypes.h"
#include "SamplerState.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "EngineUtilities/Utilities/EditorViewportPass.h"

class Device;
class DeviceContext;
class Camera;
class Material;

/**
 * @class DeferredRenderer
 * @brief Implementación de un renderizador Deferred basado en DirectX 11.
 *
 * Este renderizador utiliza un pipeline Deferred Rendering compuesto por:
 * - Geometry Pass (G-Buffer)
 * - Shadow Pass
 * - Deferred Lighting Pass
 * - Skybox Pass
 * - Transparent Pass
 *
 * También proporciona recursos de depuración para visualizar los diferentes
 * buffers generados durante el proceso de renderizado.
 */
class
DeferredRenderer : public ISceneRenderer {
public:
  /**
   * @brief Inicializa todos los recursos necesarios del renderizador.
   *
   * @param device Dispositivo gráfico utilizado para crear recursos.
   * @return HRESULT Resultado de la operación.
   */
  HRESULT
    init(Device& device) override;

  /**
   * @brief Redimensiona los recursos dependientes de la resolución.
   *
   * @param device Dispositivo gráfico.
   * @param width Nuevo ancho.
   * @param height Nueva altura.
   */
  void
  resize(Device& device, unsigned int width, unsigned int height) override;

  /**
   * @brief Ejecuta el pipeline completo de renderizado.
   *
   * @param deviceContext Contexto de dispositivo.
   * @param camera Cámara activa.
   * @param scene Escena a renderizar.
   * @param viewport Render target de salida.
   */
  void
  render(DeviceContext& deviceContext,
      const Camera& camera,
      RenderScene& scene,
      EditorViewportPass& viewport) override;

  /**
   * @brief Libera todos los recursos utilizados por el renderizador.
   */
  void
  destroy() override;

  /**
   * @brief Obtiene el Shader Resource View del Shadow Map.
   *
   * @return SRV asociado al mapa de sombras.
   */
  ID3D11ShaderResourceView*
  getShadowMapSRV() const override { return m_shadowDepthSRV.m_textureFromImg; }

  /**
   * @brief Obtiene el SRV del pase de depuración previo a sombras.
   *
   * @return Shader Resource View de depuración.
   */
  ID3D11ShaderResourceView*
  getPreShadowSRV() const override { return m_preShadowDebugPass.getSRV(); }

  /**
   * @brief Obtiene el SRV del buffer Albedo/Metallic.
   *
   * @return Shader Resource View correspondiente.
   */
  ID3D11ShaderResourceView*
  getGBufferAlbedoMetallicSRV() const override { return m_gBufferAlbedoMetallicSRV.m_textureFromImg; }

  /**
   * @brief Obtiene el SRV del buffer Normal/Roughness.
   *
   * @return Shader Resource View correspondiente.
   */
  ID3D11ShaderResourceView*
  getGBufferNormalRoughnessSRV() const override { return m_gBufferNormalRoughnessSRV.m_textureFromImg; }

  /**
   * @brief Obtiene el SRV del buffer World Position/AO.
   *
   * @return Shader Resource View correspondiente.
   */
  ID3D11ShaderResourceView*
  getGBufferWorldAoSRV() const override { return m_gBufferWorldAoSRV.m_textureFromImg; }

  /**
   * @brief Obtiene el SRV del buffer Emissive/Alpha.
   *
   * @return Shader Resource View correspondiente.
   */
  ID3D11ShaderResourceView*
  getGBufferEmissiveAlphaSRV() const override { return m_gBufferEmissiveAlphaSRV.m_textureFromImg; }

  /**
   * @brief Activa o desactiva la visualización del factor de sombras.
   *
   * @param enabled Estado de activación.
   */
  void
  setShadowFactorDebugEnabled(bool enabled) override { m_shadowFactorDebugEnabled = enabled; }

  /**
   * @brief Configura el modo de visualización de depuración Deferred.
   *
   * @param mode Modo de depuración.
   */
  void
  setDeferredDebugViewMode(int mode) override { m_deferredDebugViewMode = mode; }

  /**
   * @brief Obtiene el nombre del renderizador.
   *
   * @return Nombre del renderizador.
   */
  const char*
  getDebugName() const override { return "DeferredRenderer"; }

private:
  /**
   * @brief Construye las colas de renderizado para objetos opacos y transparentes.
   */
  void
  buildQueues(RenderScene& scene, const Camera& camera);

  /**
   * @brief Actualiza la información global de frame enviada a GPU.
   */
  void 
  updatePerFrame(const Camera& camera, const RenderScene& scene, DeviceContext& deviceContext);

  /**
   * @brief Actualiza las matrices utilizadas para el cálculo de sombras.
   */
  void 
  updateLightMatrices(const Camera& camera, const RenderScene& scene);

  /**
   * @brief Renderiza la escena completa hacia un render target.
   */
  void
  renderSceneToTarget(DeviceContext& deviceContext, RenderScene& scene, EditorViewportPass& targetPass, bool applyShadows);

  /**
   * @brief Vincula los render targets del G-Buffer.
   */
  void
  bindGBufferTargets(DeviceContext& deviceContext, ID3D11DepthStencilView* depthStencilView);

  /**
   * @brief Vincula el render target final para la etapa de iluminación.
   */
  void 
  bindFinalTarget(DeviceContext& deviceContext, ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* depthStencilView);

  /**
   * @brief Limpia los SRV utilizados durante el renderizado Deferred.
   */
  void
  clearDeferredSRVs(DeviceContext& deviceContext);

  /**
   * @brief Ejecuta el Geometry Pass.
   */
  void 
  renderGeometryPass(DeviceContext& deviceContext);

  /**
   * @brief Renderiza un objeto dentro del Geometry Pass.
   */
  void 
  renderGeometryObject(DeviceContext& deviceContext, const RenderObject& object);

  /**
   * @brief Ejecuta el Deferred Lighting Pass.
   */
  void
  renderLightPass(DeviceContext& deviceContext);

  /**
   * @brief Renderiza el Skybox.
   */
  void 
  renderSkyboxPass(DeviceContext& deviceContext, RenderScene& scene);

  /**
   * @brief Ejecuta el renderizado de objetos transparentes.
   */
  void 
  renderTransparentPass(DeviceContext& deviceContext);

  /**
   * @brief Renderiza un objeto utilizando Forward Rendering.
   */
  void 
  renderForwardObject(DeviceContext& deviceContext, const RenderObject& object, RenderPassType passType);

  /**
   * @brief Genera el mapa de sombras de la escena.
   */
  void 
  renderShadowPass(DeviceContext& deviceContext);

  /**
   * @brief Renderiza un objeto dentro del Shadow Pass.
   */
  void 
  renderShadowObject(DeviceContext& deviceContext, const RenderObject& object);

  /**
   * @brief Crea los recursos necesarios para Shadow Mapping.
   */
  HRESULT 
  createShadoeResources(Device& device);

  /**
   * @brief Crea los recursos asociados al G-Buffer.
   */
  HRESULT 
  createGBufferResources(Device& device, unsigned int width, unsigned int height);

  /**
   * @brief Crea un render target individual del G-Buffer.
   */
  HRESULT 
  createGBufferTargtes(Device& device,
                              unsigned int width,
                              unsigned int height,
                              DXGI_FORMAT format,
                              Texture& texture,
                              Texture& srv,
                              RenderTargetView& rtv);

  /**
   * @brief Crea los recursos necesarios para la etapa de iluminación.
   */
  HRESULT 
  createLightingResources(Device& device);

  /**
   * @brief Crea la geometría de pantalla completa utilizada en el Lighting Pass.
   */
  HRESULT
  createFullScreenQuad(Device& device);

  /**
   * @brief Crea los estados de mezcla utilizados por el renderizador.
   */
  HRESULT 
  createBlendState(Device& device);

  /**
   * @brief Obtiene el Blend State apropiado para un material.
   */
  ID3D11BlendState* resolveBlendState(const Material& material) const;

private:
  /// Constant Buffer con información global por frame.
  Buffer m_perFrameBuffer;

  /// Constant Buffer con información por objeto.
  Buffer m_perObjectBuffer;

  /// Constant Buffer con información por material.
  Buffer m_perMaterialBuffer;

  /// Constant Buffer para opciones de depuración de iluminación.
  Buffer m_lightingDebugBuffer;

  /// Vertex Buffer del Fullscreen Quad.
  Buffer m_fullscreenVertexBuffer;

  /// Index Buffer del Fullscreen Quad.
  Buffer m_fullscreenIndexBuffer;

  /// Estado de profundidad para objetos transparentes.
  DepthStencilState m_transparentDepthStencil;

  /// Estado de profundidad deshabilitado.
  DepthStencilState m_disabledDepthStencil;

  /// Estado de profundidad utilizado en sombras.
  DepthStencilState m_shadowDepthStencil;

  /// Estado de mezcla Alpha Blend.
  ID3D11BlendState* m_alphaBlendState = nullptr;

  /// Estado de mezcla para objetos opacos.
  ID3D11BlendState* m_opaqueBlendState = nullptr;

  /// Estado de mezcla aditivo.
  ID3D11BlendState* m_additiveBlendState = nullptr;

  /// Estado de mezcla Premultiplied Alpha.
  ID3D11BlendState* m_premultipliedBlendState = nullptr;

  /// Factor de mezcla utilizado por DirectX.
  float m_blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

  /// Textura principal del Shadow Map.
  Texture m_shadowDepthTexture;

  /// Shader Resource View del Shadow Map.
  Texture m_shadowDepthSRV;

  /// Depth Stencil View para sombras.
  DepthStencilView m_shadowDSV;

  /// Shader utilizado para generar sombras.
  ShaderProgram m_shadowShader;

  /// Rasterizer State utilizado en Shadow Mapping.
  RasterizerState m_shadowRasterizer;

  /// Resolución del Shadow Map.
  unsigned int m_shadowMapSize = 2048;

  /// Shader del Geometry Pass.
  ShaderProgram m_gBufferShader;

  /// Shader del Deferred Lighting Pass.
  ShaderProgram m_deferredLightingShader;

  /// Sampler utilizado durante iluminación.
  SamplerState m_lightingSampler;

  /// Rasterizer State del Fullscreen Quad.
  RasterizerState m_fullscreenRasterizer;

  Texture m_gBufferAlbedoMetallicTexture;
  Texture m_gBufferAlbedoMetallicSRV;
  RenderTargetView m_gBufferAlbedoMetallicRTV;

  Texture m_gBufferNormaLRoughnessTexture;
  Texture m_gBufferNormalRoughnessSRV;
  RenderTargetView m_gBufferNormalRoughnessRTV;

  Texture m_gBufferWorldAoTexture;
  Texture m_gBufferWorldAoSRV;
  RenderTargetView m_gBufferWorLdAoRTV;

  Texture m_gBufferEmissiveAlphaTexture;
  Texture m_gBufferEmissiveAlphaSRV;
  RenderTargetView m_gBufferEmissiveAlphaRTV;

  /// Render target auxiliar utilizado para depuración.
  EditorViewportPass m_preShadowDebugPass;

  /// Indica si las sombras deben aplicarse.
  bool m_aplyShadows = true;

  /// Ancho actual de renderizado.
  unsigned int m_renderWidth = 1280;

  /// Alto actual de renderizado.
  unsigned int m_renderHeight = 720;

  /// Datos globales enviados por frame.
  CBPerFrame m_cbPerFrame{};

  /// Datos enviados por objeto.
  CBPerObject m_cbPerObject{};

  /// Datos enviados por material.
  CBPerMaterial m_cbPerMaterial{};

  /**
   * @brief Estructura utilizada para controlar la depuración
   * del Deferred Lighting Pass.
   */
  struct DeferredLightingDebugData {
    int DebugViewMode = 0;
    float ShadowStrength = 1.0f;
    float pado = 0.0f;
    float pad1 = 0.0f;
  } m_lightingDebugData{};

  /// Indica si se visualiza el factor de sombras.
  bool m_shadowFactorDebugEnabled = false;

  /// Modo de depuración activo.
  int m_deferredDebugViewMode = 0;

  /// Cola de objetos opacos.
  std::vector<const RenderObject*> m_opaqueQueue;

  /// Cola de objetos transparentes.
  std::vector<const RenderObject*> m_transparentQueue;
};