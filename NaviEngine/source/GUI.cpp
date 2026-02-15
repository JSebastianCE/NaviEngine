#include "GUI.h"
#include "Window.h"
#include "Device.h"
#include "DeviceContext.h"
#include "MeshComponent.h"
#include "ECS\Actor.h"

// Inicializar variables estáticas para el Gizmo
static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);

void
GUI::awake() {
  // Lógica inicial si se requiere
}

// Declaración adelantada del estilo (la definimos abajo)
void
appleLiquidStyle(float opacity, ImVec4 accent);

void 
GUI::init(void* window, ID3D11Device* device, ID3D11DeviceContext* deviceContext) {
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  ImGuiStyle& style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  // APLICAR ESTILO DEL PROFESOR (Apple Liquid)
  appleLiquidStyle(0.85f, ImVec4(0.0f, 0.515f, 1.0f, 1.0f));

  // Setup Platform/Renderer backends
  ImGui_ImplWin32_Init((HWND)window);
  ImGui_ImplDX11_Init(device, deviceContext);
}

void
GUI::update() {
  // Start the Dear ImGui frame
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  // ImGuizmo Start
  ImGuizmo::BeginFrame();
  ImGuizmo::SetOrthographic(false);
  ImGuiIO& io = ImGui::GetIO();
  ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

  // Renderizar elementos fijos
  ToolBar();
  closeApp();
  drawGizmoToolbar();
}

void
GUI::render() {
  ImGui::Render();
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

  ImGuiIO& io = ImGui::GetIO();
  // Update and Render additional Platform Windows
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

void
GUI::ToolBar() {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Exit", "Alt+F4")) {
        show_exit_popup = true;
      }
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
    }
    ImGui::SetItemDefaultFocus();
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

void 
GUI::vec3Control(const std::string& label, float* values, float resetValues, float columnWidth) {
  ImGui::PushID(label.c_str());

  ImGui::Columns(2);
  ImGui::SetColumnWidth(0, columnWidth);
  ImGui::Text(label.c_str());
  ImGui::NextColumn();

  ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

  float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
  ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

  // EJE X
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
  if (ImGui::Button("X", buttonSize)) values[0] = resetValues;
  ImGui::PopStyleColor(3);

  ImGui::SameLine();
  ImGui::DragFloat("##X", &values[0], 0.1f, 0.0f, 0.0f, "%.2f");
  ImGui::PopItemWidth();
  ImGui::SameLine();

  // EJE Y
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
  if (ImGui::Button("Y", buttonSize)) values[1] = resetValues;
  ImGui::PopStyleColor(3);

  ImGui::SameLine();
  ImGui::DragFloat("##Y", &values[1], 0.1f, 0.0f, 0.0f, "%.2f");
  ImGui::PopItemWidth();
  ImGui::SameLine();

  // EJE Z
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
  if (ImGui::Button("Z", buttonSize)) values[2] = resetValues;
  ImGui::PopStyleColor(3);

  ImGui::SameLine();
  ImGui::DragFloat("##Z", &values[2], 0.1f, 0.0f, 0.0f, "%.2f");
  ImGui::PopItemWidth();

  ImGui::PopStyleVar();
  ImGui::Columns(1);

  ImGui::PopID();
}

void 
GUI::outliner(const std::vector<EU::TSharedPointer<Actor>>& actors) {
  ImGui::Begin("Hierarchy");

  static ImGuiTextFilter filter;
  filter.Draw("Search...", ImGui::GetContentRegionAvail().x);
  ImGui::Separator();

  for (int i = 0; i < actors.size(); ++i) {
    const auto& actor = actors[i];
    if (actor.isNull()) continue;

    std::string actorName = actor->getName();
    if (actorName.empty()) actorName = "Actor " + std::to_string(i);

    if (!filter.PassFilter(actorName.c_str())) continue;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (selectedActorIndex == i)
      flags |= ImGuiTreeNodeFlags_Selected;

    bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)i, flags, "%s", actorName.c_str());

    if (ImGui::IsItemClicked()) {
      selectedActorIndex = i;
    }

    if (nodeOpen) {
      ImGui::TreePop();
    }
  }

  ImGui::End();
}

