#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

float random(vec3 st) {
    return fract(sin(dot(st.xyz, vec3(12.9898, 78.233, 45.164))) * 43758.5453123);
}

void main()
{    
    vec3 direction = normalize(TexCoords);
    
    vec3 color = vec3(0.01, 0.02, 0.05); 
    
    // generare procedurala stele
    vec3 coord = direction * 300.0; 
    vec3 i = floor(coord);
    
    float r = random(i);
    
    if(r > 0.99) 
    {
        float brightness = (r - 0.99) / (1.0 - 0.99);
        color += vec3(brightness);
    }
    
   
    float horizon = 1.0 - abs(direction.y);
    color += vec3(0.1, 0.1, 0.2) * pow(horizon, 5.0);

    FragColor = vec4(color, 1.0);
}