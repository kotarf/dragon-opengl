//
// Created by francisk on 4/15/23.
//

#ifndef DRAGON_GL_SCENE_H
#define DRAGON_GL_SCENE_H

#include <iostream>
#include <filesystem>
#include <vector>
#include <memory>

/* OpenGL headers */
#include <glad/glad.h>
#include <GLFW/glfw3.h> // after glad.h
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>  // Debugging via glm::to_string

/* Internal */
#include "attributes.h"
#include "../load-utils/load_utils.h"
#include "../load-utils/image.h"

using VertexListPtr = std::unique_ptr<Vertex[]>;
using BufferHandle = GLuint;

struct InputOptions {
    ModelChoice model = ModelChoice::dragon_obj;
    std::optional<ShadingOption> opt;
    bool save_image = false;
};

struct BufferParams {
    GLuint vao;
    VertexListPtr vertex_list;
};

struct ShaderParams {
    GLuint program;
};

struct SceneParams {
    BufferParams buffer_tris;
    BufferHandle transforms_handle;
    unsigned int vertices_count_tris;
};

struct DestroyGLFWindow{
    // https://stackoverflow.com/a/35794019
    void operator()(GLFWwindow* ptr){
        glfwDestroyWindow(ptr);
    }
};

using WindowPtr = std::unique_ptr<GLFWwindow, DestroyGLFWindow>;

// Inputs
const std::string dragon_model_str = "dragon";
const std::string dragon_model_off_str = "dragon_off";
const std::string bunny_model_str = "bunny";
const std::string save_to_image_str = "image";
const std::string flat_str = "flat";
const std::string wireframe_str = "wireframe";

// Camera
const VecPosition eye_pos(0,0,3);
const VecDirection look_up(0,1,0);
const VecDirection gaze_dir(0,0,-1);

// Lighting
const VecColor light_color(.3f, .45f, .3f);

// Perspective
const float fov_initial = 45.0;
const float near_plane = 0.1;
const float far_plane = 10.0f;

// Initial Window size
const unsigned int width_init = 1000;
const unsigned int height_init = 1000;

// Controls
const double zoom_tick = .09;
const float rotation_tick = 1.35f;

// Texture names
const std::string color_texture_name = "diffuseMap";
const std::string normal_texture_name = "normalMap";

// Shader error log size
const short shader_log_buffer_size = 512;

// Antialiasing
const short antialiasing_subsamples = 8;

// these fields can be changed through user input
// in a game, this would probably be behind a semaphore
struct SceneGlobals {
    unsigned int width = width_init;
    unsigned int height = height_init;
    float rotate_x = 0.0;  // degrees
    float rotate_y = 0.0;  // degrees
    float fov = fov_initial;

    volatile bool dirty_ = false;
};

static void ErrorCallback([[maybe_unused]] int error, const char* description);
static void ScrollCallback([[maybe_unused]] GLFWwindow* window, [[maybe_unused]] double xoffset, double yoffset);
static void FrameBufferSizeCallback([[maybe_unused]] GLFWwindow* window, int width, int height);
static void InputCallback(GLFWwindow* window, int key, [[maybe_unused]] int scancode,
                          [[maybe_unused]] int action, [[maybe_unused]] int mods);

void SetResizeCallback(const WindowPtr &window_ptr);

GlmVec4 GetLightPosition(ModelChoice model);
GlmMat4 GetWorldSpaceMatrix(ModelChoice model, const SceneGlobals &scene_globals);
GlmMat4 GetViewMatrix();
GlmMat4 GetPerspectiveMatrix(double fov, double aspect_ratio, double near, double far);
GlmMat4 GetNormalUpdateMatrix(const GlmMat4 &model_view);

unsigned int LoadTexture(const std::string &filename);

WindowPtr InitializeWindow(int width, int height, const std::string& title, SceneGlobals &scene_globals);
GLuint InitTransformUniforms(ModelChoice model, const SceneGlobals &scene_globals);
void InitLightingUniforms(ModelChoice model);
BufferHandle InitializeUniforms(ModelChoice model, SceneGlobals &scene_globals);
void UpdateTransformUniforms(const BufferHandle &ubo_matrices, ModelChoice model, const SceneGlobals &scene_globals);

BufferParams CreateVertexBuffer(const std::vector<Vertex>& vertices);
GLuint CompileShader(const std::string& path, GLenum shader_type);
std::pair<unsigned int, unsigned int> CreateTextures();
ShaderParams CreateShaderProgram(const std::string& vertex_shader_path, const std::string& fragment_shader_path);
SceneParams CreateScene(ModelChoice model, ShadingOption opt, SceneGlobals &scene_globals);

void SaveToFile(const WindowPtr &window);
InputOptions ParseArgs(const int &argc, char* argv[]);

#endif
