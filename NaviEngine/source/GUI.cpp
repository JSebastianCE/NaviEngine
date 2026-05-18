#include "GUI.h"
#include "Viewport.h"
#include "Window.h"
#include "Device.h"
#include "DeviceContext.h"
#include "MeshComponent.h"
#include "ECS/Actor.h"
#include "ECS/Transform.h"
#include "ECS/LightComponent.h"
#include "ECS/MeshRendererComponent.h"
#include "Rendering/Mesh.h"
#include "Rendering/Material.h"
#include "Rendering/MaterialInstance.h"
#include "EngineUtilities/Utilities/Camera.h"

static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);

namespace {

  float RadToDeg(float radians)
  {
    return XMConvertToDegrees(radians);
  }

  float DegToRad(float degrees)
  {
    return XMConvertToRadians(degrees);
  }

  const char* GetLightTypeLabel(LightType type)
  {
    switch (type)
    {
    case LightType::Directional: return "Directional";
    case LightType::Point:       return "Point";
    case LightType::Spot:        return "Spot";
    default:                     return "Unknown";
    }
  }

  const char* GetMaterialDomainLabel(MaterialDomain domain)
  {
    switch (domain)
    {
    case MaterialDomain::Opaque:      return "Opaque";
    case MaterialDomain::Masked:      return "Masked";
    case MaterialDomain::Transparent: return "Transparent";
    default:                          return "Unknown";
    }
  }

  const char* GetBlendModeLabel(BlendMode blendMode)
  {
    switch (blendMode)
    {
    case BlendMode::Opaque:              return "Opaque";
    case BlendMode::Alpha:               return "Alpha";
    case BlendMode::Additive:            return "Additive";
    case BlendMode::PremultipliedAlpha:  return "Premultiplied";
    default:                             return "Unknown";
    }
  }

}

// ==========================================================
// CORE
// ==========================================================

void
GUI::awake() {
}

void
GUI::init(Window& window, Device& device, DeviceContext& deviceContext) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  ImGui::StyleColorsDark();

  ImGuiStyle& style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
    style.WindowRounding = 18.0f;
    style.Colors[ImGuiCol_WindowBg].w = 0.82f;
  }

  // Verde oscuro tipo liquid glass
  appleLiquidStyle(0.78f, ImVec4(0.20f, 0.72f, 0.42f, 1.0f));

  ImGui_ImplWin32_Init(window.m_hWnd);
  ImGui_ImplDX11_Init(device.m_device, deviceContext.m_deviceContext);

  toolTipData();

  selectedActorIndex = -1;
}

void
GUI::update(Viewport& viewport, Window& window) {
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  ImGuizmo::BeginFrame();

  ImGuiIO& io = ImGui::GetIO();
  if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
    m_requestSaveScene = true;
  }

  ImGuizmo::SetOrthographic(false);

  drawStudioTopRibbon();
  drawEditorDockspace();
  closeApp();
  drawGizmoToolbar();
}

void
GUI::render() {
  ImGui::Render();
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

  ImGuiIO& io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
  }
}

void
GUI::destroy() {
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();
}

// ==========================================================
// STYLES
// ==========================================================

void
GUI::toolTipData() {
}

