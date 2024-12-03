#include "Geometry.h"
namespace MY {
	GlobalConfig globalConfig = GlobalConfig(); // 在命名空间中定义变量
}

bool OpenSoftRenderer::isPointInClipSpace(const MY::VertexShader_out& vertex, const Eigen::Vector4i& clipPlane)
{
	const Eigen::Vector4f& v = vertex.clip_pos;
	return clipPlane.x() * v.x() + clipPlane.y() * v.y() + clipPlane.z() * v.z() + clipPlane.w() * v.w() >= 0;
}

float OpenSoftRenderer::Distance_PointToPlane(const MY::VertexShader_out& vertex, const Eigen::Vector4f& clipPlane)
{
	const Eigen::Vector4f& v = vertex.clip_pos;
	return clipPlane.x() * v.x() + clipPlane.y() * v.y() + clipPlane.z() * v.z() + clipPlane.w() * v.w();
}

MY::VertexShader_out OpenSoftRenderer::ComputeIntersection(const MY::VertexShader_out& v1, const MY::VertexShader_out& v2, const float t)
{
	Eigen::Vector4f v1_PJ = v1.clip_pos;
	Eigen::Vector4f v2_PJ = v2.clip_pos;

	MY::VertexShader_out intersection;
	intersection.world_pos = v1.world_pos + t * (v2.world_pos - v1.world_pos);

	intersection.view_pos = v1.view_pos + t * (v2.view_pos - v1.view_pos);

	intersection.clip_pos = v1_PJ + t * (v2_PJ - v1_PJ);

	intersection.NDC_pos = intersection.clip_pos / intersection.clip_pos.w();

	intersection.viewport_pos.x() = (intersection.NDC_pos.x() + 1.0f) * 0.5f * (MY::globalConfig.SCR_Size.x() - 1);
	intersection.viewport_pos.y() = (intersection.NDC_pos.y() + 1.0f) * 0.5f * (MY::globalConfig.SCR_Size.y() - 1);
	intersection.viewport_pos.z() = (intersection.NDC_pos.z() + 1.0f) * 0.5f;
	intersection.viewport_pos.w() = 1.0f;

	intersection.world_normal = v1.world_normal + t * (v2.world_normal - v1.world_normal);
	intersection.vertex_uv = v1.vertex_uv + t * (v2.vertex_uv - v1.vertex_uv);

	return intersection;
}

Eigen::Vector3f OpenSoftRenderer::BarycentricFast(Eigen::Vector4f& a, Eigen::Vector4f& b, Eigen::Vector4f& c, Eigen::Vector4f& p, bool& isInLine)
{
	Eigen::Vector4f v0 = b - a, v1 = c - a, v2 = p - a;

	float d00 = v0.x() * v0.x() + v0.y() * v0.y();
	float d01 = v0.x() * v1.x() + v0.y() * v1.y();
	float d11 = v1.x() * v1.x() + v1.y() * v1.y();
	float d20 = v2.x() * v0.x() + v2.y() * v0.y();
	float d21 = v2.x() * v1.x() + v2.y() * v1.y();

	float denom = d00 * d11 - d01 * d01;
	//三角形变成了一条线
	if (abs(denom) < 0.000001)
	{
		isInLine = true;
		return -1 * Eigen::Vector3f(1, 1, 1);
	}
	else
	{
		isInLine = false;
	}

	float v = (d11 * d20 - d01 * d21) / denom;
	float w = (d00 * d21 - d01 * d20) / denom;
	float u = 1.0f - v - w;

	return Eigen::Vector3f(u, v, w);
}

int OpenSoftRenderer::EnCode(Eigen::Vector2i& pos, Eigen::Vector2i& min, Eigen::Vector2i& max)
{
	int code;
	code = LINE_INSIDE;          // initialised as being inside of clip window

	if (pos.x() < min.x())           // to the left of clip window
		code |= LINE_LEFT;
	else if (pos.x() > max.x())      // to the right of clip window
		code |= LINE_RIGHT;
	if (pos.y() < min.y())           // below the clip window
		code |= LINE_BOTTOM;
	else if (pos.y() > max.y())      // above the clip window
		code |= LINE_TOP;
	return code;
}

bool OpenSoftRenderer::isViewFrontFace(const std::vector<MY::VertexShader_out>& v_in, const int index0, const int index1, const int index2)
{
	Eigen::Vector3f edge1 = (v_in[index1].view_pos - v_in[index0].view_pos).block<3, 1>(0, 0);
	Eigen::Vector3f edge2 = (v_in[index2].view_pos - v_in[index0].view_pos).block<3, 1>(0, 0);
	Eigen::Vector3f normal = edge1.cross(edge2).normalized();
	Eigen::Vector3f viewDir = Eigen::Vector3f(0.0f, 0.0f, 1.0f) - v_in[index0].view_pos.block<3, 1>(0, 0);
	viewDir.normalize();
	return normal.dot(viewDir) > 0.0f;
}

OpenSoftRenderer::BBox_SCR::BBox_SCR(const std::vector<MY::VertexShader_out>& vIn, const int index0, const int index1, const int index2)
{
	int minX = (int)std::floor(std::min({ vIn[index0].viewport_pos[0], vIn[index1].viewport_pos[0], vIn[index2].viewport_pos[0] }));
	int maxX =  (int)std::ceil(std::max({ vIn[index0].viewport_pos[0], vIn[index1].viewport_pos[0], vIn[index2].viewport_pos[0] }));
	int minY = (int)std::floor(std::min({ vIn[index0].viewport_pos[1], vIn[index1].viewport_pos[1], vIn[index2].viewport_pos[1] }));
	int maxY =  (int)std::ceil(std::max({ vIn[index0].viewport_pos[1], vIn[index1].viewport_pos[1], vIn[index2].viewport_pos[1] }));
	max = Eigen::Vector2i(maxX, maxY);
	min = Eigen::Vector2i(minX, minY);
	max.x() = std::min(max.x(), MY::globalConfig.SCR_Size.x() - 1);
	max.y() = std::min(max.y(), MY::globalConfig.SCR_Size.y() - 1);
	min.x() = std::max(min.x(), 0);
	min.y() = std::max(min.y(), 0);
}