//
// Created by francisk on 4/29/23.
//
#include "image.h"

#define STB_IMAGE_IMPLEMENTATION   // see stb_image.h comments
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

ImagePointer ImageLoader::LoadImageFile(const std::string& image_filename, int &width, int &height,
                                        int &components) {
    // Invert y-axis on load (critical for textures)
    stbi_set_flip_vertically_on_load(true);

    auto stbi_data = stbi_load(image_filename.c_str(), &width, &height,
                               &components, 0);

    image_data = std::make_unique<ImagePointer>(stbi_data);

    return stbi_data;
}

ImagePointer ImageLoader::WriteImageFile(const std::string& image_filename, int &width, int &height,
                                         int components, int stride, CharBufferPtr data_buffer) {
    // Writes an image to a file
    stbi_flip_vertically_on_write(true);

    int ret = stbi_write_png(image_filename.c_str(), width, height, components,
                              data_buffer.get()->data(), stride);

    if (ret == 0) {
        std::cerr << "ERROR: could not write image to " << image_filename << std::endl;
    }

    // struggled to clean exit via glfw in the main loop; this isn't great but works
    exit(EXIT_SUCCESS);
}

ImageLoader::~ImageLoader() {
    // Cleans out a loaded image
    if(image_data) {
        auto stbi_data = image_data.release();

        stbi_image_free(stbi_data);
    }
}
