#include "Renderer.h"
#include "Model.h"
#include "Shader.h"
#include "Geometry.h"
#include "Camera.h"
#include <iostream>
Renderer::Renderer()
{
    m_CurrentShaderProgram = std::make_shared<ShaderProgram>();
    m_zBuf = std::vector<double>(MY::globalConfig.SCR_Size.x() * MY::globalConfig.SCR_Size.y(), 1.0);
    m_VertexShaderOuts = std::vector<MY::VertexShader_out>(10000);
    m_RasterizeOut = std::vector<MY::Rasterization_out>(MY::globalConfig.SCR_Size.x() * MY::globalConfig.SCR_Size.y());
    m_RasterizeIndex = std::vector<Eigen::Vector2i>();
    m_TriangleTrimOut = std::vector<MY::VertexShader_out>();
    m_shadowMap = std::make_shared<std::vector<double>>(MY::globalConfig.SCR_Size.x() * MY::globalConfig.SCR_Size.y(), 999999999.0);

    m_Clock = sf::Clock();
}

void Renderer::renderModel(std::shared_ptr<Model> model, sf::Image& image)
{
    for (MY::Mesh& mesh : model->getMeshes()) {

        m_VertexShaderOuts.clear();
        m_TriangleTrimOut.clear();

        // 顶点着色器
        for (int i = 0; i < mesh.vertices.size(); i++) {
            MY::VertexShader_out vertexShader_out;
            (*m_CurrentShaderProgram.*m_CurrentShaderProgram->m_vertexShader)(mesh.vertices[i], vertexShader_out);
            m_VertexShaderOuts.push_back(vertexShader_out);
        }

        for (int i = 0; i < mesh.indices.size(); i += 3) {
            // todo: 图元装配

            // todo: 背面细分

            // todo: 几何着色器

            // 背面剔除
            if (!MY::isViewFrontFace(m_VertexShaderOuts, i, i + 1, i + 2) && MY::globalConfig.isBackFaceCulling) {
                continue;
            }

            // 裁剪
            TriangleTrim(m_VertexShaderOuts, i, i + 1, i + 2, m_TriangleTrimOut);
        }

        for (int i = 0; i < m_TriangleTrimOut.size(); i += 3) {

            // 光栅化
            rasterize(m_TriangleTrimOut, i, i + 1, i + 2, m_RasterizeOut, m_RasterizeIndex, m_zBuf);

            for (auto& pixel : m_RasterizeIndex) {

                int pixelIndex = pixel.x() + pixel.y() * MY::globalConfig.SCR_Size.x();

                // 片段着色器
                (*m_CurrentShaderProgram.*m_CurrentShaderProgram->m_fragmentShader)(m_RasterizeOut[pixelIndex]);

                // 深度测试
                if (MY::globalConfig.isDepthTest) {
                    if (m_RasterizeOut[pixelIndex].fragment_depth >= m_zBuf[pixelIndex]) {
                        continue;
                    }
                }
                if (MY::globalConfig.isDepthWrite) {
                    m_zBuf[pixelIndex] = m_RasterizeOut[pixelIndex].fragment_depth;
                }

                // 写入图像
                image.setPixel(pixel.x(), pixel.y(), m_RasterizeOut[pixelIndex].fragment_color);
            }
        }

        // 绘制三角面裁剪线
        if (MY::globalConfig.isClipLine) {
            for (int i = 0; i < m_TriangleTrimOut.size(); i += 3) {
                for (int j = 0; j < 3; j++) {
                    int index0 = i + j; // i间隔为3，j间隔为1 所以i+j就是当前顶点的索引
                    int index1 = i + (j + 1) % 3;
                    Eigen::Vector2i start = Eigen::Vector2i(m_TriangleTrimOut[index0].viewport_pos.x(), m_TriangleTrimOut[index0].viewport_pos.y());
                    Eigen::Vector2i end = Eigen::Vector2i(m_TriangleTrimOut[index1].viewport_pos.x(), m_TriangleTrimOut[index1].viewport_pos.y());
                    CohenSutherlandLineClip(start, end, Eigen::Vector2i(0, 0), Eigen::Vector2i(MY::globalConfig.SCR_Size.x(), MY::globalConfig.SCR_Size.y()));
                    drawLine(start, end, image, sf::Color::White);
                }
            }
        }
        //绘制顶点法线
        if (MY::globalConfig.isShowVertexNormal) {
            for (int i = 0; i < m_TriangleTrimOut.size(); i += 3) {
                for (int j = 0; j < 3; j++) {
                    int index = i + j; // i间隔为3，j间隔为1 所以i+j就是当前顶点的索引
                    Eigen::Matrix4f model = m_CurrentShaderProgram->uniformStorage.getUniform<Eigen::Matrix4f>("Model");
                    Eigen::Matrix4f view = m_CurrentShaderProgram->uniformStorage.getUniform<Eigen::Matrix4f>("View");
                    Eigen::Matrix4f projection = m_CurrentShaderProgram->uniformStorage.getUniform<Eigen::Matrix4f>("Projection");
                    Eigen::Vector3f start = m_TriangleTrimOut[index].world_pos.block<3, 1>(0, 0);
                    Eigen::Vector3f end = start + 0.2f * m_TriangleTrimOut[index].world_normal;
                    Eigen::Vector4f Start2 = (projection * view * model) * Eigen::Vector4f(start.x(), start.y(), start.z(), 1.0f);
                    Eigen::Vector4f End2 = (projection * view * model) * Eigen::Vector4f(end.x(), end.y(), end.z(), 1.0f);
                    Start2 /= Start2.w();
                    End2 /= End2.w();
                    Eigen::Vector2i screenStart;
                    Eigen::Vector2i screenEnd;
                    screenStart.x() = (Start2.x() + 1.0f) * 0.5f * float(MY::globalConfig.SCR_Size.x() - 1);
                    screenStart.y() = (Start2.y() + 1.0f) * 0.5f * float(MY::globalConfig.SCR_Size.y() - 1);
                    screenEnd.x() = (End2.x() + 1.0f) * 0.5f * float(MY::globalConfig.SCR_Size.x() - 1);
                    screenEnd.y() = (End2.y() + 1.0f) * 0.5f * float(MY::globalConfig.SCR_Size.y() - 1);
                    CohenSutherlandLineClip(screenStart, screenEnd, Eigen::Vector2i(0, 0), Eigen::Vector2i(MY::globalConfig.SCR_Size.x() - 1, MY::globalConfig.SCR_Size.y() - 1));
                    drawLine(screenStart, screenEnd, image, sf::Color::Green);
                }
            }
        }
    }
}

