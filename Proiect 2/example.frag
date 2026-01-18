#version 330 core

in vec3 ex_Color;
in vec2 ex_TexCoord;
in vec3 ex_Normal;
in vec3 ex_FragPos;

out vec4 out_Color;

uniform int codCol;
uniform int objectType;
uniform sampler2D myTexture;
uniform vec3 lightPos; 

void main()
{
    vec4 baseColor;

    if (codCol == 0) 
    {
        if (objectType == 1) 
        {
             baseColor = texture(myTexture, ex_TexCoord) * 1.5;
        }
        else if (objectType >= 2 && objectType <= 10) 
        {
             baseColor = texture(myTexture, ex_TexCoord);
        }
        else if (objectType == 12) // orbita
        {
             baseColor = vec4(1.0, 1.0, 1.0, 1.0);
        }
        else if (objectType == 13) // inel Saturn
        {
            // culoare bej cu transparenta (alpha = 0.6)
            // alpha < 1.0 => transparent
            baseColor = vec4(0.8, 0.7, 0.5, 0.55); 
        }
        else if (objectType == 14) // asteroid
        {
            baseColor = vec4(ex_Color, 1.0);
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

    
    // Soarele emite lumina, nu are umbra, iar inelul lui Saturn si desenarea orbitelor nu ar trebui afectate
    if (objectType == 1 || objectType == 12 || codCol != 0 || objectType == 13) 
    {
        out_Color = baseColor; 
        return;
    }

    // implementare formule curs 10 

    // proprietati lumina
    vec3 lightColor = vec3(2.5, 2.5, 2.5); 
    float ambientStrength = 0.15;
    float specularStrength = 0.5;
    float shininess = 32.0;

    vec3 N = normalize(ex_Normal);
    vec3 L = normalize(lightPos - ex_FragPos); 
    vec3 V = normalize(-ex_FragPos);

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