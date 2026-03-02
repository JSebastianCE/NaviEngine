#pragma once
#include "Prerequisites.h"
#include "EngineUtilities/Vectors/Vector3.h"
#include "Component.h"

/**
 * @class Transform
 * @brief Componente que almacena la transformación espacial de una entidad.
 *
 * Gestiona posición, rotación y escala, además de su matriz de transformación.
 * Puede ser controlado externamente por herramientas como ImGuizmo o
 * reconstruido manualmente desde sus vectores.
 */
class
Transform : public
Component {
public:

  /**
   * @brief Constructor por defecto.
   *
   * Inicializa vectores y asigna el tipo de componente TRANSFORM.
   */
  Transform() : position(), rotation(), scale(), matrix(), Component(ComponentType::TRANSFORM) {}

  /**
   * @brief Inicializa valores por defecto del Transform.
   *
   * Establece la escala en uno y la matriz como identidad.
   */
  void
  init() {
    scale.one();
    matrix = XMMatrixIdentity();
  }

  /**
   * @brief Actualización por frame.
   *
   * Actualmente vacío intencionalmente para evitar conflictos
   * con herramientas externas como ImGuizmo que controlan la matriz.
   *
   * @param deltaTime Tiempo transcurrido desde el último frame.
   */
  void
  update(float deltaTime) override {
    // Vacío intencionalmente para evitar pelear con ImGuizmo
  }

  /**
   * @brief Render del componente.
   *
   * No realiza ninguna operación visual directamente.
   *
   * @param deviceContext Contexto de dispositivo DirectX.
   */
  void
  render(DeviceContext& deviceContext) override {}

  /**
   * @brief Libera recursos asociados al Transform.
   */
  void
  destroy() {}

  /**
   * @brief Obtiene la posición actual.
   *
   * @return Referencia constante a la posición.
   */
  const EU::Vector3& getPosition() const { return position; }

  /**
   * @brief Establece la posición.
   *
   * @param newPos Nueva posición en espacio mundo.
   */
  void
  setPosition(const EU::Vector3& newPos) { position = newPos; }

  /**
   * @brief Obtiene la rotación actual.
   *
   * @return Referencia constante a la rotación (en grados).
   */
  const
  EU::Vector3& getRotation() const { return rotation; }

  /**
   * @brief Establece la rotación.
   *
   * @param newRot Nueva rotación en grados (Pitch, Yaw, Roll).
   */
  void
  setRotation(const EU::Vector3& newRot) { rotation = newRot; }

  /**
   * @brief Obtiene la escala actual.
   *
   * @return Referencia constante a la escala.
   */
  const
  EU::Vector3& getScale() const { return scale; }

  /**
   * @brief Establece la escala.
   *
   * @param newScale Nueva escala por eje.
   */
  void
  setScale(const EU::Vector3& newScale) { scale = newScale; }

  /**
   * @brief Establece posición, rotación y escala simultáneamente.
   *
   * @param newPos Nueva posición.
   * @param newRot Nueva rotación.
   * @param newSca Nueva escala.
   */
  void
  setTransform(const EU::Vector3& newPos, 
                const EU::Vector3& newRot, 
                const EU::Vector3& newSca) {
    position = newPos;
    rotation = newRot;
    scale = newSca;
  }

  /**
   * @brief Reconstruye la matriz de transformación a partir de los vectores.
   *
   * Debe llamarse únicamente cuando el objeto se modifica por código
   * (por ejemplo física, IA o animaciones).
   *
   * El orden de multiplicación es:
   * Scale * Rotation * Translation
   */
  void 
  rebuildMatrixFromVectors() {
    XMMATRIX scaleMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);
    XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(
      XMConvertToRadians(rotation.x),
      XMConvertToRadians(rotation.y),
      XMConvertToRadians(rotation.z)
    );
    XMMATRIX translationMatrix = XMMatrixTranslation(position.x, position.y, position.z);

    matrix = scaleMatrix * rotationMatrix * translationMatrix;
  }

  /**
   * @brief Aplica una traslación incremental al Transform.
   *
   * @param translation Vector de desplazamiento.
   *
   * @note Implementación definida en el archivo .cpp.
   */
  void 
  transform(const EU::Vector3& translation); // (Asumo que implementas esto en un .cpp)

private:

  /**
   * @brief Posición en espacio mundo.
   */
  EU::Vector3 position;

  /**
   * @brief Rotación en grados (Pitch, Yaw, Roll).
   */
  EU::Vector3 rotation;

  /**
   * @brief Escala por eje.
   */
  EU::Vector3 scale;

public:

  /**
   * @brief Matriz de transformación final.
   *
   * Puede ser modificada externamente (ej. ImGuizmo).
   */
  XMMATRIX matrix;
};
