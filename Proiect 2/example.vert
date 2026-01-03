#version 330 core

layout (location = 0) in vec4 in_Position;
layout (location = 1) in vec3 in_Color;
// ADAGUARE: Coordonate de texturare
layout (location = 2) in vec2 in_TexCoord; 

out vec3 ex_Color;
// ADAGUARE: Transmitem coordonatele la fragment shader
out vec2 ex_TexCoord; 

uniform mat4 viewModel;
uniform mat4 projection;

void main()
{
    gl_Position = projection * viewModel * in_Position;
    ex_Color = in_Color;
    
    // Pasarea coordonatelor mai departe
    ex_TexCoord = in_TexCoord; 
}