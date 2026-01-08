#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

#include "Scene.hpp"

Shader Object::s_vertexNormalShader;
Shader Object::s_faceNormalShader;

const std::string RESOURCE_PATH = "../res/";
const std::string CONFIG_PATH = "../config/";

std::unique_ptr<Camera> Scene::createCamera(GLFWwindow* window, unsigned int screenWidth, unsigned int screenHeight)
{
    float FOV = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 150.0f;
    float aspectRatio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);

    return std::make_unique<Camera>(
        glm::vec3(0.0f, 25.0f, 80.0f),
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        FOV,
        aspectRatio,
        nearPlane,
        farPlane,
        window
    );
}

std::unique_ptr<ShaderManager> Scene::loadShaders()
{
    auto shaderManager = std::make_unique<ShaderManager>();
    std::vector<std::unique_ptr<Shader>> shaders;

    const std::vector<std::tuple<const char*, const char*, const char*>> shaderData = {
        {"vertexNormal", "vertexNormal.vsh", "vertexNormal.fsh"},
        {"faceNormal", "faceNormal.vsh", "faceNormal.fsh"},
        {"platform", "platform.vsh", "platform.fsh"},
        {"dirtblock", "dirtblock.vsh", "dirtblock.fsh"},
    };

    for (const auto& [name, vsh, fsh] : shaderData) {
        std::string vshPath = std::string(RESOURCE_PATH) + "shaders/" + vsh;
        std::string fshPath = std::string(RESOURCE_PATH) + "shaders/" + fsh;
        try {
            shaders.push_back(std::make_unique<Shader>(name, vshPath.c_str(), fshPath.c_str()));
            std::cout << "Shader loaded: " << name << '\n';
        } catch (const std::exception& e) {
            std::cerr << "Failed to load shader " << name << ": " << e.what() << '\n';
        }
    }

    shaderManager->addResources(std::move(shaders));
    return shaderManager;
}

std::unique_ptr<MeshManager> Scene::loadMeshes()
{
    auto meshManager = std::make_unique<MeshManager>();
    std::vector<std::unique_ptr<Mesh>> meshes;

    const std::vector<std::tuple<const char*, const char*>> meshData = {
        {"cube", "cube.obj"},
    };

    for (const auto& [name, filename] : meshData) {
        std::string meshPath = std::string(RESOURCE_PATH) + "meshes/" + filename;
        try {
            meshes.push_back(std::make_unique<Mesh>(name, meshPath.c_str()));
            std::cout << "Mesh loaded: " << name << '\n';
        } catch (const std::exception& e) {
            std::cerr << "Failed to load mesh " << name << ": " << e.what() << '\n';
        }
    }

    meshManager->addResources(std::move(meshes));
    return meshManager;
}

std::unique_ptr<TextureManager> Scene::loadTextures()
{
    auto textureManager = std::make_unique<TextureManager>();
    std::vector<std::unique_ptr<Texture>> textures;

    const std::vector<std::tuple<const char*, const char*>> textureData = {
        {"dirtblock", "dirtblock.jpg"}
    };

    for (const auto& [name, filename] : textureData) {
        std::string texturePath = std::string(RESOURCE_PATH) + "textures/" + filename;
        try {
            textures.push_back(std::make_unique<Texture>(name, texturePath.c_str()));
            std::cout << "Texture loaded: " << name << '\n';
        } catch (const std::exception& e) {
            std::cerr << "Failed to load texture " << name << ": " << e.what() << '\n';
        }
    }

    textureManager->addResources(std::move(textures));
    return textureManager;
}

void Scene::loadResources()
{
    m_shaderManager = loadShaders();
    m_meshManager = loadMeshes();
    m_textureManager = loadTextures();
}

std::unique_ptr<Object> Scene::createObject(const ObjectConfig& config)
{
    Transform transform;
    transform.setProjection(*m_camera);

    glm::mat4 model = glm::translate(glm::mat4(1.0f), config.position);
    model = glm::rotate(model, glm::radians(config.rotationDeg), config.rotationAxis);
    model = glm::scale(model, config.scale);

    transform.setModel(model);
    transform.setView(*m_camera);

    Shader shader = m_shaderManager->getResource(config.shaderName);
    Mesh mesh = m_meshManager->getResource(config.meshName);

    if (config.hasTexture) {
        Texture texture = m_textureManager->getResource(config.textureName);
        return std::make_unique<Object>(
            config.name,
            transform,
            m_k,
            shader,
            mesh,
            texture,
            config.isStatic
        );
    } else {
        return std::make_unique<Object>(
            config.name,
            transform,
            m_k,
            shader,
            mesh,
            std::nullopt,
            config.isStatic
        );
    }
}

