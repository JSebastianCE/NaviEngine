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

  bool GetSRVSize(ID3D11ShaderResourceView* srv, float& outW, float& outH) {
    outW = outH = 0.0f;
    if (!srv) return false;

    ID3D11Resource* resource = nullptr;
    srv->GetResource(&resource);
    if (!resource) return false;

    ID3D11Texture2D* texture = nullptr;
    HRESULT hr = resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&texture));
    bool ok = false;
    if (SUCCEEDED(hr) && texture) {
      D3D11_TEXTURE2D_DESC desc;
      texture->GetDesc(&desc);
      outW = static_cast<float>(desc.Width);
      outH = static_cast<float>(desc.Height);
      ok = (outW > 0.0f && outH > 0.0f);
      texture->Release();
    }
    resource->Release();
    return ok;
  }

  // Dibuja 'srv' dentro de 'region' conservando su aspect ratio (centrado, con margen).
  void ImageAspectFit(ID3D11ShaderResourceView* srv, ImVec2 region) {
    if (region.x < 1.0f) region.x = 1.0f;
    if (region.y < 1.0f) region.y = 1.0f;

    if (!srv) {
      ImGui::Dummy(region);
      return;
    }

    float texW = 0.0f, texH = 0.0f;
    if (!GetSRVSize(srv, texW, texH)) {
      ImGui::Image((ImTextureID)srv, region); // fallback
      return;
    }

    const float texAspect = texW / texH;
    const float regionAspect = region.x / region.y;

    ImVec2 drawSize = region;
    if (regionAspect > texAspect) {
      drawSize.y = region.y;
      drawSize.x = region.y * texAspect;
    }
    else {
      drawSize.x = region.x;
      drawSize.y = region.x / texAspect;
    }

    const ImVec2 cursor = ImGui::GetCursorPos();
    const ImVec2 offset((region.x - drawSize.x) * 0.5f, (region.y - drawSize.y) * 0.5f);
    ImGui::SetCursorPos(ImVec2(cursor.x + offset.x, cursor.y + offset.y));
    ImGui::Image((ImTextureID)srv, drawSize);
  }

  // Llena TODA la region conservando proporciones (recorta el sobrante con UVs).
  void ImageAspectFill(ID3D11ShaderResourceView* srv, ImVec2 region) {
    if (region.x < 1.0f) region.x = 1.0f;
    if (region.y < 1.0f) region.y = 1.0f;

    if (!srv) {
      ImGui::Dummy(region);
      return;
    }

    float texW = 0.0f, texH = 0.0f;
    if (!GetSRVSize(srv, texW, texH)) {
      ImGui::Image((ImTextureID)srv, region); // fallback
      return;
    }

    const float texAspect = texW / texH;
    const float regionAspect = region.x / region.y;

    ImVec2 uv0(0.0f, 0.0f);
    ImVec2 uv1(1.0f, 1.0f);

    if (regionAspect > texAspect) {
      // La caja es mas ancha que la textura -> recorta arriba/abajo.
      const float visible = texAspect / regionAspect; // fraccion visible en Y
      const float off = (1.0f - visible) * 0.5f;
      uv0.y = off;
      uv1.y = 1.0f - off;
    }
    else {
      // La caja es mas alta que la textura -> recorta izquierda/derecha.
      const float visible = regionAspect / texAspect; // fraccion visible en X
      const float off = (1.0f - visible) * 0.5f;
      uv0.x = off;
      uv1.x = 1.0f - off;
    }

    ImGui::Image((ImTextureID)srv, region, uv0, uv1);
  }



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

  // Tema: fondo naranja translucido + interactuables verde claro translucido.
  // El "accent" es el verde de marcas (checkmark / slider).
  appleLiquidStyle(0.80f, ImVec4(0.50f, 0.88f, 0.45f, 1.0f));

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

  //drawStudioTopRibbon();
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

  // Texto calido (sobre fondo naranja)
  const ImVec4 textMain = ImVec4(1.00f, 0.97f, 0.92f, 0.96f);
  const ImVec4 textDisabled = ImVec4(0.92f, 0.82f, 0.70f, 0.78f);

  // Fondos NARANJA translucido
  const ImVec4 bgMain = ImVec4(0.85f, 0.38f, 0.08f, o);
  const ImVec4 bgChild = ImVec4(0.78f, 0.34f, 0.06f, o * 0.92f);
  const ImVec4 bgPopup = ImVec4(0.88f, 0.42f, 0.10f, o * 0.98f);

  // Interactuables VERDE claro translucido
  const ImVec4 frame = ImVec4(0.55f, 0.85f, 0.45f, 0.26f);
  const ImVec4 frameHover = ImVec4(0.62f, 0.92f, 0.52f, 0.44f);
  const ImVec4 frameActive = ImVec4(0.68f, 0.96f, 0.58f, 0.60f);

  const ImVec4 button = ImVec4(0.55f, 0.85f, 0.45f, 0.30f);
  const ImVec4 buttonHover = ImVec4(0.62f, 0.92f, 0.52f, 0.48f);
  const ImVec4 buttonActive = ImVec4(0.68f, 0.96f, 0.58f, 0.64f);

  const ImVec4 header = ImVec4(0.55f, 0.85f, 0.45f, 0.26f);
  const ImVec4 headerHover = ImVec4(0.62f, 0.92f, 0.52f, 0.44f);
  const ImVec4 headerActive = ImVec4(0.68f, 0.96f, 0.58f, 0.58f);

  const ImVec4 border = ImVec4(0.55f, 0.90f, 0.50f, 0.22f);
  const ImVec4 borderSoft = ImVec4(1.00f, 0.95f, 0.85f, 0.05f);

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

  colors[ImGuiCol_TitleBg] = ImVec4(0.70f, 0.30f, 0.05f, 0.90f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.38f, 0.08f, 0.98f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.60f, 0.26f, 0.04f, 0.75f);
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.80f, 0.36f, 0.07f, 0.92f);

  colors[ImGuiCol_Button] = button;
  colors[ImGuiCol_ButtonHovered] = buttonHover;
  colors[ImGuiCol_ButtonActive] = buttonActive;

  colors[ImGuiCol_Header] = header;
  colors[ImGuiCol_HeaderHovered] = headerHover;
  colors[ImGuiCol_HeaderActive] = headerActive;

  colors[ImGuiCol_Tab] = ImVec4(0.55f, 0.85f, 0.45f, 0.24f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.62f, 0.92f, 0.52f, 0.44f);
  colors[ImGuiCol_TabActive] = ImVec4(0.68f, 0.96f, 0.58f, 0.60f);
  colors[ImGuiCol_TabUnfocused] = ImVec4(0.70f, 0.32f, 0.06f, 0.55f);
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.78f, 0.36f, 0.07f, 0.72f);

  colors[ImGuiCol_CheckMark] = accent;
  colors[ImGuiCol_SliderGrab] = accent;
  colors[ImGuiCol_SliderGrabActive] = ImVec4(
    accent.x + 0.06f,
    accent.y + 0.06f,
    accent.z + 0.06f,
    1.0f
  );

  colors[ImGuiCol_Separator] = ImVec4(0.55f, 0.90f, 0.50f, 0.20f);
  colors[ImGuiCol_SeparatorHovered] = ImVec4(0.62f, 0.92f, 0.52f, 0.40f);
  colors[ImGuiCol_SeparatorActive] = ImVec4(0.68f, 0.96f, 0.58f, 0.55f);

  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.55f, 0.90f, 0.50f, 0.22f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.62f, 0.92f, 0.52f, 0.36f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.68f, 0.96f, 0.58f, 0.48f);

  colors[ImGuiCol_DockingPreview] = ImVec4(accent.x, accent.y, accent.z, 0.32f);
  colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.55f, 0.25f, 0.05f, 0.65f);

  colors[ImGuiCol_TableHeaderBg] = ImVec4(0.78f, 0.34f, 0.06f, 0.88f);
  colors[ImGuiCol_TableBorderStrong] = ImVec4(0.55f, 0.90f, 0.50f, 0.20f);
  colors[ImGuiCol_TableBorderLight] = ImVec4(0.55f, 0.90f, 0.50f, 0.10f);
  colors[ImGuiCol_TableRowBg] = ImVec4(1, 1, 1, 0.01f);
  colors[ImGuiCol_TableRowBgAlt] = ImVec4(1, 1, 1, 0.04f);

  colors[ImGuiCol_TextSelectedBg] = ImVec4(accent.x, accent.y, accent.z, 0.24f);
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
    ImGui::Text("Estas a punto de salir de la aplicacion.Estas seguro ?");
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

      static const char* kLightTypes[] = { "Directional", "Point", "Spot" };
      int currentType = static_cast<int>(data.type);
      if (currentType < 0 || currentType > 2) currentType = 0;
      if (ImGui::Combo("Type", &currentType, kLightTypes, IM_ARRAYSIZE(kLightTypes))) {
        data.type = static_cast<LightType>(currentType);
        if (data.type == LightType::Point && data.range <= 0.0f) {
          data.range = 12.0f;
        }
      }

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