void
GUI::appleLiquidStyle(float opacity, ImVec4 accent) {
  ImGuiStyle& style = ImGui::GetStyle();
  ImVec4* colors = style.Colors;

  style.WindowRounding = 18.0f;
  style.ChildRounding = 18.0f;
  style.PopupRounding = 16.0f;
  style.FrameRounding = 12.0f;
  style.GrabRounding = 12.0f;
  style.ScrollbarRounding = 14.0f;
  style.TabRounding = 12.0f;

  style.WindowBorderSize = 1.0f;
  style.FrameBorderSize = 1.0f;
  style.PopupBorderSize = 1.0f;
  style.TabBorderSize = 0.0f;

  style.WindowPadding = ImVec2(14, 12);
  style.FramePadding = ImVec2(12, 8);
  style.ItemSpacing = ImVec2(10, 8);
  style.ItemInnerSpacing = ImVec2(8, 6);

  const float o = opacity;

  const ImVec4 textMain = ImVec4(0.92f, 0.98f, 0.94f, 0.96f);
  const ImVec4 textDisabled = ImVec4(0.66f, 0.78f, 0.70f, 0.75f);

  const ImVec4 bgMain = ImVec4(0.05f, 0.11f, 0.08f, o);
  const ImVec4 bgChild = ImVec4(0.07f, 0.14f, 0.10f, o * 0.92f);
  const ImVec4 bgPopup = ImVec4(0.08f, 0.16f, 0.11f, o * 0.98f);

  const ImVec4 frame = ImVec4(0.10f, 0.20f, 0.14f, 0.70f);
  const ImVec4 frameHover = ImVec4(0.13f, 0.28f, 0.18f, 0.82f);
  const ImVec4 frameActive = ImVec4(0.16f, 0.34f, 0.22f, 0.95f);

  const ImVec4 button = ImVec4(0.10f, 0.22f, 0.15f, 0.62f);
  const ImVec4 buttonHover = ImVec4(0.14f, 0.30f, 0.20f, 0.82f);
  const ImVec4 buttonActive = ImVec4(0.18f, 0.38f, 0.24f, 0.96f);

  const ImVec4 header = ImVec4(0.09f, 0.20f, 0.14f, 0.66f);
  const ImVec4 headerHover = ImVec4(0.13f, 0.28f, 0.19f, 0.84f);
  const ImVec4 headerActive = ImVec4(0.18f, 0.36f, 0.24f, 0.96f);

  const ImVec4 border = ImVec4(0.40f, 0.78f, 0.56f, 0.18f);
  const ImVec4 borderSoft = ImVec4(0.70f, 0.95f, 0.80f, 0.05f);

  colors[ImGuiCol_Text] = textMain;
  colors[ImGuiCol_TextDisabled] = textDisabled;

  colors[ImGuiCol_WindowBg] = bgMain;
  colors[ImGuiCol_ChildBg] = bgChild;
  colors[ImGuiCol_PopupBg] = bgPopup;
  colors[ImGuiCol_Border] = border;
  colors[ImGuiCol_BorderShadow] = borderSoft;

  colors[ImGuiCol_FrameBg] = frame;
  colors[ImGuiCol_FrameBgHovered] = frameHover;
  colors[ImGuiCol_FrameBgActive] = frameActive;

  colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.10f, 0.07f, 0.86f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.16f, 0.11f, 0.96f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.03f, 0.08f, 0.06f, 0.70f);
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.05f, 0.12f, 0.08f, 0.90f);

  colors[ImGuiCol_Button] = button;
  colors[ImGuiCol_ButtonHovered] = buttonHover;
  colors[ImGuiCol_ButtonActive] = buttonActive;

  colors[ImGuiCol_Header] = header;
  colors[ImGuiCol_HeaderHovered] = headerHover;
  colors[ImGuiCol_HeaderActive] = headerActive;

  colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.18f, 0.12f, 0.70f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.12f, 0.28f, 0.18f, 0.86f);
  colors[ImGuiCol_TabActive] = ImVec4(0.16f, 0.34f, 0.22f, 0.96f);
  colors[ImGuiCol_TabUnfocused] = ImVec4(0.06f, 0.14f, 0.10f, 0.58f);
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.10f, 0.22f, 0.15f, 0.75f);

  colors[ImGuiCol_CheckMark] = accent;
  colors[ImGuiCol_SliderGrab] = accent;
  colors[ImGuiCol_SliderGrabActive] = ImVec4(
    accent.x + 0.08f,
    accent.y + 0.08f,
    accent.z + 0.08f,
    1.0f
  );

  colors[ImGuiCol_Separator] = ImVec4(0.40f, 0.78f, 0.56f, 0.16f);
  colors[ImGuiCol_SeparatorHovered] = ImVec4(0.40f, 0.78f, 0.56f, 0.34f);
  colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.78f, 0.56f, 0.48f);

  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.40f, 0.78f, 0.56f, 0.18f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.78f, 0.56f, 0.30f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.40f, 0.78f, 0.56f, 0.42f);

  colors[ImGuiCol_DockingPreview] = ImVec4(accent.x, accent.y, accent.z, 0.30f);
  colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.03f, 0.07f, 0.05f, 0.65f);

  colors[ImGuiCol_TableHeaderBg] = ImVec4(0.08f, 0.18f, 0.12f, 0.85f);
  colors[ImGuiCol_TableBorderStrong] = ImVec4(0.40f, 0.78f, 0.56f, 0.16f);
  colors[ImGuiCol_TableBorderLight] = ImVec4(0.40f, 0.78f, 0.56f, 0.08f);
  colors[ImGuiCol_TableRowBg] = ImVec4(1, 1, 1, 0.01f);
  colors[ImGuiCol_TableRowBgAlt] = ImVec4(1, 1, 1, 0.03f);

  colors[ImGuiCol_TextSelectedBg] = ImVec4(accent.x, accent.y, accent.z, 0.22f);
  colors[ImGuiCol_NavHighlight] = ImVec4(accent.x, accent.y, accent.z, 0.44f);
  colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1, 1, 1, 0.20f);
  colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0, 0, 0, 0.20f);
  colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0, 0, 0, 0.36f);
}