void Scene::loadSceneConfig(const std::string& configPath)
{
    std::ifstream configFile(configPath);
    if (!configFile.is_open()) {
        std::cerr << "Failed to open scene config: " << configPath << '\n';
        return;
    }

    nlohmann::json sceneJson;
    configFile >> sceneJson;
    configFile.close();

    std::vector<ObjectConfig> objectConfigs = parseObjectConfigs(sceneJson);

    Object::setVertexNormalShader(m_shaderManager->getResource("vertexNormal"));
    Object::setFaceNormalShader(m_shaderManager->getResource("faceNormal"));

    for (const auto& config : objectConfigs) {
        m_objects.push_back(createObject(config));
    }
}

std::vector<ObjectConfig> Scene::parseObjectConfigs(const nlohmann::json& sceneJson)
{
    std::vector<ObjectConfig> configs;

    const auto& objectsJson = sceneJson["scene"]["objects"];
    for (const auto& objJson : objectsJson) {
        ObjectConfig config;
        config.name = objJson["name"];
        config.position = glm::vec3(
            objJson["position"][0],
            objJson["position"][1],
            objJson["position"][2]
        );
        config.rotationAxis = glm::vec3(
            objJson["rotationAxis"][0],
            objJson["rotationAxis"][1],
            objJson["rotationAxis"][2]
        );
        config.rotationDeg = objJson["rotationDeg"];
        config.scale = glm::vec3(
            objJson["scale"][0],
            objJson["scale"][1],
            objJson["scale"][2]
        );
        config.shaderName = objJson["shader"];
        config.meshName = objJson["mesh"];
        config.textureName = objJson["texture"];
        config.isStatic = objJson["isStatic"];
        config.hasTexture = objJson["hasTexture"];

        configs.push_back(config);
    }

    return configs;
}

void Scene::setupEnvCollisionConstraints()
{
    for (const auto& obj : m_objects)
    {
        if (obj->isStatic()) continue;

        std::vector<Object*> candidateObjects;
        candidateObjects.reserve(m_objects.size());
        for (const auto& objPtr : m_objects) {
            candidateObjects.push_back(objPtr.get());
        }

        Mesh& mesh = obj->getMesh();
        mesh.setCandidateObjectMeshes(candidateObjects);
        mesh.constructEnvCollisionConstraints();
    }
}

Scene::Scene(
    const std::string& name,
    GLFWwindow* window,
    unsigned int screenWidth,
    unsigned int screenHeight
)
    :   m_name(name),
        m_camera(createCamera(window, screenWidth, screenHeight)),
        m_shaderManager(std::make_unique<ShaderManager>()),
        m_meshManager(std::make_unique<MeshManager>()),
        m_textureManager(std::make_unique<TextureManager>()),
        m_gravitationalAcceleration(0.0f),
        m_enableDistanceConstraints(true),
        m_enableVolumeConstraints(true),
        m_enableEnvCollisionConstraints(true),
        m_pbdSubsteps(10),
        m_alpha(0.001f),
        m_beta(5.0f),
        m_k(1.0f)
{
    loadResources();
    loadSceneConfig(CONFIG_PATH + "/test_scene.json");
    setupEnvCollisionConstraints();
    std::cout << name << " created.\n";
}

void Scene::applyGravity(
    Object& object,
    float deltaTime
)
{
    for (auto& vertexTransform : object.getVertexTransforms())
    {
        vertexTransform.setAcceleration(m_gravitationalAcceleration);
    }
}

float Scene::calculateDeltaLambda(
    float C_j,
    const std::vector<glm::vec3>& gradC_j,
    const std::vector<glm::vec3>& posDiff,
    std::span<const unsigned int> constraintVertices,
    const std::vector<float>& M,
    float alphaTilde,
    float gamma
)
{
    float gradCMInverseGradCT = 0.0f;
    float gradCPosDiff = 0.0f;
    size_t n = constraintVertices.size();

    for (size_t i = 0; i < n; ++i)
    {
        unsigned int v = constraintVertices[i];
        float w = 1.0f / M[v];
        gradCMInverseGradCT += w * glm::dot(gradC_j[i], gradC_j[i]);
        gradCPosDiff += glm::dot(gradC_j[i], posDiff[v]);
    }

    return (-C_j - gamma * gradCPosDiff) / ((1 + gamma) * gradCMInverseGradCT + alphaTilde);
}

std::vector<glm::vec3> Scene::calculateDeltaX(
    float lambda,
    const std::vector<float>& M,
    std::vector<glm::vec3>& gradC_j,
    std::span<const unsigned int> constraintVertices
)
{
    std::vector<glm::vec3> deltaX(M.size(), glm::vec3(0.0f));
    size_t n = constraintVertices.size();
    for (size_t i = 0; i < n; ++i)
    {
        unsigned int v = constraintVertices[i];
        float w = 1.0f / M[v];
        deltaX[v] = lambda * w * gradC_j[i];
    }

    return deltaX;
}

