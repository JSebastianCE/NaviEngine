#pragma once
#include "Prerequisites.h"
#include "ECS/Component.h"
#include "Rendering/RenderTypes.h"

class DeviceContext;

/**
 * @class LightComponent
 * @brief Componente que representa una fuente de luz en el ECS.
 * * Almacena los datos de iluminacion (como color, direccion) y
 * determina si esta luz debe proyectar sombras dentro de la escena.
 */
class
LightComponent : public Component {
public:
	/**
	 * @brief Constructor por defecto.
	 * Inicializa el componente con un tipo generico (ComponentType::NONE).
	 */
	LightComponent()
		: Component(ComponentType::NONE) {
	}

	/**
	 * @brief Inicializa los recursos o estados iniciales de la luz.
	 */
	void
	init() override {}

	/**
	 * @brief Actualiza la logica del componente en cada frame.
	 * @param deltaTime Tiempo transcurrido desde el ultimo frame (en segundos).
	 */
	void
	update(float deltaTime) override {}

	/**
	 * @brief Envia los datos de la luz a la tarjeta grafica o pipeline.
	 * @param deviceContext Referencia al contexto del dispositivo grafico.
	 */
	void
	render(DeviceContext& deviceContext) override {}

	/**
	 * @brief Libera la memoria o recursos asociados a este componente antes de destruirse.
	 */
	void
	destroy() override {}

	/**
	 * @brief Obtiene los atributos fisicos de la luz para ser modificados.
	 * @return Referencia a la estructura interna LightData.
	 */
	LightData& getLightData() { return m_light; }

	/**
	 * @brief Obtiene los atributos fisicos de la luz en modo de solo lectura.
	 * @return Referencia constante a la estructura interna LightData.
	 */
	const LightData& getLightData() const { return m_light; }

	/**
	 * @brief Establece si esta luz debe generar mapas de sombras (Shadow Maps).
	 * @param value true para habilitar el casteo de sombras, false para deshabilitarlo.
	 */
	void
  setCastShadow(bool value) { m_castShadow = value; }

	/**
	 * @brief Comprueba si el casteo de sombras esta activo para esta luz.
	 * @return true si proyecta sombras, de lo contrario false.
	 */
	bool
	canCastShadow() const { return m_castShadow; }

private:
	/** @brief Contenedor con las propiedades de la luz (color, posicion, intensidad, etc). */
	LightData m_light;

	/** @brief Bandera que determina si esta luz participa en el pase de sombras. */
	bool m_castShadow = false;
};