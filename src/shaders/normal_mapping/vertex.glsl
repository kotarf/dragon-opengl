#version 420 core

// Vertex attributes
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec2 aTextureCoords;

// Uniform attributes
layout (std140, binding=0) uniform Matrices
{
    mat4 world;
    mat4 view;
    mat4 projection;
    mat4 normalToView;
    mat4 normalToWorld;
};

layout (std140, binding=1) uniform Lighting
{
    vec4 eyePos;
    vec4 lightPos;
    vec4 lightColor;
};

// Outputs
out VS_OUTPUT {
    vec3 oPosTangentSpace; // computed
    vec3 oLightPosTangentSpace; // computed
    vec3 oEyePosTangentSpace; // computed
    vec2 oTextureCoords; // forwarded
} outputs;

// Forward declarations
mat3 tbn_matrix();

void main() {
    mat3 tbn;

    // Model space -> Perspective
    gl_Position = projection * view * world * vec4(aPos, 1.0);

    // World space -> Tangent space
    tbn = tbn_matrix();

    // World space -> Tangent space transforms to vertex, light and eye positions
    outputs.oPosTangentSpace = tbn * vec3(world * vec4(aPos, 1.0));
    outputs.oLightPosTangentSpace = tbn * lightPos.xyz;
    outputs.oEyePosTangentSpace = tbn * eyePos.xyz;

    // Forward texture coords
    outputs.oTextureCoords = aTextureCoords;
}

mat3 tbn_matrix() {
    vec3 tangent_ws = normalize(mat3(normalToWorld) * aTangent);
    vec3 normal_ws = normalize(mat3(normalToWorld) * aNormal);  // "N"

    // Orthonormalize via Gram–Schmidt process
    tangent_ws = normalize(tangent_ws - dot(tangent_ws, normal_ws) * normal_ws);  // "T"

    // Compute bitangent
    vec3 bitangent_ws = cross(normal_ws, tangent_ws);  // "B"

    // "TBN" matrix
    return transpose(mat3(tangent_ws, bitangent_ws, normal_ws));
}