void Scene::solveDistanceConstraints(
    std::vector<glm::vec3>& x,
    const std::vector<glm::vec3>& posDiff,
    const std::vector<float>& M,
    float alphaTilde,
    float gamma,
    const Mesh::DistanceConstraints& distanceConstraints
)
{
    size_t edgesSize = distanceConstraints.edges.size();
    size_t CSize = distanceConstraints.C.size();
    size_t gradCSize = distanceConstraints.gradC.size();
    if (edgesSize != gradCSize)
    {
        std::cerr << "DistanceConstraints size mismatch:\n"
                  << "constraints = " << CSize << ", "
                  << "gradConstraints = " << gradCSize << ", "
                  << "edgeVertices = " << edgesSize << std::endl;
        return;
    }

    for (size_t j = 0; j < distanceConstraints.edges.size(); ++j)
    {
        float C_j = distanceConstraints.C[j](x);
        std::vector<glm::vec3> gradC_j = distanceConstraints.gradC[j](x);
        const Edge& edge = distanceConstraints.edges[j];
        const std::array<unsigned int, 2> constraintVertices = { edge.v1, edge.v2 };

        float deltaLambda = calculateDeltaLambda(C_j, gradC_j, posDiff, constraintVertices, M, alphaTilde, gamma);
        std::vector<glm::vec3> deltaX = calculateDeltaX(deltaLambda, M, gradC_j, constraintVertices);

        {
            for (size_t k = 0; k < deltaX.size(); ++k)
            {
                x[k] += deltaX[k];
            }
        }

    }
}

void Scene::solveVolumeConstraints(
    std::vector<glm::vec3>& x,
    const std::vector<glm::vec3>& posDiff,
    const std::vector<float>& M,
    float alphaTilde,
    float gamma,
    const Mesh::VolumeConstraints& volumeConstraints
)
{
    size_t trianglesSize = volumeConstraints.triangles.size();
    size_t CSize = volumeConstraints.C.size();
    size_t gradCSize = volumeConstraints.gradC.size();
    if (trianglesSize != gradCSize)
    {
        std::cerr << "VolumeConstraints size mismatch:\n"
                  << "constraints = " << CSize << ", "
                  << "gradConstraints = " << gradCSize << ", "
                  << "triangleVertices = " << trianglesSize << std::endl;
        return;
    }

    for (size_t j = 0; j < volumeConstraints.triangles.size(); ++j)
    {
        float C_j = volumeConstraints.C[0](x);
        std::vector<glm::vec3> gradC_j = volumeConstraints.gradC[j](x);
        const Triangle& tri = volumeConstraints.triangles[j];
        const std::array<unsigned int, 3> constraintVertices = { tri.v1, tri.v2, tri.v3 };

        float deltaLambda = calculateDeltaLambda(C_j, gradC_j, posDiff, constraintVertices, M, alphaTilde, gamma);
        std::vector<glm::vec3> deltaX = calculateDeltaX(deltaLambda, M, gradC_j, constraintVertices);

        for (size_t k = 0; k < deltaX.size(); ++k)
        {
            x[k] += deltaX[k];
        }
    }
}

void Scene::solveEnvCollisionConstraints(
    std::vector<glm::vec3>& x,
    const std::vector<glm::vec3>& posDiff,
    const std::vector<float>& M,
    float alphaTilde,
    float gamma,
    std::vector<Mesh::EnvCollisionConstraints> perEnvCollisionConstraints
)
{
    for (size_t setIdx = 0; setIdx < perEnvCollisionConstraints.size(); ++setIdx)
    {
        const auto& constraints = perEnvCollisionConstraints[setIdx];
        size_t verticesSize = constraints.vertices.size();
        size_t CSize = constraints.C.size();
        size_t gradCSize = constraints.gradC.size();

        if (verticesSize != gradCSize)
        {
            std::cerr << "EnvCollisionConstraints size mismatch in set " << setIdx << std::endl;
            continue;
        }

        for (const auto& [vertex, constraintIndices] : constraints.vertexToConstraints)
        {
            bool allNegative = true;
            float maxNegativeC = -std::numeric_limits<float>::max(); // Initialize to most negative possible value
            size_t maxIdx = 0;
            for (size_t idx : constraintIndices)
            {
                float C_j = constraints.C[idx](x);
                if (C_j >= 0.0f)
                {
                    allNegative = false;
                }

                // Track the constraint with biggest negative value
                if (C_j < 0.0f && C_j > maxNegativeC)
                {
                    maxNegativeC = C_j;
                    maxIdx = idx;
                }
            }

            // If all constraints are negative, we have a collision with this vertex
            if (allNegative && !constraintIndices.empty())
            {
                float C_j = maxNegativeC;
                std::vector<glm::vec3> gradC_j = constraints.gradC[maxIdx](x);

                std::array<unsigned int, 1> constraintVertices = { vertex };

                float deltaLambda = calculateDeltaLambda(C_j, gradC_j, posDiff, constraintVertices, M, alphaTilde, gamma);
                std::vector<glm::vec3> deltaX = calculateDeltaX(deltaLambda, M, gradC_j, constraintVertices);

                for (size_t k = 0; k < deltaX.size(); ++k)
                {
                    x[k] += deltaX[k];
                }
            }
        }
    }
}

