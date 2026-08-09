/**
 * @file RenderScene.cpp
 * @brief Implementa la logica de RenderScene dentro del subsistema Rendering.
 * @ingroup rendering
 */
#include "Rendering/RenderScene.h"

void
RenderScene::clear() {
	opaqueObjects.clear();
	transparentObjects.clear();
	directionalLights.clear();
	skybox = nullptr;

	// Limpiamos la lista de partículas en cada frame
	particleEmitters.clear();
}