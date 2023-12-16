//
// Created by francisk on 4/15/23.
//

#include "scene.h"

/* GLFW callbacks */
static void ErrorCallback([[maybe_unused]] int error, const char *description) {
    // on GLFW error, eg. window initialization
    std::cerr << description << std::endl;
}

static void ScrollCallback([[maybe_unused]] GLFWwindow *window, [[maybe_unused]] double xoffset, double yoffset) {
    // Reference to globals is stored in the GLFW window
    auto scene_globals_ref = static_cast<SceneGlobals *>(glfwGetWindowUserPointer(window));

    // Adjust FOV
    scene_globals_ref->fov -= (float) (yoffset * zoom_tick);

    if (scene_globals_ref->fov < 10.0) {
        scene_globals_ref->fov = 10.0;
    }
    if (scene_globals_ref->fov > 60.0) {
        scene_globals_ref->fov = 60.0;
    }

    scene_globals_ref->dirty_ = true;
}

static void FrameBufferSizeCallback([[maybe_unused]] GLFWwindow *window, int width, int height) {
    // On resize window (ie. drag cursor)
    auto scene_globals_ref = static_cast<SceneGlobals *>(glfwGetWindowUserPointer(window));

    // Save viewport dimensions
    scene_globals_ref->height = height;
    scene_globals_ref->width = width;

    scene_globals_ref->dirty_ = true;

    // Set the viewport
    glViewport(0, 0, width, height);
}

static void InputCallback(GLFWwindow *window, int key, [[maybe_unused]] int scancode,
                          [[maybe_unused]] int action, [[maybe_unused]] int mods) {
    // Callback on key press - Escape, Left arrow, Right arrow
    // Reference to globals is stored in the GLFW window
    auto scene_globals_ref = static_cast<SceneGlobals *>(glfwGetWindowUserPointer(window));

    // Escape key closes the app
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    // Rotation updates
    if (key == GLFW_KEY_LEFT) {
        scene_globals_ref->rotate_y += rotation_tick;
        scene_globals_ref->dirty_ = true;
    }
    if (key == GLFW_KEY_RIGHT) {
        scene_globals_ref->rotate_y -= rotation_tick;
        scene_globals_ref->dirty_ = true;
    }
    if (key == GLFW_KEY_UP) {
        scene_globals_ref->rotate_x -= rotation_tick;
        scene_globals_ref->dirty_ = true;
    }
    if (key == GLFW_KEY_DOWN) {
        scene_globals_ref->rotate_x += rotation_tick;
        scene_globals_ref->dirty_ = true;
    }
}

void SetResizeCallback(const WindowPtr &window_ptr) {
    // Attach resize callback
    glfwSetFramebufferSizeCallback(window_ptr.get(), FrameBufferSizeCallback);
}

GlmVec4 GetLightPosition(ModelChoice model) {
    // Returns single light source position depending on the selected model
    float light_z;

    if (model == ModelChoice::dragon_obj) {
        light_z = 1.85f;
    } else if (model == ModelChoice::dragon_off) {
        light_z = 2.15f;
    } else {
        light_z = 2.0f;
    }
    return {0, 0, light_z, 1.0f};
}

