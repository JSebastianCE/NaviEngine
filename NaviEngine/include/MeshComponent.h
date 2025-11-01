#pragma once
#include "Prerequisites.h"

/**
 * @brief Declaraci�n adelantada de la clase DeviceContext.
 */
class 
DeviceContext;

/**
 * @class MeshComponent
 * @brief Clase encargada de almacenar y manejar los datos de una malla,
 *        incluyendo sus v�rtices e �ndices, as� como su inicializaci�n,
 *        actualizaci�n y renderizado.
 */
class
MeshComponent {
public:

  /**
   * @brief Constructor por defecto de MeshComponent.
   *        Inicializa el n�mero de v�rtices e �ndices en cero.
   */
  MeshComponent() : m_numVertex(0), m_numIndex(0) {}

  /**
   * @brief Destructor virtual por defecto.
   */
  virtual
  ~MeshComponent() = default;

  /**
   * @brief Inicializa los recursos o configuraciones necesarias de la malla.
   */
  void
  init();

  /**
   * @brief Actualiza el estado de la malla seg�n el tiempo transcurrido.
   * @param deltaTime Tiempo en segundos desde el �ltimo frame.
   */
  void
  update(float deltaTime);

  /**
   * @brief Renderiza la malla utilizando el contexto del dispositivo.
   * @param deviceContext Contexto del dispositivo utilizado para dibujar.
   */
  void
  render(DeviceContext& deviceContext);

  /**
   * @brief Libera los recursos asociados a la malla.
   */
  void
  desrtroy();

public:

  /** @brief Nombre identificador de la malla. */
  std::string m_name;

  /** @brief Vector que contiene los v�rtices de la malla. */
  std::vector<SimpleVertex> m_vertex;

  /** @brief Vector que contiene los �ndices de la malla. */
  std::vector<unsigned int> m_index;

  /** @brief N�mero total de v�rtices de la malla. */
  int m_numVertex;

  /** @brief N�mero total de �ndices de la malla. */
  int m_numIndex;
};
