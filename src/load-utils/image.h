//
// Created by francisk on 4/29/23.
//

/* Image library */
#ifndef DRAGON_GL_LOAD_IMAGE_H
#define DRAGON_GL_LOAD_IMAGE_H

#include <string>
#include <memory>
#include <iostream>
#include <vector>

using ImagePointer = unsigned char*;
using ImageUniquePtr = std::unique_ptr<unsigned char*>;
using CharBuffer = std::vector<char>;
using CharBufferPtr = std::unique_ptr<CharBuffer>;

class ImageLoader {
private:
    ImageUniquePtr image_data;

public:
    ~ImageLoader();

    ImagePointer LoadImageFile(const std::string& image_filename, int &width, int &height, int &components);
    static ImagePointer WriteImageFile(const std::string& image_filename, int &width, int &height,
                                       int components, int stride, CharBufferPtr data_buffer);
};

#endif // DRAGON_GL_LOAD_IMAGE_H
