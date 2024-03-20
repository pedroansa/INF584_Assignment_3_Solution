#version 410 core

// Saída do fragment shader, a profundidade e a profundidade ao quadrado.
layout(location = 0) out vec4 fragDepth;

void main() {
    // Calcula a profundidade do fragmento atual normalizada para o intervalo [0,1].
    float depth = gl_FragCoord.z / gl_FragCoord.w;

    // Armazena a profundidade e a profundidade ao quadrado na saída.
    // As componentes z e w são deixadas como 0.0 e 1.0, respectivamente, podendo ser usadas para outros dados se necessário.
    fragDepth = vec4(depth, depth * depth, 0.0, 1.0);
}
