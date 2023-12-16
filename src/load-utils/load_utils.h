//
// Created by francisk on 3/26/23.
//
// #define EIGEN_NO_DEBUG // disables eigen assertions

#ifndef DRAGON_GL_LOAD_UTILS_H
#define DRAGON_GL_LOAD_UTILS_H

#include <cassert>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <Eigen/Dense>

#include <igl/readOFF.h>
#include <igl/readOBJ.h>

#include "attributes.h"

struct FacetInfo {
    Eigen::Vector3d face_normal;
    Eigen::Vector3d tangent;
};

using VertexList = std::vector<Vertex>;
using FaceInfo = std::vector<FacetInfo>;
using NeighboringFaces = std::vector<std::vector<unsigned int>>;

// Filesystem paths
const std::string data_dir = DATA_DIR;  // injected by cmake
const std::string mesh_off_filename(data_dir + "dragon.off");
const std::string mesh_obj_filename(data_dir + "dragon.obj");
const std::string mesh_off_bunny_filename(data_dir + "bunny.off");
const std::string output_filename("output.png");

const std::string texture_diffuse_filename(data_dir + "texture/DefaultMaterial_albedo.jpg");
const std::string texture_normal_map_filename(data_dir + "texture/DefaultMaterial_normal.png");

const std::string per_vertex_dir = SHADERS_GOURAUD_DIR; // injected by cmake
const std::string normal_mapping_dir = SHADERS_NORMAL_MAPPING_DIR;
const std::string flat_dir = SHADERS_FLAT_DIR;

void ExistsOk(const std::string &filename);
std::string GetVertexShaderPath(ShadingOption opt);
std::string GetFragmentShaderPath(ShadingOption opt);

Eigen::Vector3d ComputeTangent(const Eigen::Vector3d &a, const Eigen::Vector3d &b, const Eigen::Vector3d &c,
                               const Eigen::Vector2d &uv_1,
                               const Eigen::Vector2d &uv_2,
                               const Eigen::Vector2d &uv_3);
Eigen::Vector3d ComputeTriangleNormal(const Eigen::Vector3d &a, const Eigen::Vector3d &b,
                                      const Eigen::Vector3d &c);

void LoadOffFile(const std::string &mesh_fname, Eigen::MatrixXd &vertices, Eigen::MatrixXi &facets);
void LoadObjFile(const std::string &mesh_fname, Eigen::MatrixXd &vertices, Eigen::MatrixXi &facets,
                 Eigen::MatrixXd &uv_coords);

std::pair<FaceInfo, NeighboringFaces> ProcessFacets(const Eigen::MatrixXd &vertices,
                                                    const Eigen::MatrixXi &facets,
                                                    const std::optional<Eigen::MatrixXd> &uv_coords);
VertexList CreateTriangles(const Eigen::MatrixXd &vertices, const Eigen::MatrixXi &facets,
                           const std::optional<Eigen::MatrixXd> &uv_coords, ShadingOption opt);
VertexList LoadDragonOff(const std::string &mesh_fname, ShadingOption opt);
VertexList LoadDragonObj(const std::string &mesh_fname, ShadingOption opt);

#endif // DRAGON_GL_LOAD_UTILS_H
