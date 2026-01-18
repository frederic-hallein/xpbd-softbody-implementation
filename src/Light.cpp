#include <glm/glm.hpp>

#include "Light.hpp"

Light::Light(
    glm::vec3 lightPos
)
    : m_lightPos(lightPos),
      m_color(glm::vec3(1.0, 1.0f, 1.0f))
{
}