void Renderer::renderShadowMap(std::shared_ptr<Model> model)
{
    for (MY::Mesh& mesh : model->getMeshes()) {

        m_VertexShaderOuts.clear();
        m_TriangleTrimOut.clear();

        // 顶点着色器
        for (int i = 0; i < mesh.vertices.size(); i++) {
            MY::VertexShader_out vertexShader_out;
            (*m_CurrentShaderProgram.*m_CurrentShaderProgram->m_vertexShader)(mesh.vertices[i], vertexShader_out);
            m_VertexShaderOuts.push_back(vertexShader_out);
        }

        for (int i = 0; i < mesh.indices.size(); i += 3) {
            // todo: 图元装配

            // todo: 背面细分

            // todo: 几何着色器

            // 背面剔除
            if (!MY::isViewFrontFace(m_VertexShaderOuts, i, i + 1, i + 2) && MY::globalConfig.isBackFaceCulling) {
                continue;
            }

            // 裁剪
            TriangleTrim(m_VertexShaderOuts, i, i + 1, i + 2, m_TriangleTrimOut);
        }

        for (int i = 0; i < m_TriangleTrimOut.size(); i += 3) {
            // 光栅化
            rasterizeShadowMap(m_TriangleTrimOut, i, i + 1, i + 2, m_shadowMap);
        }       
    }
}

