#pragma once
#include "Prerequisites.h"

/**
 * @brief Declaraci�n adelantada de la clase Device.
 */
class
Device;

/**
 * @brief Declaraci�n adelantada de la clase DeviceContext.
 */
class
DeviceContext;

/**
 * @class InputLayout
 * @brief Clase encargada de gestionar la creaci�n, actualizaci�n, renderizado
 *        y destrucci�n del Input Layout en DirectX. Define c�mo los datos de
 *        los v�rtices se env�an al pipeline gr�fico.
 */
class
InputLayout {
public:
  /**
   * @brief Constructor por defecto de InputLayout.
   */
  InputLayout() = default;

  /**
   * @brief Destructor por defecto de InputLayout.
   */
  ~InputLayout() = default;

  /**
   * @brief Inicializa el Input Layout a partir de la descripci�n de entrada y los datos del Vertex Shader.
   * @param device Referencia al dispositivo de renderizado.
   * @param Layout Vector con la descripci�n de los elementos de entrada (atributos de v�rtice).
   * @param VertexShaderData Datos compilados del Vertex Shader necesarios para validar el layout.
   * @return HRESULT que indica el resultado de la creaci�n.
   */
  HRESULT
  init(Device& device,
      std::vector<D3D11_INPUT_ELEMENT_DESC>& Layout,
      ID3DBlob* VertexShaderData);

  /**
   * @brief Actualiza la informaci�n o el estado del Input Layout si es necesario.
   */
  void
  update();

  /**
   * @brief Aplica el Input Layout al contexto del dispositivo para el renderizado.
   * @param deviceContext Contexto del dispositivo utilizado para establecer el layout.
   */
  void
  render(DeviceContext& deviceContext);

  /**
   * @brief Libera los recursos asociados al Input Layout.
   */
  void
  destroy();


public:
  /** @brief Puntero al objeto Input Layout de Direct3D. */
  ID3D11InputLayout* m_inputLayout = nullptr;
};
