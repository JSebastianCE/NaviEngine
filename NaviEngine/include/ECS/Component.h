#pragma once
#include "Prerequisites.h"

class 
DeviceContext;

class
  Component {
public:

  Component() = default;

 
  Component(const ComponentType type) : m_type(type) {}

 
  virtual
    ~Component() = default;

  virtual void
    init() = 0;

  
  virtual void
    update(float deltaTime) = 0;


  virtual void
    render(DeviceContext& deviceContext) = 0;

  virtual void
    destroy() = 0;

  ComponentType
    getType() const { return m_type; }

protected:
  ComponentType m_type; 
};