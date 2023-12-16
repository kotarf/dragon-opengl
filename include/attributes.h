//
// Created by francisk on 4/16/23.
//

#ifndef DRAGON_GL_ATTRIBUTES_H
#define DRAGON_GL_ATTRIBUTES_H

// Linear algebra lib + window
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include <Eigen/Dense>

// glm aliases
using GlmVec4 = glm::vec4;
using GlmVec3 = glm::vec3;
using VecPosition = GlmVec3;
using VecDirection = GlmVec3;
using VecNormal = GlmVec3;
using VecColor = GlmVec3;
using VecTextureCoord = glm::vec2;
using GlmMat4 = glm::mat4;

enum ShadingOption { per_vertex, normal_mapping, wireframe, flat };
enum ModelChoice { dragon_off, dragon_obj, bunny_off };

// Vertex data as loaded into the shader
// Offsets (in memory) must exactly correspond to the definitions in shaders
struct Vertex {
    VecPosition pos;
    VecNormal normal;
    VecDirection tangent;
    VecTextureCoord uv_coord;
};

#endif
