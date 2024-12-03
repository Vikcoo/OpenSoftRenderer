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
	static Application* s_Instance;					//����ģʽ

	sf::RenderWindow m_Window;						//��Ⱦ����

	sf::Image m_Image;								//��Ⱦͼ��
	sf::Texture m_Texture;							//��Ⱦ����
	sf::Sprite m_Sprite;							//��Ⱦ����

	World m_World;									//���糡��

	std::shared_ptr<Camera> m_Camera;				//�����
	std::shared_ptr<Camera> m_LightCamera;			//���������

	Renderer m_Renderer;							//��Ⱦ��

	sf::Clock m_Clock;								//ʱ��
	float m_LastFrameTime = 0.0f;					//��һ֡ʱ��
	float m_DeltaTime = 0.0f;						//ʱ����
	float m_MouseSpeed = 200.0f;					//����ٶ�
	
	//debug
	std::shared_ptr<sf::Image> m_diff_texture;				//����
	std::shared_ptr<sf::Image> m_spec_texture;				//�߹�����
	std::shared_ptr<sf::Image> m_normal_texture;			//����ָ��
		
	Application() = default;
	void processEvents();
};