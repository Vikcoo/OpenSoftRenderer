#include "Shader.h"
#include "Geometry.h"
#include "Eigen/Dense"
#include <iostream>
ShaderProgram::ShaderProgram()
{
	uniformStorage = UniformStorage();
	Eigen::Matrix4f identity = Eigen::Matrix4f::Identity();
	uniformStorage.setUniform("Model", identity);
	uniformStorage.setUniform("View", identity);
	uniformStorage.setUniform("Projection", identity);

	m_vertexShader = &ShaderProgram::vertexShader3D;
	m_fragmentShader = &ShaderProgram::fragmentShader;
}

float ShaderProgram::PCF(int x, int y, double depth, std::shared_ptr<std::vector<double>> shadowMap, int SearchRange)
{
	int width = MY::globalConfig.SCR_Size.x();
	int height = MY::globalConfig.SCR_Size.y();
	int halfSearchRange = SearchRange / 2;
	int searchCount = 0;
	int shadowCount = 0;
	for (int i = -halfSearchRange; i < halfSearchRange; i++) {
		for (int j = -halfSearchRange; j < halfSearchRange; j++) {
			int shadowX = x + i;
			int shadowY = y + j;
			int index = shadowX + shadowY * width;
			if (shadowX < 0 || shadowX >= width || shadowY < 0 || shadowY >= height) {
				continue;
			}
			searchCount++;
			if ((*shadowMap)[index] - depth > MY::globalConfig.shadowOffset) {
				shadowCount++;
			}
		}
	}
	return 1.0f - (float)shadowCount/(float)searchCount;
}

float ShaderProgram::PCSS(int x, int y, double depth, std::shared_ptr<std::vector<double>> shadowMap)
{
	int shadowMapWidth = MY::globalConfig.SCR_Size.x();
	int shadowMapHeight = MY::globalConfig.SCR_Size.y();

	int blockSearchSize = 5;
	int halfBlockSearchSize = int(blockSearchSize / 2);
	int blockCount = 0;
	double sumBlockerDepth = 0.0f;

	int lightRadius = 41;
	int searchRange = 0;

	//被遮挡
	for (int i = -halfBlockSearchSize; i < halfBlockSearchSize; i++) {
		for (int j = -halfBlockSearchSize; j < halfBlockSearchSize; j++) {
			int shadowX = x + i;
			int shadowY = y + j;
			int index = shadowX + shadowY * shadowMapWidth;
			if (shadowX < 0 || shadowX >= shadowMapWidth || shadowY < 0 || shadowY >= shadowMapHeight) {
				continue;
			}
			double shadowDepth = (*shadowMap)[index];
			if(shadowDepth - depth > MY::globalConfig.shadowOffset){
				blockCount++;
				sumBlockerDepth += shadowDepth;
			}
		}
	}

	if (blockCount == 0) {
		return 1.0f;
	}

	double avgBlockerDepth = sumBlockerDepth / blockCount; //计算blocker深度的平均值

	searchRange = lightRadius * ((depth - avgBlockerDepth) / avgBlockerDepth);
	searchRange = std::max(searchRange, 1);

	//PCF
	int halfSearchRange = int(searchRange / 2);
	int searchCount = 0;
	int shadowCount = 0;
	for (int i = -halfSearchRange; i < halfSearchRange; i++) {
		for (int j = -halfSearchRange; j < halfSearchRange; j++) {
			int shadowX = x + i;
			int shadowY = y + j;
			int index = shadowX + shadowY * shadowMapWidth;
			if (shadowX < 0 || shadowX >= shadowMapWidth || shadowY < 0 || shadowY >= shadowMapHeight) {
				continue;
			}
			searchCount++;
			if ((*shadowMap)[index] - depth > MY::globalConfig.shadowOffset) {
				shadowCount++;
			}
		}
	}
	return  1.0f - (float)shadowCount / (float)searchCount;
}

Eigen::Vector3f ShaderProgram::computeAmbientColor(const sf::Color& diffTexColor, const float ambientIntensity)
{
	return Eigen::Vector3f( diffTexColor.r * ambientIntensity, diffTexColor.g * ambientIntensity, diffTexColor.b * ambientIntensity);
}