void 
GUI::inspectorGeneral(EU::TSharedPointer<Actor> actor) {
  ImGui::Begin("Inspector");

  if (!actor.isNull()) {
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    std::string name = actor->getName();
    strcpy_s(buffer, name.c_str());

    if (ImGui::InputText("##ObjectName", buffer, sizeof(buffer))) {
      actor->setName(std::string(buffer));
    }

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
      inspectorContainer(actor);
    }
  }
  else {
    ImGui::Text("No actor selected.");
  }
  ImGui::End();
}

void GUI::inspectorContainer(EU::TSharedPointer<Actor> actor) {
  auto transform = actor->getComponent<Transform>();
  if (!transform) return;

  EU::Vector3 pos = transform->getPosition();
  EU::Vector3 rot = transform->getRotation();
  EU::Vector3 sca = transform->getScale();

  float p[3] = { pos.x, pos.y, pos.z };
  float r[3] = { rot.x, rot.y, rot.z };
  float s[3] = { sca.x, sca.y, sca.z };

  vec3Control("Position", p);
  vec3Control("Rotation", r);
  vec3Control("Scale", s, 1.0f);

  // 1. Detectar si el usuario modificó los números a mano en la UI
  bool changed = (
      p[0] != pos.x || p[1] != pos.y || p[2] != pos.z ||
      r[0] != rot.x || r[1] != rot.y || r[2] != rot.z ||
      s[0] != sca.x || s[1] != sca.y || s[2] != sca.z
  );

  if (changed) {
      transform->setPosition(EU::Vector3(p[0], p[1], p[2]));
      transform->setRotation(EU::Vector3(r[0], r[1], r[2]));
      transform->setScale(EU::Vector3(s[0], s[1], s[2]));

      // 2. Si cambiaste números manualmente, construimos la matriz usando la misma 
      // matemática de ImGuizmo para que no pelee con el Gizmo visual.
      float newMatrix[16];
      ImGuizmo::RecomposeMatrixFromComponents(p, r, s, newMatrix);
      XMFLOAT4X4 newF(newMatrix); // Cargamos el arreglo a formato DirectX
      transform->matrix = XMLoadFloat4x4(&newF);
  }
}

void GUI::editTransform(const XMMATRIX& view, const XMMATRIX& projection, EU::TSharedPointer<Actor> actor) {
  if (actor.isNull()) return;
  auto transform = actor->getComponent<Transform>();
  if (!transform) return;

  // 1. Usar la matriz DIRECTAMENTE del Transform. 
  // Nada de recomponer con ángulos de Euler al inicio.
  XMFLOAT4X4 modelF, viewF, projF;
  XMStoreFloat4x4(&modelF, transform->matrix);
  XMStoreFloat4x4(&viewF, view);
  XMStoreFloat4x4(&projF, projection);

  // 2. Configurar Snap
  ImGuiIO& io = ImGui::GetIO();
  float snapValue = (mCurrentGizmoOperation == ImGuizmo::ROTATE) ? 5.0f : 0.5f;
  float snap[3] = { snapValue, snapValue, snapValue };
  bool useSnap = io.KeyCtrl;

  // 3. Dibujar el Gizmo
  ImGuizmo::SetID(0);
  ImGuizmo::Manipulate(
    &viewF.m[0][0],
    &projF.m[0][0],
    mCurrentGizmoOperation,
    mCurrentGizmoMode,
    &modelF.m[0][0],
    nullptr,
    useSnap ? snap : nullptr
  );

  // 4. Si el usuario está moviendo la flecha en pantalla
  if (ImGuizmo::IsUsing()) {
    // A) Guardar la matriz modificada DIRECTAMENTE al Transform.
    // Esto evita que DirectX reinterprete los giros mal.
    transform->matrix = XMLoadFloat4x4(&modelF);

    // B) Extraer los valores SOLO para que los números en tu panel Gris se actualicen.
    float newP[3], newR[3], newS[3];
    ImGuizmo::DecomposeMatrixToComponents(&modelF.m[0][0], newP, newR, newS);
    transform->setPosition(EU::Vector3(newP[0], newP[1], newP[2]));
    transform->setRotation(EU::Vector3(newR[0], newR[1], newR[2]));
    transform->setScale(EU::Vector3(newS[0], newS[1], newS[2]));
  }
}




