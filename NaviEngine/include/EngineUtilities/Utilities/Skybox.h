#pragma once
#include "Prerequisites.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "Buffer.h"
#include "SamplerState.h"
//#include "RasterizerState"
//#include "DepthStencilState.h"
#include "Model3D.h"
#include "EngineUtilities\Utilities\Camera.h"
#include "ECS\Actor.h"


struct 
SkyboxVertex{
  float x, y, z;
};


class
Skybox {
public:
  Skybox() = default;
  ~Skybox() = default;

  HRESULT
  init(Device& device, 
       DeviceContext* deviceContext, 
       Texture& cubemap);

  void
  update();

  void
  render(DeviceContext& deviceContext,
         Camera& camera);

  void
  destroy();


private:
  ShaderProgram m_shaderProgram;
  Buffer m_constantBuffer;
  SamplerState m_samplerState;
  //ResterizerState m_rasterizerState;
  //DepthStencilState m_depthStencilState;

  Texture m_skyboxTexture;
  Model3D* m_cubeModel = nullptr;
  EU::TSharedPointer<Actor> m_skybox;

};