Eigen::Vector3f ShaderProgram::computeBRDF(const sf::Color& diffTexColor, const float roughness, const sf::Color& specClolr, const float metallic, const int concentration, const Eigen::Vector3f& normal, const Eigen::Vector3f& lightDir, const Eigen::Vector3f& viewDir)
{
	
	Eigen::Vector3f diffColor = Eigen::Vector3f(diffTexColor.r, diffTexColor.g, diffTexColor.b);
	Eigen::Vector3f specColor = Eigen::Vector3f(specClolr.r, specClolr.g, specClolr.b);

	Eigen::Vector3f halfwayDir = (lightDir + viewDir).normalized();
	float cosTheta = std::max( 0.0f, normal.dot(lightDir));
	float cosAlpha = std::max(0.0f, normal.dot(halfwayDir));

	float diffFactor = roughness;
	float visiblitiy = 1.0f;
	float NDF = metallic * pow(cosAlpha, concentration);
	float specFactor = metallic * visiblitiy * NDF;
	//Eigen::Vector3f L = ((diffColor / M_PI) + (concentration + 8.0f) / M_PI * specColor * pow(cosAlpha, concentration)) * cosTheta;
	Eigen::Vector3f L = (diffColor * diffFactor + specColor * specFactor) * cosTheta;

	if (L.x() > 255.0f) {
		L.x() = 255.0f;
	}
	if (L.y() > 255.0f) {
		L.y() = 255.0f;
	}
	if (L.z() > 255.0f) {
		L.z() = 255.0f;
	}

	return L;
}

void ShaderProgram::vertexShader3D(const MY::VertexShader_in& in, MY::VertexShader_out& out)
{
	Eigen::Matrix4f model = uniformStorage.getUniform<Eigen::Matrix4f>("Model");
	Eigen::Matrix4f view = uniformStorage.getUniform<Eigen::Matrix4f>("View");
	Eigen::Matrix4f projection = uniformStorage.getUniform<Eigen::Matrix4f>("Projection");

	out.model_pos = in.model_pos.homogeneous();
	//model to world
	out.world_pos = model * out.model_pos;
	//world to view 
	out.view_pos = view *out.world_pos;
	//view to clip 
	out.clip_pos = projection * out.view_pos;
	//clip to NDC
	out.NDC_pos = out.clip_pos / out.clip_pos.w();
	//NDC to viewport
	out.viewport_pos.x() = (out.NDC_pos.x() + 1.0f) * 0.5f * (MY::globalConfig.SCR_Size.x() - 1);
	out.viewport_pos.y() = (out.NDC_pos.y() + 1.0f) * 0.5f * (MY::globalConfig.SCR_Size.y() - 1);
	out.viewport_pos.z() = (out.NDC_pos.z() + 1.0f) * 0.5f;
	out.viewport_pos.w() = 1.0f;

	// normal
	Eigen::Matrix4f M_inv_transpose = model.inverse().transpose();
	out.world_normal = M_inv_transpose.block<3, 3>(0, 0) * in.model_normal;
	out.world_normal.normalize();

	// uv
	out.vertex_uv = in.vertex_texcoord;
}