void
GUI::classicWiiStyle() {
  ImGuiStyle& style = ImGui::GetStyle();
  ImVec4* colors = style.Colors;

  ImGui::StyleColorsLight();

  style.WindowRounding = 16.0f;
  style.ChildRounding = 14.0f;
  style.PopupRounding = 14.0f;
  style.FrameRounding = 24.0f;
  style.GrabRounding = 12.0f;
  style.ScrollbarRounding = 12.0f;
  style.TabRounding = 10.0f;

  style.WindowBorderSize = 1.0f;
  style.FrameBorderSize = 1.5f;
  style.PopupBorderSize = 1.0f;

  style.WindowPadding = ImVec2(16, 16);
  style.FramePadding = ImVec2(16, 6);
  style.ItemSpacing = ImVec2(10, 10);

  const ImVec4 textDark = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
  const ImVec4 wiiCyan = ImVec4(0.15f, 0.75f, 0.95f, 1.0f);
  const ImVec4 wiiBlueLight = ImVec4(0.85f, 0.93f, 0.98f, 1.0f);
  const ImVec4 bgLightGrey = ImVec4(0.86f, 0.86f, 0.86f, 1.0f);
  const ImVec4 btnGreyWhite = ImVec4(0.88f, 0.89f, 0.90f, 1.0f);

  colors[ImGuiCol_Text] = textDark;
  colors[ImGuiCol_TextDisabled] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);

  colors[ImGuiCol_WindowBg] = bgLightGrey;
  colors[ImGuiCol_ChildBg] = ImVec4(0.92f, 0.92f, 0.92f, 1.0f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.95f, 0.95f, 0.95f, 0.98f);
  colors[ImGuiCol_MenuBarBg] = bgLightGrey;

  colors[ImGuiCol_Border] = wiiCyan;
  colors[ImGuiCol_BorderShadow] = ImVec4(0.2f, 0.2f, 0.2f, 0.15f);

  colors[ImGuiCol_Button] = btnGreyWhite;
  colors[ImGuiCol_ButtonHovered] = wiiBlueLight;
  colors[ImGuiCol_ButtonActive] = ImVec4(0.60f, 0.85f, 0.98f, 1.0f);

  colors[ImGuiCol_FrameBg] = btnGreyWhite;
  colors[ImGuiCol_FrameBgHovered] = wiiBlueLight;
  colors[ImGuiCol_FrameBgActive] = wiiCyan;

  colors[ImGuiCol_Tab] = bgLightGrey;
  colors[ImGuiCol_TabHovered] = wiiBlueLight;
  colors[ImGuiCol_TabActive] = btnGreyWhite;
  colors[ImGuiCol_TabUnfocused] = bgLightGrey;
  colors[ImGuiCol_TabUnfocusedActive] = btnGreyWhite;

  colors[ImGuiCol_TitleBg] = bgLightGrey;
  colors[ImGuiCol_TitleBgActive] = bgLightGrey;
  colors[ImGuiCol_TitleBgCollapsed] = bgLightGrey;

  colors[ImGuiCol_Header] = wiiBlueLight;
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.75f, 0.88f, 0.98f, 1.0f);
  colors[ImGuiCol_HeaderActive] = wiiCyan;

  colors[ImGuiCol_CheckMark] = wiiCyan;
  colors[ImGuiCol_SliderGrab] = wiiCyan;
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.50f, 0.80f, 1.0f);
  colors[ImGuiCol_Separator] = ImVec4(0.70f, 0.70f, 0.70f, 0.50f);
  colors[ImGuiCol_SeparatorHovered] = wiiCyan;
  colors[ImGuiCol_NavHighlight] = wiiCyan;
}

// ==========================================================
// CONTROLS
// ==========================================================

void
GUI::vec3Control(const std::string& label,
  float* values,
  float resetValue,
  float columnWidth,
  bool displayAsDegrees) {
  ImGuiIO& io = ImGui::GetIO();
  auto boldFont = io.Fonts->Fonts[0];

  float displayValues[3] = { values[0], values[1], values[2] };
  if (displayAsDegrees) {
    displayValues[0] = RadToDeg(values[0]);
    displayValues[1] = RadToDeg(values[1]);
    displayValues[2] = RadToDeg(values[2]);
  }

  ImGui::PushID(label.c_str());

  ImGui::Columns(2);
  ImGui::SetColumnWidth(0, columnWidth);
  ImGui::Text("%s", label.c_str());
  ImGui::NextColumn();

  ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

  float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
  ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };
  float dragSpeed = displayAsDegrees ? 0.5f : 0.1f;
  const char* dragFormat = displayAsDegrees ? "%.1f" : "%.2f";

  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
  ImGui::PushFont(boldFont);
  if (ImGui::Button("X", buttonSize)) {
    values[0] = resetValue;
    displayValues[0] = displayAsDegrees ? RadToDeg(resetValue) : resetValue;
  }
  ImGui::PopFont();
  ImGui::PopStyleColor(3);

  ImGui::SameLine();
  if (ImGui::DragFloat("##X", &displayValues[0], dragSpeed, 0.0f, 0.0f, dragFormat)) {
    values[0] = displayAsDegrees ? DegToRad(displayValues[0]) : displayValues[0];
  }
  ImGui::PopItemWidth();
  ImGui::SameLine();

  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
  ImGui::PushFont(boldFont);
  if (ImGui::Button("Y", buttonSize)) {
    values[1] = resetValue;
    displayValues[1] = displayAsDegrees ? RadToDeg(resetValue) : resetValue;
  }
  ImGui::PopFont();
  ImGui::PopStyleColor(3);

  ImGui::SameLine();
  if (ImGui::DragFloat("##Y", &displayValues[1], dragSpeed, 0.0f, 0.0f, dragFormat)) {
    values[1] = displayAsDegrees ? DegToRad(displayValues[1]) : displayValues[1];
  }
  ImGui::PopItemWidth();
  ImGui::SameLine();

  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
  ImGui::PushFont(boldFont);
  if (ImGui::Button("Z", buttonSize)) {
    values[2] = resetValue;
    displayValues[2] = displayAsDegrees ? RadToDeg(resetValue) : resetValue;
  }
  ImGui::PopFont();
  ImGui::PopStyleColor(3);

  ImGui::SameLine();
  if (ImGui::DragFloat("##Z", &displayValues[2], dragSpeed, 0.0f, 0.0f, dragFormat)) {
    values[2] = displayAsDegrees ? DegToRad(displayValues[2]) : displayValues[2];
  }
  ImGui::PopItemWidth();

  ImGui::PopStyleVar();
  ImGui::Columns(1);
  ImGui::PopID();
}

