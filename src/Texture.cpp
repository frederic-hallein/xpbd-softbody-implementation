#include "logger.hpp"
#include "Texture.hpp"

Texture::Texture(
    const std::string& name,
    const std::string& texturePath
)
    : m_name(name), m_texturePath(texturePath)
{
    glGenTextures(1, &m_ID);

    // Load the texture
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrComponents;
    unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &nrComponents, 3);
    if (data)
    {
        glBindTexture(GL_TEXTURE_2D, m_ID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Set texture wrapping and filtering options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        logger::error("Failed to load texture: {}", texturePath);
        stbi_image_free(data);
    }
}

void Texture::bind()
{
    glBindTexture(GL_TEXTURE_2D, m_ID);
}

void Texture::destroy()
{
    glDeleteTextures(1, &m_ID);
}