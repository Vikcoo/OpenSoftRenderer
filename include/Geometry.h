#pragma once
#include "Eigen/Dense"
#include <SFML/Graphics.hpp>
#define MY OpenSoftRenderer
#define M_PI 3.14159265358979323846
#define LINE_INSIDE		0
#define LINE_LEFT		1 << 0
#define LINE_RIGHT		1 << 1
#define LINE_BOTTOM		1 << 2
#define LINE_TOP		1 << 3
namespace OpenSoftRenderer {

	struct GlobalConfig			// 全局参数配置
	{
		//背部剔除
		bool isBackFaceCulling = true;
		//EarlyZ测试
		bool isEarlyZ = true;
		//深度测试
		bool isDepthTest = false;
		//深度测试写入
		bool isDepthWrite = false;
		//裁切线可见性
		bool isClipLine = false;
		//顶点法线可见性
		bool isShowVertexNormal = false;
		
		//阴影
		bool isShadow = true;
		//阴影偏置
		float shadowOffset = 0.04f;
		//PCF阴影
		bool isPCF = false;
		//PCF阴影步长
		int PCF_step = 11;
		//PCSS阴影
		bool isPCSS = false;

		//屏幕大小
		Eigen::Vector2i SCR_Size = Eigen::Vector2i(800, 600);

		//相机参数
		Eigen::Vector3f camera_initial_position = Eigen::Vector3f(0.0f, 0.0f, 5.0f);
		float camera_pitch = 0.0f;
		float camera_yaw = -90.0f;
		float camera_fov = 45.0f;
		float camera_aspectRatio = static_cast<float>(SCR_Size.x()) / static_cast<float>(SCR_Size.y());
		float camera_nearPlane_z = 0.1f;
		float camera_farPlane_z = 100.0f;
		float camera_speed = 4.5f;
		float camera_sensitivity = 0.1f;
	};

	extern GlobalConfig globalConfig;

	struct Vertex {						// 顶点
		Eigen::Vector3f model_pos;
		Eigen::Vector3f model_normal;
		Eigen::Vector2f vertex_texcoord;
	};
	struct TriangleFace {				// 三角面
		std::vector<Eigen::Vector3f> vertices;
	};
	struct Mesh {						
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::string material_name;
	};
	struct Material {					// 材质
		std::string name;
		Eigen::Vector3f diffuse_color;
		Eigen::Vector3f specular_color;
		Eigen::Vector3f ambient_color;
		std::string diffuse_texture;
	};
	
	typedef Vertex VertexShader_in;	

	struct VertexShader_out				
	{
		Eigen::Vector4f model_pos;
		Eigen::Vector4f world_pos;
		Eigen::Vector4f view_pos;
		Eigen::Vector4f clip_pos;

		Eigen::Vector4f NDC_pos;

		Eigen::Vector4f viewport_pos;

		Eigen::Vector2f vertex_uv;

		Eigen::Vector3f world_normal;

		Eigen::Vector3f tangent;

		Eigen::Vector3f bitangent;
	};

	struct Rasterization_out
	{
		Eigen::Vector2i viewport_pos;
		double fragment_depth = 0.0f;
		Eigen::Vector4f fragment_world_position;
		Eigen::Vector3f fragment_world_normal;
		Eigen::Vector2f fragment_uv;
		sf::Color fragment_color;
	};
	typedef Rasterization_out FragmentShader_in;	

	struct BBox_SCR {								// 屏幕空间包围盒
		Eigen::Vector2i min, max;
		BBox_SCR(const std::vector<MY::VertexShader_out>& vIn, const int index0, const int index1, const int index2);
	};

	// 判断点是否在裁剪空间外
	bool isPointInClipSpace(const MY::VertexShader_out& vertex, const Eigen::Vector4i& clipPlane);
	// 判断点到平面的距离
	float Distance_PointToPlane(const MY::VertexShader_out& vertex, const Eigen::Vector4f& clipPlane);
	// 三角形裁剪边界编码
	int EnCode(Eigen::Vector2i& pos, Eigen::Vector2i& min, Eigen::Vector2i& max);

	// 插值
	MY::VertexShader_out ComputeIntersection(const MY::VertexShader_out& v1, const MY::VertexShader_out& v2, const float t);

	// 求重心坐标
	Eigen::Vector3f BarycentricFast(Eigen::Vector4f& a, Eigen::Vector4f& b, Eigen::Vector4f& c, Eigen::Vector4f& p, bool& isInLine);
	
	// 判断是否正面
	bool isViewFrontFace(const std::vector<MY::VertexShader_out>& v_in, const int index0, const int index1, const int index2);
}