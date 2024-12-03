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

	//uniform�����洢
	UniformStorage uniformStorage;
	//������ɫ����Ա����ָ��	
	void (ShaderProgram::*m_vertexShader)(const MY::VertexShader_in&, MY::VertexShader_out&);
	//Ƭ����ɫ����Ա����ָ��
	void (ShaderProgram::*m_fragmentShader)(MY::FragmentShader_in&);

	//����uniform����
	void setUniform(const std::string& name, UniformValue value) {uniformStorage.setUniform(name, value);}
	//����Ƿ���uniform����
	bool hasUniform(const std::string& name) const {return uniformStorage.hasUniform(name);}
	//��ȡuniform������ֵ
	template<typename T>
	T getUniform(const std::string& name) const {return uniformStorage.getUniform(name);}


	//������Ӱ����
	float PCF(int x, int y,double depth, std::shared_ptr<std::vector<double>> shadowMap, int SearchRange);
	float PCSS(int x, int y, double depth, std::shared_ptr<std::vector<double>> shadowMap);

	//���㻷����
	Eigen::Vector3f computeAmbientColor(const sf::Color& diffColor,const float ambientIntensity);
	//����BRDF����
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