#pragma once
#include <vector>
#include <Eigen/Dense>
#include <string>
#include "Geometry.h"
#include "Uniform.h"
#include "tiny_obj_loader.h"
#include "Shader.h"
#define TINYOBJLOADER_IMPLEMENTATION

class Model
{
public:
	Model();
	~Model() = default;

	unsigned int vertexCount = 0;

	void addMaterial(const MY::Material & material);
	void addMesh(const MY::Mesh& mesh);
	void setName(const std::string& name);
	const std::string& getName() const;
	std::vector<MY::Mesh>& getMeshes();
	std::vector<MY::Material>& getMaterials();
	std::shared_ptr<ShaderProgram> getShaderProgram();
	void setShaderProgramUniform(const std::string& name, UniformValue value);

private:
	std::string m_name;
	std::vector<MY::Mesh> m_meshes;
	std::vector<MY::Material> m_materials;
	std::shared_ptr<ShaderProgram> m_shaderProgram;
};