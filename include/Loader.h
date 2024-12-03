#pragma once
#include <string>
#include "Model.h"
class Loader
{
public:
	Loader() = default;
	~Loader() = default;
	static std::shared_ptr<Model> LoadObj(const std::string& filename);
};