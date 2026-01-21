#pragma once
#include "Prerequisites.h"
#include "EngineUtilities/Vectors/Vector3.h"
#include "Component.h"


/**
 * @class Transform
 * @brief Componente encargado de manejar posición, rotación y escala de un objeto.
 *
 * Genera y actualiza la matriz de transformación combinando traslación,
 * rotación y escala. Hereda de la clase Component.
 */
class
Transform :
public
Component {

public:

  /**
   * @brief Constructor por defecto. Inicializa vectores y matriz.
   */
  Transform() : position(),
    rotation(),
    scale(),
    matrix(),
    Component(ComponentType::TRANSFORM) {
  }



  /**
   * @brief Inicializa el componente transform estableciendo escala a uno
   * y la matriz a identidad.
   */
  void
  init() {
    scale.one();
    matrix = XMMatrixIdentity();
  }


  /**
   * @brief Actualiza la matriz de transformación combinando escala, rotación
   * y traslación en el orden apropiado.
   * @param deltaTime Tiempo transcurrido entre frames.
   */
  void
  update(float deltaTime) override {

    // Aplicar escala
    XMMATRIX scaleMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);

    // Aplicar rotacion (CORREGIDO: Convertir Grados a Radianes)
    XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(XMConvertToRadians(rotation.x), XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z));

    // Aplicar traslacion
    XMMATRIX translationMatrix = XMMatrixTranslation(position.x, position.y, position.z);

    // Componer la matriz final en el orden: scale -> rotation -> translation
    matrix = scaleMatrix * rotationMatrix * translationMatrix;
  }


  /**
   * @brief Render del componente. No realiza ninguna operación en este caso.
   */
  void
  render(DeviceContext& deviceContext) override {}


  /**
   * @brief Libera o destruye recursos del componente. No realiza operaciones.
   */
  void
  destroy() {}


  /**
   * @brief Obtiene la posición actual.
   * @return Referencia constante al vector de posición.
   */
  const EU::Vector3&
  getPosition() const { return position; }

  /**
   * @brief Establece la posición del transform.
   * @param newPos Nueva posición.
   */
  void
  setPosition(const EU::Vector3& newPos) { position = newPos; }

  /**
   * @brief Obtiene la rotación actual.
   * @return Referencia constante al vector de rotación.
   */
  const EU::Vector3&
  getRotation() const { return rotation; }

  /**
   * @brief Establece la rotación del transform.
   * @param newRot Nueva rotación.
   */
  void
  setRotation(const EU::Vector3& newRot) { rotation = newRot; }

  /**
   * @brief Obtiene la escala actual.
   * @return Referencia constante al vector de escala.
   */
  const EU::Vector3&
  getScale() const { return scale; }

  /**
   * @brief Establece la escala del transform.
   * @param newScale Nueva escala.
   */
  void
  setScale(const EU::Vector3& newScale) { scale = newScale; }

  /**
   * @brief Establece todos los valores de transformación (posición, rotación, escala).
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
   * @brief Aplica un vector de traslación a la posición actual.
   * @param translation Vector de traslación.
   */
  void
  transform(const EU::Vector3& translation);


private:
  EU::Vector3 position;  ///< Posición del objeto.
  EU::Vector3 rotation;  ///< Rotación del objeto.
  EU::Vector3 scale;     ///< Escala del objeto.

public:
  XMMATRIX matrix; ///< Matriz de transformación final.
};