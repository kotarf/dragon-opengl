#include "pipeline/scene.h"

int main(int argc, char* argv[]) {
    // Handle arguments
    auto input_options = ParseArgs(argc, argv);

    // Scene inputs
    auto model_choice = input_options.model;  // can set to ModelChoice::dragon_off etc
    auto save_to_image = input_options.save_image;

    ShadingOption render_mode;

    if(!input_options.opt.has_value()) {
        // No additional shading option specified
        render_mode = model_choice == ModelChoice::dragon_off || model_choice == ModelChoice::bunny_off ?
                      ShadingOption::per_vertex : ShadingOption::normal_mapping;
    } else {
        // Special shading option (flat, wireframe) specified
        render_mode = input_options.opt.value();
    }

    // Globals
    static SceneGlobals scene_globals;

    // Initialize GLFW window
    auto window = InitializeWindow(width_init, height_init, "Dragon OpenGL", scene_globals);

    // Read mesh, initialize uniforms and create vertex buffers
    auto scene_params = CreateScene(model_choice, render_mode, scene_globals);

    auto buffer_tris = scene_params.buffer_tris.vao;

    auto vertex_shader_path = GetVertexShaderPath(render_mode);
    auto fragment_shader_path = GetFragmentShaderPath(render_mode);

    // Create and link shaders, and load textures
    ShaderParams shader_program = CreateShaderProgram(vertex_shader_path,
                                                      fragment_shader_path);

    // Depth buffer
    glEnable(GL_DEPTH_TEST);

    // Install shader
    glUseProgram(shader_program.program);

    // free vertex lists
    scene_params.buffer_tris.vertex_list.reset();

    // set texture uniforms
    glUniform1i(glGetUniformLocation(shader_program.program, color_texture_name.c_str()),
                0);
    glUniform1i(glGetUniformLocation(shader_program.program, normal_texture_name.c_str()),
                1);

    // initial viewport dimensions
    glViewport(0, 0, scene_globals.width, scene_globals.height);

    // Enable wireframe if provided as an input
    if(input_options.opt == ShadingOption::wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    // resize callback
    SetResizeCallback(window);

    while (!glfwWindowShouldClose(window.get())) {
        // new frame - clear color and depth buffers
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // update uniforms based on glfw events and callbacks
        if(scene_globals.dirty_) {
            UpdateTransformUniforms(scene_params.transforms_handle, model_choice, scene_globals);

            // Uniforms are up-to-date now
            scene_globals.dirty_ = false;
        }

        // render
        glBindVertexArray(buffer_tris);
        glDrawArrays(GL_TRIANGLES, 0, scene_params.vertices_count_tris);

        // swap buffers and poll for user input
        glfwSwapBuffers(window.get());

        if(save_to_image) {
            // Save to a png
            SaveToFile(window);
        }

        glfwPollEvents();
    }
    // on exit clean up / free operations
    glDeleteBuffers(1, &buffer_tris);

    glfwTerminate();

    exit(EXIT_SUCCESS);
}
