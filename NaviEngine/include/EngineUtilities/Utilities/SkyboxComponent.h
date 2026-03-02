#pragma once
#include "Prerequisites.h"
#include "MeshComponent.h"
#include "Skybox.h"

class 
SkyboxComponent : 
public 
MeshComponent
{
public:
  SkyboxComponent() : MeshComponent() {}
  ~SkyboxComponent() = default;

private:
    std::vector<SkyboxVertex> m_skyVertex;


};