#include "obj_file.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

using namespace std;
using namespace ge1;
using namespace tinyobj;

obj_file::obj_file(const char* filename) {
    attrib_t attributes;
    vector<shape_t> shapes;
    vector<material_t> materials;

    string error;

    bool success = LoadObj(&attributes, &shapes, &materials, &error, filename);

    if (!success) {
        throw runtime_error(error);
    }

    material_faces.reserve(materials.size());
    material_diffuse_color.reserve(materials.size());

    for (const auto& material : materials) {
        material_faces.push_back({0, 0, 0, 0});
        material_diffuse_color.push_back({
            material.diffuse[0], material.diffuse[1], material.diffuse[2]
        });
    }

    auto face_vertex_count = 0u;
    for (const auto& shape : shapes) {
        face_vertex_count += shape.mesh.indices.size();
    }

    for (const auto& shape : shapes) {
        for (const auto& material : shape.mesh.material_ids) {
            material_faces[static_cast<unsigned>(material)].count += 3;
        }
    }

    auto first = 0u;
    for (auto& material : material_faces) {
        material.first = first;
        first += material.count;
        material.count = 0;
    }

    face_positions.resize(face_vertex_count);
    for (const auto& shape : shapes) {
        for (auto face = 0u; face < shape.mesh.material_ids.size(); face++) {
            auto material =
                static_cast<unsigned>(shape.mesh.material_ids[face]);
            for (auto vertex = 0u; vertex < 3; vertex++) {
                face_positions[
                    material_faces[material].first +
                    material_faces[material].count
                ] = {
                    attributes.vertices[face * 9 + vertex * 3],
                    attributes.vertices[face * 9 + vertex * 3 + 1],
                    attributes.vertices[face * 9 + vertex * 3 + 2]
                };
                material_faces[material].count++;
            }
        }
    }
}
