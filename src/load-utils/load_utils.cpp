//
// Created by francisk on 3/26/23.
//

#include "load_utils.h"

void ExistsOk(const std::string &filename){
    // Simple filesystem check; exit if file doesn't exist
    auto exists_ok = std::filesystem::exists(filename);

    if(!exists_ok) {
        std::cout << filename << " does not exist or cannot be found " << std::endl;

        exit(EXIT_FAILURE);
    }
}

std::string GetVertexShaderPath(ShadingOption opt) {
    // Concatenates path to .glsl vertex shader
    if(opt == ShadingOption::per_vertex) {
        return per_vertex_dir + "/vertex.glsl";
    }
    else if(opt == ShadingOption::normal_mapping) {
        return normal_mapping_dir + "/vertex.glsl";
    }
    else if(opt == ShadingOption::flat || opt == ShadingOption::wireframe) {
        return flat_dir + "/vertex.glsl";
    }
    return "";
}

std::string GetFragmentShaderPath(ShadingOption opt) {
    // Concatenates path to .glsl fragment shader
    if(opt == ShadingOption::per_vertex) {
        return per_vertex_dir + "/fragment.glsl";
    }
    else if(opt == ShadingOption::normal_mapping) {
        return normal_mapping_dir + "/fragment.glsl";
    }
    else if(opt == ShadingOption::flat || opt == ShadingOption::wireframe) {
        return flat_dir + "/fragment.glsl";
    }
    return "";
}

Eigen::Vector3d ComputeTangent(const Eigen::Vector3d &a, const Eigen::Vector3d &b, const Eigen::Vector3d &c,
                               const Eigen::Vector2d &uv_1,
                               const Eigen::Vector2d &uv_2,
                               const Eigen::Vector2d &uv_3) {
    // Compute tangent vector, to be stored as a vertex attribute
    Eigen::Vector3d edge1 = b - a;
    Eigen::Vector3d edge2 = c - a;
    Eigen::Vector2d delta_uv1 = uv_2 - uv_1;
    Eigen::Vector2d delta_uv2 = uv_3 - uv_1;

    double f = 1.0 / (delta_uv1.x() * delta_uv2.y() - delta_uv2.x() * delta_uv1.y());

    auto tangent_x = f * (delta_uv2.y() * edge1.x() - delta_uv1.y() * edge2.x());
    auto tangent_y = f * (delta_uv2.y() * edge1.y() - delta_uv1.y() * edge2.y());
    auto tangent_z = f * (delta_uv2.y() * edge1.z() - delta_uv1.y() * edge2.z());

    Eigen::Vector3d tangent_vec(tangent_x, tangent_y, tangent_z);

    return tangent_vec;
}

Eigen::Vector3d ComputeTriangleNormal(const Eigen::Vector3d &a, const Eigen::Vector3d &b,
                                      const Eigen::Vector3d &c) {
    // Computes the normal of a triangle as part of the mesh
    Eigen::Vector3d n = (b - a).cross((c - a)).normalized();

    return n;
}

void LoadOffFile(const std::string &mesh_fname, Eigen::MatrixXd &vertices, Eigen::MatrixXi &facets) {
    // Loads an off file
    ExistsOk(mesh_fname);

    igl::readOFF(mesh_fname, vertices, facets);
}

void LoadObjFile(const std::string &mesh_fname, Eigen::MatrixXd &vertices, Eigen::MatrixXi &facets,
                 Eigen::MatrixXd &uv_coords) {
    // https://libigl.github.io/libigl-python-bindings/igl_docs/#read_obj (similar arguments)
    ExistsOk(mesh_fname);

    Eigen::MatrixXd discard_1;
    Eigen::MatrixXd discard_2;
    Eigen::MatrixXd discard_3;

    igl::readOBJ(mesh_fname, vertices, uv_coords, discard_1, facets, discard_2, discard_3);
}


std::pair<FaceInfo, NeighboringFaces> ProcessFacets(const Eigen::MatrixXd &vertices,
                                                    const Eigen::MatrixXi &facets,
                                                    const std::optional<Eigen::MatrixXd> &uv_coords) {
    // Store information per face
    FaceInfo load_info(facets.size());  // face ordinal: vertex normals & bitangents
    NeighboringFaces neighboring_faces(vertices.size()); // vertex ordinal: list of faces

    // Normals and neighboring faces
    for (unsigned int i = 0; i < facets.rows(); ++i) {

        auto pos_a = facets(i, 0);
        auto pos_b = facets(i, 1);
        auto pos_c = facets(i, 2);

        Eigen::Vector3d a = vertices.row(pos_a);
        Eigen::Vector3d b = vertices.row(pos_b);
        Eigen::Vector3d c = vertices.row(pos_c);

        // if not provided, tangent is undefined and assumed to not be used
        Eigen::Vector3d uv1(0, 0, 0);
        Eigen::Vector3d uv2(0, 0, 0);
        Eigen::Vector3d uv3(0, 0, 0);

        if(uv_coords.has_value()) {
            uv1 = uv_coords.value().row(pos_a);
            uv2 = uv_coords.value().row(pos_b);
            uv3 = uv_coords.value().row(pos_c);
        }

        auto face_normal = ComputeTriangleNormal(a, b, c);
        auto tangent = ComputeTangent(a, b, c, uv1.head(2), uv2.head(2),
                                      uv3.head(2));

        FacetInfo info;

        info.face_normal = face_normal;
        info.tangent = tangent;

        load_info[i] = info;

        neighboring_faces[pos_a].push_back(i);
        neighboring_faces[pos_b].push_back(i);
        neighboring_faces[pos_c].push_back(i);
    }
    return {load_info, neighboring_faces};
}

