#pragma once
#include "Prerequisites.h"

/**
 * @brief Declaraci�n adelantada de la clase Window.
 */
class
Window;

/**
 * @brief Declaraci�n adelantada de la clase DeviceContext.
 */
class
DeviceContext;

/**
 * @class Viewport
 * @brief Clase encargada de administrar el �rea visible de renderizado (viewport)
 *        dentro de una ventana o superficie de dibujo en DirectX.
 */
class
Viewport {
public:

  /**
   * @brief Constructor por defecto de Viewport.
   */
  Viewport() = default;

  /**
   * @brief Destructor por defecto de Viewport.
   */
  ~Viewport() = default;

  /**
   * @brief Inicializa el viewport utilizando las dimensiones de una ventana.
   * @param window Referencia a la ventana desde la cual se obtienen las dimensiones.
   * @return HRESULT que indica el resultado de la operaci�n.
   */
  HRESULT
  init(const Window& window);

  /**
   * @brief Inicializa el viewport con dimensiones personalizadas.
   * @param width Ancho del �rea de renderizado.
   * @param height Alto del �rea de renderizado.
   * @return HRESULT que indica el resultado de la operaci�n.
   */
  HRESULT
  init(unsigned int width, unsigned int height);

  /**
   * @brief Actualiza los par�metros del viewport si es necesario.
   */
  void
  update();

  /**
   * @brief Establece el viewport en el contexto del dispositivo para el renderizado.
   * @param deviceContext Contexto del dispositivo utilizado para aplicar el viewport.
   */
  void
  render(DeviceContext& deviceContext);

  /**
   * @brief Libera los recursos asociados al viewport (si aplica).
   */
  void
  destroy() {}

public:
  /** @brief Estructura que almacena la configuraci�n del viewport de Direct3D. */
  D3D11_VIEWPORT m_viewport;
};
