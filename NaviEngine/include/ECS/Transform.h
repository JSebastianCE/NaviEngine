#pragma once
#include "Prerequisites.h"
#include "EngineUtilities/Vectors/Vector3.h"
#include "Component.h"

class Transform : public Component {
public:
  Transform() : position(),
                rotation(),
                scale(),
                matrix(),
                Component(ComponentType::TRANSFORM) {
  }

  

  void 
  init() {
        scale.one();
        matrix = XMMatrixIdentity();
  }


  void update(float deltaTime) override {
    // Aplicar escala
    XMMATRIX scaleMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);

    // Aplicar rotacion
    XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);

    // Aplicar traslacion
    XMMATRIX translationMatrix = XMMatrixTranslation(position.x, position.y, position.z);

    // Componer la matriz final en el orden: scale -> rotation -> translation
    matrix = scaleMatrix * rotationMatrix * translationMatrix;
  }


  void
    render(DeviceContext& deviceContext) override {}


  void
    destroy(){}

  const EU::Vector3&
    getPosition() const { return position; }

  void 
    setPosition(const EU::Vector3& newPos) { position = newPos; }

  const EU::Vector3&
    getRotation() const { return rotation; }

  void
  setRotation(const EU::Vector3& newRot) { rotation = newRot; }

  const EU::Vector3&
    getScale() const { return scale; }

  void
    setScale(const EU::Vector3& newScale) { scale = newScale; }

  void
    setTransform(const EU::Vector3& newPos,
                 const EU::Vector3& newRot,
                 const EU::Vector3& newSca) {
    position = newPos;
    rotation = newRot;
    scale = newSca;
  }

  void
    transform(const EU::Vector3& translation);

private:
  EU::Vector3 position;
  EU::Vector3 rotation;
  EU::Vector3 scale;

public:
  XMMATRIX matrix;
};