GlmMat4 GetWorldSpaceMatrix(ModelChoice model, const SceneGlobals &scene_globals) {
    /* Model Space -> World Space
     * GLM is utilized. Good resources:
     * https://web.engr.oregonstate.edu/~mjb/cs557/Handouts/GLM.1pp.pdf
     * https://open.gl/transformations
     * */
    GlmMat4 model_transform = GlmMat4(1.0f);  // identity

    // Scale
    if (model == ModelChoice::dragon_off || model == ModelChoice::bunny_off) {
        model_transform = glm::scale(model_transform, GlmVec3(1.25f, 1.25f, 1.25f));
    } else if (model == ModelChoice::dragon_obj) {
        model_transform = glm::scale(model_transform, GlmVec3(.01f, .01f, .01f));
    }

    // Translate
    if (model == ModelChoice::dragon_off) {
        model_transform = glm::translate(model_transform, GlmVec3(.1, 0.15f, 1.2f));
    } else if (model == ModelChoice::dragon_obj) {
        model_transform = glm::translate(model_transform, GlmVec3(0.0f, -35.00f, 100.0f));
    } else if (model == ModelChoice::bunny_off) {
        model_transform = glm::translate(model_transform, GlmVec3(.1, 0.15f, .2f));
    }

    // Rotation
    // https://glm.g-truc.net/0.9.9/api/a00668.html#gaee9e865eaa9776370996da2940873fd4
    model_transform = glm::rotate(model_transform, glm::radians(scene_globals.rotate_y),
                                  GlmVec3(0.0, 1.0f, 0.0));
    model_transform = glm::rotate(model_transform, glm::radians(scene_globals.rotate_x),
                                  GlmVec3(1.0, 0.0f, 0.0));

    return model_transform;
}

GlmMat4 GetViewMatrix() {
    // World Space -> View Space
    // https://glm.g-truc.net/0.9.9/api/a00668.html#gaa64aa951a0e99136bba9008d2b59c78e
    return glm::lookAt(eye_pos, eye_pos + gaze_dir, look_up);
}

GlmMat4 GetPerspectiveMatrix(double fov, double aspect_ratio, double near_z, double far_z) {
    // View Space (or Camera) -> Perspective
    // https://glm.g-truc.net/0.9.9/api/a00665.html#ga747c8cf99458663dd7ad1bb3a2f07787
    // This also adapts to viewport resize events
    GlmMat4 projection = glm::perspective(fov, aspect_ratio, near_z, far_z);

    // clamp values
    if (projection[0][0] < 0) {
        projection[0][0] = 0.0;
    }
    if (projection[1][1] < 0) {
        projection[1][1] = 0.0;
    }

    return projection;
}

GlmMat4 GetNormalUpdateMatrix(const GlmMat4 &model_view) {
    // Normals in Model Space -> World Space or View Space
    return GlmMat4(glm::transpose(glm::inverse(model_view)));
}

unsigned int LoadTexture(const std::string &filename) {
    /* Load a texture into uniform memory */
    // https://learnopengl.com
    unsigned int texture_id;

    ExistsOk(filename);

    glGenTextures(1, &texture_id);

    int width, height, nrComponents;

    ImageLoader image_loader;

    auto data = image_loader.LoadImageFile(filename, width, height, nrComponents);

    if (data) {
        GLenum format;

        if (nrComponents == 1) {
            format = GL_RED;
        } else if (nrComponents == 3) {
            format = GL_RGB;
        } else if (nrComponents == 4) {
            format = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                     GL_UNSIGNED_BYTE, data);

        // Automatically generates mipmaps for the current texture
        // https://www.khronos.org/opengl/wiki/Common_Mistakes#Automatic_mipmap_generation
        glGenerateMipmap(GL_TEXTURE_2D);  // invoked once since the texture does not change

        // Sampling settings - these will affect image quality
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    } else {
        std::cout << "Failed to load texture via stb_image" << std::endl;

        exit(EXIT_FAILURE);
    }

    return texture_id;
}

