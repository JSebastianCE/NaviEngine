#include "GUI.h"
#include "Viewport.h"
#include "Window.h"
#include "Device.h"
#include "DeviceContext.h"
#include "MeshComponent.h"
#include "ECS\Actor.h"
#include "ECS\Transform.h"
#include "EngineUtilities\Utilities\Camera.h"

//#include "imgui_internal.h"
static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
void
GUI::init(Window& window, Device& device, DeviceContext& deviceContext) {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	//appleLiquidStyle(0.72f, ImVec4(0.0f, 0.515f, 1.0f, 1.0f));

	classicWiiStyle();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(window.m_hWnd);
	ImGui_ImplDX11_Init(device.m_device, deviceContext.m_deviceContext);

	// Init ToolTips
	toolTipData();

	selectedActorIndex = 0;
}

void
GUI::update(Viewport& viewport, Window& window) {
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGuizmo::BeginFrame();
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetOrthographic(false);
	//ImGuizmo::SetRect(0, 0, (float)window.m_width, (float)window.m_height);

	// In Program always
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
	// Update and Render additional Platform Windows
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void
GUI::destroy() {
	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void
GUI::vec3Control(const std::string& label, float* values, float resetValue, float columnWidth) {
	ImGuiIO& io = ImGui::GetIO();
	auto boldFont = io.Fonts->Fonts[0];

	ImGui::PushID(label.c_str());

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::Text(label.c_str());
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
	ImGui::DragFloat("##X", &values[0], 0.1f, 0.0f, 0.0f, "%.2f");
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
	ImGui::DragFloat("##Y", &values[1], 0.1f, 0.0f, 0.0f, "%.2f");
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
	ImGui::DragFloat("##Z", &values[2], 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();
	ImGui::Columns(1);

	ImGui::PopID();
}

void
GUI::vec3ControlSlider(const std::string& label, float* values, float min, float max, float resetValue, float columnWidth) {
	ImGuiIO& io = ImGui::GetIO();
	auto boldFont = io.Fonts->Fonts[0];

	ImGui::PushID(label.c_str());

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::Text(label.c_str());
	ImGui::NextColumn();

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

	float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

	// Botón X
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

	// Botón Y
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

	// Botón Z
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
GUI::toolTipData() {
}

void GUI::classicWiiStyle() {
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	ImGui::StyleColorsLight();

	// 1. FORMAS MÁS REDONDEADAS (Efecto "Píldora" para los botones)
	style.WindowRounding = 16.0f;
	style.ChildRounding = 14.0f;
	style.PopupRounding = 14.0f;
	style.FrameRounding = 24.0f;   // <- CLAVE: Valor muy alto para hacerlos tipo píldora
	style.GrabRounding = 12.0f;
	style.ScrollbarRounding = 12.0f;
	style.TabRounding = 10.0f;

	// 2. BORDES (Para lograr el borde celeste del botón)
	style.WindowBorderSize = 1.0f;
	style.FrameBorderSize = 1.5f;  // <- CLAVE: Borde visible y un poco más grueso
	style.PopupBorderSize = 1.0f;

	// Espaciado ajustado para que respiren los botones ovalados
	style.WindowPadding = ImVec2(16, 16);
	style.FramePadding = ImVec2(16, 6);
	style.ItemSpacing = ImVec2(10, 10);

	// 3. PALETA DE COLORES
	const ImVec4 textDark = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
	const ImVec4 wiiCyan = ImVec4(0.15f, 0.75f, 0.95f, 1.0f);      // Celeste vibrante de los bordes
	const ImVec4 wiiBlueLight = ImVec4(0.85f, 0.93f, 0.98f, 1.0f); // Celeste muy suave (Hover)
	const ImVec4 bgLightGrey = ImVec4(0.86f, 0.86f, 0.86f, 1.0f);  // Gris clarito (pestañas y barras)

	// CAMBIO: Color grisáceo para darle el efecto "mancha" o volumen característico de Wii
	const ImVec4 btnGreyWhite = ImVec4(0.88f, 0.89f, 0.90f, 1.0f);

	colors[ImGuiCol_Text] = textDark;
	colors[ImGuiCol_TextDisabled] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);

	// Fondos Generales
	colors[ImGuiCol_WindowBg] = bgLightGrey;
	colors[ImGuiCol_ChildBg] = ImVec4(0.92f, 0.92f, 0.92f, 1.0f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.95f, 0.95f, 0.95f, 0.98f);
	colors[ImGuiCol_MenuBarBg] = bgLightGrey; // Menú superior gris claro

	// BORDES: Aplicamos el celeste globalmente para enmarcar botones e inputs
	colors[ImGuiCol_Border] = wiiCyan;

	// CAMBIO: Añadimos una ligera sombra al borde para dar sensación de profundidad
	colors[ImGuiCol_BorderShadow] = ImVec4(0.2f, 0.2f, 0.2f, 0.15f);

	// Botones (Grisáceo por dentro, borde celeste heredado de Color_Border)
	colors[ImGuiCol_Button] = btnGreyWhite;
	colors[ImGuiCol_ButtonHovered] = wiiBlueLight;
	colors[ImGuiCol_ButtonActive] = ImVec4(0.60f, 0.85f, 0.98f, 1.0f);

	// Inputs / Frames (Siguen la misma lógica de los botones)
	colors[ImGuiCol_FrameBg] = btnGreyWhite;
	colors[ImGuiCol_FrameBgHovered] = wiiBlueLight;
	colors[ImGuiCol_FrameBgActive] = wiiCyan;

	// Pestañas (Color gris clarito como solicitaste)
	colors[ImGuiCol_Tab] = bgLightGrey;
	colors[ImGuiCol_TabHovered] = wiiBlueLight;
	colors[ImGuiCol_TabActive] = btnGreyWhite;
	colors[ImGuiCol_TabUnfocused] = bgLightGrey;
	colors[ImGuiCol_TabUnfocusedActive] = btnGreyWhite;

	// Títulos
	colors[ImGuiCol_TitleBg] = bgLightGrey;
	colors[ImGuiCol_TitleBgActive] = bgLightGrey;
	colors[ImGuiCol_TitleBgCollapsed] = bgLightGrey;

	// Cabeceras (Collapsing headers del inspector)
	colors[ImGuiCol_Header] = wiiBlueLight;
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.75f, 0.88f, 0.98f, 1.0f);
	colors[ImGuiCol_HeaderActive] = wiiCyan;

	// Elementos de acento
	colors[ImGuiCol_CheckMark] = wiiCyan;
	colors[ImGuiCol_SliderGrab] = wiiCyan;
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.50f, 0.80f, 1.0f);
	colors[ImGuiCol_Separator] = ImVec4(0.70f, 0.70f, 0.70f, 0.50f);
	colors[ImGuiCol_SeparatorHovered] = wiiCyan;
	colors[ImGuiCol_NavHighlight] = wiiCyan;
}

void
GUI::ToolBar() {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New")) {
				// Acción para "New"
			}
			if (ImGui::MenuItem("Open")) {
				// Acción para "Open"
			}
			if (ImGui::MenuItem("Save")) {
				// Acción para "Save"
			}
			if (ImGui::MenuItem("Exit")) {
				// Acción para "Exit"
				show_exit_popup = true;
				ImGui::OpenPopup("Exit?");
				//closeApp();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit")) {
			if (ImGui::MenuItem("Undo")) {
				// Acción para "Undo"
			}
			if (ImGui::MenuItem("Redo")) {
				// Acción para "Redo"
			}
			if (ImGui::MenuItem("Cut")) {
				// Acción para "Cut"
			}
			if (ImGui::MenuItem("Copy")) {
				// Acción para "Copy"
			}
			if (ImGui::MenuItem("Paste")) {
				// Acción para "Paste"
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Tools")) {
			if (ImGui::MenuItem("Options")) {
				// Acción para "Options"
			}
			if (ImGui::MenuItem("Settings")) {
				// Acción para "Settings"
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
		show_exit_popup = false; // Reset the flag
	}
	// Centrar el popup en la pantalla
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Exit?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Estas a punto de salir de la aplicacion.\nEstas seguro?\n\n");
		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0))) {
			exit(0); // Salir de la aplicación
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

void
GUI::inspectorGeneral(EU::TSharedPointer<Actor> actor) {
	ImGui::Begin("Inspector");
	// Checkbox para Static
	bool isStatic = false;
	ImGui::Checkbox("##Static", &isStatic);
	ImGui::SameLine();

	// Input text para el nombre del objeto
	char objectName[128] = "Cube";
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
	ImGui::InputText("##ObjectName", &actor->getName()[0], IM_ARRAYSIZE(objectName));
	ImGui::SameLine();

	// Icono (este puede ser una imagen, aquí solo como ejemplo de botón)
	if (ImGui::Button("Icon")) {
		// Lógica del botón de icono aquí
	}

	// Separador horizontal
	ImGui::Separator();

	// Dropdown para Tag
	const char* tags[] = { "Untagged", "Player", "Enemy", "Environment" };
	static int currentTag = 0;
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
	ImGui::Combo("Tag", &currentTag, tags, IM_ARRAYSIZE(tags));
	ImGui::SameLine();

	// Dropdown para Layer
	const char* layers[] = { "Default", "TransparentFX", "Ignore Raycast", "Water", "UI" };
	static int currentLayer = 0;
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
	ImGui::Combo("Layer", &currentLayer, layers, IM_ARRAYSIZE(layers));

	ImGui::Separator();
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
		inspectorContainer(actor);
	}
	ImGui::End();
}

