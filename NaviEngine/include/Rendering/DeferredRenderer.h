/**
 * @file DeferredRenderer.h
 * @brief Declara la API de DeferredRenderer dentro del subsistema Rendering.
 * @ingroup rendering
 */
#pragma once
#include "Buffer.h"
#include "DepthStencilState.h"
#include "DepthStencilView.h"
#include "RasterizerState.h"
#include "Rendering/ISceneRenderer.h"
#include "Rendering/RenderScene.h"
#include "Rendering/RenderTypes.h"
#include "SamplerState.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "EngineUtilities/Utilities/EditorViewportPass.h"

class Device;
class DeviceContext;
class Camera;
class Material;

/**
 * @class DeferredRenderer
 * @brief Implementa un pipeline diferido con GBuffer y lighting pass.
 *
 * El renderer usa deferred shading para superficies opacas y mantiene un subpass
 * forward para transparencias, de modo que el pipeline del editor siga funcionando
 * con el contenido actual del engine.
 */
class
	DeferredRenderer : public ISceneRenderer {
public:
	/**
	 * @brief Inicializa los recursos del pipeline diferido (G-Buffer, shaders, sombras).
	 * @param device Referencia al dispositivo de DirectX.
	 * @return HRESULT indicando si la inicializacion fue exitosa.
	 */
	HRESULT
		init(Device& device) override;

	/**
	 * @brief Reconstruye los render targets del G-Buffer cuando cambia el tamano de la pantalla.
	 * @param device Referencia al dispositivo de DirectX.
	 * @param width Nuevo ancho.
	 * @param height Nuevo alto.
	 */
	void
		resize(Device& device, unsigned int width, unsigned int height) override;

	/**
	 * @brief Orquesta y ejecuta todas las fases de dibujo (Sombras, Geometria, Iluminacion, Transparencias).
	 * @param deviceContext Contexto del dispositivo para enviar comandos a la GPU.
	 * @param camera Camara actual desde donde se ve la escena.
	 * @param scene Escena con todos los actores listos para dibujar.
	 * @param viewportPass El target final donde se guardara el renderizado.
	 */
	void
		render(DeviceContext& deviceContext,
			const Camera& camera,
			RenderScene& scene,
			EditorViewportPass& viewportPass) override;

	/**
	 * @brief Libera la memoria de todos los buffers, texturas y shaders utilizados.
	 */
	void
		destroy() override;

	/**
	 * @brief Obtiene la textura de profundidad (Shadow Map).
	 * @return Puntero al SRV del Shadow Map.
	 */
	ID3D11ShaderResourceView*
		getShadowMapSRV() const override { return m_shadowDepthSRV.m_textureFromImg; }

	/**
	 * @brief Obtiene el SRV de la pre-sombra para propositos de depuracion.
	 * @return Puntero al SRV de Pre-Shadow.
	 */
	ID3D11ShaderResourceView*
		getPreShadowSRV() const override { return m_preShadowDebugPass.getSRV(); }

	/**
	 * @brief Obtiene la textura del G-Buffer que guarda el color base y lo metalico.
	 * @return Puntero al SRV de Albedo/Metallic.
	 */
	ID3D11ShaderResourceView*
		getGBufferAlbedoMetallicSRV() const override { return m_gBufferAlbedoMetallicSRV.m_textureFromImg; }

	/**
	 * @brief Obtiene la textura del G-Buffer que guarda la direccion de las caras y su rugosidad.
	 * @return Puntero al SRV de Normal/Roughness.
	 */
	ID3D11ShaderResourceView*
		getGBufferNormalRoughnessSRV() const override { return m_gBufferNormalRoughnessSRV.m_textureFromImg; }

	/**
	 * @brief Obtiene la textura del G-Buffer que guarda la posicion 3D absoluta y la oclusion ambiental.
	 * @return Puntero al SRV de World Position/AO.
	 */
	ID3D11ShaderResourceView*
		getGBufferWorldAoSRV() const override { return m_gBufferWorldAoSRV.m_textureFromImg; }

	/**
	 * @brief Obtiene la textura del G-Buffer que guarda el color brillante y el mapa de transparencia.
	 * @return Puntero al SRV de Emissive/Alpha.
	 */
	ID3D11ShaderResourceView*
		getGBufferEmissiveAlphaSRV() const override { return m_gBufferEmissiveAlphaSRV.m_textureFromImg; }

	/**
	 * @brief Activa o desactiva la vista de depuracion para ver unicamente las sombras.
	 * @param enabled true para activar, false para regresar al render normal.
	 */
	void
		setShadowFactorDebugEnabled(bool enabled) override { m_shadowFactorDebugEnabled = enabled; }

	/**
	 * @brief Configura que textura interna del G-Buffer se mostrara en pantalla.
	 * @param mode Indice del modo de depuracion (Ej: 0 = Final, 2 = Albedo, etc).
	 */
	void
		setDeferredDebugViewMode(int mode) override { m_deferredDebugViewMode = mode; }

	/**
	 * @brief Obtiene el nombre identificador de este renderer.
	 * @return Cadena de texto "DeferredRenderer".
	 */
	const char*
		getDebugName() const override { return "DeferredRenderer"; }

private:
	/** @brief Clasifica los objetos de la escena en colas (Opacos vs Transparentes). */
	void buildQueues(RenderScene& scene, const Camera& camera);
	/** @brief Actualiza la informacion global (matrices de camara) al inicio del frame. */
	void updatePerFrame(const Camera& camera, const RenderScene& scene, DeviceContext& deviceContext);
	/** @brief Actualiza las matrices para proyectar las sombras desde el punto de vista de la luz. */
	void updateLightMatrices(const Camera& camera, const RenderScene& scene);
	/** @brief Orquesta el pase para un target especifico. */
	void renderSceneToTarget(DeviceContext& deviceContext, RenderScene& scene, EditorViewportPass& targetPass, bool applyShadows);
	/** @brief Conecta las texturas del G-Buffer a la GPU para escribir en ellas. */
	void bindGBufferTargets(DeviceContext& deviceContext, ID3D11DepthStencilView* depthStencilView);
	/** @brief Conecta el target final donde se uniran todas las luces y colores. */
	void bindFinalTarget(DeviceContext& deviceContext, ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* depthStencilView);
	/** @brief Limpia todas las texturas del G-Buffer antes de empezar a dibujar un frame nuevo. */
	void clearDeferredSRVs(DeviceContext& deviceContext);
	/** @brief Dibuja todos los objetos opacos llenando la informacion del G-Buffer. */
	void renderGeometryPass(DeviceContext& deviceContext);
	/** @brief Procesa el dibujo individual de un objeto opaco. */
	void renderGeometryObject(DeviceContext& deviceContext, const RenderObject& object);
	/** @brief Toma las texturas del G-Buffer y aplica los calculos matematicos de luces y sombras. */
	void renderLightingPass(DeviceContext& deviceContext);
	/** @brief Dibuja el fondo (Cielo) respetando la profundidad de la escena. */
	void renderSkyboxPass(DeviceContext& deviceContext, RenderScene& scene);
	/** @brief Dibuja los objetos que tienen transparencia (vidrio, hologramas) despues del pase de iluminacion. */
	void renderTransparentPass(DeviceContext& deviceContext);
	/** @brief Procesa el dibujo individual de un objeto para metodos directos (forward). */
	void renderForwardObject(DeviceContext& deviceContext, const RenderObject& object, RenderPassType passType);
	/** @brief Genera el mapa de profundidad ocultando lo que la luz no puede ver. */
	void renderShadowPass(DeviceContext& deviceContext);
	/** @brief Procesa el dibujo de la silueta de un objeto en el mapa de sombras. */
	void renderShadowObject(DeviceContext& deviceContext, const RenderObject& object);
	/** @brief Crea las texturas y buffers necesarios para las sombras. */
	HRESULT createShadowResources(Device& device);
	/** @brief Coordina la creacion de los cuatro render targets del G-Buffer. */
	HRESULT createGBufferResources(Device& device, unsigned int width, unsigned int height);
	/** @brief Funcion auxiliar para construir una textura especifica del G-Buffer. */
	HRESULT createGBufferTarget(Device& device,
		unsigned int width,
		unsigned int height,
		DXGI_FORMAT format,
		Texture& texture,
		Texture& srv,
		RenderTargetView& rtv);
	/** @brief Crea el cuadradito a pantalla completa (Quad) y sus shaders para el pase de luces. */
	HRESULT createLightingResources(Device& device);
	/** @brief Genera la geometria de un rectangulo que cubre toda la pantalla. */
	HRESULT createFullScreenQuad(Device& device);
	/** @brief Configura las formulas matematicas para mezclar transparencias (Alpha Blending). */
	HRESULT createBlendStates(Device& device);
	/** @brief Decide que estado de mezcla de transparencias usar dependiendo del material. */
	ID3D11BlendState* resolveBlendState(const Material* material) const;

private:
	/** @brief Buffers constantes para enviar datos a la GPU. */
	Buffer m_perFrameBuffer;
	Buffer m_perObjectBuffer;
	Buffer m_perMaterialBuffer;
	Buffer m_lightingDebugBuffer;

	/** @brief Buffers de geometria para el quad de pantalla completa. */
	Buffer m_fullscreenVertexBuffer;
	Buffer m_fullscreenIndexBuffer;

	/** @brief Reglas de prueba de profundidad (Z-Buffer). */
	DepthStencilState m_transparentDepthStencil;
	DepthStencilState m_disabledDepthStencil;
	DepthStencilState m_shadowDepthStencil;

	/** @brief Estados de mezcla (Blending) para las transparencias. */
	ID3D11BlendState* m_alphaBlendState = nullptr;
	ID3D11BlendState* m_opaqueBlendState = nullptr;
	ID3D11BlendState* m_additiveBlendState = nullptr;
	ID3D11BlendState* m_premultipliedBlendState = nullptr;
	float m_blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	/** @brief Recursos graficos para la generacion de sombras. */
	Texture m_shadowDepthTexture;
	Texture m_shadowDepthSRV;
	DepthStencilView m_shadowDSV;
	ShaderProgram m_shadowShader;
	RasterizerState m_shadowRasterizer;
	unsigned int m_shadowMapSize = 2048;

	/** @brief Shaders principales del G-Buffer y el pase de Iluminacion. */
	ShaderProgram m_gBufferShader;
	ShaderProgram m_deferredLightingShader;
	SamplerState m_lightingSampler;
	RasterizerState m_fullscreenRasterizer;

	/** @brief Canal 1 del G-Buffer: Albedo (RGB) y Metalico (A). */
	Texture m_gBufferAlbedoMetallicTexture;
	Texture m_gBufferAlbedoMetallicSRV;
	RenderTargetView m_gBufferAlbedoMetallicRTV;

	/** @brief Canal 2 del G-Buffer: Normal (RGB) y Rugosidad (A). */
	Texture m_gBufferNormalRoughnessTexture;
	Texture m_gBufferNormalRoughnessSRV;
	RenderTargetView m_gBufferNormalRoughnessRTV;

	/** @brief Canal 3 del G-Buffer: Posicion del mundo (RGB) y Oclusion Ambiental (A). */
	Texture m_gBufferWorldAoTexture;
	Texture m_gBufferWorldAoSRV;
	RenderTargetView m_gBufferWorldAoRTV;

	/** @brief Canal 4 del G-Buffer: Emisividad (RGB) y Transparencia global (A). */
	Texture m_gBufferEmissiveAlphaTexture;
	Texture m_gBufferEmissiveAlphaSRV;
	RenderTargetView m_gBufferEmissiveAlphaRTV;

	/** @brief Variables de estado general de visualizacion y tamano. */
	EditorViewportPass m_preShadowDebugPass;
	bool m_applyShadows = true;
	unsigned int m_renderWidth = 1280;
	unsigned int m_renderHeight = 720;

	/** @brief Estructuras en RAM (CPU) de los buffers que se enviaran a la GPU. */
	CBPerFrame m_cbPerFrame{};
	CBPerObject m_cbPerObject{};
	CBPerMaterial m_cbPerMaterial{};

	/** @brief Informacion sobre el modo de depuracion activo. */
	struct DeferredLightingDebugData {
		int DebugViewMode = 0;
		float ShadowStrength = 1.0f;
		float pad0 = 0.0f;
		float pad1 = 0.0f;
	} m_lightingDebugData{};

	bool m_shadowFactorDebugEnabled = false;
	int m_deferredDebugViewMode = 0;

	/** @brief Colas de renderizado organizadas al inicio del frame. */
	std::vector<const RenderObject*> m_opaqueQueue;
	std::vector<const RenderObject*> m_transparentQueue;
};