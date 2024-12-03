#pragma once
#include <Eigen/Dense>
class Camera
{
public:
	Camera() = default;
	Camera(Eigen::Vector3f position, 
		float pitch, float yaw,
		float fov, float aspectRatio, 
		float nearPlane_z, 
		float farPlane_z, 
		float speed, 
		float sensitivity);
	~Camera() = default;

	void SetWorldPosition(Eigen::Vector3f position) { m_world_position = position; }
	void SetFov(float fov) { m_fov = fov; };
	void SetAspectRatio(float aspectRatio) { m_aspectRatio = aspectRatio; };
	void SetNearPlane(float nearPlane) { m_nearPlane_z = nearPlane; };
	void SetFarPlane(float farPlane) { m_farPlane_z = farPlane; };
	void SetForward(Eigen::Vector3f forward) { m_forward = forward; }
	void SetRight(Eigen::Vector3f right) { m_right = right; }
	void SetUp(Eigen::Vector3f up) { m_up = up; }
	void SetPitch(float pitch) { m_pitch = pitch; }
	void SetYaw(float yaw) { m_yaw = yaw; }

	Eigen::Vector3f GetWorldPosition() const{ return m_world_position; }
	float GetFov() const { return m_fov; };
	float GetNearPlane() const { return m_nearPlane_z; };
	float GetFarPlane() const { return m_farPlane_z; };
	Eigen::Vector3f GetForward() const { return m_forward; }
	Eigen::Vector3f GetRight() const { return m_right; }
	Eigen::Vector3f GetUp() const { return m_up; }
	float GetPitch() const { return m_pitch; }
	

	void Reset();

	void UpdateCameraVectors(float deltaX, float deltaY);

	void ProcessKeyboard(Eigen::Vector3f direction);


	Eigen::Matrix4f GetViewMatrix() const;
	Eigen::Matrix4f GetProjectionMatrix() const;
	//Eigen::Vector4f GetViewportMatrix() const;

private:
	Eigen::Vector3f m_world_position;
	float m_pitch;
	float m_yaw;  // ¸©ÑöºÍÆ«º½½Ç¶È
	float m_fov;
	float m_aspectRatio;
	float m_nearPlane_z;
	float m_farPlane_z;

	float m_speed;
	float m_sensitivity;

	Eigen::Vector3f m_up;
	Eigen::Vector3f m_forward;
	Eigen::Vector3f m_right;
	Eigen::Vector3f m_worldUp;
};