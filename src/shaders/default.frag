#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec3 vertexColor;

uniform sampler2D ourTexture;

void main()
{
    FragColor = vec4(vertexColor, 1.0);
}
