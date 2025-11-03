#pragma once
#include "Prerequisites.h"
//#include "OBJ_Loader.h"

class
ModelLoader
{
public:

  ModelLoader() = default;
  ~ModelLoader() = default;

  void
  init();

  void
  update();

  void
  render();

  void
  destroy();

  LoadData
  Load(std::string objFileName);

private:
  //objl::Loader m_loader;
};