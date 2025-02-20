#include <map>
#include <Eigen/Geometry>
#include <common/mesh.h>
#include <json/json.hpp>
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <gltf/tiny_gltf.h>

using Vector3 = Eigen::Vector3f;
using Quaternion = Eigen::Quaternionf;
#define Real float
using Mesh = common::Mesh<Real>;

class GltfWriter {
    tinygltf::Model _model;
    tinygltf::Node _root;
    tinygltf::Buffer _buffer_position;
    tinygltf::Buffer _buffer_normal;
    tinygltf::Buffer _buffer_index;
    tinygltf::Buffer _buffer_animation_time_step;
    tinygltf::Buffer _buffer_animation_translation;
    tinygltf::Buffer _buffer_animation_rotation;

    int _mesh_counter = 0;
    int _accessor_counter = 0;
    size_t _buffer_position_offset = 0;
    size_t _buffer_normal_offset = 0;
    size_t _buffer_index_offset = 0;
    size_t _buffer_animation_time_step_offset = 0;
    size_t _buffer_animation_translation_offset = 0;
    size_t _buffer_animation_rotation_offset = 0;

    std::map<uint32_t, uint32_t> _mesh2nodeId;
    std::vector<int> _mesh_type;

    std::vector<std::vector<Vector3>> _animation_translation;
    std::vector<std::vector<Quaternion>> _animation_rotation;
    std::vector<std::vector<Real>> _animation_time_step;

public:
    explicit GltfWriter() {
        _model = tinygltf::Model();
        _model.asset.version = "2.0";
        _model.asset.generator = "RigidBody Dynamics - DALAB";

        _root = tinygltf::Node();
        _root.name = "root";
    }
    