void ShaderProgram::fragmentShader(MY::FragmentShader_in& fragmentShader_in) {

	// 获取所有纹理贴图
	std::shared_ptr<sf::Image> diffTex = uniformStorage.getUniform<std::shared_ptr<sf::Image>>("diffTexture");
	std::shared_ptr<sf::Image> specTex = uniformStorage.getUniform<std::shared_ptr<sf::Image>>("specTexture");
	std::shared_ptr<sf::Image> normalTex = uniformStorage.getUniform<std::shared_ptr<sf::Image>>("normalTexture");
	std::shared_ptr<std::vector<double>> shadowMap = uniformStorage.getUniform<std::shared_ptr<std::vector<double>>>("shadowMap");
	
	// 像素的纹理坐标
	Eigen::Vector2f uv = fragmentShader_in.fragment_uv;
	uv.x() = std::clamp(uv.x(), 0.0f, 1.0f);
	uv.y() = std::clamp(uv.y(), 0.0f, 1.0f);
	uv.x() = uv.x() * (diffTex->getSize().x - 1);
	uv.y() = (1 - uv.y()) * (diffTex->getSize().y - 1);
	uv.x() = std::floor(uv.x());
	uv.y() = std::floor(uv.y());

	//漫反射颜色
	sf::Color diffTexColor;
	if (diffTex == nullptr) {		
		fragmentShader_in.fragment_color = sf::Color::White;
		return;
	}
	else {
		diffTexColor = diffTex->getPixel(uv.x(), uv.y());
	}
	
	if (!hasUniform("CameraPos") || !hasUniform("LightPos")) {// 没光照
		fragmentShader_in.fragment_color = diffTexColor; 
		return;
	}
	Eigen::Vector3f farg_w_pos = fragmentShader_in.fragment_world_position.block<3, 1>(0, 0);
	Eigen::Vector3f camera_w_Pos = uniformStorage.getUniform<Eigen::Vector3f>("CameraPos");
	Eigen::Vector3f light_w_Pos = uniformStorage.getUniform<Eigen::Vector3f>("LightPos");
	Eigen::Vector3f lightDir = (light_w_Pos - farg_w_pos).normalized();
	Eigen::Vector3f viewDir = (camera_w_Pos - farg_w_pos).normalized();
	Eigen::Vector3f worldNormal;
	if (normalTex == nullptr) // 没有法线贴图
	{
		worldNormal = fragmentShader_in.fragment_world_normal;
	}
	else {//有线贴图 法线贴图采样
		sf::Color nor = normalTex->getPixel(uv.x(), uv.y());
		Eigen::Vector3f normal;
		normal = Eigen::Vector3f(nor.r / 255.0f, nor.g / 255.0f, nor.b / 255.0f);
		normal = normal * 2.0f - Eigen::Vector3f(1.0f, 1.0f, 1.0f);
		normal.normalize();
		Eigen::Matrix3f TBN = Eigen::Matrix3f::Identity();
		/*TBN.col(0) = fragmentShader_in.fragment_tangent;
		TBN.col(1) = fragmentShader_in.fragment_bitangent;
		TBN.col(2) = fragmentShader_in.fragment_world_normal;*/
		worldNormal = TBN * normal;  // 切线空间到世界空间的变换
		worldNormal.normalize();
	}

	// 环境光
	float ambientfactor = 0.1f;
	Eigen::Vector3f ambientColor = computeAmbientColor(diffTexColor, ambientfactor);
	
	// 镜面反射颜色
	sf::Color specColor;
	if (specTex == nullptr){
		specColor = sf::Color::White;
	}
	else {
		specColor = specTex->getPixel(uv.x(), uv.y());
	}
	
	Eigen::Vector3f L = computeBRDF(diffTexColor, 0.8f, specColor, 0.8f, 32, worldNormal, lightDir, viewDir);
	
	//是否在阴影中
	float shadowFactor = 1.0f;
	if (MY::globalConfig.isShadow && shadowMap != nullptr) {
		Eigen::Matrix4f LightView = uniformStorage.getUniform<Eigen::Matrix4f>("LightView");
		Eigen::Matrix4f LightProjection = uniformStorage.getUniform<Eigen::Matrix4f>("LightProjection");
		Eigen::Vector4f pos_in_lightPosView = LightView * fragmentShader_in.fragment_world_position;
		Eigen::Vector4f pos_in_lightPosProjection = LightProjection * pos_in_lightPosView;
		Eigen::Vector4f pos_in_lightPosNDC = pos_in_lightPosProjection /= pos_in_lightPosProjection.w();
		int x = int((pos_in_lightPosProjection.x() + 1.0f) * 0.5f * (MY::globalConfig.SCR_Size.x() - 1));
		int y = int((pos_in_lightPosProjection.y() + 1.0f) * 0.5f * (MY::globalConfig.SCR_Size.y() - 1));
		if (x < 0 || x >= MY::globalConfig.SCR_Size.x() || y < 0 || y >= MY::globalConfig.SCR_Size.y()) {
			//当前像素的世界位置不在shadowMap范围内，视为不在阴影中
		}
		else {
			double depth = pos_in_lightPosView.z();
			double shadowMapDepth = (*shadowMap)[x + y * MY::globalConfig.SCR_Size.x()];
			if (shadowMapDepth - depth > MY::globalConfig.shadowOffset) {
				shadowFactor = 0.0f;
				if (MY::globalConfig.isPCF) shadowFactor = PCF(x, y, depth, shadowMap, MY::globalConfig.PCF_step);
				if (MY::globalConfig.isPCSS) shadowFactor = PCSS(x, y, depth, shadowMap);
			}
		}
	}

	fragmentShader_in.fragment_color = sf::Color(
		L.x() * shadowFactor, 
		L.y() * shadowFactor, 
		L.z() * shadowFactor);
}
