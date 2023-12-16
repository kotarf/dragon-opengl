#version 420 core

// Vertex attributes
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

// Uniform variables
layout (std140, binding=0) uniform Matrices
{
    mat4 world;
    mat4 view;
    mat4 projection;
    mat4 normal_to_view;
    mat4 normal_to_world;
};

layout (std140, binding=1) uniform Lighting
{
    vec4 eyePos;
    vec4 lightPos;
    vec4 lightColor;
};

// Outputs
out vec3 oColor;

// Forward declarations
vec3 lighting(in vec3 vertex_pos, in vec3 light_pos, in vec3 eye_pos, in vec3 normal,
    in vec3 color_mat, in vec3 color_light);

void main() {
    vec3 lightpos_vs;
    vec3 eyepos_vs = vec3(0,0,0);

    // Model space -> View
    gl_Position = view * world * vec4(aPos, 1.0);

    // World space -> View
    lightpos_vs = (view * lightPos).xyz;

    // Update vertex normal from model space to view space, and normalize
    vec3 normal_vs = normalize(mat3(normal_to_view) * aNormal);

    // Compute lighting, in view space
    oColor = lighting(gl_Position.xyz, lightpos_vs, eyepos_vs, normal_vs, lightColor.xyz, lightColor.xyz);

    // Camera -> Perspective
    gl_Position = projection * gl_Position;
}

vec3 lighting(in vec3 vertex_pos, in vec3 light_pos, in vec3 eye_pos, in vec3 normal,
    in vec3 color_mat, in vec3 color_light)
{
    // Computes ambient, diffuse, and specular contributions and the overall color
    vec3 light_vec = light_pos - vertex_pos.xyz;
    vec3 light_dir = normalize(light_vec);
    float light_dist = length(light_vec);
    vec3 view_dir = normalize(eye_pos - vertex_pos);

    // Ambient contribution (very weak)
    vec3 ambient = .005 * color_light;

    // Diffuse contribution
    float diffuse = max(dot(normal, light_dir), 0.0);

    // Specular contribution
    vec3 h_vector = normalize(view_dir + light_dir);
    float specular = pow(max(dot(normal, h_vector), 0.0), 256.0);

    // Constant, linear and quadratic falloff
    float attenuation = 1.0 / (1.0f + 0.07f * light_dist + .017f * (light_dist * light_dist));

    vec3 diffuse_color = diffuse * mix(color_light, color_mat, .75);
    vec3 specular_color = specular * mix(color_light, color_mat, .75);

    diffuse *= attenuation;
    specular *= attenuation;

    // Combine lighting contributions
    vec3 color = ambient + diffuse_color + specular_color;

    return color;
}
