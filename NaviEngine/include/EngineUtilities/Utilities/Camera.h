#pragma once
#include "Prerequisites.h"
#include "EngineUtilities\Vectors\Vector3.h"

class
Camera {
public:
	Camera();
	~Camera() = default;

	
	void
		setLens(float fovYRadians, float aspectRatio, float nearPlane, float farPlane);


	void
		setPosition(float x, float y, float z);

	
	void
		setPosition(const EU::Vector3& pos);


	EU::Vector3
		getPosition() const { return m_position; }


	void
		lookAt(const EU::Vector3& pos,
			const EU::Vector3& target,
			const EU::Vector3& up = EU::Vector3(0, 1, 0));

	
	void
		walk(float d);

	
	void
		strafe(float d);


	void
		yaw(float radians);

	void
		pitch(float radians);


	void
		updateViewMatrix();

	
	XMMATRIX
		getView() const { return XMLoadFloat4x4(&m_view); }

	
	XMMATRIX
		getProj() const { return XMLoadFloat4x4(&m_proj); }

	
	XMMATRIX
		GetViewNoTranslation() const {
		XMMATRIX v = getView();
		// Quitar traslación (fila 4)
		v.r[3] = XMVectorSet(0, 0, 0, 1);
		return v;
	}

	
	float getFovY()   const { return m_fovY; }
	float getAspect() const { return m_aspectRatio; }
	float getNearZ()  const { return m_nearPlane; }
	float getFarZ()   const { return m_farPlane; }

	
	EU::Vector3 GetRight()   const { return m_right; }
	EU::Vector3 GetUp()      const { return m_up; }
	EU::Vector3 GetForward() const { return m_forward; }

	inline EU::Vector3
		FromXM(FXMVECTOR v) {
		XMFLOAT3 t;
		XMStoreFloat3(&t, v);
		return EU::Vector3(t.x, t.y, t.z);
	}


private:
	// Estado espacial
	EU::Vector3 m_position;

	// Basis Ortonormal (en mundo)
	EU::Vector3 m_right{ 1.0f, 0.0f, 0.0f }; 
	EU::Vector3 m_up{ 0.0f, 1.0f, 0.0f };
	EU::Vector3 m_forward{ 0.0f, 0.0f, 1.0f };

	// Matrices almacenadas
	XMFLOAT4X4 m_view{};
	XMFLOAT4X4 m_proj{};

	// Parametros de la proyeccion
	float m_fovY{ XM_PIDIV4 }; 
	float m_aspectRatio = 1.0f;
	float m_nearPlane = 0.01f; 
	float m_farPlane = 1000.0f;

	bool m_viewDirty = true;
};