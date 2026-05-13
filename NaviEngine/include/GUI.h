#pragma once

#include "Prerequisites.h"

#include "ECS/LightComponent.h"
#include "ECS/MeshRendererComponent.h"

#include "Rendering/Material.h"
#include "Rendering/MaterialInstance.h"
#include "Rendering/Mesh.h"

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

extern IMGUI_IMPL_API
LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class GUI {
public:
  GUI() = default;
  ~GUI() = default;

  void awake();

  void init(Window& window, Device& device, DeviceContext& deviceContext);
  void update(Viewport& viewport, Window& window);
  void render();
  void destroy();

  void ToolBar();
  void closeApp();
  void toolTipData();

  void appleLiquidStyle(float opacity, ImVec4 accent);
  void classicWiiStyle();

  void vec3Control(const std::string& label,
    float* values,
    float resetValues = 0.0f,
    float columnWidth = 100.0f,
    bool displayAsDegrees = false);

  void vec3ControlSlider(const std::string& label,
    float* values,
    float min = -100.0f,
    float max = 100.0f,
    float resetValues = 0.0f,
    float columnWidth = 100.0f);

  void debugLightsContainer(float* lightDir, float* lightColor);

  void inspectorGeneral(EU::TSharedPointer<Actor> actor);
  void inspectorContainer(EU::TSharedPointer<Actor> actor);
  void outliner(const std::vector<EU::TSharedPointer<Actor>>& actors);

  void editTransform(Camera& cam, Window& window, EU::TSharedPointer<Actor> actor);
  void drawGizmoToolbar();

  void drawStudioTopRibbon();
  void drawViewportPanel(ID3D11ShaderResourceView* viewportSRV);

  void drawRenderDebugPanel(ID3D11ShaderResourceView* preShadowSRV,
    ID3D11ShaderResourceView* finalViewportSRV,
    ID3D11ShaderResourceView* shadowMapSRV);

  void drawEditorDockspace();

  bool consumeSaveSceneRequest() {
    const bool requested = m_requestSaveScene;
    m_requestSaveScene = false;
    return requested;
  }

  void ToFloatArray(const XMMATRIX& mat, float* dest) {
    XMFLOAT4X4 temp;
    XMStoreFloat4x4(&temp, mat);
    memcpy(dest, &temp, sizeof(float) * 16);
  }

private:
  bool checkboxValue = true;
  bool checkboxValue2 = false;

  std::vector<const char*> m_objectsNames;
  std::vector<const char*> m_tooltips;

  bool show_exit_popup = false;
  bool m_requestSaveScene = false;

  ImDrawList* m_viewportDrawList = nullptr;
  bool m_viewportActive = false;

public:
  bool m_isUsingGizmo = false;
  int selectedActorIndex = -1;

  ImVec2 m_viewportPos = { 0.0f, 0.0f };
  ImVec2 m_viewportSize = { 0.0f, 0.0f };

  bool m_viewportHovered = false;
  bool m_viewportFocused = false;
};