WindowPtr InitializeWindow(int width, int height, const std::string &title, SceneGlobals &scene_globals) {
    // Initializes a GLFW window
    glfwSetErrorCallback(ErrorCallback);

    // initialize glfw. this can fail due to platform issues
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    // uses 3.3 profile, which is recent (this is not the same as the OpenGL version)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Anti-aliasing 8x MSAA if it is supported
    // Assumes that "GL_MULTISAMPLE" is enabled (it is by default in modern drivers)
    // https://www.glfw.org/docs/3.3/window_guide.html#window_hints_fb
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glEnable.xhtml
    glfwWindowHint(GLFW_SAMPLES, antialiasing_subsamples);

    // create window
    WindowPtr window = WindowPtr(glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr));

    // window creation can also fail
    if (!window) {
        std::cout << "Failed to open GLFW window" << std::endl;

        glfwTerminate();

        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window.get());

    // initialize glad for loading all OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        // https://www.khronos.org/opengl/wiki/OpenGL_Loading_Library#glad_(Multi-Language_GL/GLES/EGL/GLX/WGL_Loader-Generator)
        std::cout << "Failed to initialize OpenGL context" << std::endl;

        exit(EXIT_FAILURE);
    }

    auto renderer = glGetString(GL_RENDERER);
    auto version = glGetString(GL_VERSION);

    // print information about OpenGL version and device
    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "OpenGL supported version and driver: " << version << std::endl;

    // register user callbacks
    glfwSetKeyCallback(window.get(), InputCallback);
    glfwSetScrollCallback(window.get(), ScrollCallback);

    // Associate scene globals
    glfwSetWindowUserPointer(window.get(), &scene_globals);

    return window;
}

GLuint InitTransformUniforms(ModelChoice model, const SceneGlobals &scene_globals) {
    // https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL
    unsigned int ubo_matrices;

    glGenBuffers(1, &ubo_matrices);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_matrices);
    glBufferData(GL_UNIFORM_BUFFER, 5 * sizeof(GlmMat4), nullptr, GL_STATIC_DRAW);

    // define the range of the buffer that links to a uniform binding point
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo_matrices, 0, 5 * sizeof(GlmMat4));

    // world, view and projection matrices
    UpdateTransformUniforms(ubo_matrices, model, scene_globals);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    return ubo_matrices;
}

void InitLightingUniforms(ModelChoice model) {
    // https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL
    unsigned int ubo_lighting;

    auto eye_pos4 = GlmVec4(eye_pos, 1);
    auto light_pos4 = GetLightPosition(model);
    auto light_color4 = GlmVec4(light_color, 1);

    glGenBuffers(1, &ubo_lighting);

    glBindBuffer(GL_UNIFORM_BUFFER, ubo_lighting);
    glBufferData(GL_UNIFORM_BUFFER, 3 * sizeof(GlmVec4), nullptr, GL_STATIC_DRAW);

    // initialize buffer range
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, ubo_lighting, 0, 3 * sizeof(GlmVec4));

    // bind eye pos, light pos, and light color
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GlmVec4),
                    glm::value_ptr(eye_pos4));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(GlmVec4), sizeof(GlmVec4),
                    glm::value_ptr(light_pos4));
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(GlmVec4), sizeof(GlmVec4),
                    glm::value_ptr(light_color4));

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

BufferHandle InitializeUniforms(ModelChoice model, SceneGlobals &scene_globals) {
    // world space, view, perspective
    auto handle = InitTransformUniforms(model, scene_globals);

    // eye pos, light pos, color
    InitLightingUniforms(model);

    return handle;
}

void UpdateTransformUniforms(const BufferHandle &ubo_matrices, ModelChoice model, const SceneGlobals &scene_globals) {
    // Sets uniform buffers corresponding to transformations (eg. view)
    // These can be updated via user input
    GlmMat4 world = GetWorldSpaceMatrix(model, scene_globals);
    GlmMat4 view = GetViewMatrix();

    GlmMat4 projection = GetPerspectiveMatrix(scene_globals.fov,
                                              (float) scene_globals.width / (float) scene_globals.height,
                                              near_plane, far_plane);

    // Normal updates (model -> view, model -> world)
    GlmMat4 normal_to_view = GetNormalUpdateMatrix(view * world);
    GlmMat4 normal_to_world = GetNormalUpdateMatrix(world);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo_matrices, 0, 5 * sizeof(GlmMat4));

    // World Space
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GlmMat4),
                    glm::value_ptr(world));

    // View (or Camera)
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(GlmMat4), sizeof(GlmMat4),
                    glm::value_ptr(view));

    // Perspective
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(GlmMat4), sizeof(GlmMat4),
                    glm::value_ptr(projection));

    // Normal to View
    glBufferSubData(GL_UNIFORM_BUFFER, 3 * sizeof(GlmMat4), sizeof(GlmMat4),
                    glm::value_ptr(normal_to_view));

    // Normal to World
    glBufferSubData(GL_UNIFORM_BUFFER, 4 * sizeof(GlmMat4), sizeof(GlmMat4),
                    glm::value_ptr(normal_to_world));
}