VertexList CreateTriangles(const Eigen::MatrixXd &vertices, const Eigen::MatrixXi &facets,
                           const std::optional<Eigen::MatrixXd> &uv_coords, ShadingOption opt) {
    // Store information per face and vertex
    VertexList tris;
    Eigen::MatrixXd centroids;
    Eigen::Vector3d uv1(0, 0, 0);
    Eigen::Vector3d uv2(0, 0, 0);
    Eigen::Vector3d uv3(0, 0, 0);

    // Normals and neighboring faces
    auto faces = ProcessFacets(vertices, facets, uv_coords);

    FaceInfo vinfo = faces.first;  // face ordinal: face normal
    NeighboringFaces neighboring_faces = faces.second; // vertex ordinal: list of faces

    centroids.resize(facets.rows(), 3);

    // Create vertices
    for (unsigned int i = 0; i < facets.rows(); ++i) {
        auto pos_a = facets(i, 0);
        auto pos_b = facets(i, 1);
        auto pos_c = facets(i, 2);

        Eigen::Vector3d a = vertices.row(pos_a);
        Eigen::Vector3d b = vertices.row(pos_b);
        Eigen::Vector3d c = vertices.row(pos_c);

        // Per-vertex normals - important to initialize to 0
        Eigen::Vector3d pv_a = Eigen::Vector3d(0,0,0);
        Eigen::Vector3d pv_b = Eigen::Vector3d(0,0,0);
        Eigen::Vector3d pv_c = Eigen::Vector3d(0,0,0);

        // Tangents
        Eigen::Vector3d tangent_a = Eigen::Vector3d(0,0,0);
        Eigen::Vector3d tangent_b = Eigen::Vector3d(0,0,0);
        Eigen::Vector3d tangent_c = Eigen::Vector3d(0,0,0);

        for(auto face: neighboring_faces[pos_a]) {
            pv_a += vinfo[face].face_normal;
            tangent_a += vinfo[face].tangent;
        }
        for(auto face: neighboring_faces[pos_b]) {
            pv_b += vinfo[face].face_normal;
            tangent_b += vinfo[face].tangent;

        }
        for(auto face: neighboring_faces[pos_c]) {
            pv_c += vinfo[face].face_normal;
            tangent_c += vinfo[face].tangent;
        }

        // Average face normals and tangent vectors
        pv_a = (pv_a / neighboring_faces[pos_a].size()).normalized();
        pv_b = (pv_b / neighboring_faces[pos_b].size()).normalized();
        pv_c = (pv_c / neighboring_faces[pos_c].size()).normalized();

        tangent_a = (tangent_a / neighboring_faces[pos_a].size()).normalized();
        tangent_b = (tangent_b / neighboring_faces[pos_b].size()).normalized();
        tangent_c = (tangent_c / neighboring_faces[pos_c].size()).normalized();

        Vertex v_a = Vertex();
        Vertex v_b = Vertex();
        Vertex v_c = Vertex();

        v_a.pos = VecPosition(a.x(), a.y(), a.z());
        v_b.pos = VecPosition(b.x(), b.y(), b.z());
        v_c.pos = VecPosition(c.x(), c.y(), c.z());

        // Set vertex attribute normals dependent on the selected rendering mode
        auto use_face_normal = opt == ShadingOption::flat || opt == ShadingOption::wireframe;

        auto normal_a = use_face_normal ? vinfo[i].face_normal : pv_a;
        auto normal_b = use_face_normal ? vinfo[i].face_normal : pv_b;
        auto normal_c = use_face_normal ? vinfo[i].face_normal : pv_c;

        v_a.normal = VecNormal(normal_a.x(), normal_a.y(), normal_a.z());
        v_b.normal = VecNormal(normal_b.x(), normal_b.y(), normal_b.z());
        v_c.normal = VecNormal(normal_c.x(), normal_c.y(), normal_c.z());

        v_a.tangent = VecDirection(tangent_a.x(), tangent_a.y(), tangent_a.z());
        v_b.tangent = VecDirection(tangent_b.x(), tangent_b.y(), tangent_b.z());
        v_c.tangent = VecDirection(tangent_c.x(), tangent_c.y(), tangent_c.z());

        // if not provided, tangent is undefined and assumed to not be used
        if(uv_coords.has_value()) {
            uv1 = uv_coords.value().row(pos_a);
            uv2 = uv_coords.value().row(pos_b);
            uv3 = uv_coords.value().row(pos_c);
        }

        v_a.uv_coord = VecTextureCoord(uv1.x(), uv1.y());
        v_b.uv_coord = VecTextureCoord(uv2.x(), uv2.y());
        v_c.uv_coord = VecTextureCoord(uv3.x(), uv3.y());

        tris.push_back(v_a);
        tris.push_back(v_b);
        tris.push_back(v_c);
    }

    return tris;
}

VertexList LoadDragonOff(const std::string &mesh_fname, ShadingOption opt) {
    // Load dragon (or bunny) .off file
    Eigen::MatrixXd m_vertices;
    Eigen::MatrixXi m_faces;

    LoadOffFile(mesh_fname, m_vertices, m_faces);

    auto tris = CreateTriangles(m_vertices, m_faces, std::nullopt, opt);

    return tris;
}

VertexList LoadDragonObj(const std::string &mesh_fname, ShadingOption opt) {
    // Load dragon obj
    Eigen::MatrixXd m_vertices;
    Eigen::MatrixXi m_faces;
    Eigen::MatrixXd m_uvcoords;

    LoadObjFile(mesh_fname, m_vertices, m_faces, m_uvcoords);

    auto tris = CreateTriangles(m_vertices, m_faces, m_uvcoords, opt);

    return tris;
}