void GUI::editTransform(Camera& cam, Window& window, EU::TSharedPointer<Actor> actor)
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

  float position[3] = { transform->getPosition().x, transform->getPosition().y, transform->getPosition().z };
  float rotation[3] = { RadToDeg(transform->getRotation().x), RadToDeg(transform->getRotation().y), RadToDeg(transform->getRotation().z) };
  float scale[3] = { transform->getScale().x, transform->getScale().y, transform->getScale().z };

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

  // Transformamos nuestro int a la operación de ImGuizmo
  ImGuizmo::OPERATION activeOperation = ImGuizmo::TRANSLATE;
  if (m_currentGizmoTool == 1) activeOperation = ImGuizmo::TRANSLATE;
  if (m_currentGizmoTool == 2) activeOperation = ImGuizmo::ROTATE;
  if (m_currentGizmoTool == 3) activeOperation = ImGuizmo::SCALE;

  // Si la herramienta es 0 (Selección), apagamos el Gizmo para que no estorbe
  bool canManipulate = (m_viewportHovered || m_viewportActive || m_isUsingGizmo) && (m_currentGizmoTool != 0);

  if (canManipulate)
  {
    ImGuizmo::Manipulate(view, proj, activeOperation, ImGuizmo::LOCAL, matrix); // Cambia LOCAL por mCurrentGizmoMode si lo tienes como variable estática
  }

  m_isUsingGizmo = ImGuizmo::IsUsing();

  if (m_isUsingGizmo)
  {
    float outPos[3], outRot[3], outScale[3];
    ImGuizmo::DecomposeMatrixToComponents(matrix, outPos, outRot, outScale);

    transform->setPosition(EU::Vector3(outPos[0], outPos[1], outPos[2]));
    transform->setRotation(EU::Vector3(DegToRad(outRot[0]), DegToRad(outRot[1]), DegToRad(outRot[2])));
    transform->setScale(EU::Vector3(outScale[0], outScale[1], outScale[2]));
  }
}