BufferParams CreateVertexBuffer(const std::vector<Vertex> &vertices) {
    // Allocates and populates vertex buffer
    // copy into C array
    VertexListPtr vertex_data = std::make_unique<Vertex[]>(vertices.size());

    std::copy(vertices.begin(), vertices.end(), vertex_data.get());

    // create the vertex array object to hold vertex positions
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // create a vertex buffer that contains all vertex attributes
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertex_data.get(),
                 GL_STATIC_DRAW);

    // specify formats of data in buffer
    // vertex position (model space)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *) (offsetof(struct Vertex, pos)));
    glEnableVertexAttribArray(0);

    // vertex normal (model space)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *) (offsetof(struct Vertex, normal)));
    glEnableVertexAttribArray(1);

    // tangent vector (tangent space)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *) (offsetof(struct Vertex, tangent)));
    glEnableVertexAttribArray(2);

    // texture coordinates (uv coords)
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *) (offsetof(struct Vertex, uv_coord)));
    glEnableVertexAttribArray(3);

    BufferParams params;

    params.vao = vao;
    params.vertex_list = std::move(vertex_data);

    return params;
}

GLuint CompileShader(const std::string &path, GLenum shader_type) {
    // Reads shaders on the local filesystem and compiles them on the device
    // https://www.khronos.org/opengl/wiki/Shader_Compilation#Shader_object_compilation
    int success;
    char info_log[shader_log_buffer_size];

    // read glsl files
    std::ifstream filestream(path);
    std::string shader_source((std::istreambuf_iterator<char>(filestream)),
                              std::istreambuf_iterator<char>());

    // create and compile the shader
    GLuint shader_handle = glCreateShader(shader_type);

    auto shader_source_cstr = shader_source.c_str();

    glShaderSource(shader_handle, 1, &shader_source_cstr, nullptr);
    glCompileShader(shader_handle);

    // check if compilation was successful
    glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(shader_handle, shader_log_buffer_size, nullptr, info_log);

        std::cout << "Error compiling shader: " << info_log << std::endl;

        exit(EXIT_FAILURE);
    }

    // return the shader handle
    return shader_handle;
}

std::pair<unsigned int, unsigned int> CreateTextures() {
    // Binds a texture to uniform memory; textures are stored uniquely
    // https://docs.gl/gl4/glBindTexture
    auto color_map_texture = LoadTexture(texture_diffuse_filename);
    auto normal_texture = LoadTexture(texture_normal_map_filename);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, color_map_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normal_texture);

    return {color_map_texture, normal_texture};
}

ShaderParams CreateShaderProgram(const std::string &vertex_shader_path,
                                 const std::string &fragment_shader_path) {
    // Create and link shaders, and load textures
    // https://docs.gl/gl4/glLinkProgram
    int success;
    char info_log[512];

    // Verify shader glsl files exist
    ExistsOk(vertex_shader_path);
    ExistsOk(fragment_shader_path);

    // create and compile shaders
    GLenum vertex_shader = CompileShader(vertex_shader_path, GL_VERTEX_SHADER);
    GLenum fragment_shader = CompileShader(fragment_shader_path, GL_FRAGMENT_SHADER);

    // link shaders
    GLuint shader_program = glCreateProgram();

    // create textures
    CreateTextures();

    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    // check for errors while linking the shaders together
    glGetShaderiv(shader_program, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(shader_program, 512, nullptr, info_log);

        std::cout << "Error linking shaders: " << info_log << std::endl;

        exit(EXIT_FAILURE);
    }

    // shader is now installed and active, clean up
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // return the shader program handle and texture ordinals
    ShaderParams shader_data{};

    shader_data.program = shader_program;

    return shader_data;
}

