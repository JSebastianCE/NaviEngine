#pragma once
#include "Prerequisites.h"

class Mesh;
class MaterialInstance;

/**
 * @enum MaterialDomain
 * @brief Define el dominio del material dentro del pipeline de render.
 */
enum 
class
  MaterialDomain {
  /** @brief Material completamente opaco. */
  Opaque = 0,

  /** @brief Material con transparencia binaria (alpha cutoff). */
  Masked,

  /** @brief Material con transparencia parcial. */
  Transparent
};

/**
 * @enum BlendMode
 * @brief Define el modo de mezcla de colores.
 */
enum 
class
  BlendMode {
  /** @brief Sin blending (opaco). */
  Opaque = 0,

  /** @brief Alpha blending estándar. */
  Alpha,

  /** @brief Mezcla aditiva (brillos, efectos). */
  Additive,

  /** @brief Alpha premultiplicado. */
  PremultipliedAlpha
};

/**
 * @enum RenderPassType
 * @brief Define los distintos passes del pipeline de render.
 */
enum
class
  RenderPassType {
  /** @brief Pass de sombras. */
  Shadow = 0,

  /** @brief Pass de objetos opacos. */
  Opaque,

  /** @brief Pass de skybox. */
  Skybox,

  /** @brief Pass de objetos transparentes. */
  Transparent,

  /** @brief Pass del editor. */
  Editor
};

/**
 * @enum LightType
 * @brief Tipos de luz soportados.
 */
enum 
class
  LightType {
  /** @brief Luz direccional. */
  Directional = 0,

  /** @brief Luz puntual. */
  Point,

  /** @brief Luz tipo spot. */
  Spot
};

/**
 * @struct LightData
 * @brief Representa los datos de una fuente de luz.
 */
struct
  LightData {
  /** @brief Tipo de luz. */
  LightType type = LightType::Directional;

  /** @brief Color de la luz. */
  EU::Vector3 color = EU::Vector3(1.0f, 1.0f, 1.0f);

  /** @brief Intensidad de la luz. */
  float 
  intensity = 1.0f;

  EU::Vector3 direction = EU::Vector3(0.0f, -1.0f, 0.0f);

  /** @brief Rango de influencia (para luces puntuales/spot). */
  float 
  range = 0.0f;

  /** @brief Posición de la luz. */
  EU::Vector3 position = EU::Vector3(0.0f, 0.0f, 0.0f);

  /** @brief Ángulo del cono para luces spot. */
  float
  spotAngle = 0.0f;
};

/**
 * @struct MaterialParams
 * @brief Parámetros configurables de un material (PBR).
 */
struct
MaterialParams {
  /** @brief Color base del material. */
  XMFLOAT4 baseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

  /** @brief Factor metálico. */
  float
  metallic = 1.0f;

  /** @brief Rugosidad del material. */
  float 
  roughness = 1.0f;

  /** @brief Ambient Occlusion. */
  float
  ao = 1.0f;

  /** @brief Intensidad del normal map. */
  float 
  normalScale = 1.0f;

  /** @brief Intensidad emisiva. */
  float 
  emissiveStrength = 1.0f;

  /** @brief Umbral de corte para alpha masking. */
  float 
  alphaCutoff = 0.5f;
};

/**
 * @struct CBPerFrame
 * @brief Constant buffer con datos globales por frame.
 */
struct
CBPerFrame {
  /** @brief Matriz de vista. */
  XMFLOAT4X4 View{};

  /** @brief Matriz de proyección. */
  XMFLOAT4X4 Projection{};

  /** @brief Matriz de luz (view-projection). */
  XMFLOAT4X4 LightViewProjection{};

  /** @brief Posición de la cámara. */
  EU::Vector3 CameraPos{};

  float
  pad0 = 0.0f;

  /** @brief Dirección de la luz principal. */
  EU::Vector3 LightDir = EU::Vector3(0.0f, -1.0f, 0.0f);

  float 
  pad1 = 0.0f;

  /** @brief Color de la luz principal. */
  EU::Vector3 LightColor = EU::Vector3(1.0f, 1.0f, 1.0f);

  float
  pad2 = 0.0f;
};

/**
 * @struct CBPerObject
 * @brief Constant buffer por objeto.
 */
struct
CBPerObject {
  /** @brief Matriz de transformación mundial. */
  XMFLOAT4X4 World{};
};



/**
 * @struct CBPerMaterial
 * @brief Constant buffer con datos específicos del material.
 */
struct
  CBPerMaterial {
  /** @brief Color base. */
  XMFLOAT4 BaseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

  /** @brief Factor metálico. */
  float Metallic = 1.0f;

  /** @brief Rugosidad. */
  float 
  Roughness = 1.0f;

  /** @brief Ambient Occlusion. */
  float 
  AO = 1.0f;

  /** @brief Escala de normales. */
  float 
  NormalScale = 1.0f;

  /** @brief Intensidad emisiva. */
  float
  EmissiveStrength = 1.0f;

  /** @brief Umbral alpha. */
  float
  AlphaCutoff = 0.0f;

  float pad0 = 0.0f;
  float pad1 = 0.0f;
  float pad2 = 0.0f;
  float pad3 = 0.0f;
  float pad4 = 0.0f;
  float pad5 = 0.0f;
};

/**
 * @struct RenderObject
 * @brief Representa un objeto renderizable dentro de la escena.
 */
struct
RenderObject {
  /** @brief Malla asociada. */
  Mesh* mesh = nullptr;

  /** @brief Instancia de material principal. */
  MaterialInstance* materialInstance = nullptr;

  /** @brief Lista de instancias de material (para submeshes). */
  std::vector<MaterialInstance*> materialInstances;

  /** @brief Transformación mundial del objeto. */
  XMMATRIX world = XMMatrixIdentity();

  /** @brief Indica si el objeto proyecta sombras. */
  bool 
  castShadow = true;

  /** @brief Indica si el objeto es transparente. */
  bool 
  transparent = false;

  /** @brief Distancia a la cámara (para sorting). */
  float 
  distanceToCamera = 0.0f;
};