    void download(const std::string& path) {
        for (int i = 0; i < 4; i++) {
            tinygltf::Node node;
            switch (i) {
            case 0: node.name = "box"; break;
            case 1: node.name = "cylinder"; break;
            case 2: node.name = "mesh"; break;
            case 3: node.name = "sphere"; break;
            default: break;
            }
            for (int j = 0; j < int(_mesh_type.size()); j++) {
                if (_mesh_type[j] == i) {
                    node.children.push_back(j);
                }
            }
            _model.nodes.push_back(node);
            _root.children.push_back(_mesh_counter++);
        }

        _model.nodes.push_back(_root);

        tinygltf::Scene scene;
        scene.name = "rigidbody scene";
        scene.nodes.push_back(_mesh_counter);
        /*for (int i = 0; i < _mesh_counter; i++) {
            scene.nodes.push_back(i);
            _model.nodes[i].translation.push_back(float(_animation_translation[i][0][0]));
            _model.nodes[i].translation.push_back(float(_animation_translation[i][0][1]));
            _model.nodes[i].translation.push_back(float(_animation_translation[i][0][2]));
            _model.nodes[i].rotation.push_back(float(_animation_rotation[i][0].x()));
            _model.nodes[i].rotation.push_back(float(_animation_rotation[i][0].y()));
            _model.nodes[i].rotation.push_back(float(_animation_rotation[i][0].z()));
            _model.nodes[i].rotation.push_back(float(_animation_rotation[i][0].w()));
        }*/
        _model.scenes.push_back(scene);
        _model.defaultScene = 0;

        tinygltf::Animation animation;
        animation.name = "animation";
        int anim_count = int(_animation_translation.size());
        for (int i = 0; i < anim_count; i++) {
            tinygltf::AnimationChannel channel_translation;
            channel_translation.target_path = "translation";
            channel_translation.target_node = i;
            channel_translation.sampler = i * 2;
            tinygltf::AnimationChannel channel_rotation;
            channel_rotation.target_path = "rotation";
            channel_rotation.target_node = i;
            channel_rotation.sampler = i * 2 + 1;
            tinygltf::AnimationSampler sampler_translation;
            sampler_translation.input = _accessor_counter + 2;
            sampler_translation.output = _accessor_counter;
            tinygltf::AnimationSampler sampler_rotation;
            sampler_rotation.input = _accessor_counter + 2;
            sampler_rotation.output = _accessor_counter + 1;
            animation.channels.push_back(channel_translation);
            animation.channels.push_back(channel_rotation);
            animation.samplers.push_back(sampler_translation);
            animation.samplers.push_back(sampler_rotation);

            tinygltf::Accessor translation_accessor;
            translation_accessor.bufferView = _accessor_counter;
            translation_accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            translation_accessor.count = _animation_translation[i].size();
            translation_accessor.type = TINYGLTF_TYPE_VEC3;
            _model.accessors.push_back(translation_accessor);
            tinygltf::BufferView translation_buffer_view;
            translation_buffer_view.buffer = 3;
            translation_buffer_view.byteOffset = _buffer_animation_translation_offset;
            _buffer_animation_translation_offset += translation_accessor.count * sizeof(float) * 3;
            translation_buffer_view.byteLength = translation_accessor.count * sizeof(float) * 3;
            _model.bufferViews.push_back(translation_buffer_view);
            std::vector<float> data;
            data.reserve(translation_accessor.count * 3);
            for (auto& v : _animation_translation[i]) {
                data.push_back(float(v.x()));
                data.push_back(float(v.y()));
                data.push_back(float(v.z()));
            }
            _buffer_animation_translation.data.insert(_buffer_animation_translation.data.end(), (const uint8_t*)data.data(), (const uint8_t*)data.data() + data.size() * sizeof(float));

            tinygltf::Accessor rotation_accessor;
            rotation_accessor.bufferView = _accessor_counter + 1;
            rotation_accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            rotation_accessor.count = _animation_rotation[i].size();
            rotation_accessor.type = TINYGLTF_TYPE_VEC4;
            _model.accessors.push_back(rotation_accessor);
            tinygltf::BufferView rotation_buffer_view;
            rotation_buffer_view.buffer = 4;
            rotation_buffer_view.byteOffset = _buffer_animation_rotation_offset;
            _buffer_animation_rotation_offset += rotation_accessor.count * sizeof(float) * 4;
            rotation_buffer_view.byteLength = rotation_accessor.count * sizeof(float) * 4;
            _model.bufferViews.push_back(rotation_buffer_view);
            data.clear();
            data.reserve(rotation_accessor.count * 4);
            for (auto& q : _animation_rotation[i]) {
                data.push_back(float(q.x()));
                data.push_back(float(q.y()));
                data.push_back(float(q.z()));
                data.push_back(float(q.w()));
            }
            _buffer_animation_rotation.data.insert(_buffer_animation_rotation.data.end(), (const uint8_t*)data.data(), (const uint8_t*)data.data() + data.size() * sizeof(float));

            tinygltf::Accessor time_step_accessor;
            time_step_accessor.bufferView = _accessor_counter + 2;
            time_step_accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            time_step_accessor.count = _animation_time_step[i].size();
            time_step_accessor.type = TINYGLTF_TYPE_SCALAR;
            _model.accessors.push_back(time_step_accessor);
            tinygltf::BufferView time_step_buffer_view;
            time_step_buffer_view.buffer = 5;
            time_step_buffer_view.byteOffset = _buffer_animation_time_step_offset;
            _buffer_animation_time_step_offset += time_step_accessor.count * sizeof(float);
            time_step_buffer_view.byteLength = time_step_accessor.count * sizeof(float);
            _model.bufferViews.push_back(time_step_buffer_view);
            data.clear();
            data.reserve(time_step_accessor.count);
            for (auto& t : _animation_time_step[i]) {
                data.push_back(float(t));
            }
            _buffer_animation_time_step.data.insert(_buffer_animation_time_step.data.end(), (const uint8_t*)data.data(), (const uint8_t*)data.data() + data.size() * sizeof(float));

            _accessor_counter += 3;
        }
        _model.animations.push_back(animation);

        _model.buffers.push_back(_buffer_position);
        _model.buffers.push_back(_buffer_normal);
        _model.buffers.push_back(_buffer_index);
        _model.buffers.push_back(_buffer_animation_translation);
        _model.buffers.push_back(_buffer_animation_rotation);
        _model.buffers.push_back(_buffer_animation_time_step);

        tinygltf::TinyGLTF gltf;
        mergeGLTFBuffers(_model);
        gltf.WriteGltfSceneToFile(&_model, path, true, true, true, false);
    }

    static void mergeGLTFBuffers(tinygltf::Model& mdl) {
        for (auto& bv : mdl.bufferViews) {
            for (int i = 0; i < bv.buffer; i++) {
                bv.byteOffset += mdl.buffers[i].data.size();
            }
            bv.buffer = 0;
        }
        tinygltf::Buffer buffer;
        for (auto& b : mdl.buffers) {
            buffer.data.insert(buffer.data.end(), b.data.begin(), b.data.end());
        }
        mdl.buffers.clear();
        mdl.buffers.push_back(buffer);
    }

    bool isTrackingMesh(const uint32_t id) {
        return _mesh2nodeId.find(id) != _mesh2nodeId.end();
    }

