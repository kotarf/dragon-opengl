#version 420 core

// Inputs
in VS_OUTPUT {
    vec3 oPosTangentSpace; // computed
    vec3 oLightPosTangentSpace; // computed
    vec3 oEyePosTangentSpace; // computed
    vec2 oTextureCoords; // forwarded
} vs_inputs;

// Uniform variables
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

// Uniform textures
uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

// Outputs
layout(location = 0) out vec3 outColor;

// Forward declarations
vec3 lighting(in vec3 vertex_pos, in vec3 light_pos, in vec3 eye_pos, in vec3 normal,
    in vec3 color_mat, in vec3 color_light);

void main() {
    // Sample from diffuse map
    vec3 color_texture = texture(diffuseMap, vs_inputs.oTextureCoords).xyz;

    // Sample from normal map
    vec3 normal_ts = texture(normalMap, vs_inputs.oTextureCoords).xyz;

    // Transform sampled normal to range [-1,1]
    normal_ts = normalize(2.0 * normal_ts - vec3(1.0, 1.0, 1.0));

    // Compute lighting in tangent space; normal is in tangent space
    outColor = lighting(vs_inputs.oPosTangentSpace, vs_inputs.oLightPosTangentSpace,
                        vs_inputs.oEyePosTangentSpace, normal_ts, color_texture, lightColor.xyz);
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