void GUI::drawGizmoToolbar()
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
    // Cambiamos el parámetro ImGuizmo::OPERATION por int toolId
    auto buttonMode = [&](const char* label, int toolId, const char* shortcut)
      {
        bool isActive = (m_currentGizmoTool == toolId);

        if (isActive) {
          ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.4f, 0.0f, 1.0f));
        }

        if (ImGui::Button(label, ImVec2(50, 30))) {
          m_currentGizmoTool = toolId;
        }

        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("%s (%s)", label, shortcut);
        }

        if (isActive) {
          ImGui::PopStyleColor();
        }
      };

    // Botones rediseñados y conectados a los números
    buttonMode("Sel", 0, "1");
    buttonMode("T", 1, "2");
    buttonMode("R", 2, "3");
    buttonMode("S", 3, "4");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Mantenemos el botón de global/local asumiendo que mCurrentGizmoMode está declarado arriba
    // Si mCurrentGizmoMode te marca error, dímelo y lo corregimos rápido
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
  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.80f, 0.36f, 0.07f, 0.85f));

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
  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.84f, 0.39f, 0.09f, 0.85f));
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.85f, 0.45f, 0.30f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.62f, 0.92f, 0.52f, 0.48f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.68f, 0.96f, 0.58f, 0.64f));

  if (ImGui::Begin("##StudioRibbon", nullptr, ribbonFlags))
  {
    auto ribbonButton = [&](const char* id, const char* topText, const char* bottomText, ImVec2 size, bool active = false) -> bool
      {
        if (active)
          ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.30f, 0.62f, 0.28f, 1.0f));

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
          IM_COL32(120, 200, 110, 255),
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

    if (ribbonButton("##CreateLight", "Light", "Create", btnSize, false))
      m_requestCreateLightActor = true;

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

  // OVERLAY DE ESTADÍSTICAS ESTILO UNREAL ---
  ImVec2 window_pos = ImGui::GetWindowPos();
  ImVec2 stats_pos = ImVec2(window_pos.x + 20.0f, window_pos.y + 40.0f); // Margen superior izquierdo

  ImGui::SetNextWindowPos(stats_pos, ImGuiCond_Always);
  ImGui::SetNextWindowBgAlpha(0.35f); // Fondo muy transparente

  ImGuiWindowFlags stats_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
    ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing |
    ImGuiWindowFlags_NoNav;

  if (ImGui::Begin("##ViewportStats", nullptr, stats_flags)) {
    float fps = ImGui::GetIO().Framerate;
    float ms = 1000.0f / fps;

    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.4f, 1.0f), "FPS: %.1f", fps);
    ImGui::Text("Frame Time: %.2f ms", ms);

    // Como no pasamos la lista de actores aquí, podemos dejar un contador visual genérico temporal:
    ImGui::TextDisabled("Engine DX11 Ready");
    ImGui::End();
  }

}

