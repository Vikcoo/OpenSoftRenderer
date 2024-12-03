#include <Eigen/Core>
#include <iostream>
#include "Camera.h"
#include "Geometry.h"
Camera::Camera(Eigen::Vector3f position, float pitch, float yaw, float fov, float aspectRatio, float nearPlane_z, float farPlane_z, float speed, float sensitivity)
{
	m_world_position = position;
	m_pitch = pitch;
	m_yaw = yaw;
	m_fov = fov;
	m_aspectRatio = aspectRatio;
	m_nearPlane_z = nearPlane_z;
	m_farPlane_z = farPlane_z;

	m_speed = speed;
	m_sensitivity = sensitivity;

	m_up = Eigen::Vector3f::UnitY();
	m_forward = -Eigen::Vector3f::UnitZ();
	m_right = Eigen::Vector3f::UnitX();
	m_worldUp = Eigen::Vector3f::UnitY();
}
void Camera::Reset()
{
	m_world_position = Eigen::Vector3f(0.0f, 0.0f, 2.0f);
	m_pitch = 0.0f;
	m_yaw = -90.0f;  // 俯仰和偏航角度

	m_up = Eigen::Vector3f::UnitY();
	m_forward = -Eigen::Vector3f::UnitZ();
	m_right = Eigen::Vector3f::UnitX();
}
void Camera::UpdateCameraVectors(float deltaX, float deltaY)
{
	float sensitivity = 0.15f;
	m_yaw += deltaX * sensitivity;
	m_pitch += deltaY * sensitivity;

	m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);

	float pitchRad = m_pitch * 3.14159265358979323846f / 180.0f;
	float yawRad = m_yaw * 3.14159265358979323846f / 180.0f;

	m_forward.x() = cos(pitchRad) * cos(yawRad);
	m_forward.y() = sin(pitchRad);
	m_forward.z() = cos(pitchRad) * sin(yawRad);
	m_forward.normalize();

	m_right = m_forward.cross(m_worldUp);  //世界上向量叉乘得到右向量
	m_right.normalize();

	m_up = m_right.cross(m_forward);	   //右向量叉乘得到上向量
	m_up.normalize();
}
void Camera::ProcessKeyboard(Eigen::Vector3f direction)
{
	m_world_position += direction * m_speed;
}
Eigen::Matrix4f Camera::GetViewMatrix() const
{
	Eigen::Matrix4f R = Eigen::Matrix4f::Zero();
	R << m_right.x(),   m_right.y(),	m_right.z(),	0,
		 m_up.x(),      m_up.y(),		m_up.z(),		0,
		 -m_forward.x(), -m_forward.y(),	-m_forward.z(),	0,
		 0,             0,				0,				1;
	Eigen::Matrix4f T = Eigen::Matrix4f::Zero();
	T << 1, 0, 0, -m_world_position.x(),
		 0, 1, 0, -m_world_position.y(),
		 0, 0, 1, -m_world_position.z(),
		 0, 0, 0, 1;
	return R * T;
}

Eigen::Matrix4f Camera::GetProjectionMatrix() const
{
	Eigen::Matrix4f result = Eigen::Matrix4f::Zero();

	float tanHalfFovy = tan(m_fov / 2.0f);

	result << 1.0f / (m_aspectRatio * tanHalfFovy), 0, 0, 0,
		0, 1.0f / tanHalfFovy, 0, 0,
		0, 0, -(m_farPlane_z + m_nearPlane_z) / (m_farPlane_z - m_nearPlane_z), -1.0f,
		0, 0, -2.0f * m_farPlane_z * m_nearPlane_z / (m_farPlane_z - m_nearPlane_z), 0;


	// 填充透视矩阵
	result(0, 0) = 1.0f / (m_aspectRatio * tanHalfFovy);
	result(1, 1) = 1.0f / tanHalfFovy;
	result(2, 2) = -(m_farPlane_z + m_nearPlane_z) / (m_farPlane_z - m_nearPlane_z);
	result(2, 3) = - 2 * m_farPlane_z * m_nearPlane_z / (m_farPlane_z - m_nearPlane_z);
	result(3, 2) = -1.0f;

	return result;
}