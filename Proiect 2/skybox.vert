#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    // Scalam pozitia skybox-ului pentru a depasi zNear-ul mare (100.0)
    // Scalez la 2000.0 pentru siguranta maxima (zNear=100.0)
    vec4 pos = projection * view * vec4(aPos * 2000.0, 1.0);
    // XYWW asigura ca dupa impartirea la w, z va fi mereu 1.0 (adancimea maxima)
    gl_Position = pos.xyww;
}
