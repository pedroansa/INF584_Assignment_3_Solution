#version 410 core

layout(location = 0) out vec4 fragmentColor;

void main() {
    float depth = gl_FragCoord.z;
    // Store the depth and its square
    fragmentColor = vec4(depth, depth * depth, 0.0, 0.0);
}