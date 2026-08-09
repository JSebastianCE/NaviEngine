/**
 * @file GUI.h
 * @brief Declara la clase GUI encargada de gestionar la interfaz de usuario del editor.
 * @ingroup editor
 */
#pragma once

#include "Prerequisites.h"

#include "ECS/LightComponent.h"
#include "ECS/MeshRendererComponent.h"
#include "ECS/ParticleEmitterComponent.h"

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

/**
 * @class GUI
 * @brief Maneja la renderizacion y logica de la interfaz del editor usando Dear ImGui.
 * * Gestiona los paneles (Inspector, Outliner, Viewport), la comunicacion de atajos,
 * y la manipulacion de transformaciones en 3D usando ImGuizmo.
 */
class GUI {
public:
	GUI() = default;
	~GUI() = default;

	/**
	 * @brief Se llama al arrancar la aplicacion antes de inicializar los recursos graficos.
	 */
	void
	awake();

	//dibuja la consola
	/**
	 * @brief Dibuja la consola de registro (Logs) capturando advertencias y errores.
	 */
	void
	drawLogConsole();

	// dibuja las estadisticas de rendimiento
	/**
	 * @brief Dibuja el panel con las estadisticas de rendimiento (FPS, tiempo de frame).
	 */
	void
	drawStatsPanel();

	/**
	 * @brief Inicializa los contextos de ImGui y enlaza con DirectX 11.
	 * @param window Referencia a la ventana principal.
	 * @param device Referencia al dispositivo grafico.
	 * @param deviceContext Referencia al contexto del dispositivo grafico.
	 */
	void
	init(Window& window, Device& device, DeviceContext& deviceContext);

	/**
	 * @brief Actualiza la logica de la interfaz en el frame actual.
	 * @param viewport Referencia al viewport principal.
	 * @param window Referencia a la ventana principal.
	 */
	void
	update(Viewport& viewport, Window& window);

	/**
	 * @brief Emite los comandos de dibujo de ImGui a la tarjeta grafica.
	 */
	void 
	render();

	/**
	 * @brief Apaga ImGui y libera los recursos asociados a la interfaz.
	 */
	void 
	destroy();

	/** @brief Dibuja una barra de herramientas estandar. */
	void 
	ToolBar();

	/** @brief Muestra el dialogo de confirmacion para cerrar la aplicacion. */
	void 
	closeApp();

	/** @brief Muestra informacion adicional flotante sobre herramientas (Tooltips). */
	void
	toolTipData();

	/**
	 * @brief Aplica un tema visual moderno y translucido a la interfaz.
	 * @param opacity Nivel de opacidad de las ventanas.
	 * @param accent Color de acento para los elementos interactivos.
	 */
	void 
	appleLiquidStyle(float opacity, ImVec4 accent);

	/**
	 * @brief Aplica un tema visual nostalgico inspirado en la consola Wii.
	 */
	void 
	classicWiiStyle();

	/**
	 * @brief Crea un control UI personalizado para editar vectores de 3 componentes (XYZ).
	 * @param label Etiqueta descriptiva del control.
	 * @param values Puntero al arreglo de 3 floats a modificar.
	 * @param resetValues Valor por defecto al presionar el boton de reseteo.
	 * @param columnWidth Ancho de la columna de texto.
	 * @param displayAsDegrees Si es true, muestra los radianes como grados en la UI.
	 */
	void 
	vec3Control(const std::string& label,
		float* values,
		float resetValues = 0.0f,
		float columnWidth = 100.0f,
		bool displayAsDegrees = false);

	/**
	 * @brief Control similar a vec3Control pero usando sliders limitados por un min y max.
	 */
	void 
	vec3ControlSlider(const std::string& label,
		float* values,
		float min = -100.0f,
		float max = 100.0f,
		float resetValues = 0.0f,
		float columnWidth = 100.0f);

	/**
	 * @brief Panel rapido para modificar direccion y color de las luces globales.
	 */
	void
	debugLightsContainer(float* lightDir, float* lightColor);

	/**
	 * @brief Muestra el Inspector general de propiedades para el actor seleccionado.
	 * @param actor Puntero al actor actualmente seleccionado.
	 */
	void
	inspectorGeneral(EU::TSharedPointer<Actor> actor);

	/**
	 * @brief Contenedor interno del Inspector para dibujar componentes especificos.
	 */
	void
	inspectorContainer(EU::TSharedPointer<Actor> actor);

	/**
	 * @brief Dibuja el Outliner (jerarquia de escena) mostrando todos los actores.
	 * @param actors Lista de todos los actores en la escena.
	 */
	void
	outliner(const std::vector<EU::TSharedPointer<Actor>>& actors);

	/**
	 * @brief Dibuja y gestiona los Gizmos interactivos de transformacion en 3D sobre un actor.
	 * @param cam Camara actual del Viewport para el calculo de proyeccion.
	 * @param window Referencia a la ventana de la aplicacion.
	 * @param actor El actor sobre el cual se aplicara la transformacion.
	 */
	void
	editTransform(Camera& cam, Window& window, EU::TSharedPointer<Actor> actor);

	/**
	 * @brief Dibuja la barra de herramientas vertical con botones de seleccion, traslacion, rotacion y escala.
	 */
	void 
	drawGizmoToolbar();

	/**
	 * @brief Dibuja la barra superior del editor (Menu principal de Archivo, Edicion, etc).
	 */
	void 
	drawStudioTopRibbon();