void
GUI::inspectorContainer(EU::TSharedPointer<Actor> actor) {
	auto transform = actor->getComponent<Transform>();
	if (!transform) return;

	// 1. Extraemos los valores actuales de forma segura
	EU::Vector3 pos = transform->getPosition();
	EU::Vector3 rot = transform->getRotation();
	EU::Vector3 sca = transform->getScale();

	// 2. Los pasamos a arreglos que ImGui pueda leer y modificar
	float posArr[3] = { pos.x, pos.y, pos.z };
	float rotArr[3] = { rot.x, rot.y, rot.z };
	float scaArr[3] = { sca.x, sca.y, sca.z };

	// 3. Dibujamos las pestañas
	if (ImGui::BeginTabBar("TransformTabs")) {
		// Pestaña 1: Numérica tradicional
		if (ImGui::BeginTabItem("Numeric")) {
			vec3Control("Position", posArr, 0.0f);
			vec3Control("Rotation", rotArr, 0.0f);
			vec3Control("Scale", scaArr, 1.0f);
			ImGui::EndTabItem();
		}

		// Pestaña 2: Sliders
		if (ImGui::BeginTabItem("Sliders")) {
			vec3ControlSlider("Position", posArr, -50.0f, 50.0f, 0.0f);
			vec3ControlSlider("Rotation", rotArr, -360.0f, 360.0f, 0.0f);
			vec3ControlSlider("Scale", scaArr, 0.0f, 10.0f, 1.0f);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	// 4. Comparamos si ImGui modificó los valores temporales. 
	// Si cambiaron, ahora SÍ usamos los Setters del componente.
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

void
GUI::outliner(const std::vector<EU::TSharedPointer<Actor>>& actors) {
	ImGui::Begin("Hierarchy");

	// Barra de búsqueda
	static ImGuiTextFilter filter;
	filter.Draw("Search...", 180.0f); // Barra de búsqueda con ancho ajustable

	ImGui::Separator();

	// Recorrer y mostrar cada actor que pase el filtro de búsqueda
	for (int i = 0; i < actors.size(); ++i) {
		const auto& actor = actors[i];

		// Obtener el nombre del actor o asignar un nombre genérico
		std::string actorName = actor ? actor->getName() : "Actor";

		// Verificar si el actor pasa el filtro de búsqueda
		if (!filter.PassFilter(actorName.c_str())) {
			continue; // Saltar actores que no coincidan con el filtro
		}

		// Si el actor es seleccionable
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		if (selectedActorIndex == i)
			flags |= ImGuiTreeNodeFlags_Selected;

		// Crear un nodo de árbol para cada actor
		bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)i, flags, "%s", actorName.c_str());

		// Selección de actor
		if (ImGui::IsItemClicked()) {
			selectedActorIndex = i;
			// Aquí puedes llamar a alguna función para mostrar los detalles del actor en otra ventana
		}

		// Mostrar nodos hijos si el nodo está abierto
		if (nodeOpen) {
			ImGui::Text("Position: %.2f, %.2f, %.2f",
				actor->getComponent<Transform>().get()->getPosition().x,
				actor->getComponent<Transform>().get()->getPosition().y,
				actor->getComponent<Transform>().get()->getPosition().z);
			ImGui::TreePop();
		}
	}

	ImGui::End();
}

void GUI::editTransform(Camera& cam, Window& window, EU::TSharedPointer<Actor> actor)
{
	if (!actor) return;

	static ImGuizmo::MODE mCurrentGizmoMode = ImGuizmo::WORLD;
	auto transform = actor->getComponent<Transform>();
	if (!transform) return;

	float rectX = m_viewportPos.x;
	float rectY = m_viewportPos.y;
	float rectW = m_viewportSize.x;
	float rectH = m_viewportSize.y;

	if (rectW < 64.0f || rectH < 64.0f)
	{
		m_isUsingGizmo = false;
		return;
	}

	float* pos = const_cast<float*>(transform->getPosition().data());
	float* rot = const_cast<float*>(transform->getRotation().data());
	float* sca = const_cast<float*>(transform->getScale().data());

	float mArr[16];
	ImGuizmo::RecomposeMatrixFromComponents(pos, rot, sca, mArr);

	float vArr[16], pArr[16];
	ToFloatArray(cam.getView(), vArr);
	ToFloatArray(cam.getProj(), pArr);

	ImGuizmo::SetOrthographic(false);

	// MUY IMPORTANTE: usar el drawlist del viewport, no el actual
	if (m_viewportDrawList)
		ImGuizmo::SetDrawlist(m_viewportDrawList);
	else
		ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

	ImGuizmo::SetID(0);
	ImGuizmo::SetGizmoSizeClipSpace(0.15f);
	ImGuizmo::AllowAxisFlip(false);
	ImGuizmo::SetRect(rectX, rectY, rectW, rectH);

	float snapValue = 25.0f;
	if (mCurrentGizmoOperation == ImGuizmo::ROTATE)    snapValue = 5.0f;
	if (mCurrentGizmoOperation == ImGuizmo::TRANSLATE) snapValue = 0.5f;

	float snap[3] = { snapValue, snapValue, snapValue };
	bool useSnap = ImGui::GetIO().KeyCtrl;

	bool canManipulate = m_viewportHovered || m_viewportActive || m_isUsingGizmo;

	if (canManipulate)
	{
		ImGuizmo::Manipulate(
			vArr,
			pArr,
			mCurrentGizmoOperation,
			mCurrentGizmoMode,
			mArr,
			nullptr,
			useSnap ? snap : nullptr
		);
	}

	m_isUsingGizmo = ImGuizmo::IsUsing();

	if (m_isUsingGizmo)
	{
		float newPos[3], newRot[3], newSca[3];
		ImGuizmo::DecomposeMatrixToComponents(mArr, newPos, newRot, newSca);

		transform->setPosition(EU::Vector3(newPos[0], newPos[1], newPos[2]));
		transform->setRotation(EU::Vector3(newRot[0], newRot[1], newRot[2]));
		transform->setScale(EU::Vector3(newSca[0], newSca[1], newSca[2]));
	}
}

void GUI::drawGizmoToolbar()
{
	//ImGui::SetNextWindowPos(ImVec2(300, 150), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.0f); // 0 = transparente total

	ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_AlwaysAutoResize |/*
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings |*/
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

				if (isActive) ImGui::PopStyleColor();
				ImGui::SameLine();
			};

		buttonMode("T", ImGuizmo::TRANSLATE, "W");
		buttonMode("R", ImGuizmo::ROTATE, "E");
		buttonMode("S", ImGuizmo::SCALE, "R");

		static ImGuizmo::MODE mCurrentGizmoMode = ImGuizmo::WORLD;
		if (ImGui::Button(mCurrentGizmoMode == ImGuizmo::WORLD ? "Global" : "Local"))
			mCurrentGizmoMode = (mCurrentGizmoMode == ImGuizmo::WORLD) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
	}
	ImGui::End();

	ImGui::PopStyleVar();
}

