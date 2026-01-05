#version 330 core

layout (location = 0) in vec4 in_Position;
layout (location = 1) in vec3 in_Color;
layout (location = 2) in vec2 in_TexCoord;
layout (location = 3) in vec3 in_Normal;

out vec3 ex_Color;
out vec2 ex_TexCoord;
out vec3 ex_Normal;
out vec3 ex_FragPos; 

uniform mat4 viewModel;
uniform mat4 projection;

void main()
{
    gl_Position = projection * viewModel * in_Position;
    ex_Color = in_Color;
    ex_TexCoord = in_TexCoord;

    ex_FragPos = vec3(viewModel * in_Position);
    
    mat3 normalMatrix = transpose(inverse(mat3(viewModel)));
    ex_Normal = normalize(normalMatrix * in_Normal);
}