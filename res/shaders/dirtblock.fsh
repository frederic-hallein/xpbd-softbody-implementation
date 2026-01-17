#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform sampler2D ourTexture;
uniform bool hasTexture;

void main()
{
    if (hasTexture)
    {
        vec4 texColor = texture(ourTexture, TexCoord);
        FragColor = texColor * vec4(objectColor, 1.0f);
    }
    else
    {
        FragColor = vec4(objectColor, 1.0f);
    }
}