void
GUI::drawViewportLightIcons(const std::vector<EU::TSharedPointer<Actor>>& actors,
  Camera& camera,
  ID3D11ShaderResourceView* lightIconSRV)
{
  if (m_viewportSize.x <= 1.0f || m_viewportSize.y <= 1.0f) {
    return;
  }

  ImDrawList* drawList = ImGui::GetForegroundDrawList();
  const ImVec2 iconSize(40.0f, 40.0f); // mas grande para que se note

  for (int i = 0; i < static_cast<int>(actors.size()); ++i) {
    const auto& actor = actors[i];
    if (actor.isNull() || actor->getComponent<LightComponent>().isNull()) {
      continue;
    }

    auto transform = actor->getComponent<Transform>();
    if (transform.isNull()) {
      continue;
    }

    const EU::Vector3& position = transform->getPosition();
    XMVECTOR worldPos = XMVectorSet(position.x, position.y, position.z, 1.0f);

    XMVECTOR projected = XMVector3Project(
      worldPos,
      m_viewportPos.x, m_viewportPos.y,
      m_viewportSize.x, m_viewportSize.y,
      0.0f, 1.0f,
      camera.getProj(), camera.getView(), XMMatrixIdentity());

    const float sz = XMVectorGetZ(projected);
    if (sz < 0.0f || sz > 1.0f) continue; // detras de la camara

    ImVec2 center(XMVectorGetX(projected), XMVectorGetY(projected));

    // Marcador SIEMPRE visible (confirma posicion aunque no haya PNG)
    const bool selected = (selectedActorIndex == i);
    ImU32 ring = selected ? IM_COL32(170, 255, 150, 255) : IM_COL32(255, 230, 120, 255);
    drawList->AddCircleFilled(center, 6.0f, IM_COL32(0, 0, 0, 120), 16);
    drawList->AddCircle(center, iconSize.x * 0.5f, ring, 24, 2.0f);

    // PNG encima (si cargo). Tinte blanco para respetar sus colores.
    if (lightIconSRV) {
      ImU32 tint = selected ? IM_COL32(200, 255, 190, 255) : IM_COL32(255, 255, 255, 255);
      ImVec2 iconMin(center.x - iconSize.x * 0.5f, center.y - iconSize.y * 0.5f);
      ImVec2 iconMax(center.x + iconSize.x * 0.5f, center.y + iconSize.y * 0.5f);
      drawList->AddImage((ImTextureID)lightIconSRV, iconMin, iconMax,
        ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), tint);
    }

    // Etiqueta para ubicarla facil
    drawList->AddText(ImVec2(center.x + iconSize.x * 0.5f + 4.0f, center.y - 8.0f),
      IM_COL32(255, 255, 255, 230), actor->getName().c_str());
  }
}

