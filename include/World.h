#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include "Model.h"
#include "Loader.h"
class World
{
public:
	World();
	~World() = default;

	void addModel(const std::string& filename);

	std::shared_ptr<Model> getModel(const std::string& name);
	std::vector<std::shared_ptr<Model>>& getModels() { return m_models; }
private:
	std::vector<std::shared_ptr<Model>> m_models;
	std::unordered_map<std::string, int> m_modelIndexMap;
};