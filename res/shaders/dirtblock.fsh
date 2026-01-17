#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform sampler2D ourTexture;
uniform bool hasTexture;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    float ambientStrength = 0.5;
    vec3 ambient = ambientStrength * lightColor;
    vec3 result = (ambient + diffuse) * objectColor;
    if (hasTexture)
    {
        vec4 texColor = texture(ourTexture, TexCoord);
        FragColor = texColor * vec4(result, 1.0f);
    }
    else
    {
        FragColor = vec4(result, 1.0f);
    }
}