void
GUI::vec3ControlSlider(const std::string& label,
  float* values,
  float min,
  float max,
  float resetValue,
  float columnWidth) {
  ImGuiIO& io = ImGui::GetIO();
  auto boldFont = io.Fonts->Fonts[0];

  ImGui::PushID(label.c_str());

  ImGui::Columns(2);
  ImGui::SetColumnWidth(0, columnWidth);
  ImGui::Text("%s", label.c_str());
  ImGui::NextColumn();

  ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

  float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
  ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
  ImGui::PushFont(boldFont);
  if (ImGui::Button("X", buttonSize)) values[0] = resetValue;
  ImGui::PopFont();
  ImGui::PopStyleColor(3);

  ImGui::SameLine();
  ImGui::SliderFloat("##X", &values[0], min, max, "%.2f");
  ImGui::PopItemWidth();
  ImGui::SameLine();

  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
  ImGui::PushFont(boldFont);
  if (ImGui::Button("Y", buttonSize)) values[1] = resetValue;
  ImGui::PopFont();
  ImGui::PopStyleColor(3);

  ImGui::SameLine();
  ImGui::SliderFloat("##Y", &values[1], min, max, "%.2f");
  ImGui::PopItemWidth();
  ImGui::SameLine();

  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
  ImGui::PushFont(boldFont);
  if (ImGui::Button("Z", buttonSize)) values[2] = resetValue;
  ImGui::PopFont();
  ImGui::PopStyleColor(3);

  ImGui::SameLine();
  ImGui::SliderFloat("##Z", &values[2], min, max, "%.2f");
  ImGui::PopItemWidth();

  ImGui::PopStyleVar();
  ImGui::Columns(1);
  ImGui::PopID();
}

void
GUI::debugLightsContainer(float* lightDir, float* lightColor) {
  if (ImGui::BeginTabBar("LightTabs")) {
    if (ImGui::BeginTabItem("Numeric")) {
      vec3Control("Light Dir", lightDir);
      vec3Control("Light Color", lightColor);
      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Sliders")) {
      vec3ControlSlider("Light Dir", lightDir, -1.0f, 1.0f, 0.0f);
      vec3ControlSlider("Light Color", lightColor, 0.0f, 1.0f, 1.0f);
      ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
  }
}

// ==========================================================
// BASIC UI
// ==========================================================

void
GUI::ToolBar() {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      ImGui::MenuItem("New");
      ImGui::MenuItem("Open");
      if (ImGui::MenuItem("Save", "Ctrl+S")) {
        m_requestSaveScene = true;
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Exit")) {
        show_exit_popup = true;
        ImGui::OpenPopup("Exit?");
      }
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit")) {
      ImGui::MenuItem("Undo");
      ImGui::MenuItem("Redo");
      ImGui::MenuItem("Cut");
      ImGui::MenuItem("Copy");
      ImGui::MenuItem("Paste");
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Tools")) {
      ImGui::MenuItem("Options");
      ImGui::MenuItem("Settings");
      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }
}