void Renderer::renderTriangle(std::vector<MY::VertexShader_out>& vIn, const int index0, const int index1, const int index2, sf::Image& image)
{
    // 光栅化
    rasterize(vIn, index0, index1, index2, m_RasterizeOut, m_RasterizeIndex, m_zBuf);
    
    for (auto& pixel : m_RasterizeIndex) {
       
        int pixelIndex = pixel.x() + pixel.y() * MY::globalConfig.SCR_Size.x();
        
        // 片段着色器
        (*m_CurrentShaderProgram.*m_CurrentShaderProgram->m_fragmentShader)(m_RasterizeOut[pixelIndex]);

        // 深度测试
        // 输入：
        // 输出：
        if (MY::globalConfig.isDepthTest) {
            if (m_RasterizeOut[pixelIndex].fragment_depth >= m_zBuf[pixelIndex]) {
                continue;
            }
        }
        if (MY::globalConfig.isDepthWrite) {
            m_zBuf[pixelIndex] = m_RasterizeOut[pixelIndex].fragment_depth;
        }
        
        // 写入图像
        // 输入：位置 颜色
        // 输出：
        image.setPixel(pixel.x(), pixel.y(), m_RasterizeOut[pixelIndex].fragment_color);
    }
}

void Renderer::rasterize(std::vector<MY::VertexShader_out>& vIn, const int index0, const int index1, const int index2, std::vector<MY::Rasterization_out>& rasterizeOut, std::vector<Eigen::Vector2i>& rasterizeIndexOut, std::vector<double>& zBuf)
{
    rasterizeIndexOut.clear();
    // 顶点
    MY::VertexShader_out& vertex0 = vIn[index0];
    MY::VertexShader_out& vertex1 = vIn[index1];
    MY::VertexShader_out& vertex2 = vIn[index2];
    // 三角形三个点 不包含z的二维平面坐标（屏幕坐标）
    Eigen::Vector2f p0 = Eigen::Vector2f(vertex0.viewport_pos.x(), vertex0.viewport_pos.y());
    Eigen::Vector2f p1 = Eigen::Vector2f(vertex1.viewport_pos.x(), vertex1.viewport_pos.y());
    Eigen::Vector2f p2 = Eigen::Vector2f(vertex2.viewport_pos.x(), vertex2.viewport_pos.y());
    // 齐次坐标
    Eigen::Vector4f p0_h = p0.homogeneous().homogeneous();
    Eigen::Vector4f p1_h = p1.homogeneous().homogeneous();
    Eigen::Vector4f p2_h = p2.homogeneous().homogeneous();

    // 边缘函数
    auto edgeFunction_Eigen = [](const Eigen::Vector2f& a, const Eigen::Vector2f& b, const Eigen::Vector2f& c, bool isViewFrontFace) -> float {
        float result = (b.x() - a.x()) * (c.y() - a.y()) - (b.y() - a.y()) * (c.x() - a.x());
        return isViewFrontFace ? result : -result;
        };

    // 计算三角形aabb包围盒
    MY::BBox_SCR bbox = MY::BBox_SCR(vIn, index0, index1, index2);

    // 计算三角形是正面
    bool isFrontFace = MY::isViewFrontFace(vIn, index0, index1, index2);
    // 遍历屏幕像素
    for (int y = bbox.min.y(); y <= bbox.max.y(); y++)
    {
        for (int x = bbox.min.x(); x <= bbox.max.x(); x++)
        {
            Eigen::Vector2f pixel_center = Eigen::Vector2f(x + 0.5f, y + 0.5f);
            int pixelIndex = y * MY::globalConfig.SCR_Size.x() + x;

            float w0 = edgeFunction_Eigen(p0, p1, pixel_center, isFrontFace); // 对应边 (v0, v1)
            float w1 = edgeFunction_Eigen(p1, p2, pixel_center, isFrontFace); // 对应边 (v1, v2)
            float w2 = edgeFunction_Eigen(p2, p0, pixel_center, isFrontFace); // 对应边 (v2, v0)

            // 像素点在三角形内部
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {

                // 重心坐标
                bool isLine = false;
                Eigen::Vector3f BC = MY::BarycentricFast(p0_h, p1_h, p2_h, Eigen::Vector4f(pixel_center.x(), pixel_center.y(), 0.0f, 0.0f), isLine);
                if (isLine) continue;

                double frag_depth = vertex0.viewport_pos.z() * BC.x() + vertex1.viewport_pos.z() * BC.y() + vertex2.viewport_pos.z() * BC.z(); // 计算深度值

                //Early Z-Test
                if (MY::globalConfig.isEarlyZ && frag_depth >= zBuf[pixelIndex]) continue;
                zBuf[pixelIndex] = frag_depth;

                //透视矫正插值
                float k0 = BC.x() / vertex0.clip_pos.w();
                float k1 = BC.y() / vertex1.clip_pos.w();
                float k2 = BC.z() / vertex2.clip_pos.w();
                float k_sum = k0 + k1 + k2;
                rasterizeOut[pixelIndex].viewport_pos = Eigen::Vector2i(x, y);
                rasterizeOut[pixelIndex].fragment_depth = frag_depth;
                rasterizeOut[pixelIndex].fragment_world_position = (vertex0.world_pos * k0 + vertex1.world_pos * k1 + vertex2.world_pos * k2) / k_sum;
                rasterizeOut[pixelIndex].fragment_world_normal = ((vertex0.world_normal * k0 + vertex1.world_normal * k1 + vertex2.world_normal * k2) / k_sum).normalized();
                rasterizeOut[pixelIndex].fragment_uv = (vertex0.vertex_uv * k0 + vertex1.vertex_uv * k1 + vertex2.vertex_uv * k2) / k_sum;

                rasterizeIndexOut.push_back(Eigen::Vector2i(x, y));
            }
        }
    }
}