void GUI::drawGBufferDebugPanel(ID3D11ShaderResourceView* albedoMetallicSRV,
  ID3D11ShaderResourceView* normalRoughnessSRV,
  ID3D11ShaderResourceView* worldAoSRV,
  ID3D11ShaderResourceView* emissiveAlphaSRV,
  EU::TSharedPointer<Actor> selectedActor)
{
  ImGui::Begin("GBuffer Debug");

  // ----------------------------------------------------------------------
  // 1. HERRAMIENTAS DE ESCENA
  // ----------------------------------------------------------------------
  ImGui::TextDisabled("Herramientas de Escena:");
  if (ImGui::Button("Create Light Actor", ImVec2(-1, 30))) {
    m_requestCreateLightActor = true;
  }
  ImGui::Checkbox("Ver Factor de Sombra", &m_visualizeDeferredShadowFactor);

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  // ---------------------------------------------------------
  // 2. CONFIGURACIÓN DE LOS 10 MODOS SEPARADOS
  // ---------------------------------------------------------
  // Estructura para organizar nuestros 10 botones de forma independiente
  struct DebugMode {
    const char* name;
    int engineShaderMode; // El ID que tu RenderPipeline usa para el Viewport
    ID3D11ShaderResourceView* rawSrv; // La textura fuente
  };

  DebugMode modes[10] = {
    // Columna Izquierda (Canales RGB principales)
    { "Final Render",  0, m_renderDebugFinalSRV },
    { "Albedo",        2, albedoMetallicSRV },
    { "Normal",        3, normalRoughnessSRV },
    { "World Pos",     6, worldAoSRV },
    { "Emissive",      8, emissiveAlphaSRV },

    // Columna Derecha (Canales Alfa y Sombras)
    { "Metallic",      5, albedoMetallicSRV },
    { "Roughness",     4, normalRoughnessSRV },
    { "AO",            7, worldAoSRV },
    { "Pre-Shadow",   -1, m_renderDebugPreShadowSRV },
    { "Shadow Map",   -2, m_renderDebugShadowMapSRV }
  };

  static int selectedIndex = 0; // Recuerda qué botón pulsamos

  // ---------------------------------------------------------
  // 3. PREVIEW PRINCIPAL (ARRIBA)
  // ---------------------------------------------------------
  ImGui::Text("Preview Principal:");

  // Texto verde que indica qué estamos viendo
  ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Mostrando: %s", modes[selectedIndex].name);

  ID3D11ShaderResourceView* mainSRV = modes[selectedIndex].rawSrv;

  // Fallback de seguridad: si Final Render no está listo, mostrar Albedo
  if (selectedIndex == 0 && !mainSRV) mainSRV = albedoMetallicSRV;

  if (mainSRV) {
    float w, h;
    if (GetSRVSize(mainSRV, w, h)) {
      float aspect = h / w;
      float drawW = ImGui::GetContentRegionAvail().x;
      float drawH = drawW * aspect;
      ImGui::Image((void*)mainSRV, ImVec2(drawW, drawH));
    }
  }
  else {
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Textura no disponible.");
  }

  if (modes[selectedIndex].engineShaderMode >= 0) {
    m_deferredDebugViewMode = modes[selectedIndex].engineShaderMode;
  }

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  // ---------------------------------------------------------
  // 4. LOS 10 BOTONES SEPARADOS EN 2 COLUMNAS (ABAJO)
  // ---------------------------------------------------------
  ImGui::TextDisabled("Render Targets (Selecciona para ver en Viewport):");
  ImGui::Spacing();

  if (ImGui::BeginTable("GBufferTable", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchSame)) {

    // Función lambda para dibujar cada celda limpia y sin crasheos
    auto DrawGridItem = [&](int index) {

      // 1. si ESTABA seleccionado ANTES de hacer clic
      bool wasSelected = (index == selectedIndex);

      if (wasSelected) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
      }

      // 2. Si hacemos clic, actualiza el índice global
      if (ImGui::Button(modes[index].name, ImVec2(ImGui::GetContentRegionAvail().x, 25))) {
        selectedIndex = index;
      }

      // 3. Uvariable guardada para hacer el Pop (¡esto evita el crasheo!)
      if (wasSelected) {
        ImGui::PopStyleColor();
      }

      // 4. Dibujar miniatura debajo del botón
      if (modes[index].rawSrv) {
        float w, h;
        if (GetSRVSize(modes[index].rawSrv, w, h)) {
          float aspect = h / w;
          float previewW = ImGui::GetContentRegionAvail().x;
          float previewH = previewW * aspect;
          ImGui::Image((void*)modes[index].rawSrv, ImVec2(previewW, previewH));
        }
      }
      else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "[ Vacio ]");
      }
    };

    // Dibuja las 5 filas (10 elementos en total)
    for (int row = 0; row < 5; ++row) {
      ImGui::TableNextRow();

      // Columna 1 (Índices del 0 al 4: Final, Albedo, Normal, World Pos, Emissive)
      ImGui::TableSetColumnIndex(0);
      DrawGridItem(row);

      // Columna 2 (Índices del 5 al 9: Metallic, Roughness, AO, Sombras)
      ImGui::TableSetColumnIndex(1);
      DrawGridItem(row + 5);
    }

    ImGui::EndTable();
  }

  ImGui::End();
}

