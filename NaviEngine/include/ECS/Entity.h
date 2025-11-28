#pragma once
#include "Prerequisites.h"
#include "Component.h"

class DeviceContext;

class
Entity {
public:
  Entity() = default;


  virtual
    ~Entity() = default;

  
  virtual void
    init() = 0;


  virtual void
    update(float deltaTime, DeviceContext& deviceContext) = 0;

  virtual void
    render(DeviceContext& deviceContext) = 0;


  virtual void
    destroy() = 0;

  template <typename T> void
    addComponent(EU::TSharedPointer<T> component) {
    //static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");
    m_components.push_back(component.template dynamic_pointer_cast<Component>());
  }
  

  template<typename T>
  EU::TSharedPointer<T>
    getComponent() {
    for (auto& component : m_components) {
      EU::TSharedPointer<T> specificComponent = component.template dynamic_pointer_cast<T>();
      if (specificComponent) {
        return specificComponent;
      }
    }
    return EU::TSharedPointer<T>();
  }

private:
protected:
  bool m_isActive;
  int m_id;
  std::vector<EU::TSharedPointer<Component>> m_components;
};