/**
 * @file MeshRendererComponent.h
 * @brief Declara la API de MeshRendererComponent dentro del subsistema ECS.
 * @ingroup ecs
 */
#pragma once
#include "Prerequisites.h"
#include "ECS/Component.h"

class Mesh;
class MaterialInstance;
class DeviceContext;

/**
 * @class MeshRendererComponent
 * @brief Componente que enlaza un modelo 3D y sus materiales a un Actor.
 * * Es el responsable de proporcionar a los sistemas de renderizado (Forward o Deferred)
 * toda la informacion visual de una entidad, incluyendo geometria, texturas,
 * visibilidad y su participacion en el mapeo de sombras.
 */
class
	MeshRendererComponent : public Component {
public:
	/**
	 * @brief Constructor por defecto.
	 * Inicializa el componente estableciendo su tipo como ComponentType::MESH.
	 */
	MeshRendererComponent()
		: Component(ComponentType::MESH) {
	}

	/**
	 * @brief Inicializa los recursos internos de la malla.
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
	 * @brief Dibuja la malla utilizando el pipeline grafico activo.
	 * @param deviceContext Referencia al contexto del dispositivo grafico.
	 */
	void
		render(DeviceContext& deviceContext) override {}

	/**
	 * @brief Libera de forma segura la memoria y recursos asociados a este componente.
	 */
	void
		destroy() override {}

	/**
	 * @brief Asigna la geometria (modelo 3D) que sera renderizada.
	 * @param mesh Puntero al recurso de tipo Mesh.
	 */
	void
		setMesh(Mesh* mesh) { m_mesh = mesh; }

	/**
	 * @brief Obtiene la malla 3D actual asociada al componente.
	 * @return Puntero a la malla (Mesh).
	 */
	Mesh* getMesh() const { return m_mesh; }

	/**
	 * @brief Asigna un material principal y elimina cualquier material previo en la lista.
	 * @param materialInstance Puntero a la instancia del material.
	 */
	void
		setMaterialInstance(MaterialInstance* materialInstance) {
		m_materialInstance = materialInstance;
		m_materialInstances.clear();
		if (materialInstance) {
			m_materialInstances.push_back(materialInstance);
		}
	}

	/**
	 * @brief Obtiene el material principal de la malla.
	 * @return Puntero a la instancia del material base.
	 */
	MaterialInstance* getMaterialInstance() const { return m_materialInstance; }

	/**
	 * @brief Reemplaza todos los materiales actuales por una lista nueva (ideal para sub-mallas).
	 * @param materialInstances Vector que contiene los punteros a los nuevos materiales.
	 */
	void
		setMaterialInstances(const std::vector<MaterialInstance*>& materialInstances) {
		m_materialInstances = materialInstances;
		m_materialInstance = m_materialInstances.empty() ? nullptr : m_materialInstances.front();
	}

	/**
	 * @brief Agrega un material adicional al final de la lista de materiales.
	 * @param materialInstance Puntero al nuevo material que se desea añadir.
	 */
	void
		addMaterialInstance(MaterialInstance* materialInstance) {
		if (!materialInstance) {
			return;
		}
		if (!m_materialInstance) {
			m_materialInstance = materialInstance;
		}
		m_materialInstances.push_back(materialInstance);
	}

	/**
	 * @brief Obtiene el conjunto completo de materiales asignados a este componente.
	 * @return Referencia constante al vector de materiales.
	 */
	const std::vector<MaterialInstance*>& getMaterialInstances() const { return m_materialInstances; }

	/**
	 * @brief Comprueba si el objeto esta habilitado para ser dibujado en el Viewport.
	 * @return true si es visible, false si esta oculto.
	 */
	bool
		isVisible() const { return m_visible; }

	/**
	 * @brief Oculta o muestra el objeto en el motor de render.
	 * @param visible true para mostrar, false para ocultar.
	 */
	void
		setVisible(bool visible) { m_visible = visible; }

	/**
	 * @brief Comprueba si la malla proyecta sombras sobre el entorno.
	 * @return true si el mapeo de sombras esta activo para este componente.
	 */
	bool
		canCastShadow() const { return m_castShadow; }

	/**
	 * @brief Habilita o deshabilita la proyeccion de sombras de esta malla.
	 * @param value true para participar en el Shadow Pass, false para ignorarlo.
	 */
	void
		setCastShadow(bool value) { m_castShadow = value; }

private:
	/** @brief Puntero a la estructura geometrica (vertices e indices) cargada en la GPU. */
	Mesh* m_mesh = nullptr;

	/** @brief Acceso rapido al primer material de la malla (Material Principal). */
	MaterialInstance* m_materialInstance = nullptr;

	/** @brief Coleccion completa de materiales, util cuando un Mesh tiene diferentes sub-geometrias. */
	std::vector<MaterialInstance*> m_materialInstances;

	/** @brief Bandera maestra que le indica al RenderPipeline si debe saltarse este objeto. */
	bool m_visible = true;

	/** @brief Bandera que dicta si el objeto bloquea el paso de la luz. */
	bool m_castShadow = true;
};