void
GUI::closeApp() {
  if (show_exit_popup) {
    ImGui::OpenPopup("Exit?");
    show_exit_popup = false;
  }

  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

  if (ImGui::BeginPopupModal("Exit?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Estas a punto de salir de la aplicacion.\nEstas seguro?\n\n");
    ImGui::Separator();

    if (ImGui::Button("OK", ImVec2(120, 0))) {
      exit(0);
      ImGui::CloseCurrentPopup();
    }
    ImGui::SetItemDefaultFocus();
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

// ==========================================================
// INSPECTOR
// ==========================================================

void
GUI::inspectorGeneral(EU::TSharedPointer<Actor> actor) {
  ImGui::Begin("Inspector");

  if (actor.isNull()) {
    ImGui::TextDisabled("No actor selected");
    ImGui::End();
    return;
  }

  static char objectName[128] = "";
  static std::string previousName;

  if (previousName != actor->getName()) {
    previousName = actor->getName();
    strcpy_s(objectName, sizeof(objectName), actor->getName().c_str());
  }

  if (ImGui::InputText("Name", objectName, sizeof(objectName))) {
    actor->setName(objectName);
  }

  auto transform = actor->getComponent<Transform>();
  if (!transform.isNull()) {
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
      inspectorContainer(actor);
    }
  }

  auto renderer = actor->getComponent<MeshRendererComponent>();
  if (!renderer.isNull()) {
    if (ImGui::CollapsingHeader("Mesh Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
      bool visible = renderer->isVisible();
      if (ImGui::Checkbox("Visible", &visible)) {
        renderer->setVisible(visible);
      }

      bool castShadow = renderer->canCastShadow();
      if (ImGui::Checkbox("Cast Shadows", &castShadow)) {
        renderer->setCastShadow(castShadow);
      }

      const std::vector<MaterialInstance*>& materialInstances = renderer->getMaterialInstances();
      for (size_t i = 0; i < materialInstances.size(); ++i) {
        MaterialInstance* materialInstance = materialInstances[i];
        if (!materialInstance) {
          continue;
        }

        std::string header = "Material Slot " + std::to_string(i);
        if (ImGui::TreeNode(header.c_str())) {
          Material* material = materialInstance->getMaterial();
          MaterialParams& params = materialInstance->getParams();

          if (material) {
            ImGui::Text("Domain: %s", GetMaterialDomainLabel(material->getDomain()));
            ImGui::Text("Blend: %s", GetBlendModeLabel(material->getBlendMode()));
          }

          ImGui::ColorEdit4(("Base Color##" + std::to_string(i)).c_str(), &params.baseColor.x);
          ImGui::SliderFloat(("Metallic##" + std::to_string(i)).c_str(), &params.metallic, 0.0f, 1.0f);
          ImGui::SliderFloat(("Roughness##" + std::to_string(i)).c_str(), &params.roughness, 0.0f, 1.0f);
          ImGui::SliderFloat(("AO##" + std::to_string(i)).c_str(), &params.ao, 0.0f, 1.0f);
          ImGui::SliderFloat(("Normal Scale##" + std::to_string(i)).c_str(), &params.normalScale, 0.0f, 2.0f);
          ImGui::SliderFloat(("Alpha Cutoff##" + std::to_string(i)).c_str(), &params.alphaCutoff, 0.0f, 1.0f);

          ImGui::TreePop();
        }
      }
    }
  }

  auto light = actor->getComponent<LightComponent>();
  if (!light.isNull()) {
    if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen)) {
      LightData& data = light->getLightData();

      ImGui::Text("Type: %s", GetLightTypeLabel(data.type));
      ImGui::ColorEdit3("Color", &data.color.x);
      ImGui::SliderFloat("Intensity", &data.intensity, 0.0f, 10.0f);

      if (data.type == LightType::Directional || data.type == LightType::Spot) {
        ImGui::SliderFloat3("Direction", &data.direction.x, -1.0f, 1.0f);
      }

      if (data.type == LightType::Point || data.type == LightType::Spot) {
        ImGui::SliderFloat("Range", &data.range, 0.0f, 100.0f);
      }

      bool castShadow = light->canCastShadow();
      if (ImGui::Checkbox("Cast Shadow", &castShadow)) {
        light->setCastShadow(castShadow);
      }
    }
  }

  ImGui::End();
}

