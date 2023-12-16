#version 420 core
/* The only difference between this and the Gouraud shader is the 'flat' keyword below */

// Inputs
flat in vec3 oColor; /* https://www.khronos.org/opengl/wiki/Type_Qualifier_(GLSL)#Interpolation_qualifiers */

// Outputs
out vec3 outColor;

void main() {
    outColor = oColor;
}
