#pragma once
#include <unordered_map>
#include <string>
#include <variant>
#include <iostream>
#include <Eigen/Dense>
#include <SFML/Graphics.hpp>

using UniformValue = std::variant<
    int, 
    float, 
    Eigen::Vector2i,
    Eigen::Vector2f,
    Eigen::Vector3f, 
    Eigen::Matrix4f, 
    std::shared_ptr<sf::Image>,
    std::shared_ptr<std::vector<double>>
>;

class UniformStorage {
public:
    // 设置 Uniform
    void setUniform(const std::string& name, UniformValue value) {
        uniforms[name] = std::move(value);
    }

    // 获取 Uniform
    template<typename T>
    T getUniform(const std::string & name) const {
        if (uniforms.find(name) != uniforms.end()) {
            return std::get<T>(uniforms.at(name));
        }
        //std::cout << "Uniform not found or type mismatch: " << name << std::endl;
        return T();
    }

    // 检查 Uniform 是否存在
    bool hasUniform(const std::string& name) const {
        return uniforms.find(name) != uniforms.end();
    }

private:
    std::unordered_map<std::string, UniformValue> uniforms;
};