#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform vec3 color;
uniform sampler2D ourTexture;

void main()
{
    vec2 scaledTexCoord = TexCoord * 100.0f;
    FragColor = texture(ourTexture, scaledTexCoord);
}