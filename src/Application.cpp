#include "Application.h"
#include <SFML/Graphics.hpp>
#include <filesystem>
#include <iostream>
#include "Camera.h"
Application* Application::s_Instance = nullptr;
void Application::processEvents()
{
    //std::cout << "\033[2J\033[H";//����
    //ÿ֡�¼�
    m_DeltaTime = m_Clock.getElapsedTime().asSeconds() - m_LastFrameTime; //��ȡʱ����
    m_LastFrameTime = m_Clock.getElapsedTime().asSeconds(); //��¼��һ֡ʱ��
    std::cout << "FPS: " << 1 / m_DeltaTime << std::endl;
    //std::cout << "dt: " << m_DeltaTime << std::endl;
    OnMouseMove_Frame();//ÿ֡��������ƶ��������λ��
    OnKeyPressed_Frame();
    

    //�����������¼�
    sf::Event event;
    while (m_Window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            m_Window.close();
        }
        // ���������¼�
        if (event.type == sf::Event::KeyPressed) {
            OnKeyPressed_Event(event);
        }
    }
}
Application* Application::GetInstance()
{
    if (s_Instance == nullptr) {
        s_Instance = new Application();
    }
    return s_Instance;
}

void Application::addObjToWorld(const std::string& filename)
{
    m_World.addModel(filename);
}

