#version 330 core

in vec3 ex_Color;
in vec2 ex_TexCoord;
// Variabile din shader-ul de varfuri
in vec3 ex_Normal;
in vec3 ex_FragPos;

out vec4 out_Color;

uniform int codCol;
uniform int objectType;
uniform sampler2D myTexture;
uniform vec3 lightPos; //Pozitia soarelui

void main()
{
    vec4 baseColor;

    if (codCol == 0) 
    {
        // Toate planetele (1-Soare .. 10-Neptun) folosesc acum textura
        if (objectType == 1) 
        {
             // Soarele - mai stralucitor
             baseColor = texture(myTexture, ex_TexCoord) * 1.5;
        }
        else if (objectType >= 2 && objectType <= 10) 
        // 1-Soare, 2-Pamant, 3-Luna, 4-Mercur, 5-Venus, 6-Marte, 7-Jupiter, 8-Saturn, 9-Uranus, 10-Neptun
        {
             baseColor = texture(myTexture, ex_TexCoord);
        }
        else if (objectType == 11)
        {
             // Pluto removed, keeping generic fallback or ignore
            baseColor = vec4(0.6, 0.5, 0.5, 1.0);
        }
        else if (objectType == 12) // Orbita
        {
             baseColor = vec4(1.0, 1.0, 1.0, 1.0);
        }
        else if (objectType == 13) // Inelele lui Saturn
        {
            // Culoare bej cu transparenta (Alpha = 0.6)
            // Curs Pag 3: Alpha < 1.0 inseamna transparent
            baseColor = vec4(0.8, 0.7, 0.5, 0.55); 
        }
        else if (objectType == 14) // Asteroid
        {
            baseColor = vec4(ex_Color, 1.0);
        }
        else if (objectType == 20) // Atmosfera Pamant
        {
             vec3 viewDir = normalize(-ex_FragPos);
             vec3 normal = normalize(ex_Normal);
             float dotProduct = max(dot(viewDir, normal), 0.0);
             // Efect Fresnel pentru glow la margine
             // Alpha este mic in centru (dot ~ 1) si mare la margine (dot ~ 0)
             // Inversam: 1 - dot
             // Crestem puterea la 4.0 pentru un gradient mai fin spre margine
             float alpha = pow(1.0 - dotProduct, 4.0); 
             // Culoare albastra pentru atmosfera, cu alpha redus global (0.65)
             baseColor = vec4(0.0, 0.5, 1.0, alpha * 0.65);
        }
        else
        {
            baseColor = vec4(ex_Color, 1.0);
        }
    }
    else 
    {
        baseColor = vec4(1.0, 1.0, 1.0, 1.0);
    }

    
    // Soarele emite lumina, nu are umbra
    if (objectType == 1 || objectType == 12 || codCol != 0 || objectType == 13) 
    {
        out_Color = baseColor; 
        return; // Inelul nu primeste umbre complexe in aceasta faza
    }

    // implementare formule curs 10 

    // proprietati lumina
    vec3 lightColor = vec3(2.5, 2.5, 2.5); // Increased light intensity for distant planets
    float ambientStrength = 0.15;
    float specularStrength = 0.5;
    float shininess = 32.0;

    vec3 N = normalize(ex_Normal);
    vec3 L = normalize(lightPos - ex_FragPos); // vector spre sursa de lumina
    vec3 V = normalize(-ex_FragPos); // vector spre observator

    vec3 ambient = ambientStrength * lightColor;

    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 R = reflect(-L, N);
    float spec = pow(max(dot(V, R), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    // atenuare (lumina scade cu distanta)
    float d = length(lightPos - ex_FragPos);
    float attenuation = 1.0 / (1.0 + 0.0002 * d + 0.000001 * d * d);

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    // sa se faca lumina
    vec3 result = (ambient + diffuse + specular) * baseColor.rgb;
    out_Color = vec4(result, baseColor.a);
}