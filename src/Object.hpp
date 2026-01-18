#pragma once

#include <string>
#include <glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <optional>

#include "Transform.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "Light.hpp"
#include "Texture.hpp"

class Object
{
public:
    Object() = default;
    Object(
        std::string name,
        Transform transform,
        float& k,
        Shader shader,
        Mesh mesh,
        std::optional<Texture> texture = std::nullopt,
        bool isStatic = true, // TODO : make it actually optional in YAML
        glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f) // TODO : make it actually optional in YAML
    );
    ~Object();

    std::string getName() const { return m_name; }
    const glm::vec3& getColor() const { return m_color; }

    void update(float deltaTime);
    void updateTransformWithCOM();
    void render(Light* light, const glm::vec3& cameraPosition);

    void setPolygonMode(GLenum mode) { m_polygonMode = mode; }
    GLenum getPolygonMode() const { return m_polygonMode; }

    const bool isStatic() const { return m_isStatic; }

    Transform& getTransform() { return m_transform; }
    std::vector<Transform>& getVertexTransforms() { return m_vertexTransforms; }
    Mesh& getMesh() { return m_mesh; }
    const std::vector<float>& getMass() const { return m_M; }

    void resetVertexTransforms();

    void setProjectionViewUniforms(const Shader& shader);
    static void setVertexNormalShader(const Shader& shader) { s_vertexNormalShader = shader; }
    static void setFaceNormalShader(const Shader& shader)   { s_faceNormalShader   = shader; }

private:
    std::string m_name;
    Transform m_transform;
    Shader m_shader;
    static Shader s_vertexNormalShader;
    static Shader s_faceNormalShader;
    Mesh m_mesh;
    std::optional<Texture> m_texture;
    glm::vec3 m_color;
    bool m_isStatic;
    GLenum m_polygonMode;


    std::vector<Transform> m_initialVertexTransforms;
    std::vector<Transform> m_vertexTransforms;
    std::vector<float> m_M;

};