void GUI::drawStudioTopRibbon()
{
	// =========================================================
	// CONFIGURACION GENERAL DE LA BARRA SUPERIOR TIPO STUDIO
	// =========================================================
	ImGuiViewport* viewport = ImGui::GetMainViewport();

	const float menuBarHeight = 24.0f;
	const float ribbonHeight = 72.0f;
	const float totalHeight = menuBarHeight + ribbonHeight;

	// -----------------------------
	// 1) MENU SUPERIOR
	// -----------------------------
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
	//ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.11f, 0.14f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

	if (ImGui::Begin("##StudioMenuBar", nullptr, menuFlags))
	{
		if (ImGui::BeginMenuBar())
		{
			// Lógica para que los menús principales parezcan botones de Wii
			const ImVec4 wiiCyan = ImVec4(0.15f, 0.75f, 0.95f, 1.0f);
			const ImVec4 btnGreyWhite = ImVec4(0.88f, 0.89f, 0.90f, 1.0f);
			const ImVec4 wiiBlueLight = ImVec4(0.85f, 0.93f, 0.98f, 1.0f);

			auto beginEstilizedMenu = [&](const char* label) -> bool {
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 24.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(18, 6));
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 0));
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

				ImGui::PushStyleColor(ImGuiCol_FrameBg, btnGreyWhite);
				ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, wiiBlueLight);
				ImGui::PushStyleColor(ImGuiCol_FrameBgActive, wiiCyan);
				ImGui::PushStyleColor(ImGuiCol_Border, wiiCyan);
				ImGui::PushStyleColor(ImGuiCol_Header, btnGreyWhite);
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, wiiBlueLight);
				ImGui::PushStyleColor(ImGuiCol_HeaderActive, wiiCyan);
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));

				bool isOpen = ImGui::BeginMenu(label);

				// FIX: Si el menú no está abierto (false), debemos hacer el Pop de los estilos
				// aquí mismo, porque el bloque if() en el que se llama no se ejecutará.
				if (!isOpen) {
					ImGui::PopStyleColor(8);
					ImGui::PopStyleVar(5);
				}

				return isOpen;
				};

			auto endEstilizedMenu = []() {
				ImGui::EndMenu();
				ImGui::PopStyleColor(8);
				ImGui::PopStyleVar(5);
				};

			if (beginEstilizedMenu("File"))
			{
				ImGui::MenuItem("New Place");
				ImGui::MenuItem("Open Place");
				ImGui::MenuItem("Save");
				ImGui::Separator();
				if (ImGui::MenuItem("Exit"))
				{
					show_exit_popup = true;
				}
				endEstilizedMenu();
			}

			if (beginEstilizedMenu("Edit"))
			{
				ImGui::MenuItem("Undo");
				ImGui::MenuItem("Redo");
				ImGui::Separator();
				ImGui::MenuItem("Cut");
				ImGui::MenuItem("Copy");
				ImGui::MenuItem("Paste");
				endEstilizedMenu();
			}

			if (beginEstilizedMenu("View"))
			{
				ImGui::MenuItem("Explorer");
				ImGui::MenuItem("Properties");
				ImGui::MenuItem("Toolbox");
				endEstilizedMenu();
			}

			if (beginEstilizedMenu("Plugins"))
			{
				ImGui::MenuItem("Manage Plugins");
				ImGui::MenuItem("Plugin Folder");
				endEstilizedMenu();
			}

			if (beginEstilizedMenu("Test"))
			{
				ImGui::MenuItem("Play");
				ImGui::MenuItem("Pause");
				ImGui::MenuItem("Stop");
				endEstilizedMenu();
			}

			if (beginEstilizedMenu("Window"))
			{
				ImGui::MenuItem("Reset Layout");
				endEstilizedMenu();
			}

			if (beginEstilizedMenu("Help"))
			{
				ImGui::MenuItem("Documentation");
				ImGui::MenuItem("About");
				endEstilizedMenu();
			}

			ImGui::EndMenuBar();
		}
	}
	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);

	// -----------------------------
	// 2) RIBBON PRINCIPAL
	// -----------------------------
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
	//ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.09f, 0.12f, 1.0f));
	//ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.15f, 0.19f, 1.0f));
	//ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.22f, 0.28f, 1.0f));
	//ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.24f, 0.26f, 0.34f, 1.0f));

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.96f, 0.96f, 0.98f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.93f, 0.98f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.00f, 0.65f, 0.95f, 1.0f));

	if (ImGui::Begin("##StudioRibbon", nullptr, ribbonFlags))
	{
		auto ribbonButton = [&](const char* id, const char* topText, const char* bottomText, ImVec2 size, bool active = false) -> bool
			{
				if (active)
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.24f, 0.34f, 0.58f, 1.0f));

				bool pressed = ImGui::Button(id, size);

				ImVec2 min = ImGui::GetItemRectMin();
				ImVec2 max = ImGui::GetItemRectMax();

				// Texto centrado manualmente dentro del boton
				ImDrawList* drawList = ImGui::GetWindowDrawList();

				ImVec2 topSize = ImGui::CalcTextSize(topText);
				ImVec2 bottomSize = ImGui::CalcTextSize(bottomText);

				float centerX = (min.x + max.x) * 0.5f;

				// CAMBIO: Al hacer el botón más bajo (36.0f), ajustamos el offset en Y del texto para que no se salga.
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

		// CAMBIO: Nuevo tamaño para los botones. (Ancho 110, Alto 36).
		const ImVec2 btnSize(110.0f, 36.0f);

		// Herramientas de transformacion
		if (ribbonButton("##Select", "Select", "Cursor", btnSize, false))
		{
			// modo seleccion
		}
		ImGui::SameLine();

		if (ribbonButton("##Move", "Move", "W", btnSize, mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
		{
			mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
		}
		ImGui::SameLine();

		if (ribbonButton("##Rotate", "Rotate", "E", btnSize, mCurrentGizmoOperation == ImGuizmo::ROTATE))
				{
					mCurrentGizmoOperation = ImGuizmo::ROTATE;
				}
				ImGui::SameLine();

		if (ribbonButton("##Scale", "Scale", "R", btnSize, mCurrentGizmoOperation == ImGuizmo::SCALE))
		{
			mCurrentGizmoOperation = ImGuizmo::SCALE;
		}
		ImGui::SameLine();

		if (ribbonButton("##Transform", "Transform", "Tool", btnSize, false))
		{
			// herramienta extra
		}

		/*
		separatorGroup();

		// Creacion / escena
		if (ribbonButton("##Part", "Part", "Mesh", btnSize, false))
		{
			// crear parte
		}
		ImGui::SameLine();

		if (ribbonButton("##Terrain", "Terrain", "Edit", btnSize, false))
		{
			// abrir terrain tools
		}
		ImGui::SameLine();

		if (ribbonButton("##Material", "Material", "Editor", btnSize, false))
		{
			// material editor
		}
		ImGui::SameLine();

		if (ribbonButton("##Color", "Color", "Picker", btnSize, false))
		{
			// color picker
		}
		*/


		separatorGroup();

		// Ventanas / paneles
		if (ribbonButton("##Explorer", "Explorer", "Panel", btnSize, false))
		{
			// toggle explorer
		}
		ImGui::SameLine();

		if (ribbonButton("##Properties", "Properties", "Panel", btnSize, false))
		{
			// toggle properties
		}
		ImGui::SameLine();

		if (ribbonButton("##Toolbox", "Toolbox", "Assets", btnSize, false))
		{
			// toggle toolbox
		}
	}


	ImGui::End();

	ImGui::PopStyleColor(4);
	ImGui::PopStyleVar(3);
}

