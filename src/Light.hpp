#pragma once

#include <glm/glm.hpp>

class Light
{
public:
    Light(glm::vec3 lightPos);

    const glm::vec3& getPosition() const { return m_lightPos; }
    void setPosition(const glm::vec3& position) { m_lightPos = position; }
    const glm::vec3& getColor() const { return m_color; }

private:
    glm::vec3 m_lightPos;
    glm::vec3 m_color;
};