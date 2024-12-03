#include "Model.h"

Model::Model()
{
	m_name = "Default Model";
	m_meshes = std::vector<MY::Mesh>();
	m_materials = std::vector<MY::Material>();
	m_shaderProgram = std::make_shared<ShaderProgram>();
}

void Model::addMaterial(const MY::Material& material)
{
	m_materials.push_back(material);
}

void Model::addMesh(const MY::Mesh& mesh)
{
	m_meshes.push_back(mesh);
}

void Model::setName(const std::string& name)
{
	m_name = name;
}

const std::string& Model::getName() const
{
	return m_name;
}

std::vector<MY::Mesh>& Model::getMeshes()
{
	return m_meshes;
}

std::vector<MY::Material>& Model::getMaterials()
{
	return m_materials;
}

std::shared_ptr<ShaderProgram> Model::getShaderProgram()
{
	return m_shaderProgram;
}

void Model::setShaderProgramUniform(const std::string& name, UniformValue value)
{
	m_shaderProgram->setUniform(name, value);
}
