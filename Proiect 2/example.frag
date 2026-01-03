#version 330 core

in vec3 ex_Color;
// ADAGUARE: Coordonatele interpolate
in vec2 ex_TexCoord; 

out vec4 out_Color;

uniform int codCol;
uniform int objectType;

// ADAGUARE: Variabila uniforma pentru textura
uniform sampler2D myTexture; 

void main()
{
    if (codCol == 0) // Randare normala
    {
        // 1 = Soare (pastram culoarea procedurala sau punem o textura de Soare daca vrei)
        if (objectType == 1) 
        {
             out_Color = vec4(1.0, 0.9, 0.2, 1.0);
        }
        // 2 = Pamant, 3 = Luna -> APLICAM TEXTURA
        else if (objectType == 2 || objectType == 3) 
        {
            // Functia texture() combina sampler-ul cu coordonatele
            out_Color = texture(myTexture, ex_TexCoord);
        }
        else
        {
            out_Color = vec4(ex_Color, 1.0);
        }
    }
    else // Randare solida (pentru debug etc.)
    {
        out_Color = vec4(1.0, 1.0, 1.0, 1.0); 
    }
}