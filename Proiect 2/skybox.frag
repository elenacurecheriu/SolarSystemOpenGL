#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

// Pseudo-random number generator function
float random(vec3 st) {
    return fract(sin(dot(st.xyz, vec3(12.9898, 78.233, 45.164))) * 43758.5453123);
}

void main()
{    
    vec3 direction = normalize(TexCoords);
    
    // 1. Background color (Deep Space - Dark Blue/Black)
    vec3 color = vec3(0.01, 0.02, 0.05); 
    
    // 2. Procedural Stars
    // Map the direction to a grid to ensure uniform distribution
    vec3 coord = direction * 300.0; // Increase this number for denser/smaller stars
    vec3 i = floor(coord);
    
    // Generate a random value for this grid cell
    float r = random(i);
    
    // If the random value is above a threshold, draw a star
    // Threshold 0.99 means top 1% of cells will have a star
    if(r > 0.99) {
        // Vary star brightness
        float brightness = (r - 0.99) / (1.0 - 0.99);
        color += vec3(brightness);
    }
    
    // 3. Optional: Add a subtle horizon glow (if Y is close to 0)
    // float horizon = 1.0 - abs(direction.y);
    // color += vec3(0.1, 0.1, 0.2) * pow(horizon, 5.0);

    FragColor = vec4(color, 1.0);
}