#include <iostream>
#include <memory>
#include "Loader.h"
#include "Model.h"
#include "tiny_obj_loader.h"
std::shared_ptr<Model> Loader::LoadObj(const std::string& filename)
{
	// ָ�� .obj �ļ�·��
	std::string inputfile = OBJ_PATH + filename;
    // �����洢���ص�����
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warning;
    std::string err;
    // ���� .obj �ļ�
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &err, inputfile.c_str());

    // �������Ƿ�ɹ�
    if (!ret) {
        std::cerr << "Failed to load/parse .obj file: " << err << std::endl;
        return false;
    }

    // ���һЩ��Ϣ
    std::cout << "Loaded " << shapes.size() << " shapes" << std::endl;
    for (size_t i = 0; i < shapes.size(); i++) {
        std::cout << "Shape " << i << ": " << shapes[i].name << std::endl;
        std::cout << "Number of faces: " << shapes[i].mesh.num_face_vertices.size() << std::endl;
    }

    //תΪModel����
    Model m = Model();

    // ����ģ������
    size_t pos = filename.find_last_of('.');
    if (pos != std::string::npos) {
        m.setName(filename.substr(0, pos));
    }

    // ������״
    for (const auto& shape : shapes) {
        MY::Mesh mesh;
        // ��䶥�������
        for (const tinyobj::index_t& index : shape.mesh.indices) {
            MY::Vertex vertex;
            // ����λ��
            vertex.model_pos = Eigen::Vector3f(
                attrib.vertices[3 * index.vertex_index + 0],    // x
                attrib.vertices[3 * index.vertex_index + 1],    // y
                attrib.vertices[3 * index.vertex_index + 2]);   // z

            // ����
            if (!attrib.normals.empty() && index.normal_index >= 0) {
                vertex.model_normal = Eigen::Vector3f(
                    attrib.normals[3 * index.normal_index + 0], // x
                    attrib.normals[3 * index.normal_index + 1], // y
                    attrib.normals[3 * index.normal_index + 2]);// z
            }
            else {
                vertex.model_normal = Eigen::Vector3f(0.0f, 0.0f, 0.0f); // Ĭ��ֵ
            }

            // ��������
            if (!attrib.texcoords.empty() && index.texcoord_index >= 0) {
                vertex.vertex_texcoord = Eigen::Vector2f(
                    attrib.texcoords[2 * index.texcoord_index + 0],  // u
                    attrib.texcoords[2 * index.texcoord_index + 1]); // v
            }
            else {
                vertex.vertex_texcoord = Eigen::Vector2f(0.0f, 0.0f); // Ĭ��ֵ
            }


            mesh.vertices.push_back(vertex);
            mesh.indices.push_back(static_cast<uint32_t>(mesh.vertices.size() - 1));
        }

        // ���ò������ƣ�����У�
        if (!shape.mesh.material_ids.empty() && shape.mesh.material_ids[0] >= 0) {
            mesh.material_name = materials[shape.mesh.material_ids[0]].name;
        }
        
        m.addMesh(mesh);
        m.vertexCount += mesh.vertices.size();
    }

    //����в�������
    if (!materials.empty()) {
        std::cout << "Number of materials: " << materials.size() << std::endl;
        for (const auto& mat : materials) {
            MY::Material material;
            material.name = mat.name;
            material.diffuse_color = Eigen::Vector3f(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
            material.specular_color = Eigen::Vector3f(mat.specular[0], mat.specular[1], mat.specular[2]);
            material.ambient_color = Eigen::Vector3f(mat.ambient[0], mat.ambient[1], mat.ambient[2]);
            material.diffuse_texture = mat.diffuse_texname; // �������ƣ��ļ�����·����
            m.addMaterial(material);
        }
    }
   
    return std::make_shared<Model>(m);
}