void Scene::applyPBD(
    Object& object,
    float deltaTime
)
{
    const auto& mesh = object.getMesh();
    const auto& distanceConstraints = mesh.distanceConstraints;
    const auto& volumeConstraints = mesh.volumeConstraints;
    const auto& perEnvCollisionConstraints = mesh.perEnvCollisionConstraints;

    auto& vertexTransforms = object.getVertexTransforms();
    const size_t numVerts = vertexTransforms.size();

    std::vector<float> M = object.getMass();
    std::vector<glm::vec3> x(numVerts, glm::vec3(0.0f));
    std::vector<glm::vec3> v(numVerts, glm::vec3(0.0f));
    std::vector<glm::vec3> a(numVerts, glm::vec3(0.0f));
    std::vector<glm::vec3> p(numVerts, glm::vec3(0.0f));
    std::vector<glm::vec3> posDiff(numVerts, glm::vec3(0.0f));
    std::vector<glm::vec3> deltaX(numVerts, glm::vec3(0.0f));

    int subStep = 1;
    const int n = m_pbdSubsteps;
    float deltaTime_s = deltaTime / static_cast<float>(n);

    float alphaTilde;
    float betaTilde;
    float gamma;

    while (subStep < n + 1)
    {
        for (size_t i = 0; i < numVerts; ++i)
        {
            const Transform& vertexTransform = vertexTransforms[i];
            a[i] = vertexTransform.getAcceleration();
            v[i] = vertexTransform.getVelocity() + deltaTime_s * a[i];
            x[i] = vertexTransform.getPosition() + deltaTime_s * v[i];
            p[i] = vertexTransform.getPosition();
            posDiff[i] = x[i] - p[i];
        }

        // Environment Collision constraints
        if (m_enableEnvCollisionConstraints)
        {
            alphaTilde = 0.0f;
            betaTilde = 0.0f;
            gamma = 0.0f;
            solveEnvCollisionConstraints(
                x,
                posDiff,
                M,
                alphaTilde,
                gamma,
                perEnvCollisionConstraints
            );
        }

        // Distance constraints
        if (m_enableDistanceConstraints)
        {
            alphaTilde = m_alpha / (deltaTime_s * deltaTime_s);
            betaTilde = (deltaTime_s * deltaTime_s) * m_beta;
            gamma = (alphaTilde * betaTilde) / deltaTime_s;
            solveDistanceConstraints(
                x,
                posDiff,
                M,
                alphaTilde,
                gamma,
                distanceConstraints
            );
        }

        // Volume constraints
        if (m_enableVolumeConstraints)
        {
            alphaTilde = m_alpha / (deltaTime_s * deltaTime_s);
            betaTilde = (deltaTime_s * deltaTime_s) * m_beta;
            gamma = (alphaTilde * betaTilde) / deltaTime_s;
            solveVolumeConstraints(
                x,
                posDiff,
                M,
                alphaTilde,
                gamma,
                volumeConstraints
            );
        }

        // Update positions and velocities
        for (size_t i = 0; i < numVerts; ++i)
        {
            Transform& vertexTransform = vertexTransforms[i];

            glm::vec3 newX = x[i];
            glm::vec3 newV = (newX - p[i]) / deltaTime_s;

            vertexTransform.setPosition(newX);
            vertexTransform.setVelocity(newV);
        }

        subStep++;
    }
}

void Scene::update(float deltaTime)
{
    m_camera->setDeltaTime(deltaTime);
    // m_camera->move();

    // gravity and PBD
    for (auto& object : m_objects)
    {
        Transform& transform = object->getTransform();
        transform.setView(*m_camera);

        if (!object->isStatic())
        {
            applyGravity(*object, deltaTime);
            applyPBD(*object, deltaTime);
        }

        object->update(deltaTime);
    }
}

void Scene::render()
{
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glClearColor(0.1f, 0.5f, 0.4f, 1.0f); // background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (const auto& object : m_objects)
    {
        object->render();
    }

}

void Scene::clear()
{
    m_textureManager->deleteAllResources();
    m_meshManager->deleteAllResources();
    m_shaderManager->deleteAllResources();
    m_objects.clear();

    std::cout << m_name << " cleared.\n";
}