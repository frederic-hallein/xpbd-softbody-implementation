#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform sampler2D ourTexture;
uniform bool hasTexture;

void main()
{
    float ambientStrength = 0.01f;
    float specularStrength = 0.01f;

    // Lighting calculations
    vec3 ambient = ambientStrength * lightColor;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = diff * lightColor;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;

    // Checkerboard pattern
    float scale = 10.0; // Controls size of squares
    float checker = mod(floor(TexCoord.x * scale) + floor(TexCoord.y * scale), 2.0);
    vec3 checkerColor = mix(vec3(1.0), vec3(0.0), checker); // White and black

    FragColor = vec4(checkerColor * result, 1.0);
}
