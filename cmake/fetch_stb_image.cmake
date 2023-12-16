cmake_minimum_required(VERSION 3.18)

include(FetchContent)

FetchContent_Declare(
        stb_image-github
        URL https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
        DOWNLOAD_NO_EXTRACT ON)

FetchContent_Declare(
        stb_image_write-github
        URL https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h
        DOWNLOAD_NO_EXTRACT ON)

FetchContent_MakeAvailable(stb_image-github)
FetchContent_MakeAvailable(stb_image_write-github)

add_library(stb_image INTERFACE)
target_sources(stb_image INTERFACE ${stb_image-github_SOURCE_DIR}/stb_image.h
        ${stb_image_write-github_SOURCE_DIR}/stb_image_write.h)
target_include_directories(stb_image INTERFACE ${stb_image-github_SOURCE_DIR}
        ${stb_image_write-github_SOURCE_DIR})