void Renderer::rasterizeShadowMap(std::vector<MY::VertexShader_out>& vIn, const int index0, const int index1, const int index2,std::shared_ptr<std::vector<double>>& shadowMap)
{
    // 顶点
    MY::VertexShader_out& vertex0 = vIn[index0];
    MY::VertexShader_out& vertex1 = vIn[index1];
    MY::VertexShader_out& vertex2 = vIn[index2];
    // 三角形三个点 不包含z的二维平面坐标（屏幕坐标）
    Eigen::Vector2f p0 = Eigen::Vector2f(vertex0.viewport_pos.x(), vertex0.viewport_pos.y());
    Eigen::Vector2f p1 = Eigen::Vector2f(vertex1.viewport_pos.x(), vertex1.viewport_pos.y());
    Eigen::Vector2f p2 = Eigen::Vector2f(vertex2.viewport_pos.x(), vertex2.viewport_pos.y());
    // 齐次坐标
    Eigen::Vector4f p0_h = p0.homogeneous().homogeneous();
    Eigen::Vector4f p1_h = p1.homogeneous().homogeneous();
    Eigen::Vector4f p2_h = p2.homogeneous().homogeneous();

    // 边缘函数
    auto edgeFunction_Eigen = [](const Eigen::Vector2f& a, const Eigen::Vector2f& b, const Eigen::Vector2f& c, bool isViewFrontFace) -> float {
        float result = (b.x() - a.x()) * (c.y() - a.y()) - (b.y() - a.y()) * (c.x() - a.x());
        return isViewFrontFace ? result : -result;
        };

    // 计算三角形aabb包围盒
    MY::BBox_SCR bbox = MY::BBox_SCR(vIn, index0, index1, index2);

    // 计算三角形是正面
    bool isFrontFace = MY::isViewFrontFace(vIn, index0, index1, index2);
    // 遍历屏幕像素
    for (int y = bbox.min.y(); y <= bbox.max.y(); y++)
    {
        for (int x = bbox.min.x(); x <= bbox.max.x(); x++)
        {
            Eigen::Vector2f pixel_center = Eigen::Vector2f(x + 0.5f, y + 0.5f);
            int pixelIndex = y * MY::globalConfig.SCR_Size.x() + x;

            float w0 = edgeFunction_Eigen(p0, p1, pixel_center, isFrontFace); // 对应边 (v0, v1)
            float w1 = edgeFunction_Eigen(p1, p2, pixel_center, isFrontFace); // 对应边 (v1, v2)
            float w2 = edgeFunction_Eigen(p2, p0, pixel_center, isFrontFace); // 对应边 (v2, v0)

            // 像素点在三角形内部
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {

                // 重心坐标
                bool isLine = false;
                Eigen::Vector3f BC = MY::BarycentricFast(p0_h, p1_h, p2_h, Eigen::Vector4f(pixel_center.x(), pixel_center.y(), 0.0f, 0.0f), isLine);
                if (isLine) continue;

                double frag_depth = vertex0.view_pos.z() * BC.x() + vertex1.view_pos.z() * BC.y() + vertex2.view_pos.z() * BC.z(); // 计算深度值
                if (frag_depth > (*shadowMap)[pixelIndex]) {
                    (*shadowMap)[pixelIndex] = frag_depth;
                };
            }
        }
    }
}