void
GUI::inspectorContainer(EU::TSharedPointer<Actor> actor) {
  auto transform = actor->getComponent<Transform>();
  if (transform.isNull()) return;

  EU::Vector3 pos = transform->getPosition();
  EU::Vector3 rot = transform->getRotation();
  EU::Vector3 sca = transform->getScale();

  float posArr[3] = { pos.x, pos.y, pos.z };
  float rotArr[3] = { rot.x, rot.y, rot.z };
  float scaArr[3] = { sca.x, sca.y, sca.z };

  if (ImGui::BeginTabBar("TransformTabs")) {
    if (ImGui::BeginTabItem("Numeric")) {
      vec3Control("Position", posArr, 0.0f);
      vec3Control("Rotation", rotArr, 0.0f, 100.0f, true);
      vec3Control("Scale", scaArr, 1.0f);
      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Sliders")) {
      vec3ControlSlider("Position", posArr, -50.0f, 50.0f, 0.0f);
      vec3ControlSlider("Rotation", rotArr, -360.0f, 360.0f, 0.0f);
      vec3ControlSlider("Scale", scaArr, 0.0f, 10.0f, 1.0f);
      ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
  }

  if (pos.x != posArr[0] || pos.y != posArr[1] || pos.z != posArr[2]) {
    transform->setPosition(EU::Vector3(posArr[0], posArr[1], posArr[2]));
  }

  if (rot.x != rotArr[0] || rot.y != rotArr[1] || rot.z != rotArr[2]) {
    transform->setRotation(EU::Vector3(rotArr[0], rotArr[1], rotArr[2]));
  }

  if (sca.x != scaArr[0] || sca.y != scaArr[1] || sca.z != scaArr[2]) {
    transform->setScale(EU::Vector3(scaArr[0], scaArr[1], scaArr[2]));
  }
}

// ==========================================================
// OUTLINER
// ==========================================================

void
GUI::outliner(const std::vector<EU::TSharedPointer<Actor>>& actors) {
  ImGui::Begin("Hierarchy");

  static ImGuiTextFilter filter;
  filter.Draw("Search...", 180.0f);

  ImGui::Separator();

  for (int i = 0; i < static_cast<int>(actors.size()); ++i) {
    const auto& actor = actors[i];
    std::string actorName = actor ? actor->getName() : "Actor";

    if (!filter.PassFilter(actorName.c_str())) {
      continue;
    }

    bool selected = selectedActorIndex == i;
    if (ImGui::Selectable(actorName.c_str(), selected)) {
      selectedActorIndex = i;
    }
  }

  ImGui::End();
}

// ==========================================================
// GIZMOS
// ==========================================================

void
GUI::editTransform(Camera& cam, Window& window, EU::TSharedPointer<Actor> actor)
{
  if (actor.isNull()) return;

  auto transform = actor->getComponent<Transform>();
  if (transform.isNull()) return;

  float rectX = m_viewportPos.x;
  float rectY = m_viewportPos.y;
  float rectW = m_viewportSize.x;
  float rectH = m_viewportSize.y;

  if (rectW < 64.0f || rectH < 64.0f)
  {
    m_isUsingGizmo = false;
    return;
  }

  float position[3] = {
    transform->getPosition().x,
    transform->getPosition().y,
    transform->getPosition().z
  };

  float rotation[3] = {
    RadToDeg(transform->getRotation().x),
    RadToDeg(transform->getRotation().y),
    RadToDeg(transform->getRotation().z)
  };

  float scale[3] = {
    transform->getScale().x,
    transform->getScale().y,
    transform->getScale().z
  };

  float matrix[16];
  ImGuizmo::RecomposeMatrixFromComponents(position, rotation, scale, matrix);

  float view[16];
  float proj[16];
  ToFloatArray(cam.getView(), view);
  ToFloatArray(cam.getProj(), proj);

  ImGuizmo::SetOrthographic(false);

  if (m_viewportDrawList)
    ImGuizmo::SetDrawlist(m_viewportDrawList);
  else
    ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

  ImGuizmo::SetRect(rectX, rectY, rectW, rectH);

  bool canManipulate = m_viewportHovered || m_viewportActive || m_isUsingGizmo;
  if (canManipulate)
  {
    ImGuizmo::Manipulate(
      view,
      proj,
      mCurrentGizmoOperation,
      mCurrentGizmoMode,
      matrix
    );
  }

  m_isUsingGizmo = ImGuizmo::IsUsing();

  if (m_isUsingGizmo)
  {
    float outPos[3];
    float outRot[3];
    float outScale[3];

    ImGuizmo::DecomposeMatrixToComponents(matrix, outPos, outRot, outScale);

    transform->setPosition(EU::Vector3(outPos[0], outPos[1], outPos[2]));
    transform->setRotation(EU::Vector3(
      DegToRad(outRot[0]),
      DegToRad(outRot[1]),
      DegToRad(outRot[2])
    ));
    transform->setScale(EU::Vector3(outScale[0], outScale[1], outScale[2]));
  }
}

void
GUI::drawGizmoToolbar()
{
  ImGui::SetNextWindowBgAlpha(0.0f);

  ImGuiWindowFlags window_flags =
    ImGuiWindowFlags_NoDecoration |
    ImGuiWindowFlags_AlwaysAutoResize |
    ImGuiWindowFlags_NoFocusOnAppearing |
    ImGuiWindowFlags_NoNav;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

  if (ImGui::Begin("GizmoToolBar", nullptr, window_flags))
  {
    auto buttonMode = [&](const char* label, ImGuizmo::OPERATION op, const char* shortcut)
      {
        bool isActive = (mCurrentGizmoOperation == op);
        if (isActive)
          ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.8f, 1.0f));

        if (ImGui::Button(label))
          mCurrentGizmoOperation = op;

        if (ImGui::IsItemHovered())
          ImGui::SetTooltip("%s (%s)", label, shortcut);

        if (isActive)
          ImGui::PopStyleColor();

        ImGui::SameLine();
      };

    buttonMode("T", ImGuizmo::TRANSLATE, "W");
    buttonMode("R", ImGuizmo::ROTATE, "E");
    buttonMode("S", ImGuizmo::SCALE, "R");

    if (ImGui::Button(mCurrentGizmoMode == ImGuizmo::WORLD ? "Global" : "Local"))
      mCurrentGizmoMode = (mCurrentGizmoMode == ImGuizmo::WORLD) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
  }

  ImGui::End();
  ImGui::PopStyleVar();
}

