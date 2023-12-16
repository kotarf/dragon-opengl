cmake_minimum_required(VERSION 3.18)

project(dragon-mesh-opengl LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(EXECUTABLE_NAME dragon-opengl)

# Folder where data files are stored (meshes & stuff) and .glsl shader files
set(DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data/")
set(SHADERS_GOURAUD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/gouraud")
set(SHADERS_NORMAL_MAPPING_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/normal_mapping")
set(SHADERS_FLAT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/flat")

# Fetch dependencies automatically
include(fetch_glm)
include(fetch_stb_image)
include(libigl)

# Enable the sub-dependency igl::glfw
igl_include(glfw)

add_executable(${EXECUTABLE_NAME})
target_sources(${EXECUTABLE_NAME} PRIVATE src/main.cpp
        src/load-utils/image.cpp
        src/load-utils/load_utils.cpp
        src/pipeline/scene.cpp )
target_include_directories(${EXECUTABLE_NAME} PUBLIC include)
target_compile_definitions(${EXECUTABLE_NAME} PUBLIC
        -DDATA_DIR=\"${DATA_DIR}\"
        -DSHADERS_GOURAUD_DIR=\"${SHADERS_GOURAUD_DIR}\"
        -DSHADERS_NORMAL_MAPPING_DIR=\"${SHADERS_NORMAL_MAPPING_DIR}\"
        -DSHADERS_FLAT_DIR=\"${SHADERS_FLAT_DIR}\")

target_include_directories(${EXECUTABLE_NAME} SYSTEM PUBLIC)

# dependencies
target_link_libraries(${EXECUTABLE_NAME} PUBLIC igl::glfw glad glm stb_image )

set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES)

# Optimizations (release)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Debugging and warnings
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address")

# Windows exclusive flags
IF (WIN32)
    # https://github.com/raspberrypi/pico-sdk/issues/1017#issuecomment-1242814904
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-volatile")
ENDIF ()