    void addMesh(const Mesh& mesh, const uint32_t id, const int type) {
        tinygltf::Node node;
        node.name = "node-" + std::to_string(_mesh_counter);
        node.mesh = _mesh_counter;
        _mesh2nodeId[id] = _mesh_counter;
        _mesh_type.push_back(type);

        tinygltf::Mesh gltf_mesh;
        gltf_mesh.name = "mesh-" + std::to_string(_mesh_counter);
        tinygltf::Primitive primitive;
        primitive.mode = TINYGLTF_MODE_TRIANGLES;
        primitive.attributes["POSITION"] = _accessor_counter;
        primitive.attributes["NORMAL"] = _accessor_counter + 1;
        primitive.indices = _accessor_counter + 2;
        gltf_mesh.primitives.push_back(primitive);
        _model.meshes.push_back(gltf_mesh);

        tinygltf::Accessor position_accessor;
        position_accessor.bufferView = _accessor_counter;
        position_accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        position_accessor.count = mesh.vertices.size();
        position_accessor.type = TINYGLTF_TYPE_VEC3;
        _model.accessors.push_back(position_accessor);
        tinygltf::BufferView position_buffer_view;
        position_buffer_view.buffer = 0;
        position_buffer_view.byteOffset = _buffer_position_offset;
        _buffer_position_offset += position_accessor.count * sizeof(float) * 3;
        position_buffer_view.byteLength = position_accessor.count * sizeof(float) * 3;
        _model.bufferViews.push_back(position_buffer_view);
        std::vector<float> data;
        data.reserve(position_accessor.count * 3);
        for (auto& v : mesh.vertices) {
            data.push_back(float(v.position.x()));
            data.push_back(float(v.position.y()));
            data.push_back(float(v.position.z()));
        }
        _buffer_position.data.insert(_buffer_position.data.end(), (const uint8_t*)data.data(), (const uint8_t*)data.data() + data.size() * sizeof(float));

        tinygltf::Accessor normal_accessor;
        normal_accessor.bufferView = _accessor_counter + 1;
        normal_accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        normal_accessor.count = mesh.vertices.size();
        normal_accessor.type = TINYGLTF_TYPE_VEC3;
        _model.accessors.push_back(normal_accessor);
        tinygltf::BufferView normal_buffer_view;
        normal_buffer_view.buffer = 1;
        normal_buffer_view.byteOffset = _buffer_normal_offset;
        _buffer_normal_offset += normal_accessor.count * sizeof(float) * 3;
        normal_buffer_view.byteLength = normal_accessor.count * sizeof(float) * 3;
        _model.bufferViews.push_back(normal_buffer_view);
        data.clear();
        data.reserve(normal_accessor.count * 3);
        for (auto& v : mesh.vertices) {
            data.push_back(float(v.normal.x()));
            data.push_back(float(v.normal.y()));
            data.push_back(float(v.normal.z()));
        }
        _buffer_normal.data.insert(_buffer_normal.data.end(), (const uint8_t*)data.data(), (const uint8_t*)data.data() + data.size() * sizeof(float));

        tinygltf::Accessor index_accessor;
        index_accessor.bufferView = _accessor_counter + 2;
        index_accessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
        index_accessor.count = 0;
        for (auto& f : mesh.faces) {
            index_accessor.count += (f.indices.size() - 2) * 3;
        }
        index_accessor.type = TINYGLTF_TYPE_SCALAR;
        _model.accessors.push_back(index_accessor);
        tinygltf::BufferView index_buffer_view;
        index_buffer_view.buffer = 2;
        index_buffer_view.byteOffset = _buffer_index_offset;
        _buffer_index_offset += index_accessor.count * sizeof(uint32_t);
        index_buffer_view.byteLength = index_accessor.count * sizeof(uint32_t);
        _model.bufferViews.push_back(index_buffer_view);
        std::vector<uint32_t> indices;
        indices.reserve(index_accessor.count);
        for (auto& f : mesh.faces) {
            for (size_t i = 1; i < f.indices.size() - 1; i++) {
                indices.push_back(f.indices[0]);
                indices.push_back(f.indices[i]);
                indices.push_back(f.indices[i + 1]);
            }
        }
        _buffer_index.data.insert(_buffer_index.data.end(), (const uint8_t*)indices.data(), (const uint8_t*)indices.data() + indices.size() * sizeof(uint32_t));

        _model.nodes.push_back(node);
        _animation_translation.emplace_back();
        _animation_rotation.emplace_back();
        _animation_time_step.emplace_back();
        _mesh_counter++;
        _accessor_counter += 3;
    }

    void addAnimation(const uint32_t id, const Vector3& pos, const Quaternion& quat, Real time_step) {
        auto& anim_trans = _animation_translation[_mesh2nodeId[id]];
        auto& anim_rot = _animation_rotation[_mesh2nodeId[id]];
        auto& anim_time = _animation_time_step[_mesh2nodeId[id]];
        anim_trans.push_back(pos);
        anim_rot.push_back(quat);
        anim_time.push_back(time_step);
    }
};