SceneParams CreateScene(ModelChoice model, ShadingOption opt, SceneGlobals &scene_globals) {
    /* Loads a given mode, allocates and sets uniforms, and creates vertex buffer */
    VertexList loaded_vertices;

    // Selected model is loaded into vector of structs
    if (model == ModelChoice::dragon_off) {
        loaded_vertices = LoadDragonOff(mesh_off_filename, opt);
    } else if (model == ModelChoice::dragon_obj) {
        loaded_vertices = LoadDragonObj(mesh_obj_filename, opt);
    } else if (model == ModelChoice::bunny_off) {
        loaded_vertices = LoadDragonOff(mesh_off_bunny_filename, opt);
    }

    // Uniforms initialized and set
    auto transforms_handle = InitializeUniforms(model, scene_globals);

    // Vertices initialized and set
    BufferParams buffer_tris = CreateVertexBuffer(loaded_vertices);

    SceneParams params;

    // Used in main.cpp
    params.transforms_handle = transforms_handle;
    params.buffer_tris = std::move(buffer_tris);
    params.vertices_count_tris = loaded_vertices.size();

    return params;
}

void SaveToFile(const WindowPtr &window) {
    // https://lencerf.github.io/post/2019-09-21-save-the-opengl-rendering-to-image-file/
    int width, height;

    // Delete if exists
    if (std::filesystem::exists(output_filename)) {
        std::filesystem::remove(output_filename);
    }

    glfwGetFramebufferSize(window.get(), &width, &height);

    GLsizei nr_channels = 3;

    GLsizei stride = nr_channels * width;
    stride += (stride % 4) ? (4 - stride % 4) : 0;

    GLsizei buffer_size = stride * height;

    CharBufferPtr buffer = std::make_unique<CharBuffer>(buffer_size);

    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.get()->data());

    ImageLoader image_loader;

    ImageLoader::WriteImageFile(output_filename, width, height, nr_channels,
                                stride, std::move(buffer));
}

InputOptions ParseArgs(const int &argc, char *argv[]) {
    // Reads command line arguments
    InputOptions input_opts;

    ModelChoice model_choice = ModelChoice::dragon_obj;  // the default model
    std::optional<ShadingOption> shade_opt = std::nullopt;
    bool save_image_bool = false;

    if (argc == 1) {
        model_choice = ModelChoice::dragon_obj;
    } else if (argc >= 2) {
        auto model_choice_str = std::string(argv[1]);

        if (model_choice_str == dragon_model_str) {
            model_choice = ModelChoice::dragon_obj;
        } else if (model_choice_str == dragon_model_off_str) {
            model_choice = ModelChoice::dragon_off;
        } else if (model_choice_str == bunny_model_str) {
            model_choice = ModelChoice::bunny_off;
        } else {
            std::cout << "Invalid model choice, try 'dragon' 'dragon_off' 'bunny'";

            exit(1);
        }
    }
    if (argc == 3) {
        auto extras = std::string(argv[2]);

        if (extras == save_to_image_str) {
            save_image_bool = true;
        } else if (extras == flat_str) {
            shade_opt = ShadingOption::flat;
        } else if (extras == wireframe_str) {
            shade_opt = ShadingOption::wireframe;
        } else {
            std::cout << "Invalid option, try 'image' 'flat' 'wireframe'";

            exit(1);
        }
    }
    input_opts.model = model_choice;
    input_opts.opt = shade_opt;
    input_opts.save_image = save_image_bool;

    return input_opts;
}
