#include "World.h"

World::World()
{
    m_models = std::vector<std::shared_ptr<Model>>();
    m_modelIndexMap = std::unordered_map<std::string, int>();
}

void World::addModel(const std::string& filename)
{
    std::shared_ptr<Model> model = Loader::LoadObj(filename);
    m_models.push_back(model);
    m_modelIndexMap[model->getName()] = m_models.size() - 1;
}

std::shared_ptr<Model> World::getModel(const std::string& name)
{
    auto it = m_modelIndexMap.find(name);
    if (it != m_modelIndexMap.end()) {
        return m_models[it->second];
    }
    return nullptr;
}
