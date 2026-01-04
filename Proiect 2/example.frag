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
        // 4 = Mercur (Gri/Maro)
        else if (objectType == 4)
        {
            out_Color = vec4(0.7, 0.6, 0.5, 1.0);
        }
        // 5 = Venus (Galbui)
        else if (objectType == 5)
        {
            out_Color = vec4(0.9, 0.85, 0.7, 1.0);
        }
        // 6 = Mars (Rosu)
        else if (objectType == 6)
        {
            out_Color = vec4(0.8, 0.3, 0.2, 1.0);
        }
        // 7 = Jupiter (Portocaliu/Bej)
        else if (objectType == 7)
        {
            out_Color = vec4(0.8, 0.6, 0.4, 1.0);
        }
        // 8 = Saturn (Galben pal)
        else if (objectType == 8)
        {
            out_Color = vec4(0.9, 0.8, 0.5, 1.0);
        }
        // 9 = Uranus (Cyan)
        else if (objectType == 9)
        {
            out_Color = vec4(0.5, 0.8, 0.9, 1.0);
        }
        // 10 = Neptune (Albastru)
        else if (objectType == 10)
        {
            out_Color = vec4(0.2, 0.3, 0.8, 1.0);
        }
        // 11 = Pluto (Maro deschis/Gri)
        else if (objectType == 11)
        {
            out_Color = vec4(0.6, 0.5, 0.5, 1.0);
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