void Renderer::CohenSutherlandLineClip(Eigen::Vector2i& start, Eigen::Vector2i& end, Eigen::Vector2i& min, Eigen::Vector2i& max)
{
    int x0 = start.x();
    int x1 = end.x();

    int y0 = start.y();
    int y1 = end.y();

    int outcode0 = MY::EnCode(start, min, max);
    int outcode1 = MY::EnCode(end, min, max);
    bool accept = false;

    while (true) {
        // Bitwise OR is 0. Trivially accept and get out of loop. start and end all in center.
        if (!(outcode0 | outcode1))
        {
            accept = true;
            break;
        }
        else if (outcode0 & outcode1) { // Bitwise AND is not 0. Trivially reject and get out of loop
            break;
        }
        else {
            // failed both tests, so calculate the line segment to clip
            // from an outside point to an intersection with clip edge
            double x, y;

            // At least one endpoint is outside the clip rectangle; pick it.
            int outcodeOut = outcode0 ? outcode0 : outcode1;

            // Now find the intersection point;
            // use formulas y = y0 + slope * (x - x0), x = x0 + (1 / slope) * (y - y0)
            if (outcodeOut & LINE_TOP) {           // point is above the clip rectangle
                x = x0 + (x1 - x0) * (max.y() - y0) / (y1 - y0);
                y = max.y();
            }
            else if (outcodeOut & LINE_BOTTOM) { // point is below the clip rectangle
                x = x0 + (x1 - x0) * (min.y() - y0) / (y1 - y0);
                y = min.y();
            }
            else if (outcodeOut & LINE_RIGHT) {  // point is to the right of clip rectangle
                y = y0 + (y1 - y0) * (max.x() - x0) / (x1 - x0);
                x = max.x();
            }
            else if (outcodeOut & LINE_LEFT) {   // point is to the left of clip rectangle
                y = y0 + (y1 - y0) * (min.x() - x0) / (x1 - x0);
                x = min.x();
            }

            // Now we move outside point to intersection point to clip
            // and get ready for next pass.
            if (outcodeOut == outcode0) {
                x0 = static_cast<int>(x);
                y0 = static_cast<int>(y);
                outcode0 = MY::EnCode(Eigen::Vector2i(x0, y0), min, max);
            }
            else {
                x1 = static_cast<int>(x);
                y1 = static_cast<int>(y);
                outcode1 = MY::EnCode(Eigen::Vector2i(x1, y1), min, max);
            }
        }
    }
    if (accept) {
        start.x() = x0;
        start.y() = y0;
          end.x() = x1;
          end.y() = y1;
    }
}


