#include "SceneGraph/SceneGraph.h"
#include "SceneGraph/HierarchyComponent.h"
#include "ECS/Entity.h"
#include "DeviceContext.h"
#include "ECS/Transform.h" 

void 
SceneGraph::init() {
  m_entities.clear();
}

void 
SceneGraph::addEntity(const EU::TSharedPointer<Entity>& e) {
  if (!e) {
    return;
  }

  // registra si no existe
  for (auto& it : m_entities) {
    if (it == e) {
      return;
    }
  }

  m_entities.push_back(e);

  // Validar que existen los componentes minimos
  if (!e->getComponent<Transform>()) {
    e->addComponent(EU::MakeShared<Transform>());
    e->getComponent<Transform>()->init();
  }

  if (!e->getComponent<HierarchyComponent>()) {
    e->addComponent(EU::MakeShared<HierarchyComponent>());
    e->getComponent<HierarchyComponent>()->init();
  }
}

void
SceneGraph::attach(const EU::TSharedPointer<Entity>& child,
                   const EU::TSharedPointer<Entity>& parent) {
  if (!child || !parent || child == parent) {
    return;
  }

  addEntity(child);
  addEntity(parent);

  auto childHierarchy = child->getComponent<HierarchyComponent>();
  auto parentHierarchy = parent->getComponent<HierarchyComponent>();

  // Detach from revious parent any
  if (auto oldParent = childHierarchy->getParent()) {
    detach(child);
  }

  //Set new parent
  childHierarchy->setParent(parent);
  parentHierarchy->addChild(child);

  //Update child transform relative to new parent - diry world transform

}

void
SceneGraph::detach(const EU::TSharedPointer<Entity>& child) {
  if (!child) {
    return;
  }


  auto childHierarchy = child->getComponent<HierarchyComponent>();
  if (!childHierarchy) {
    return;
  }

  auto parent = childHierarchy->getParent();
  if (parent) {
    auto parentHierarchy = parent->getComponent<HierarchyComponent>();
    if (parentHierarchy) {
      parentHierarchy->removeChild(child);
    }
  }

  childHierarchy->setParent(EU::TSharedPointer<Entity>());
}

void
SceneGraph::updateWorldRecursive(const EU::TSharedPointer<Entity>& node,
                                 const XMMATRIX& parentWorld) {
  auto t = node->getComponent<Transform>();

  auto h = node->getComponent<HierarchyComponent>();

  if (!t || !h) {
    return;
  }

  auto worldMatrix = t->matrix * parentWorld;

  for (auto& wChild : h->getChildren()) {
    auto child = wChild.lock();
    if (child) {
      updateWorldRecursive(child, worldMatrix);
    }
  }
}

void SceneGraph::render(DeviceContext& deviceContext) {
  //render all entities
  for (auto& e : m_entities) {
    if (e) {
      e->render(deviceContext);
    }
  }
}



bool
SceneGraph::isRoot(const EU::TSharedPointer<Entity>& e) const {

  auto hierarchy = e->getComponent<HierarchyComponent>();
  if (!hierarchy) {
    return true;
  }
  return (hierarchy->getParent() == EU::TSharedPointer<Entity>());
}

void
SceneGraph::update(float deltaTime, DeviceContext& deviceContext) {
  //Actalizar todas la entidades
  for (auto& e : m_entities) {
    if (e) {
      e->update(deltaTime, deviceContext);
    }
  }

  for (auto& e : m_entities) {
    if (e && isRoot(e)) {
      XMMATRIX identity = XMMatrixIdentity();
      updateWorldRecursive(e, identity);
    }
  }


}