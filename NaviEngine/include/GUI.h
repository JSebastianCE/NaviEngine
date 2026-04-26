#pragma once

#include "Prerequisites.h"

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include <imgui_internal.h>
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "ImGuizmo.h"

// Forward declarations
class Viewport;
class Window;
class Device;
class DeviceContext;
class Actor;
class Camera;

/**
 * @class GUI
 * @brief Sistema de interfaz gráfica para el editor basado en ImGui + ImGuizmo.
 *
 * Esta clase encapsula toda la lógica de:
 * - Inicialización y destrucción de ImGui
 * - Renderizado de la interfaz (dockspace, viewport, panels)
 * - Manipulación de objetos mediante gizmos (translate/rotate/scale)
 * - Controles UI reutilizables (vec3, sliders)
 *
 * Está diseñada para funcionar como una capa de editor sobre el motor.
 */
class 
GUI {
public:

  /** @brief Constructor por defecto */
  GUI() = default;

  /** @brief Destructor por defecto */
  ~GUI() = default;

  // ==========================================================
  // CORE LIFECYCLE
  // ==========================================================

  /**
   * @brief Inicialización temprana del sistema GUI.
   * Se usa para preparar estados antes de init().
   */
  void 
  awake();

  /**
   * @brief Inicializa ImGui y sus bindings con Win32 + DX11.
   * @param window Referencia a la ventana principal.
   * @param device Dispositivo gráfico (DirectX).
   * @param deviceContext Contexto del dispositivo.
   */
  void
  init(Window& window, Device& device, DeviceContext& deviceContext);

  /**
   * @brief Actualiza la UI cada frame.
   * @param viewport Viewport del editor.
   * @param window Ventana principal.
   */
  void 
  update(Viewport& viewport, Window& window);

  /**
   * @brief Renderiza la UI en el backbuffer.
   */
  void 
  render();

  /**
   * @brief Libera todos los recursos de ImGui.
   */
  void
  destroy();


  // ==========================================================
  // UI SYSTEMS
  // ==========================================================

  /**
   * @brief Dibuja la barra de herramientas principal.
   */
  void 
  ToolBar();

  /**
   * @brief Maneja popup de salida de la aplicación.
   */
  void 
  closeApp();

  /**
   * @brief Inicializa datos de tooltips.
   */
  void
  toolTipData();


  // ==========================================================
  // STYLES
  // ==========================================================

  /**
   * @brief Aplica estilo visual tipo "Apple Liquid UI".
   * @param opacity Opacidad global (0..1).
   * @param accent Color principal del tema.
   */
  void 
  appleLiquidStyle(float opacity, ImVec4 accent);

  /**
   * @brief Aplica estilo visual tipo Wii (simple y limpio).
   */
  void 
  classicWiiStyle();


  // ==========================================================
  // CONTROLS
  // ==========================================================

  /**
   * @brief Control UI para edición de vectores vec3 (DragFloat).
   * @param label Etiqueta del control.
   * @param values Puntero a arreglo float[3].
   * @param resetValues Valor al reiniciar.
   * @param columnWidth Ancho de columna para label.
   */
  void
  vec3Control(const std::string& label,
    float* values,
    float resetValues = 0.0f,
    float columnWidth = 100.0f);

  /**
   * @brief Control UI tipo slider para vec3.
   * @param label Etiqueta del control.
   * @param values Puntero a arreglo float[3].
   * @param min Valor mínimo.
   * @param max Valor máximo.
   * @param resetValues Valor al reiniciar.
   * @param columnWidth Ancho de columna.
   */
  void
  vec3ControlSlider(const std::string& label,
    float* values,
    float min = -100.0f,
    float max = 100.0f,
    float resetValues = 0.0f,
    float columnWidth = 100.0f);


  // ==========================================================
  // DEBUG
  // ==========================================================

  /**
   * @brief Panel de depuración para luces.
   * @param lightDir Dirección de la luz (vec3).
   * @param lightColor Color de la luz (vec3).
   */
  void 
  debugLightsContainer(float* lightDir, float* lightColor);


  // ==========================================================
  // EDITOR PANELS
  // ==========================================================

  /**
   * @brief Panel principal del inspector.
   * @param actor Actor seleccionado.
   */
  void 
  inspectorGeneral(EU::TSharedPointer<Actor> actor);

  /**
   * @brief Contenedor interno del inspector (Transform, etc).
   * @param actor Actor seleccionado.
   */
  void 
  inspectorContainer(EU::TSharedPointer<Actor> actor);

  /**
   * @brief Lista jerárquica de actores en la escena.
   * @param actors Lista de actores activos.
   */
  void 
  outliner(const std::vector<EU::TSharedPointer<Actor>>& actors);


  // ==========================================================
  // GIZMOS
  // ==========================================================

  /**
   * @brief Permite manipular transformaciones con ImGuizmo.
   * @param cam Cámara activa.
   * @param window Ventana principal.
   * @param actor Actor seleccionado.
   */
  void 
  editTransform(Camera& cam, Window& window, EU::TSharedPointer<Actor> actor);

  /**
   * @brief Barra de herramientas para cambiar modo de gizmo.
   */
  void 
  drawGizmoToolbar();


  // ==========================================================
  // UI LAYOUT
  // ==========================================================

  /**
   * @brief Barra superior estilo editor (File, etc).
   */
  void 
  drawStudioTopRibbon();

  /**
   * @brief Dibuja el panel del viewport (render texture).
   * @param viewportSRV Shader Resource View del render.
   */
  void 
  drawViewportPanel(ID3D11ShaderResourceView* viewportSRV);

  /**
   * @brief Sistema de docking principal del editor.
   */
  void 
  drawEditorDockspace();


  // ==========================================================
  // UTILS
  // ==========================================================

  /**
   * @brief Convierte XMMATRIX a float[16] para ImGuizmo.
   * @param mat Matriz fuente.
   * @param dest Arreglo destino.
   */
  void
  ToFloatArray(const XMMATRIX& mat, float* dest) {
    XMFLOAT4X4 temp;
    XMStoreFloat4x4(&temp, mat);
    memcpy(dest, &temp, sizeof(float) * 16);
  }


private:
  /** @brief Estado de checkbox UI */
  bool 
  checkboxValue = true;

  /** @brief Estado secundario de checkbox */
  bool 
  checkboxValue2 = false;

  /** @brief Lista de nombres de objetos */
  std::vector<const char*> m_objectsNames;

  /** @brief Tooltips asociados a UI */
  std::vector<const char*> m_tooltips;

  /** @brief Indica si mostrar popup de salida */
  bool
  show_exit_popup = false;

  /** @brief Draw list del viewport (para overlays/gizmos) */
  ImDrawList* m_viewportDrawList = nullptr;

  /** @brief Indica si el viewport está activo */
  bool 
  m_viewportActive = false;


public:
  /** @brief Indica si el gizmo está en uso */
  bool
  m_isUsingGizmo = false;

  /** @brief Índice del actor seleccionado */
  int 
  selectedActorIndex = -1;

  /** @brief Posición del viewport en pantalla */
  ImVec2 
  m_viewportPos = { 0, 0 };

  /** @brief Tamaño del viewport */
  ImVec2 
  m_viewportSize = { 0, 0 };

  /** @brief Indica si el mouse está sobre el viewport */
  bool 
  m_viewportHovered = false;

  /** @brief Indica si el viewport tiene foco */
  bool 
  m_viewportFocused = false;
};