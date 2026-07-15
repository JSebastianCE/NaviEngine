/**
 * @file RenderPipeline.h
 * @brief Declara el orquestador de renderers de escena del motor.
 * @ingroup rendering
 */
#pragma once
#include "Rendering/ForwardRenderer.h"
#include "Rendering/DeferredRenderer.h"

 /**
	* @class RenderPipeline
	* @brief Selecciona y ejecuta el renderer activo para el frame actual.
	*/
class
	RenderPipeline {
public:
	/**
	 * @brief Inicializa el pipeline y establece el renderer inicial.
	 * @param device Referencia al dispositivo grafico de DirectX.
	 * @param initialRenderer Tipo de renderer a utilizar por defecto (Deferred o Forward).
	 * @return HRESULT indicando el exito o fallo de la inicializacion.
	 */
	HRESULT init(Device& device, RendererType initialRenderer = RendererType::Deferred);

	/**
	 * @brief Cambia el tipo de renderer activo en tiempo de ejecucion.
	 * @param rendererType El nuevo tipo de renderer a utilizar.
	 * @param device Referencia al dispositivo grafico (necesario por si el nuevo renderer no ha sido inicializado).
	 * @return HRESULT indicando el exito o fallo al realizar el cambio.
	 */
	HRESULT setRendererType(RendererType rendererType, Device& device);

	/**
	 * @brief Redimensiona los buffers y recursos asociados al renderer activo.
	 * @param device Referencia al dispositivo grafico.
	 * @param width Nuevo ancho de la ventana o viewport.
	 * @param height Nuevo alto de la ventana o viewport.
	 */
	void resize(Device& device, unsigned int width, unsigned int height);

	/**
	 * @brief Ejecuta el pase de renderizado completo para el frame actual.
	 * @param deviceContext Contexto del dispositivo para emitir comandos de dibujo.
	 * @param camera Camara activa desde la cual se renderiza la escena.
	 * @param scene Escena actual que contiene los actores y luces a procesar.
	 * @param viewportPass El target o pase del editor donde se dibujara el resultado final.
	 */
	void render(DeviceContext& deviceContext,
		const Camera& camera,
		RenderScene& scene,
		EditorViewportPass& viewportPass);

	/**
	 * @brief Libera la memoria y destruye los recursos de todos los renderers.
	 */
	void destroy();

	/**
	 * @brief Obtiene el identificador del tipo de renderer que esta actualmente en uso.
	 * @return Valor actual del enumerador RendererType.
	 */
	RendererType getRendererType() const { return m_activeRendererType; }

	/**
	 * @brief Obtiene el nombre del renderer activo como cadena de texto.
	 * @return Puntero constante a la cadena con el nombre.
	 */
	const char* getActiveRendererName() const;

	/**
	 * @brief Obtiene la textura final del mapa de sombras (Shadow Map).
	 * @return Puntero al SRV del mapa de sombras.
	 */
	ID3D11ShaderResourceView* getShadowMapSRV() const;

	/**
	 * @brief Obtiene el SRV de pre-sombras (usualmente para la generacion del mapa de profundidad).
	 * @return Puntero al SRV de Pre-Shadow.
	 */
	ID3D11ShaderResourceView* getPreShadowSRV() const;

	/**
	 * @brief Obtiene el recurso de la tarjeta grafica que almacena el Albedo (RGB) y Metallic (A).
	 * @return Puntero al SRV correspondiente del G-Buffer.
	 */
	ID3D11ShaderResourceView* getGBufferAlbedoMetallicSRV() const;

	/**
	 * @brief Obtiene el recurso de la tarjeta grafica que almacena las Normales (RGB) y Roughness (A).
	 * @return Puntero al SRV correspondiente del G-Buffer.
	 */
	ID3D11ShaderResourceView* getGBufferNormalRoughnessSRV() const;

	/**
	 * @brief Obtiene el recurso de la tarjeta grafica que almacena el World Position (RGB) y Ambient Occlusion (A).
	 * @return Puntero al SRV correspondiente del G-Buffer.
	 */
	ID3D11ShaderResourceView* getGBufferWorldAoSRV() const;

	/**
	 * @brief Obtiene el recurso de la tarjeta grafica que almacena el Emissive (RGB) y el canal Alpha (A).
	 * @return Puntero al SRV correspondiente del G-Buffer.
	 */
	ID3D11ShaderResourceView* getGBufferEmissiveAlphaSRV() const;

	/**
	 * @brief Activa o desactiva la visualizacion exclusiva del factor de sombras en el Viewport.
	 * @param enabled true para activar el modo debug de sombras, false para desactivarlo.
	 */
	void setShadowFactorDebugEnabled(bool enabled);

	/**
	 * @brief Configura el modo de visualizacion para depurar un canal especifico del G-Buffer (solo en modo Deferred).
	 * @param mode El indice del modo de visualizacion (ej. Final, Albedo, Normales, Metallic, etc).
	 */
	void setDeferredDebugViewMode(int mode);

private:
	/**
	 * @brief Verifica si un renderer ha sido inicializado. Si no es asi, lo inicializa de forma segura (Lazy Initialization).
	 * @param rendererType Tipo de renderer a comprobar.
	 * @param device Referencia al dispositivo grafico.
	 * @return HRESULT indicando el estado final de la inicializacion.
	 */
	HRESULT ensureRendererInitialized(RendererType rendererType, Device& device);

	/**
	 * @brief Resuelve y devuelve un puntero a la clase concreta del renderer solicitado.
	 * @param rendererType Tipo de renderer a buscar.
	 * @return Puntero a la interfaz ISceneRenderer (Forward o Deferred).
	 */
	ISceneRenderer* resolveRenderer(RendererType rendererType);

	/**
	 * @brief Resuelve y devuelve un puntero constante a la clase concreta del renderer solicitado.
	 * @param rendererType Tipo de renderer a buscar.
	 * @return Puntero constante a la interfaz ISceneRenderer.
	 */
	const ISceneRenderer* resolveRenderer(RendererType rendererType) const;

private:
	/** @brief Instancia responsable del renderizado directo tradicional. */
	ForwardRenderer m_forwardRenderer;

	/** @brief Instancia responsable del renderizado diferido (Deferred Rendering) mediante G-Buffer. */
	DeferredRenderer m_deferredRenderer;

	/** @brief Puntero polimorfico al renderer que esta dibujando el frame actual. */
	ISceneRenderer* m_activeRenderer = nullptr;

	/** @brief Estado actual del tipo de renderer seleccionado. */
	RendererType m_activeRendererType = RendererType::Deferred;

	/** @brief Bandera para saber si el ForwardRenderer ya tiene recursos en la GPU. */
	bool m_forwardInitialized = false;

	/** @brief Bandera para saber si el DeferredRenderer ya tiene recursos en la GPU. */
	bool m_deferredInitialized = false;
};