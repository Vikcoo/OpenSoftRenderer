#pragma once
#include <Eigen/Dense>
#include "Geometry.h"
#include <SFML/Graphics.hpp>
#include "Uniform.h"
class ShaderProgram
{
public:
	ShaderProgram();
	~ShaderProgram() = default;

	//uniform变量存储
	UniformStorage uniformStorage;
	//顶点着色器成员函数指针	
	void (ShaderProgram::*m_vertexShader)(const MY::VertexShader_in&, MY::VertexShader_out&);
	//片段着色器成员函数指针
	void (ShaderProgram::*m_fragmentShader)(MY::FragmentShader_in&);

	//设置uniform变量
	void setUniform(const std::string& name, UniformValue value) {uniformStorage.setUniform(name, value);}
	//检查是否有uniform变量
	bool hasUniform(const std::string& name) const {return uniformStorage.hasUniform(name);}
	//获取uniform变量的值
	template<typename T>
	T getUniform(const std::string& name) const {return uniformStorage.getUniform(name);}


	//计算阴影因子
	float PCF(int x, int y,double depth, std::shared_ptr<std::vector<double>> shadowMap, int SearchRange);
	float PCSS(int x, int y, double depth, std::shared_ptr<std::vector<double>> shadowMap);

	//计算环境光
	Eigen::Vector3f computeAmbientColor(const sf::Color& diffColor,const float ambientIntensity);
	//计算BRDF光照
	Eigen::Vector3f computeBRDF(
		const sf::Color& diffTexColor,
		const float roughness,
		const sf::Color& specClolr,
		const float metallic,
		const int concentration,
		const Eigen::Vector3f& normal, 
		const Eigen::Vector3f& lightDir, 
		const Eigen::Vector3f& viewDir);

	//vs
	void vertexShader3D(const MY::VertexShader_in& in, MY::VertexShader_out& out);
	//fs
	void fragmentShader(MY::FragmentShader_in& fragmentShader_in);
};