#pragma once
#include <SFML/Graphics.hpp>
#include "World.h"
#include "Renderer.h"
#include "Camera.h"

class Application
{
public:
	~Application() = default;

	static Application* GetInstance();
	void addObjToWorld(const std::string& filename);
	bool init(const sf::String& title = "Soft Renderer");
	void run();

	void OnMouseMove_Frame();
	void OnKeyPressed_Frame();
	void OnKeyPressed_Event(sf::Event& event);

	//debug
	void loadDiffTexture(const std::string& filename);
	void loadSpecTexture(const std::string& filename);
	void loadNormalTexture(const std::string& filename);

private:
	static Application* s_Instance;					//单例模式

	sf::RenderWindow m_Window;						//渲染窗口

	sf::Image m_Image;								//渲染图像
	sf::Texture m_Texture;							//渲染纹理
	sf::Sprite m_Sprite;							//渲染精灵

	World m_World;									//世界场景

	std::shared_ptr<Camera> m_Camera;				//摄像机
	std::shared_ptr<Camera> m_LightCamera;			//调试摄像机

	Renderer m_Renderer;							//渲染器

	sf::Clock m_Clock;								//时钟
	float m_LastFrameTime = 0.0f;					//上一帧时间
	float m_DeltaTime = 0.0f;						//时间间隔
	float m_MouseSpeed = 200.0f;					//鼠标速度
	
	//debug
	std::shared_ptr<sf::Image> m_diff_texture;				//纹理
	std::shared_ptr<sf::Image> m_spec_texture;				//高光纹理
	std::shared_ptr<sf::Image> m_normal_texture;			//纹理指针
		
	Application() = default;
	void processEvents();
};