void
GUI::drawEditorDockspace()
{
  ImGuiViewport* mainViewport = ImGui::GetMainViewport();

  const float topOffset = 0.0f;
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

void GUI::drawLogConsole() {
  ImGui::Begin("Log Console");

  if (ImGui::Button("Clear Console")) {
    EditorLog::Clear();
  }

  ImGui::Separator();

  // Zona de scroll para los textos
  ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

  for (const auto& log : EditorLog::s_Logs) {
    ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Blanco por defecto (Info)

    if (log.type == 1) color = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); // Amarillo (Warning)
    if (log.type == 2) color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); // Rojo (Error)

    ImGui::TextColored(color, "%s", log.text.c_str());
  }

  // Auto-scroll automático hacia abajo cuando entran nuevos logs
  if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
    ImGui::SetScrollHereY(1.0f);
  }

  ImGui::EndChild();
  ImGui::End();
}

void GUI::drawStatsPanel()
{
  // Crea la pestaña dedicada
  ImGui::Begin("Engine Statistics");

  // 1. OBTENCIÓN DE DATOS DE RENDIMIENTO
  float fps = ImGui::GetIO().Framerate;
  float ms = 1000.0f / (fps > 0.0f ? fps : 60.0f); // Evitamos división por cero

  // 2. LÓGICA DEL BÚFER CIRCULAR PARA EL GRÁFICO (Historial de 120 frames)
  const int kBufferSize = 120;
  static float fpsHistory[kBufferSize] = { 0.0f };
  static int currentOffset = 0;

  // Guarda el frame actual en el historial
  fpsHistory[currentOffset] = fps;
  currentOffset = (currentOffset + 1) % kBufferSize;

  // 3. SECCIÓN DE TEXTOS NUMÉRICOS
  ImGui::TextDisabled("Metricas de Rendimiento General:");
  ImGui::Spacing();

  //  los FPS en verde brillante
  ImGui::Text("Frames Per Second (FPS): ");
  ImGui::SameLine();
  ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.4f, 1.0f), "%.1f FPS", fps);

  // tiempo de ciclo de CPU/GPU
  ImGui::Text("Frame Time (Tiempo de Frame): ");
  ImGui::SameLine();
  ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "%.2f ms", ms);

  ImGui::Separator();
  ImGui::Spacing();

  // 4. CONSTRUCCIÓN DEL GRÁFICO FUNCIONAL
  ImGui::TextDisabled("Grafico de Estabilidad de FPS (Historial):");
  ImGui::Spacing();

  // Calcula el FPS máximo y mínimo del historial para escalar el gráfico correctamente
  float maxFps = 0.0f;
  float minFps = 60.0f;
  for (int i = 0; i < kBufferSize; ++i) {
    if (fpsHistory[i] > maxFps) maxFps = fpsHistory[i];
    if (fpsHistory[i] < minFps && fpsHistory[i] > 0.0f) minFps = fpsHistory[i];
  }

  // Si los límites son iguales, les damos un margen visual predeterminado
  if (maxFps == minFps) { maxFps += 10.0f; minFps -= 10.0f; }

  // Formateamos un texto flotante para mostrar el FPS actual dentro del gráfico
  char overlayText[32];
  sprintf_s(overlayText, "Actual: %.1f FPS", fps);

  float scaleMin = minFps - 1.5f;
  float scaleMax = maxFps + 1.5f;

  // Dibuja el gráfico de líneas dinámico
  // Parámetros: Etiqueta, puntero al arreglo, tamaño, desfase del búfer, texto superpuesto, escala mínima, escala máxima, tamaño del gráfico (ancho automático, alto 80px)
  ImGui::PlotLines("##FPSGraph", fpsHistory, kBufferSize, currentOffset, overlayText, scaleMin, scaleMax, ImVec2(-1, 150));

  ImGui::Spacing();
  ImGui::TextDisabled("Limites del Historial -> Min: %.1f FPS | Max: %.1f FPS", minFps, maxFps);

  ImGui::End();
}