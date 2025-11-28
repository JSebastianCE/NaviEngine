#pragma once
#include "Prerequisites.h"
#include "Entity.h"
#include "Buffer.h"
#include "Texture.h"
#include "Transform.h" 
#include "SamplerState.h"

#include "ShaderProgram.h"

//DepthStencilState.h

class Device;
class DeviceContext;
class MeshComponent;

class Actor : public Entity {
public:

  Actor() = default;

  Actor(Device& device);

  virtual ~Actor() = default;

  void 
    init() override {}

  void 
    update(float deltaTime, DeviceContext& deviceContext) override;

  void 
    render(DeviceContext& deviceContext) override;

  void 
    destroy();

  void 
    setMesh(Device& device, std::vector<MeshComponent> meshes);

  std::string getName() { return m_name; }

  void 
    setName(const std::string& name) { m_name = name; }

  void 
    setTextures(std::vector<Texture> textures) { m_textures = textures; }

  void 
    setCastShadow(bool v) { castShadow = v; }

  bool 
    canCastShadow() const { return castShadow; }

  void 
    renderShadow(DeviceContext& deviceContext);

private:
  std::vector<MeshComponent> m_meshes;      // Conjunto de componentes de malla del actor.
  std::vector<Texture> m_textures;          // Texturas aplicadas al actor.
  std::vector<Buffer> m_vertexBuffers;      // Buffers de vértices asociados a las mallas.
  std::vector<Buffer> m_indexBuffers;       // Buffers de índices asociados a las mallas.

  //BlendState m_blendState;                // Estado de blending usado por el actor.
  //Rasterizer m_rasterizer;                // Estado de rasterización usado por el actor.
  SamplerState m_sampler;                   // Estado de muestreo de texturas.
  CBChangesEveryFrame m_model;              // Constante de buffer para transformaciones por frame.
  Buffer m_modelBuffer;                     // Constant buffer que contiene @c m_model.

  // Recursos para sombras
  ShaderProgram m_shaderShadow;             // Shader program usado para renderizar sombras.
  Buffer m_shaderBuffer;                    // Buffer auxiliar para datos de sombras.
  //BlendState m_shadowBlendState;          // Estado de blending específico para sombras.
  //DepthStencilState m_shadowDepthStencilState; // Estado de profundidad/estencil para sombras.
  CBChangesEveryFrame m_cbShadow;           // Constant buffer específico de sombras.

  XMFLOAT4 m_LightPos;                      // Posición de la luz usada para proyectar sombras.
  std::string m_name = "Actor";             // Nombre identificador del actor.
  bool castShadow = true;                   // Indica si el actor proyecta sombras.
};