	/**
	 * @brief Dibuja la ventana del Viewport 3D y le inyecta la textura renderizada por DirectX.
	 * @param viewportSRV Textura (Shader Resource View) que contiene la escena graficada.
	 */
	void 
	drawViewportPanel(ID3D11ShaderResourceView* viewportSRV);

	/**
	 * @brief Dibuja iconos 2D superpuestos a las posiciones 3D de las luces en el Viewport.
	 * @param actors Lista de actores de la escena.
	 * @param camera Camara actual (necesaria para proyectar la coordenada 3D a la pantalla 2D).
	 * @param lightIconSRV Textura de la imagen del icono de luz.
	 */
	void
	drawViewportLightIcons(const std::vector<EU::TSharedPointer<Actor>>& actors,
		Camera& camera,
		ID3D11ShaderResourceView* lightIconSRV);


	/**
	 * @brief (Legado) Panel para depurar pases de render de sombras en modo Forward.
	 */
	void 
	drawRenderDebugPanel(ID3D11ShaderResourceView* preShadowSRV,
		ID3D11ShaderResourceView* finalViewportSRV,
		ID3D11ShaderResourceView* shadowMapSRV);

	// NUEVO: panel de depuracion del G-Buffer (deferred)
	/**
	 * @brief Panel interactivo para visualizar los distintos canales y variables del G-Buffer en tiempo real.
	 * @param albedoMetallicSRV Textura del canal de color base/metalico.
	 * @param normalRoughnessSRV Textura del canal de normales/rugosidad.
	 * @param worldAoSRV Textura del canal de posicion global/oclusion ambiental.
	 * @param emissiveAlphaSRV Textura del canal emisivo/transparencia.
	 * @param selectedActor Actor seleccionado para extraer o editar su material desde el panel.
	 */
	void
	drawGBufferDebugPanel(ID3D11ShaderResourceView* albedoMetallicSRV,
		ID3D11ShaderResourceView* normalRoughnessSRV,
		ID3D11ShaderResourceView* worldAoSRV,
		ID3D11ShaderResourceView* emissiveAlphaSRV,
		EU::TSharedPointer<Actor> selectedActor);

	/**
	 * @brief Configura el entorno general permitiendo arrastrar y anclar ventanas por toda la pantalla (Docking).
	 */
	void
	drawEditorDockspace();

	/**
	 * @brief Consulta si el usuario disparo el comando de guardar la escena y reinicia el estado.
	 * @return true si se solicito guardar.
	 */
	bool
	consumeSaveSceneRequest() {
		const bool requested = m_requestSaveScene;
		m_requestSaveScene = false;
		return requested;
	}

	/**
	 * @brief Consulta si el usuario solicito la creacion de una luz desde la interfaz.
	 * @return true si se solicito crear un Actor Luz.
	 */
	bool 
	consumeCreateLightActorRequest() {
		const bool requested = m_requestCreateLightActor;
		m_requestCreateLightActor = false;
		return requested;
	}

	/**
	 * @brief Consulta si el usuario solicito la creacion de un emisor de particulas desde la interfaz.
	 * @return true si se solicito crear un Actor Emisor de Particulas.
	 */
	bool
	consumeCreateParticleActorRequest() {
		const bool requested = m_requestCreateParticleActor;
		m_requestCreateParticleActor = false;
		return requested;
	}

	/**
	 * @brief Utilidad matematica para convertir una matriz de DirectX a un arreglo nativo (Usado por ImGuizmo).
	 */
	void 
	ToFloatArray(const XMMATRIX& mat, float* dest) {
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

	// SRVs de debug almacenados por drawRenderDebugPanel
	ID3D11ShaderResourceView* m_renderDebugPreShadowSRV = nullptr;
	ID3D11ShaderResourceView* m_renderDebugFinalSRV = nullptr;
	ID3D11ShaderResourceView* m_renderDebugShadowMapSRV = nullptr;

	bool m_requestCreateLightActor = false;

	bool m_requestCreateParticleActor = false;


public:
	/** @brief Bandera que indica si el usuario esta arrastrando actualmente el Gizmo 3D. */
	bool m_isUsingGizmo = false;

	/** @brief Indice del actor actualmente seleccionado en el vector global (-1 = Ninguno). */
	int selectedActorIndex = -1;

	/** @brief Herramienta actual de transformacion (0 = Seleccion, 1 = Traslacion, 2 = Rotacion, 3 = Escala). */
	int m_currentGizmoTool = 1;

	// control de visualizacion deferred (lo lee BaseApp -> RenderPipeline)
	/** @brief Habilita la visualizacion cruda del mapa de oclusion/sombras en el Viewport. */
	bool m_visualizeDeferredShadowFactor = false;

	/** @brief Indice del canal grafico que se esta previsualizando (0 = Final, 2 = Albedo, 3 = Normal, etc). */
	int m_deferredDebugViewMode = 0;

	/** @brief Coordenadas de pantalla 2D donde inicia la esquina de la ventana del Viewport. */
	ImVec2 m_viewportPos = { 0.0f, 0.0f };

	/** @brief Dimensiones (ancho, alto) de la imagen renderizada actual en el Viewport. */
	ImVec2 m_viewportSize = { 0.0f, 0.0f };

	/** @brief Bandera que confirma si el cursor del mouse esta situado encima del Viewport 3D. */
	bool m_viewportHovered = false;

	/** @brief Bandera que confirma si el usuario hizo clic dentro del Viewport y este tiene el foco. */
	bool m_viewportFocused = false;
};