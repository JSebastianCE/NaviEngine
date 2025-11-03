#include "ModelLoader.h"
#include "OBJ_Loader.h"

void 
ModelLoader::init()
{

}

void
ModelLoader::update()
{

}

void
ModelLoader::render()
{

}

void 
ModelLoader::destroy()
{

}

LoadData 
ModelLoader::Load(std::string objFileName)
{
  LoadData LD;
  objl::Loader m_loader;

  if (!m_loader.LoadFile(objFileName)) {
    ERROR("ModelLoader", "Load", "No se pudo cargar el archivo .obj");
    return LD; 
  }

  LD.name = objFileName;

  size_t vertexCount = m_loader.LoadedVertices.size();
  LD.vertex.resize(vertexCount);

  for (int i = 0; i < LD.vertex.size(); i++)
  {
    LD.vertex[i].Pos.x = m_loader.LoadedVertices[i].Position.X;
    LD.vertex[i].Pos.y = m_loader.LoadedVertices[i].Position.Y;
    LD.vertex[i].Pos.z = m_loader.LoadedVertices[i].Position.Z;

    LD.vertex[i].Tex.x = m_loader.LoadedVertices[i].TextureCoordinate.X;
    LD.vertex[i].Tex.y = m_loader.LoadedVertices[i].TextureCoordinate.Y;

    LD.vertex[i].Normal.x = m_loader.LoadedVertices[i].Normal.X;
    LD.vertex[i].Normal.y = m_loader.LoadedVertices[i].Normal.Y;
    LD.vertex[i].Normal.z = m_loader.LoadedVertices[i].Normal.Z;
  }

  size_t indexCount = m_loader.LoadedIndices.size();
  LD.index.resize(indexCount);
  LD.index = m_loader.LoadedIndices;

  LD.numVertex = (int)vertexCount;
  LD.numIndex = (int)indexCount;

  return LD; 
}