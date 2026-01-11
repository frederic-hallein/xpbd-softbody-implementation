#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform vec3 color;
uniform sampler2D ourTexture;

void main()
{
    // vec3 baseColor = vec3(1.0, 1.0, 1.0);
    // FragColor = vec4(baseColor, 1.0);
    FragColor = texture(ourTexture, TexCoord);
}