// ==========================================================
// LAYOUT
// ==========================================================

void
GUI::drawStudioTopRibbon()
{
  ImGuiViewport* viewport = ImGui::GetMainViewport();

  const float menuBarHeight = 24.0f;
  const float ribbonHeight = 72.0f;

  ImGui::SetNextWindowPos(viewport->Pos, ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, menuBarHeight), ImGuiCond_Always);

  ImGuiWindowFlags menuFlags =
    ImGuiWindowFlags_NoDecoration |
    ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoSavedSettings |
    ImGuiWindowFlags_NoScrollWithMouse |
    ImGuiWindowFlags_NoScrollbar |
    ImGuiWindowFlags_MenuBar;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 4.0f));
  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.12f, 0.08f, 0.78f));

  if (ImGui::Begin("##StudioMenuBar", nullptr, menuFlags))
  {
    if (ImGui::BeginMenuBar())
    {
      if (ImGui::BeginMenu("File")) {
        ImGui::MenuItem("New Scene");
        ImGui::MenuItem("Open Scene");
        if (ImGui::MenuItem("Save", "Ctrl+S")) {
          m_requestSaveScene = true;
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Exit")) {
          show_exit_popup = true;
        }
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Edit")) {
        ImGui::MenuItem("Undo");
        ImGui::MenuItem("Redo");
        ImGui::Separator();
        ImGui::MenuItem("Copy");
        ImGui::MenuItem("Paste");
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("View")) {
        ImGui::MenuItem("Hierarchy");
        ImGui::MenuItem("Inspector");
        ImGui::MenuItem("Viewport");
        ImGui::EndMenu();
      }

      ImGui::EndMenuBar();
    }
  }
  ImGui::End();

  ImGui::PopStyleColor();
  ImGui::PopStyleVar(2);

  ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarHeight), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, ribbonHeight), ImGuiCond_Always);

  ImGuiWindowFlags ribbonFlags =
    ImGuiWindowFlags_NoDecoration |
    ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoSavedSettings |
    ImGuiWindowFlags_NoScrollWithMouse |
    ImGuiWindowFlags_NoScrollbar;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 6.0f));
  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.14f, 0.10f, 0.82f));
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.22f, 0.15f, 0.62f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.14f, 0.30f, 0.20f, 0.82f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.18f, 0.38f, 0.24f, 0.96f));

  if (ImGui::Begin("##StudioRibbon", nullptr, ribbonFlags))
  {
    auto ribbonButton = [&](const char* id, const char* topText, const char* bottomText, ImVec2 size, bool active = false) -> bool
      {
        if (active)
          ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.24f, 0.34f, 0.58f, 1.0f));

        bool pressed = ImGui::Button(id, size);

        ImVec2 min = ImGui::GetItemRectMin();
        ImVec2 max = ImGui::GetItemRectMax();
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        ImVec2 topSize = ImGui::CalcTextSize(topText);
        ImVec2 bottomSize = ImGui::CalcTextSize(bottomText);
        float centerX = (min.x + max.x) * 0.5f;

        drawList->AddText(
          ImVec2(centerX - topSize.x * 0.5f, min.y + 4.0f),
          ImGui::GetColorU32(ImGuiCol_Text),
          topText
        );

        drawList->AddText(
          ImVec2(centerX - bottomSize.x * 0.5f, min.y + 18.0f),
          ImGui::GetColorU32(ImGuiCol_TextDisabled),
          bottomText
        );

        if (active)
          ImGui::PopStyleColor();

        return pressed;
      };

    auto separatorGroup = [&]()
      {
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(6.0f, 1.0f));
        ImGui::SameLine();

        ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList* draw = ImGui::GetWindowDrawList();
        draw->AddLine(
          ImVec2(p.x, p.y),
          ImVec2(p.x, p.y + 48.0f),
          IM_COL32(80, 80, 90, 255),
          1.0f
        );

        ImGui::Dummy(ImVec2(8.0f, 48.0f));
        ImGui::SameLine();
      };

    const ImVec2 btnSize(110.0f, 36.0f);

    if (ribbonButton("##Move", "Move", "W", btnSize, mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
      mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    ImGui::SameLine();

    if (ribbonButton("##Rotate", "Rotate", "E", btnSize, mCurrentGizmoOperation == ImGuizmo::ROTATE))
      mCurrentGizmoOperation = ImGuizmo::ROTATE;
    ImGui::SameLine();

    if (ribbonButton("##Scale", "Scale", "R", btnSize, mCurrentGizmoOperation == ImGuizmo::SCALE))
      mCurrentGizmoOperation = ImGuizmo::SCALE;

    separatorGroup();

    ribbonButton("##Explorer", "Explorer", "Panel", btnSize, false);
    ImGui::SameLine();
    ribbonButton("##Properties", "Properties", "Panel", btnSize, false);
    ImGui::SameLine();
    ribbonButton("##Toolbox", "Toolbox", "Assets", btnSize, false);
  }

  ImGui::End();

  ImGui::PopStyleColor(4);
  ImGui::PopStyleVar(3);
}

