#pragma once
#include "Prerequisites.h"

#include "imgui.h"
#include <imgui_internal.h>
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "ImGuizmo.h"

/**
 * @class Window
 * @brief Declaración adelantada de la clase Window.
 */
class 
Window;

/**
 * @class Device
 * @brief Declaración adelantada del wrapper del dispositivo DirectX11.
 */
class 
Device;

/**
 * @class DeviceContext
 * @brief Declaración adelantada del contexto de dispositivo DirectX11.
 */
class 
DeviceContext;

/**
 * @class Actor
 * @brief Representa una entidad del mundo que puede ser editada desde el editor.
 */
class 
Actor;

/**
 * @class GUI
 * @brief Sistema de interfaz gráfica del motor basado en ImGui.
 *
 * Esta clase se encarga de inicializar, actualizar, renderizar y destruir
 * toda la interfaz del editor, incluyendo paneles, barras de herramientas
 * y gizmos de transformación usando ImGuizmo.
 */
class
GUI {
public:
  /**
   * @brief Constructor por defecto.
   */
  GUI() = default;

  /**
   * @brief Destructor por defecto.
   */
  ~GUI() = default;

  /**
   * @brief Inicialización temprana del sistema GUI.
   *
   * Se utiliza para preparar estados internos antes de la inicialización
   * completa con DirectX.
   */
  void
  awake();

  /**
   * @brief Inicializa ImGui con los recursos nativos de DirectX11.
   *
   * @param window Puntero nativo a la ventana (Win32 HWND).
   * @param device Puntero al dispositivo DirectX11.
   * @param deviceContext Puntero al contexto del dispositivo DirectX11.
   */
  void
  init(void* window, ID3D11Device* device, ID3D11DeviceContext* deviceContext);

  /**
   * @brief Actualiza la lógica de la interfaz gráfica.
   *
   * Aquí se construyen los paneles y se procesan interacciones del usuario.
   */
  void
  update();

  /**
   * @brief Renderiza la interfaz gráfica en pantalla.
   *
   * Debe llamarse después del render de la escena 3D.
   */
  void
  render();

  /**
   * @brief Libera los recursos asociados a ImGui y la GUI.
   */
  void
    destroy();

  /**
   * @brief Dibuja la barra de herramientas principal del editor.
   */
  void
  ToolBar();

  /**
   * @brief Cierra la aplicación mostrando un popup de confirmación.
   */
  void
  closeApp();

  /**
   * @brief Dibuja la barra de herramientas de los gizmos (translate/rotate/scale).
   */
  void
  drawGizmoToolbar();

  /**
   * @brief Control personalizado para editar un vector de 3 componentes.
   *
   * @param label Etiqueta del control.
   * @param values Puntero a los valores float (x, y, z).
   * @param resetValues Valor al que se reinician los componentes.
   * @param columnWidth Ancho de la columna del label.
   */
  void
  vec3Control(const std::string& label,
              float* values,
              float resetValues = 0.0f,
              float columnWidth = 100.0f);


  /**
   * @brief Panel inspector general para un actor seleccionado.
   *
   * @param actor Actor actualmente seleccionado.
   */
  void
  inspectorGeneral(EU::TSharedPointer<Actor> actor);

  /**
   * @brief Panel inspector para componentes del actor.
   *
   * @param actor Actor actualmente seleccionado.
   */
  void
  inspectorContainer(EU::TSharedPointer<Actor> actor);

  /**
   * @brief Panel Outliner que muestra todos los actores de la escena.
   *
   * @param actors Vector de actores existentes en la escena.
   */
  void
  outliner(const std::vector<EU::TSharedPointer<Actor>>& actors);

  /**
   * @brief Permite editar el transform de un actor usando ImGuizmo.
   *
   * @param view Matriz de vista de la cámara.
   * @param projection Matriz de proyección de la cámara.
   * @param actor Actor a modificar.
   */
  void
  editTransform(const XMMATRIX& view, const XMMATRIX& projection, EU::TSharedPointer<Actor> actor);

public:
  /**
   * @brief Índice del actor seleccionado en el Outliner.
   *
   * Un valor de -1 indica que no hay ningún actor seleccionado.
   */
  int selectedActorIndex = -1;

private:
  /**
   * @brief Indica si el popup de salida está activo.
   */
  bool 
  show_exit_popup = false;

  /**
   * @brief Operación actual del gizmo (Translate, Rotate, Scale).
   */
  ImGuizmo::OPERATION mCurrentGizmoOperation = ImGuizmo::TRANSLATE;

  /**
   * @brief Modo del gizmo (World o Local).
   */
  ImGuizmo::MODE mCurrentGizmoMode = ImGuizmo::WORLD;
};
