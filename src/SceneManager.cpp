#include "logger.hpp"
#include "ResourceConfig.hpp"
#include "SceneManager.hpp"

SceneManager::SceneManager(
    GLFWwindow* window,
    unsigned int screenWidth,
    unsigned int screenHeight,
    ShaderManager* shaderManager,
    MeshManager* meshManager,
    TextureManager* textureManager
)
    : m_window(window),
      m_screenWidth(screenWidth),
      m_screenHeight(screenHeight),
      m_shaderManager(shaderManager),
      m_meshManager(meshManager),
      m_textureManager(textureManager),
      m_currentSceneName("")
{
}

void SceneManager::createScene(const std::string& sceneName, const std::string& sceneFilename)
{
    const std::string scenePath = "../scenes/" + sceneFilename;
    try {
        m_scenes[sceneName] = std::make_unique<Scene>(
            m_window,
            m_screenWidth,
            m_screenHeight,
            m_shaderManager,
            m_meshManager,
            m_textureManager
        );
        m_scenes[sceneName]->loadSceneConfig(scenePath);
        logger::info(" - Created '{}' scene successfully", sceneName);
    } catch (const std::exception& e) {
        logger::error("Failed to load scene '{}': {}", sceneName, e.what());
    }
}

void SceneManager::createScenes()
{
    logger::info("Creating scenes...");
    for (const auto& [sceneName, sceneFilename] : SCENE_LIST) {
        createScene(std::string(sceneName), std::string(sceneFilename));
    }
}

Scene* SceneManager::getCurrentScene()
{
    auto it = m_scenes.find(m_currentSceneName);
    if (it == m_scenes.end()) {
        return nullptr;
    }
    return it->second.get();
}

void SceneManager::switchScene(const std::string& sceneName)
{
    auto it = m_scenes.find(sceneName);
    if (it == m_scenes.end()) {
        logger::error("Scene '{}' not found", sceneName); // TODO : better handle scene not found error
        return;
    }

    m_currentSceneName = sceneName;
    logger::info("Switched to scene: {}", sceneName);
}

void SceneManager::clearScenes()
{
    logger::info("Clearing scenes...");
    for (auto& [name, scene] : m_scenes) {
        scene->clear();
    }
}