void
GUI::drawViewportPanel(ID3D11ShaderResourceView* viewportSRV)
{
  ImGuiWindowFlags flags =
    ImGuiWindowFlags_NoScrollbar |
    ImGuiWindowFlags_NoScrollWithMouse |
    ImGuiWindowFlags_NoCollapse;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

  if (ImGui::Begin("Viewport", nullptr, flags))
  {
    m_viewportDrawList = ImGui::GetWindowDrawList();

    ImVec2 panelMin = ImGui::GetCursorScreenPos();
    ImVec2 panelSize = ImGui::GetContentRegionAvail();

    if (panelSize.x < 1.0f) panelSize.x = 1.0f;
    if (panelSize.y < 1.0f) panelSize.y = 1.0f;

    if (viewportSRV)
    {
      ImGui::Image((ImTextureID)viewportSRV, panelSize);
    }
    else
    {
      ImGui::InvisibleButton("##ViewportSurface", panelSize);
      ImVec2 itemMin = ImGui::GetItemRectMin();
      ImVec2 itemMax = ImGui::GetItemRectMax();
      ImDrawList* drawList = ImGui::GetWindowDrawList();

      drawList->AddRectFilled(itemMin, itemMax, IM_COL32(20, 20, 25, 255));
      drawList->AddText(
        ImVec2(itemMin.x + 12.0f, itemMin.y + 12.0f),
        IM_COL32(220, 220, 220, 255),
        "Viewport sin textura"
      );
    }

    ImVec2 itemMin = ImGui::GetItemRectMin();
    ImVec2 itemMax = ImGui::GetItemRectMax();
    m_viewportPos = itemMin;
    m_viewportSize = ImVec2(itemMax.x - itemMin.x, itemMax.y - itemMin.y);

    m_viewportHovered = ImGui::IsItemHovered();
    m_viewportActive = ImGui::IsItemActive();
    m_viewportFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
  }

  ImGui::End();
  ImGui::PopStyleVar();
}

void
GUI::drawRenderDebugPanel(ID3D11ShaderResourceView* preShadowSRV,
  ID3D11ShaderResourceView* finalViewportSRV,
  ID3D11ShaderResourceView* shadowMapSRV)
{
  ImGui::Begin("Render Debug");

  ImGui::Text("Final Viewport");
  if (finalViewportSRV) {
    ImGui::Image((ImTextureID)(intptr_t)finalViewportSRV, ImVec2(320, 180));
  }

  ImGui::Separator();

  ImGui::Text("Shadow Map");
  if (shadowMapSRV) {
    ImGui::Image((ImTextureID)(intptr_t)shadowMapSRV, ImVec2(320, 180));
  }

  ImGui::Separator();

  ImGui::Text("Pre Shadow");
  if (preShadowSRV) {
    ImGui::Image((ImTextureID)(intptr_t)preShadowSRV, ImVec2(320, 180));
  }

  ImGui::End();
}

void
GUI::drawEditorDockspace()
{
  ImGuiViewport* mainViewport = ImGui::GetMainViewport();

  const float topOffset = 96.0f;
  ImVec2 dockPos = ImVec2(mainViewport->Pos.x, mainViewport->Pos.y + topOffset);
  ImVec2 dockSize = ImVec2(mainViewport->Size.x, mainViewport->Size.y - topOffset);

  ImGuiWindowFlags window_flags =
    ImGuiWindowFlags_NoTitleBar |
    ImGuiWindowFlags_NoCollapse |
    ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoBringToFrontOnFocus |
    ImGuiWindowFlags_NoNavFocus |
    ImGuiWindowFlags_NoBackground |
    ImGuiWindowFlags_NoDecoration |
    ImGuiWindowFlags_NoSavedSettings |
    ImGuiWindowFlags_MenuBar;

  ImGui::SetNextWindowPos(dockPos, ImGuiCond_Always);
  ImGui::SetNextWindowSize(dockSize, ImGuiCond_Always);
  ImGui::SetNextWindowViewport(mainViewport->ID);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

  ImGui::Begin("##MainEditorDockspace", nullptr, window_flags);

  ImGuiID dockspace_id = ImGui::GetID("##EditorDockspace");
  ImGuiDockNodeFlags dockspace_flags =
    ImGuiDockNodeFlags_None |
    ImGuiDockNodeFlags_PassthruCentralNode;

  ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

  ImGui::End();

  ImGui::PopStyleVar(3);
}