void Renderer::drawLine(Eigen::Vector2i start, Eigen::Vector2i end, sf::Image& image, sf::Color c)
{
    int x1 = start.x();
    int y1 = start.y();
    int x2 = end.x();
    int y2 = end.y();
    if (x1 < 0 || x1 >= MY::globalConfig.SCR_Size.x() || y1 < 0 || y1 >= MY::globalConfig.SCR_Size.y() || x2 < 0 || x2 >= MY::globalConfig.SCR_Size.x() || y2 < 0 || y2 >= MY::globalConfig.SCR_Size.y()) {
        std::cout << "Line out of screen" << std::endl;
        return;
    }

    int x, y, rem = 0;

    //line is a pixel
    if (x1 == x2 && y1 == y2)
    {
        image.setPixel(x1, y1, c);
    }
    //vertical line
    else if (x1 == x2)
    {
        int inc = (y1 <= y2) ? 1 : -1;
        for (y = y1; y != y2; y += inc) image.setPixel(x1, y, c);
        image.setPixel(x2, y2, c);
    }
    //horizontal line
    else if (y1 == y2)
    {
        int inc = (x1 <= x2) ? 1 : -1;
        for (x = x1; x != x2; x += inc) image.setPixel(x, y1, c);
        image.setPixel(x2, y2, c);
    }
    else {
        int dx = (x1 < x2) ? x2 - x1 : x1 - x2;
        int dy = (y1 < y2) ? y2 - y1 : y1 - y2;

        // slope < 1
        if (dx >= dy)
        {
            if (x2 < x1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
            for (x = x1, y = y1; x <= x2; x++)
            {
                image.setPixel(x, y, c);
                rem += dy;
                if (rem >= dx)
                {
                    rem -= dx;
                    y += (y2 >= y1) ? 1 : -1;
                }
            }
            image.setPixel(x2, y2, c);
        }
        // slope > 1
        else {
            if (y2 < y1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
            for (x = x1, y = y1; y <= y2; y++)
            {
                image.setPixel(x, y, c);
                rem += dx;
                if (rem >= dy)
                {
                    rem -= dy;
                    x += (x2 >= x1) ? 1 : -1;
                }
            }
            image.setPixel(x2, y2, c);
        }
    }
}

void Renderer::TriangleTrim(std::vector<MY::VertexShader_out>& vIn, const int index0, const int index1, const int index2, std::vector<MY::VertexShader_out>& triangleTrimOut)
{
    //裁剪空间裁剪面
    std::vector<Eigen::Vector4f> planes = {
        Eigen::Vector4f(0, 0, 1, 0),
        Eigen::Vector4f(0, 0, -1, 1),
        Eigen::Vector4f(1, 0, 0, 1),
        Eigen::Vector4f(-1, 0, 0, 1),
        Eigen::Vector4f(0, 1, 0, 1),
        Eigen::Vector4f(0, -1, 0, 1)
    };
    std::vector<MY::VertexShader_out> currentVertexs = {
        vIn[index0],
        vIn[index1],
        vIn[index2]
    };
    //每个面进行裁剪
    for (auto& plane : planes) {
        currentVertexs = ClipTriangle(currentVertexs, plane);
        if (currentVertexs.empty()) return;
    }
    // 将裁剪后的多边形分割成三角形
    for (size_t i = 1; i + 1 < currentVertexs.size(); ++i) {
        triangleTrimOut.push_back(currentVertexs[0]);
        triangleTrimOut.push_back(currentVertexs[i]);
        triangleTrimOut.push_back(currentVertexs[i + 1]);
    }
}

std::vector<MY::VertexShader_out> Renderer::ClipTriangle(const std::vector<MY::VertexShader_out>& vertexs, const Eigen::Vector4f& plane)
{
    std::vector<MY::VertexShader_out> clippedVertices;
    int numVertices = static_cast<int>(vertexs.size());
    // 存储每个顶点与平面的距离
    std::vector<float> distances(numVertices);
    for (int i = 0; i < numVertices; ++i) {
        distances[i] = MY::Distance_PointToPlane(vertexs[i], plane);
    }

    for (int i = 0; i < numVertices; i++) {
        int next = (i + 1) % numVertices;

        const MY::VertexShader_out& currentVertex = vertexs[i];
        const MY::VertexShader_out& nextVertex = vertexs[(i + 1) % numVertices];

        bool currentInside = distances[i] >= 0;
        bool nextInside = distances[next] >= 0;

        // 如果当前顶点在平面一侧，加入结果列表
        if (currentInside) {
            clippedVertices.push_back(vertexs[i]);
        }

        // 如果当前顶点和下一个顶点在平面两侧，插值交点并加入
        if (currentInside != nextInside) {
            float t = distances[i] / (distances[i] - distances[next]);  // 计算交点
            MY::VertexShader_out interpolated = ComputeIntersection(vertexs[i], vertexs[next], t);
            clippedVertices.push_back(interpolated);
        }
    }

    return clippedVertices;
}
