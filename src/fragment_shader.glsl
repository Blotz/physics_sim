#version 330 core

in vec3 fragColor; // Input color from vertex shader
out vec4 color;    // Output color

void main() {
    color = vec4(fragColor, 1.0); // Set the output color
}
