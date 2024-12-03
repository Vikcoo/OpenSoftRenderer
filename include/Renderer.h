#pragma once
#include "Shader.h"
#include "World.h"
#include "Camera.h"
#include <SFML/Graphics.hpp>
#include <Eigen/Dense>
#include "Camera.h"
class Renderer
{
public:
	//ʱ��
	sf::Clock m_Clock;
	
	Renderer();
	~Renderer() = default;

	std::shared_ptr<ShaderProgram> GetShaderProgram() { return m_CurrentShaderProgram; }
	void SetShaderProgram(std::shared_ptr<ShaderProgram> shaderProgram) { m_CurrentShaderProgram = shaderProgram; }
	std::shared_ptr<std::vector<double>> GetShadowMap() { return m_shadowMap; }

	void rasterize(
		std::vector<MY::VertexShader_out>& vIn,
		const int index0,
		const int index1,
		const int index2,
		std::vector<MY::Rasterization_out>& rasterOut,
		std::vector<Eigen::Vector2i>& rasterIdxOut,
		std::vector<double>& zBuf
	);
	void rasterizeShadowMap(
		std::vector<MY::VertexShader_out>& vIn,
		const int index0,
		const int index1,
		const int index2,
		std::shared_ptr<std::vector<double>>& shadowMap
	);
	
	void renderModel(std::shared_ptr<Model> model, sf::Image& image);
	void renderShadowMap(std::shared_ptr<Model> model);


	void renderTriangle(std::vector<MY::VertexShader_out>& vIn,const int index0,const int index1,const int index2,sf::Image& image);
	void drawLine(Eigen::Vector2i start, Eigen::Vector2i end, sf::Image& image, sf::Color c);
	void CohenSutherlandLineClip(Eigen::Vector2i& start, Eigen::Vector2i& end, Eigen::Vector2i& min, Eigen::Vector2i& max);

	void TriangleTrim(std::vector<MY::VertexShader_out>& vIn, const int index0, const int index1, const int index2, std::vector<MY::VertexShader_out>& triangleTrimOut);
	std::vector<MY::VertexShader_out> ClipTriangle(const std::vector<MY::VertexShader_out>& triangle, const Eigen::Vector4f& plane);

	void ClearDepthBuffer() { std::fill(m_zBuf.begin(), m_zBuf.end(), 1.0f); }
	void ClearShadowMap() { std::fill(m_shadowMap->begin(), m_shadowMap->end(), -9999999.0f); }
private:

	std::shared_ptr<Camera> m_Camera;						
	std::shared_ptr<ShaderProgram> m_CurrentShaderProgram;	// ��ǰ��ɫ������
	std::vector<double> m_zBuf;								// ��Ȼ���
	std::vector<MY::VertexShader_out> m_VertexShaderOuts;	// ������ɫ�����  ͳһ�洢
	std::vector<MY::Rasterization_out> m_RasterizeOut;		// ��դ����� ��vector��ʾһ��ͼ
	std::vector<Eigen::Vector2i> m_RasterizeIndex;			// ��դ����־����ʾ��Щ���ر���դ��ѡ��
	std::vector<MY::VertexShader_out> m_TriangleTrimOut;	// �¶�������

	std::shared_ptr<std::vector<double>> m_shadowMap;		// ��Ӱ��Ȼ���
};