// --------------------------------------------------------------------------------------
// TOOLBAR
// --------------------------------------------------------------------------------------
void 
GUI::drawGizmoToolbar() {
  ImGuiIO& io = ImGui::GetIO();

  // Configuración de la ventana flotante
  ImGui::SetNextWindowPos(ImVec2(10, 50), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowBgAlpha(0.35f);

  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
    ImGuiWindowFlags_AlwaysAutoResize |
    ImGuiWindowFlags_NoSavedSettings |
    ImGuiWindowFlags_NoFocusOnAppearing |
    ImGuiWindowFlags_NoNav;

  if (ImGui::Begin("GizmoTools", nullptr, window_flags)) {

    // Helper Lambda para botones bonitos
    auto buttonMode = [&](const char* label, ImGuizmo::OPERATION op) {
      bool isActive = (mCurrentGizmoOperation == op);
      if (isActive) {
        // Azul si está activo
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.8f, 1.0f));
      }
      if (ImGui::Button(label)) {
        mCurrentGizmoOperation = op;
      }
      if (isActive) ImGui::PopStyleColor();
      ImGui::SameLine();
      };

    buttonMode("T", ImGuizmo::TRANSLATE);
    buttonMode("R", ImGuizmo::ROTATE);
    buttonMode("S", ImGuizmo::SCALE);

    ImGui::Separator();

    if (ImGui::Button(mCurrentGizmoMode == ImGuizmo::WORLD ? "Global" : "Local")) {
      mCurrentGizmoMode = (mCurrentGizmoMode == ImGuizmo::WORLD) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
    }
  }
  ImGui::End();
}

// --------------------------------------------------------------------------------------
// DEFINICIÓN DE ESTILO (COPIADO Y ADAPTADO DEL PROFESOR)
// --------------------------------------------------------------------------------------
void 
appleLiquidStyle(float opacity, ImVec4 accent) {
  ImGuiStyle& style = ImGui::GetStyle();
  ImVec4* colors = style.Colors;

  style.WindowRounding = 14.0f;
  style.ChildRounding = 14.0f;
  style.PopupRounding = 14.0f;
  style.FrameRounding = 10.0f;
  style.GrabRounding = 10.0f;
  style.ScrollbarRounding = 12.0f;
  style.TabRounding = 10.0f;

  const ImVec4 pane = ImVec4(0.16f, 0.16f, 0.18f, opacity);
  const ImVec4 paneHi = ImVec4(0.20f, 0.20f, 0.22f, opacity);
  const ImVec4 paneLo = ImVec4(0.13f, 0.13f, 0.15f, opacity * 0.85f);

  colors[ImGuiCol_WindowBg] = pane;
  colors[ImGuiCol_ChildBg] = paneLo;
  colors[ImGuiCol_PopupBg] = paneHi;

  colors[ImGuiCol_FrameBg] = paneLo;
  colors[ImGuiCol_FrameBgHovered] = pane;
  colors[ImGuiCol_FrameBgActive] = paneHi;

  colors[ImGuiCol_TitleBg] = pane;
  colors[ImGuiCol_TitleBgActive] = paneHi;

  colors[ImGuiCol_Button] = paneLo;
  colors[ImGuiCol_ButtonHovered] = pane;
  colors[ImGuiCol_ButtonActive] = paneHi;

  colors[ImGuiCol_Header] = paneLo;
  colors[ImGuiCol_HeaderHovered] = pane;
  colors[ImGuiCol_HeaderActive] = paneHi;

  // Acento de color
  colors[ImGuiCol_CheckMark] = accent;
  colors[ImGuiCol_SliderGrab] = accent;
  colors[ImGuiCol_SliderGrabActive] = ImVec4(accent.x, accent.y, accent.z, 1.0f);
  colors[ImGuiCol_TextSelectedBg] = ImVec4(accent.x, accent.y, accent.z, 0.35f);
}