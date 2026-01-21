#pragma once
#include "Prerequisites.h"

#include "imgui.h"
#include <imgui_internal.h>
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "ImGuizmo.h"

class Window;
class Device;
class DeviceContext;
class Actor;

class 
GUI {
public:
  GUI() = default;
  ~GUI() = default;

  void 
  awake();

  // Inicialización con punteros nativos de DX11
  void 
  init(void* window, ID3D11Device* device, ID3D11DeviceContext* deviceContext);

  void 
  update();

  void 
  render();

  void 
  destroy();

  // Herramientas de UI
  void 
  ToolBar();

  void 
  closeApp();

  void
  drawGizmoToolbar();

  // Controles personalizados
  // Nota: Usamos const std::string& asumiendo que Prerequisites.h incluye <string>. 
  // Si no, cámbialo a const char* label.
  void
  vec3Control(const std::string& label,
    float* values,
    float resetValues = 0.0f,
    float columnWidth = 100.0f);
  

  // Paneles del Editor
  void
  inspectorGeneral(EU::TSharedPointer<Actor> actor);

  void 
  inspectorContainer(EU::TSharedPointer<Actor> actor);

  // Asumiendo que Prerequisites.h incluye <vector>
  void 
  outliner(const std::vector<EU::TSharedPointer<Actor>>& actors);

  // Manipulación de Transform (Gizmos)
  void 
  editTransform(const XMMATRIX& view, const XMMATRIX& projection, EU::TSharedPointer<Actor> actor);

public:
  // Variable pública para saber qué actor está seleccionado en el vector
  int selectedActorIndex = -1;

private:
  // Estado interno
  bool show_exit_popup = false;

  // Variables para el Gizmo (Movidas aquí para persistencia correcta)
  ImGuizmo::OPERATION mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
  ImGuizmo::MODE      mCurrentGizmoMode = ImGuizmo::WORLD;
};