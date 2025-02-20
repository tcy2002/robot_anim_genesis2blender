#include <opts/cxxopts.hpp>
#include <vector>
#include <string>
#include "gltf_writer.h"

struct LinkMesh {
    std::string filename;
    Vector3 pos;
    Quaternion rot;
};

void read_config(const std::string& robot_file, std::vector<std::vector<LinkMesh>>& links) {
    std::ifstream file(robot_file);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << robot_file << std::endl;
        return;
    }

    while (true) {
        std::string filename;
        int n;
        file >> filename >> n;
        if (!file.good()) {
            break;
        }
        links.push_back({});
        for (int i = 0; i < n; i++) {
            std::string mesh_name;
            Vector3 pos;
            Quaternion rot;
            file >> mesh_name >> pos.x() >> pos.y() >> pos.z() >> rot.w() >> rot.x() >> rot.y() >> rot.z();
            links.back().push_back({mesh_name, pos, rot});
        }
    }
}

std::string to_lower(const std::string& str) {
    std::string res = str;
    for (char& c : res) {
        c = std::tolower(c);
    }
    return res;
}

int main() {
    // Mesh mesh;
    // Mesh::loadFromObj("../robots/logo_link.obj", mesh, Vector3(1, 1, 1));
    // GltfWriter writer;
    // writer.addMesh(mesh, 0, 2);
    // for (int i = 0; i < 100; i++) {
    //     Vector3 pos(0, 0, 0.01 * i);
    //     writer.addAnimation(0, pos, Quaternion::Identity(), 0.01 * i);
    // }
    // writer.download("../logo_link.gltf");

    std::vector<std::vector<LinkMesh>> links;
    read_config("../link2mesh.txt", links);
    std::cout << "read done." << links.size() << std::endl;

    GltfWriter writer;
    std::ifstream file("../anim.txt");
    int frame = 0;
    int max_frame = 1120;
    for (int i = 0; i < max_frame; i++) {
        std::cout << "processing frame: " << i << std::endl;
        int j = 0;
        for (auto& link : links) {
            Vector3 pos;
            Quaternion rot;
            file >> pos.x() >> pos.y() >> pos.z() >> rot.w() >> rot.x() >> rot.y() >> rot.z();
            for (auto& mesh : link) {
                if (!writer.isTrackingMesh(j)) {
                    Mesh m;
                    Mesh::loadFromObj("../meshes/" + to_lower(mesh.filename) + ".obj", m, Vector3(1, 1, 1));
                    std::cout << "add mesh: " << mesh.filename << std::endl;
                    writer.addMesh(m, j, 2);
                }
                Vector3 mesh_pos = pos + rot * mesh.pos;
                Quaternion mesh_rot = rot * mesh.rot;
                writer.addAnimation(j++, mesh_pos, mesh_rot, 0.01666667 * i);
            }
        }
    }
    writer.download("../robot.gltf");

    return 0;
}