bool Application::init(const sf::String& title)
{
    //����
    m_Window.create(sf::VideoMode(MY::globalConfig.SCR_Size.x(), MY::globalConfig.SCR_Size.y()), title);
    //m_Window.setMouseCursorVisible(false); //���������
    //m_Window.setMouseCursorGrabbed(true);  //����������ڴ�����
    sf::Mouse::setPosition(sf::Vector2i(MY::globalConfig.SCR_Size.x() / 2, MY::globalConfig.SCR_Size.y() / 2), m_Window); //���λ�ó�ʼ��
    //������Ⱦ��Դ
    m_Image.create(MY::globalConfig.SCR_Size.y(), MY::globalConfig.SCR_Size.y(), sf::Color::Black);

    //����
    m_World = World();

    //�����
    m_Camera = std::make_shared<Camera>(
        MY::globalConfig.camera_initial_position, 
        MY::globalConfig.camera_pitch,
        MY::globalConfig.camera_yaw,
        MY::globalConfig.camera_fov,
        MY::globalConfig.camera_aspectRatio,
        MY::globalConfig.camera_nearPlane_z,
        MY::globalConfig.camera_farPlane_z,
        MY::globalConfig.camera_speed,
        MY::globalConfig.camera_sensitivity
    );

    m_LightCamera = std::make_shared<Camera>(
        Eigen::Vector3f(0.0f, 0.0f, 3.0f),
        MY::globalConfig.camera_pitch,
        MY::globalConfig.camera_yaw,
        MY::globalConfig.camera_fov,
        MY::globalConfig.camera_aspectRatio,
        MY::globalConfig.camera_nearPlane_z,
        MY::globalConfig.camera_farPlane_z,
        MY::globalConfig.camera_speed,
        MY::globalConfig.camera_sensitivity
    );  

    m_LightCamera->SetWorldPosition(Eigen::Vector3f(0.0f, 3.0f, 5.0f));
    m_LightCamera->SetPitch(-35.0f);
    m_LightCamera->UpdateCameraVectors(0, 0);

    m_Camera->SetWorldPosition(Eigen::Vector3f(1.3f, 2.0f, 0.3f));
    m_Camera->SetPitch(-35.0f);
    m_Camera->SetYaw(-120.0f);
    m_Camera->UpdateCameraVectors(0, 0);

    //��Ⱦ��
    m_Renderer = Renderer();

    //debug
    m_diff_texture = std::make_shared<sf::Image>();
    m_spec_texture = std::make_shared<sf::Image>();
    m_normal_texture = std::make_shared<sf::Image>();
    return true;
}
void Application::run()
{
    while (m_Window.isOpen())
    {
        //������Ⱦ�޹ص�ÿ֡�¼��ʹ������¼�
        this->processEvents();
        //��Ⱦǰ������buffer
        m_Image.create(MY::globalConfig.SCR_Size.x(), MY::globalConfig.SCR_Size.y(), sf::Color(50, 50, 50, 255));
        m_Renderer.ClearDepthBuffer();
        m_Renderer.ClearShadowMap();

        /*--------------------����--------------------*/
        float scale = 1.0f;
        Eigen::Matrix4f S = Eigen::Matrix4f::Identity();
        Eigen::Matrix4f R = Eigen::Matrix4f::Identity();
        Eigen::Matrix4f T = Eigen::Matrix4f::Identity();
        Eigen::Matrix4f modelMatrix = Eigen::Matrix4f::Identity();

        Eigen::Matrix4f lightViewMatrix = m_LightCamera->GetViewMatrix();
        Eigen::Matrix4f lightProjectionMatrix = m_LightCamera->GetProjectionMatrix();
        Eigen::Matrix4f cameraViewMatrix = m_Camera->GetViewMatrix();
        Eigen::Matrix4f cameraProjectionMatrix = m_Camera->GetProjectionMatrix();

        float sinT = sin(m_Clock.getElapsedTime().asSeconds());
        float cosT = cos(m_Clock.getElapsedTime().asSeconds());
        float sin90 = sin(-90 * M_PI / 180);
        float cos90 = cos(-90 * M_PI / 180);

        std::shared_ptr<Model> M = m_World.getModel("floor");
        std::shared_ptr<Model> triModel = m_World.getModel("african_head");
        if (M == nullptr) {
            std::cout << "Model not found!" << std::endl;
            continue;
        }
        if (triModel == nullptr) {
            std::cout << "Model not found!" << std::endl;
            continue;
        }

       
        // TEXTURE
        M->setShaderProgramUniform("diffTexture", m_diff_texture);
        M->setShaderProgramUniform("shadowMap", m_Renderer.GetShadowMap());
        
        triModel->setShaderProgramUniform("diffTexture", m_diff_texture);
        triModel->setShaderProgramUniform("shadowMap", m_Renderer.GetShadowMap());
        //triModel->getShaderProgram()->setUniform("specTexture", m_spec_texture);
        //triModel->getShaderProgram()->setUniform("normalTexture", m_normal_texture);
        
        // transfrom matrix 
        M->setShaderProgramUniform("CameraPos", m_Camera->GetWorldPosition());
        M->setShaderProgramUniform("LightPos", m_LightCamera->GetWorldPosition());
        M->setShaderProgramUniform("LightView", lightViewMatrix);
        M->setShaderProgramUniform("LightProjection", lightProjectionMatrix);

        triModel->setShaderProgramUniform("CameraPos", m_Camera->GetWorldPosition());
        triModel->setShaderProgramUniform("LightPos", m_LightCamera->GetWorldPosition());
        triModel->setShaderProgramUniform("LightView", lightViewMatrix);
        triModel->setShaderProgramUniform("LightProjection", lightProjectionMatrix);

        //pass1 shadowmap ǰ��
        scale = 0.5f;
        S.block<3, 3>(0, 0) = scale * Eigen::Matrix3f::Identity();
        T << 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.7f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f;
        R = Eigen::Matrix4f::Identity();
        modelMatrix = T * R * S;
        triModel->setShaderProgramUniform("Model", modelMatrix);
        triModel->setShaderProgramUniform("View", lightViewMatrix);
        triModel->setShaderProgramUniform("Projection", lightProjectionMatrix);
        m_Renderer.SetShaderProgram(triModel->getShaderProgram());
        m_Renderer.renderShadowMap(triModel);
        //pass1 shadowmap ����
        scale = 3.0f;
        S.block<3, 3>(0, 0) = scale * Eigen::Matrix3f::Identity();
        R << 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f;
        T << 1.0f, 0.0f, 0.0f, 0,
            0.0f, 1.0f, 0.0f, 0,
            0.0f, 0.0f, 1.0f, -3.0f,
            0.0f, 0.0f, 0.0f, 1.0f;
        modelMatrix = T * R * S;
        M->setShaderProgramUniform("Model", modelMatrix);
        M->setShaderProgramUniform("View", lightViewMatrix);
        M->setShaderProgramUniform("Projection", lightProjectionMatrix);
        m_Renderer.SetShaderProgram(M->getShaderProgram());
        m_Renderer.renderShadowMap(M);
        
        //pass2 ʵ����Ⱦ  ǰ��
        scale = 0.5f;
        S.block<3, 3>(0, 0) = scale * Eigen::Matrix3f::Identity();
        T << 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.7f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f;
        R = Eigen::Matrix4f::Identity();
        modelMatrix = T * R * S;
        triModel->setShaderProgramUniform("Model", modelMatrix);
        triModel->setShaderProgramUniform("View", cameraViewMatrix);
        triModel->setShaderProgramUniform("Projection", cameraProjectionMatrix);
        m_Renderer.SetShaderProgram(triModel->getShaderProgram());
        m_Renderer.renderModel(triModel, m_Image);

        //pass2 ʵ����Ⱦ  ����
        scale = 3.0f;
        S.block<3, 3>(0, 0) = scale * Eigen::Matrix3f::Identity();
        R << 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f;
        T << 1.0f, 0.0f, 0.0f, 0,
            0.0f, 1.0f, 0.0f, 0,
            0.0f, 0.0f, 1.0f, -3.0f,
            0.0f, 0.0f, 0.0f, 1.0f;
        modelMatrix = T * R * S;
        M->setShaderProgramUniform("Model", modelMatrix);
        M->setShaderProgramUniform("View", cameraViewMatrix);
        M->setShaderProgramUniform("Projection", cameraProjectionMatrix);
        m_Renderer.SetShaderProgram(M->getShaderProgram());
        m_Renderer.renderModel(M, m_Image);
        
        /*--------------------���ƽ��� --------------------*/


        //Y�ᷴת
        for (unsigned int y = 0; y < MY::globalConfig.SCR_Size.y() / 2; ++y) {
            for (unsigned int x = 0; x < MY::globalConfig.SCR_Size.x(); ++x) {
                sf::Color topPixel = m_Image.getPixel(x, y);
                sf::Color bottomPixel = m_Image.getPixel(x, MY::globalConfig.SCR_Size.y() - y - 1);
                m_Image.setPixel(x, y, bottomPixel);
                m_Image.setPixel(x, MY::globalConfig.SCR_Size.y() - y - 1, topPixel);
            }
        }
        
        m_Window.clear();
        m_Texture.loadFromImage(m_Image);
        m_Sprite.setTexture(m_Texture);
        m_Window.draw(m_Sprite);
        m_Window.display();
    }
}