void GUI::drawViewportPanel(ID3D11ShaderResourceView* viewportSRV)
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

		m_viewportPos = panelMin;
		m_viewportSize = panelSize;

		if (viewportSRV)
		{
			ImGui::Image((ImTextureID)viewportSRV, panelSize);
		}
		else
		{
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 panelMax(panelMin.x + panelSize.x, panelMin.y + panelSize.y);

			drawList->AddRectFilled(panelMin, panelMax, IM_COL32(20, 20, 25, 255));
			drawList->AddText(
				ImVec2(panelMin.x + 12.0f, panelMin.y + 12.0f),
				IM_COL32(220, 220, 220, 255),
				"Viewport sin textura"
			);
		}

		// IMPORTANTE: el hover/active del item imagen
		m_viewportHovered = ImGui::IsItemHovered();
		m_viewportActive = ImGui::IsItemActive();
		m_viewportFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
	}
	ImGui::End();

	ImGui::PopStyleVar();
}

void GUI::drawEditorDockspace()
{
	ImGuiViewport* mainViewport = ImGui::GetMainViewport();

	// Debe coincidir con la altura total que ocupa tu ribbon superior
	const float topOffset = 96.0f; // 24 menu + 72 ribbon

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

void
GUI::debugLightsContainer(float* lightDir, float* lightColor) {
	if (ImGui::BeginTabBar("LightTabs")) {
		// Pestaña 1: Numérica tradicional
		if (ImGui::BeginTabItem("Numeric")) {
			vec3Control("Light Dir", lightDir);
			vec3Control("Light Color", lightColor);
			ImGui::EndTabItem();
		}

		// Pestaña 2: Sliders
		if (ImGui::BeginTabItem("Sliders")) {
			// La dirección suele estar normalizada entre -1.0 y 1.0
			vec3ControlSlider("Light Dir", lightDir, -1.0f, 1.0f, 0.0f);
			// El color suele ir de 0.0 a 1.0 en formato RGB (que aquí usas como XYZ)
			vec3ControlSlider("Light Color", lightColor, 0.0f, 1.0f, 1.0f);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}