void Application::OnMouseMove_Frame()
{
    //��ȡ���λ��ƫ��
    float dx = float(sf::Mouse::getPosition(m_Window).x) - float(MY::globalConfig.SCR_Size.x() / 2);
    float dy = float(sf::Mouse::getPosition(m_Window).y) - float(MY::globalConfig.SCR_Size.y() / 2);
    //��������ƶ�
    dx *= m_DeltaTime * m_MouseSpeed;
    dy *= m_DeltaTime * m_MouseSpeed;
    //��������ƶ��ٶ�
    dx = std::clamp(dx, -100.0f, 100.0f);
    dy = std::clamp(dy, -100.0f, 100.0f);
    //�������
    m_Camera->UpdateCameraVectors(dx, -dy);
    //���λ�ù�����
    sf::Mouse::setPosition(sf::Vector2i(MY::globalConfig.SCR_Size.x() / 2, MY::globalConfig.SCR_Size.y() / 2), m_Window);
}

void Application::OnKeyPressed_Frame()
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        m_Camera->ProcessKeyboard(m_Camera->GetForward() * m_DeltaTime);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        m_Camera->ProcessKeyboard(-m_Camera->GetForward() * m_DeltaTime);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        m_Camera->ProcessKeyboard(-m_Camera->GetRight() * m_DeltaTime);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        m_Camera->ProcessKeyboard(m_Camera->GetRight() * m_DeltaTime);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
        m_Camera->ProcessKeyboard(m_Camera->GetUp() * m_DeltaTime);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
        m_Camera->ProcessKeyboard(-m_Camera->GetUp() * m_DeltaTime);
    }
}

void Application::OnKeyPressed_Event(sf::Event& event)
{
    if (event.key.code == sf::Keyboard::Escape) {
        m_Window.close();
    }
    if (event.key.code == sf::Keyboard::R) {
        m_Camera->Reset();
    }
}

void Application::loadDiffTexture(const std::string& filename)
{
    const std::string inputfile = TEX_PATH + filename;
    bool ret = m_diff_texture->loadFromFile(inputfile);
    if (!ret) {
        std::cerr << "Failed to load texture!" << std::endl;
        return;
    }
}

void Application::loadSpecTexture(const std::string& filename)
{
    const std::string inputfile = TEX_PATH + filename;
    bool ret = m_spec_texture->loadFromFile(inputfile);
    if (!ret) {
        std::cerr << "Failed to load texture!" << std::endl;
        return;
    }
}

void Application::loadNormalTexture(const std::string& filename)
{
    const std::string inputfile = TEX_PATH + filename;
    bool ret = m_normal_texture->loadFromFile(inputfile);
    if (!ret) {
        std::cerr << "Failed to load texture!" << std::endl;
        return;
    }
}
