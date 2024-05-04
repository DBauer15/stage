#include "scene.h"
#include "cie.h"

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <unordered_map>

#include <tbb/tbb.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <pbrtParser/Scene.h>

#include <ufbx.h>

namespace stage {
namespace backstage {

std::unique_ptr<Scene> createScene(std::string scene, const Config& config) {
    std::string extension = std::filesystem::path(scene).extension().string();
    std::unique_ptr<Scene> scene_ptr;
    try {
        if (extension == ".obj")
            scene_ptr = std::make_unique<OBJScene>(scene, config);
        else if (extension == ".pbrt")
            scene_ptr = std::make_unique<PBRTScene>(scene, config);
        else if (extension == ".fbx")
            scene_ptr = std::make_unique<FBXScene>(scene, config);
        else
            throw std::runtime_error("Unexpected file format " + extension);
    } catch (std::runtime_error e) {
        ERR("Error parsing " + scene + ": " + std::string(e.what()));
    }
    return scene_ptr;
}

void
Scene::updateFilePaths(std::string scene) {
    m_scene_path = std::filesystem::absolute(std::filesystem::path(scene));
    m_base_path = m_scene_path.parent_path();
}

float
Scene::luminance(stage_vec3f c) {
    return 0.299f*c.r + 0.587f*c.g + 0.114f*c.b;
}

std::filesystem::path
Scene::getAbsolutePath(std::filesystem::path p) {
    std::filesystem::path result = p;

#if !defined(_WIN32)
    // On POSIX systems '\' is a valid character in filenames
    // We account for this possibility by first checking if the given path exists, and if not return an alternative with '\' replaced by '/'
    // If the alternative path should also not exist, there must be an error in the given path
    std::string result_no_backslash_str = p.string();
    std::replace(result_no_backslash_str.begin(), result_no_backslash_str.end(), '\\', '/');
    std::filesystem::path result_no_backslash(result_no_backslash_str);

    if (!result.is_absolute()) {
        result = m_base_path / result;
        result_no_backslash = m_base_path / result_no_backslash;
    }

    if (!std::filesystem::exists(result)) {
        return result_no_backslash;
    }
    return result;
#else
    if (result.is_absolute()) {
        return result;
    }
    return m_base_path / result;
#endif
}

void
Scene::updateSceneScale() {
    float min_coord = 1e30f;
    float max_coord = -1e30f;
    stage_vec3f min_vertex (1e30f);
    stage_vec3f max_vertex (-1e30f);
    std::tuple<stage_vec3f, stage_vec3f> initial = std::make_tuple(min_vertex, max_vertex);

    auto reduce_fn = [](const std::tuple<stage_vec3f, stage_vec3f>& a, const std::tuple<stage_vec3f, stage_vec3f>& b) {
        auto& a_min = std::get<0>(a);
        auto& a_max = std::get<1>(a);
        auto& b_min = std::get<0>(b);
        auto& b_max = std::get<1>(b);
        stage_vec3f min = compMin(a_min) < compMin(b_min) ? a_min : b_min;
        stage_vec3f max = compMax(a_max) > compMax(b_max) ? a_max : b_max;
        return std::make_tuple(min, max);
    };

    std::tuple<stage_vec3f, stage_vec3f> extrema = tbb::parallel_reduce(tbb::blocked_range<size_t>(0, m_instances.size()), initial, [&](const auto& r, auto current) {
    auto local = current;
    for (size_t instance_id = r.begin(); instance_id != r.end(); instance_id++) {
        stage_mat4 instance_to_world = m_instances[instance_id].instance_to_world;
        // stage_mat4 instance_to_world = make_mat4(&instance_to_world_[0][0]);

        for (auto& geometry : m_objects[m_instances[instance_id].object_id].geometries) {
            auto extent = tbb::parallel_reduce(tbb::blocked_range<size_t>(0, geometry.positions.size()), local, [&](const auto& rr, auto ccurrent) {
                auto llocal = ccurrent;
                for (size_t vertex_id = rr.begin(); vertex_id != rr.end(); vertex_id++) {
                    stage_vec3f vertex = stage_vec3f(instance_to_world * stage_vec4f(geometry.positions[vertex_id], 1.f));
                    llocal = reduce_fn(std::make_tuple(vertex, vertex), llocal);
                }
                return llocal;
            },
            reduce_fn);
            local = reduce_fn(extent, local);
        }
    }
    return local;
    }, 
    reduce_fn);

    min_vertex = std::get<0>(extrema);
    max_vertex = std::get<1>(extrema);

    min_coord = compMin(min_vertex);
    max_coord = compMax(max_vertex);

    m_scene_scale = max_coord - min_coord;
}


void
OBJScene::loadObj() {

    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = m_base_path.string() + "/";

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(m_scene_path, reader_config)) { 
        if (!reader.Error().empty()) { 
            throw std::runtime_error(reader.Error());
        }
        return;
    }
    
    if (!reader.Warning().empty()) {
        WARN("TinyObjLoader Warning: " + reader.Warning());
    }

    auto& in_attrib = reader.GetAttrib();
    auto& in_shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    // Deal with normals
    bool calculate_normals = in_attrib.normals.size() == 0;
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    if (calculate_normals) {
        LOG("Calculating normals");
        computeSmoothingShapes(in_attrib, attrib, in_shapes, shapes);
        computeAllSmoothingNormals(attrib, shapes);
    } else {
        LOG("Using normals provided by OBJ");
        attrib = in_attrib;
        shapes = in_shapes;
    }

    SUCC("Parsed OBJ file " + m_scene_path.string());

    // Parse materials and textures
    std::unordered_map<std::string, uint32_t> texture_index_map;
    for (const auto& material : materials) {
        OpenPBRMaterial pbr_mat = OpenPBRMaterial::defaultMaterial();
        pbr_mat.base_color = stage_vec3f(material.diffuse[0], material.diffuse[1], material.diffuse[2]);
        pbr_mat.specular_color = stage_vec3f(material.specular[0], material.specular[1], material.specular[2]);
        pbr_mat.specular_weight = luminance(pbr_mat.specular_color);
        pbr_mat.specular_ior = material.ior;
        pbr_mat.specular_roughness = std::clamp((1.f - std::log10(material.shininess + 1) / 3.f), 1e-5f, 1.f); // TODO: This is not a good approximation of roughness as the Phong shininess is exponential
        pbr_mat.transmission_weight = 1.f - material.dissolve;
        
        if (!material.diffuse_texname.empty()) {
            if (texture_index_map.find(material.diffuse_texname) != texture_index_map.end()) {
                pbr_mat.base_color_texid = texture_index_map.at(material.diffuse_texname);
            } else {
                std::filesystem::path texture_filename = getAbsolutePath(material.diffuse_texname);
                Image diffuse_texture(texture_filename.string());
                if (diffuse_texture.isValid()) { 
                    m_textures.push_back(std::move(diffuse_texture));
                    pbr_mat.base_color_texid = m_textures.size() - 1;
                    texture_index_map[material.diffuse_texname] = m_textures.size() - 1;
                    LOG("Read texture image '" + material.diffuse_texname + "'");
                }
            }
        }

        LOG("Read material '" + material.name + "'");
        m_materials.push_back(pbr_mat);
    }
    // Add a default material for faces that do not have a material id
    m_materials.push_back(OpenPBRMaterial::defaultMaterial());

    // Parse meshes
    Object obj(m_config.layout, m_config.vertex_alignment);
    for (const auto& shape : shapes) {
        const auto& mesh = shape.mesh;

        std::map<std::tuple<uint32_t, uint32_t, uint32_t>, uint32_t> index_map; 

        std::vector<stage_vec3f> positions;
        std::vector<stage_vec3f> normals;
        std::vector<stage_vec2f> uvs;
        std::vector<uint32_t> material_ids;
        std::vector<uint32_t> indices;

        // Keep track of all the unique indices we use
        uint32_t g_n_unique_idx_cnt = 0;

        // Loop over faces in the mesh
        for (size_t f = 0; f < mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(mesh.num_face_vertices[f]);

            if (fv != 3) {
                ERR("Found non-triangular primitive with " + std::to_string(fv) + " vertices.");
                return;
            }

            // Loop over vertices in the face
            for (size_t v = 0; v < fv; v++){

                tinyobj::index_t idx = mesh.indices[3 * f + v]; 

                auto key = std::make_tuple(idx.vertex_index, idx.normal_index, idx.texcoord_index);

                uint32_t g_index = 0;
                if (index_map.find(key) != index_map.end()) {
                    g_index = index_map.at(key);
                } else {
                    g_index = g_n_unique_idx_cnt++;

                    positions.push_back(make_vec3(&attrib.vertices[3 * idx.vertex_index]));
                    normals.push_back(make_vec3(&attrib.normals[3 * idx.normal_index]));
                    if (attrib.texcoords.size() > 0) {
                        uvs.push_back(make_vec2(&attrib.texcoords[2 * idx.texcoord_index]));
                    }
                    material_ids.push_back(mesh.material_ids[f] < 0 ? m_materials.size() - 1 : mesh.material_ids[f]);

                    index_map[key] = g_index;
                }
                indices.push_back(g_index);
            }
        }
        Geometry g(obj, positions, normals, uvs, material_ids, indices);
        obj.geometries.push_back(g);
        LOG("Read geometry (v: " + std::to_string(g.positions.size()) + ", i: " + std::to_string(g.indices.size()) + ")");
    }
    m_objects.push_back(obj);

    // OBJ does not support instancing, so each object has one instance
    for (uint32_t i = 0; i < m_objects.size(); i++) {
        ObjectInstance instance;
        instance.object_id = i;
        instance.instance_to_world = stage_mat4f(1.f);
        m_instances.push_back(instance);
    }

    // OBJ does not support lights, so we add a single infinite area light 
    Light light = Light::defaultLight();
    light.type = LightType::InfiniteLight;
    m_lights.push_back(light);
}

/*
 * Adapted from: https://github.com/tinyobjloader/tinyobjloader/blob/cc327eecf7f8f4139932aec8d75db2d091f412ef/examples/viewer/viewer.cc#L375
 */
void 
OBJScene::computeSmoothingShape(const tinyobj::attrib_t& in_attrib, const tinyobj::shape_t& in_shape,
                          std::vector<std::pair<unsigned int, unsigned int>>& sorted_ids,
                          unsigned int id_begin, unsigned int id_end,
                          std::vector<tinyobj::shape_t>& out_shapes,
                          tinyobj::attrib_t& out_attrib) {
  unsigned int sgroupid = sorted_ids[id_begin].first;
  bool hasmaterials = in_shape.mesh.material_ids.size();
  // Make a new shape from the set of faces in the range [id_begin, id_end).
  out_shapes.emplace_back();
  tinyobj::shape_t& outshape = out_shapes.back();
  outshape.name = in_shape.name;
  // Skip lines and points.

  std::unordered_map<unsigned int, unsigned int> remap;
  for (unsigned int id = id_begin; id < id_end; ++id) {
    unsigned int face = sorted_ids[id].second;

    outshape.mesh.num_face_vertices.push_back(3); // always triangles
    if (hasmaterials)
      outshape.mesh.material_ids.push_back(in_shape.mesh.material_ids[face]);
    outshape.mesh.smoothing_group_ids.push_back(sgroupid);
    // Skip tags.

    for (unsigned int v = 0; v < 3; ++v) {
      tinyobj::index_t inidx = in_shape.mesh.indices[3*face + v], outidx;
      assert(inidx.vertex_index != -1);
      auto iter = remap.find(inidx.vertex_index);
      // Smooth group 0 disables smoothing so no shared vertices in that case.
      if (sgroupid && iter != remap.end()) {
        outidx.vertex_index = (*iter).second;
        outidx.normal_index = outidx.vertex_index;
        outidx.texcoord_index = (inidx.texcoord_index == -1) ? -1 : outidx.vertex_index;
      }
      else {
        assert(out_attrib.vertices.size() % 3 == 0);
        unsigned int offset = static_cast<unsigned int>(out_attrib.vertices.size() / 3);
        outidx.vertex_index = outidx.normal_index = offset;
        outidx.texcoord_index = (inidx.texcoord_index == -1) ? -1 : offset;
        out_attrib.vertices.push_back(in_attrib.vertices[3*inidx.vertex_index  ]);
        out_attrib.vertices.push_back(in_attrib.vertices[3*inidx.vertex_index+1]);
        out_attrib.vertices.push_back(in_attrib.vertices[3*inidx.vertex_index+2]);
        out_attrib.normals.push_back(0.0f);
        out_attrib.normals.push_back(0.0f);
        out_attrib.normals.push_back(0.0f);
        if (inidx.texcoord_index != -1) {
          out_attrib.texcoords.push_back(in_attrib.texcoords[2*inidx.texcoord_index  ]);
          out_attrib.texcoords.push_back(in_attrib.texcoords[2*inidx.texcoord_index+1]);
        }
        remap[inidx.vertex_index] = offset;
      }
      outshape.mesh.indices.push_back(outidx);
    }
  }
}

/* 
 * Adapted from: https://github.com/tinyobjloader/tinyobjloader/blob/cc327eecf7f8f4139932aec8d75db2d091f412ef/examples/viewer/viewer.cc#L430
 */
void 
OBJScene::computeSmoothingShapes(const tinyobj::attrib_t& in_attrib,
                              tinyobj::attrib_t& out_attrib,
                              const std::vector<tinyobj::shape_t>& in_shapes,
                              std::vector<tinyobj::shape_t>& out_shapes) {
    for (size_t s = 0, slen = in_shapes.size() ; s < slen; ++s) {
        const tinyobj::shape_t& in_shape = in_shapes[s];

        unsigned int numfaces = static_cast<unsigned int>(in_shape.mesh.smoothing_group_ids.size());
        assert(numfaces);
        std::vector<std::pair<unsigned int,unsigned int>> sorted_ids(numfaces);
        for (unsigned int i = 0; i < numfaces; ++i)
            sorted_ids[i] = std::make_pair(in_shape.mesh.smoothing_group_ids[i], i);
        sort(sorted_ids.begin(), sorted_ids.end());

        unsigned int activeid = sorted_ids[0].first;
        unsigned int id = activeid, id_begin = 0, id_end = 0;
        // Faces are now bundled by smoothing group id, create shapes from these.
        while (id_begin < numfaces) {
            while (activeid == id && ++id_end < numfaces)
                id = sorted_ids[id_end].first;
            computeSmoothingShape(in_attrib, in_shape, sorted_ids, id_begin, id_end,
                    out_shapes, out_attrib);
            activeid = id;
            id_begin = id_end;
        }
    }
}

/* 
 * Adapted from: https://github.com/tinyobjloader/tinyobjloader/blob/cc327eecf7f8f4139932aec8d75db2d091f412ef/examples/viewer/viewer.cc#L270
 */
void 
OBJScene::computeAllSmoothingNormals(tinyobj::attrib_t& attrib,
                                  std::vector<tinyobj::shape_t>& shapes) {
    stage_vec3f p[3];
    for (size_t s = 0, slen = shapes.size(); s < slen; ++s) {
        const tinyobj::shape_t& shape(shapes[s]);
        size_t facecount = shape.mesh.num_face_vertices.size();
        assert(shape.mesh.smoothing_group_ids.size());

        for (size_t f = 0, flen = facecount; f < flen; ++f) {
            for (unsigned int v = 0; v < 3; ++v) {
                tinyobj::index_t idx = shape.mesh.indices[3*f + v];
                assert(idx.vertex_index != -1);
                p[v][0] = attrib.vertices[3*idx.vertex_index  ];
                p[v][1] = attrib.vertices[3*idx.vertex_index+1];
                p[v][2] = attrib.vertices[3*idx.vertex_index+2];
            }

            // cross(p[1] - p[0], p[2] - p[0])
            float nx = (p[1][1] - p[0][1]) * (p[2][2] - p[0][2]) -
                (p[1][2] - p[0][2]) * (p[2][1] - p[0][1]);
            float ny = (p[1][2] - p[0][2]) * (p[2][0] - p[0][0]) -
                (p[1][0] - p[0][0]) * (p[2][2] - p[0][2]);
            float nz = (p[1][0] - p[0][0]) * (p[2][1] - p[0][1]) -
                (p[1][1] - p[0][1]) * (p[2][0] - p[0][0]);

            // Don't normalize here.
            for (unsigned int v = 0; v < 3; ++v) {
                tinyobj::index_t idx = shape.mesh.indices[3*f + v];
                attrib.normals[3*idx.normal_index  ] += nx;
                attrib.normals[3*idx.normal_index+1] += ny;
                attrib.normals[3*idx.normal_index+2] += nz;
            }
        }
    }

    assert(attrib.normals.size() % 3 == 0);
    for (size_t i = 0, nlen = attrib.normals.size() / 3; i < nlen; ++i) {
        tinyobj::real_t& nx = attrib.normals[3*i  ];
        tinyobj::real_t& ny = attrib.normals[3*i+1];
        tinyobj::real_t& nz = attrib.normals[3*i+2];
        tinyobj::real_t len = std::sqrt(nx*nx + ny*ny + nz*nz);
        tinyobj::real_t scale = len == 0 ? 0 : 1 / len;
        nx *= scale;
        ny *= scale;
        nz *= scale;
    }
}


void
PBRTScene::loadPBRT() {

    std::shared_ptr<pbrt::Scene> pbrt_scene;
    pbrt_scene = pbrt::importPBRT(m_scene_path);

    // Flatten hierarchy to avoid the pain of combining hierarchical instance transforms
    pbrt_scene->makeSingleLevel();

    // Add a default material for faces that do not have a material id
    m_materials.push_back(OpenPBRMaterial::defaultMaterial());

    // Import objects
    std::map<std::shared_ptr<pbrt::Object>, uint32_t> object_map;
    std::map<std::shared_ptr<pbrt::Material>, uint32_t> material_map;
    std::map<std::shared_ptr<pbrt::Texture>, uint32_t> texture_index_map;
    loadPBRTObjectsRecursive(pbrt_scene->world, object_map, material_map, texture_index_map);

    // Import instances
    for(auto& instance : pbrt_scene->world->instances)
        loadPBRTInstancesRecursive(instance, object_map);
    if (m_instances.size() == 0 && m_objects.size() > 0) {
        ObjectInstance root;
        root.object_id = 0;
        root.instance_to_world = stage_mat4f(1.f);
        m_instances.push_back(root);
        WARN("No instance data found, adding default instance");
    }

    if (m_lights.size() == 0) {
        Image sky_texture("sky.exr", true);
        m_textures.push_back(std::move(sky_texture));

        Light light = Light::defaultLight();
        light.type = LightType::InfiniteLight;
        light.map_texid = m_textures.size() - 1;
        m_lights.push_back(light);
    }

    // Import camera
    if (pbrt_scene->cameras.size() > 0) {
        auto& camera = pbrt_scene->cameras[0]; // parse the first available camera
        m_camera = std::make_shared<Camera>();
        // m_camera->position = make_vec3(&(camera->simplified.lens_center.x));
        m_camera->position = make_vec3(&camera->simplified.lens_center.x);
        // m_camera->lookat = make_vec3(&(camera->simplified.screen_center.x));
        m_camera->lookat = make_vec3(&camera->simplified.screen_center.x);
        // m_camera->up = make_vec3(&(camera->frame.l.vy.x));
        m_camera->up = make_vec3(&camera->frame.l.vy.x);
    }
}

void
PBRTScene::loadPBRTObjectsRecursive(std::shared_ptr<pbrt::Object> current, 
                                std::map<std::shared_ptr<pbrt::Object>, uint32_t>& object_map, 
                                std::map<std::shared_ptr<pbrt::Material>, uint32_t>& material_map,
                                std::map<std::shared_ptr<pbrt::Texture>, uint32_t>& texture_index_map) {
    if (!current || object_map.find(current) != object_map.end()) return;

    // Load shapes
    Object obj(m_config.layout, m_config.vertex_alignment);
    for (auto& shape : current->shapes) {
        // Non-triangle shapes are not supported
        pbrt::TriangleMesh::SP mesh = std::dynamic_pointer_cast<pbrt::TriangleMesh>(shape);
        if (!mesh) continue;

        uint32_t material_id = 0;
        OpenPBRMaterial pbr_material = OpenPBRMaterial::defaultMaterial();
        if (material_map.find(mesh->material) != material_map.end()) {
            material_id = material_map[mesh->material];
        } else if (loadPBRTMaterial(mesh->material, pbr_material, texture_index_map)) {
            m_materials.push_back(pbr_material);
            material_id = m_materials.size() - 1;
            material_map[mesh->material] = m_materials.size() - 1;
            LOG("Parsed material '" + mesh->material->name + "'");
        }

        std::vector<stage_vec3f> positions;
        std::vector<stage_vec3f> normals;
        std::vector<stage_vec2f> uvs;
        std::vector<uint32_t> material_ids;
        std::vector<uint32_t> indices;
        uint32_t g_n_idx_cnt = 0;

        for (auto& index : mesh->index) {
            for (int i = 0; i < 3; i++) {
                auto position = mesh->vertex[*(&index.x + i)];
                positions.push_back(make_vec3(&position.x));

                if (mesh->normal.size() > 0) {
                    auto normal = mesh->normal[*(&index.x + i)];
                    normals.push_back(make_vec3(&normal.x));
                } else {
                    const auto& v0 = make_vec3(&mesh->vertex[index.x].x);
                    const auto& v1 = make_vec3(&mesh->vertex[index.y].x);
                    const auto& v2 = make_vec3(&mesh->vertex[index.z].x);
                    normals.push_back(normalize(cross((v1 - v0), (v2 - v0))));
                }

                if (mesh->texcoord.size() > 0) {
                    auto uv = mesh->texcoord[*(&index.x + i)];
                    uvs.push_back(make_vec2(&uv.x));
                }
                material_ids.push_back(material_id);
                indices.emplace_back(g_n_idx_cnt++);
            }
        }

        Geometry g(obj, positions, normals, uvs, material_ids, indices);
        obj.geometries.push_back(g);
        LOG("Read geometry (v: " + std::to_string(g.positions.size()) + ", i: " + std::to_string(g.indices.size()) + ")");
    }

    // Load light sources
    for (auto& light_source : current->lightSources) {
        std::shared_ptr<pbrt::InfiniteLightSource> infinite_light = std::dynamic_pointer_cast<pbrt::InfiniteLightSource>(light_source);
        std::shared_ptr<pbrt::DistantLightSource> distant_light = std::dynamic_pointer_cast<pbrt::DistantLightSource>(light_source);
        std::shared_ptr<pbrt::PointLightSource> point_light = std::dynamic_pointer_cast<pbrt::PointLightSource>(light_source);

        Light light = Light::defaultLight();
        if (infinite_light) {
            light.L = make_vec3(&infinite_light->L.x);
            light.type = LightType::InfiniteLight;

            if (!infinite_light->mapName.empty()) {
                std::filesystem::path filename = getAbsolutePath(infinite_light->mapName);
                Image infinite_light_map(filename.string(), true);
                m_textures.push_back(std::move(infinite_light_map));

                light.map_texid = m_textures.size() - 1;
            }
            LOG("Parsed infinite light source");
        }

        if (distant_light) {
            light.L = make_vec3(&distant_light->L.x);
            light.type = LightType::DistantLight;
            light.from = make_vec3(&distant_light->from.x);
            light.to = make_vec3(&distant_light->to.x);
            LOG("Parsed distant light source");
        }

        if (point_light) {
            light.L = make_vec3(&point_light->I.x);
            light.type = LightType::PointLight;
            light.from = make_vec3(&point_light->from.x);
            LOG("Parsed point light source");
        }

        if (infinite_light || distant_light || point_light) 
            m_lights.push_back(light);
    }

    // Load area lights, which are shapes and not lights in PBRT
    for (auto& shape : current->shapes) {
        auto sphere_shape = std::dynamic_pointer_cast<pbrt::Sphere>(shape);
        auto disk_shape = std::dynamic_pointer_cast<pbrt::Disk>(shape);
        std::shared_ptr<pbrt::DiffuseAreaLightRGB> area_light_rgb;
        std::shared_ptr<pbrt::DiffuseAreaLightBB> area_light_bb;
        if (sphere_shape && sphere_shape->areaLight) {
            area_light_rgb = std::dynamic_pointer_cast<pbrt::DiffuseAreaLightRGB>(sphere_shape->areaLight);
            area_light_bb = std::dynamic_pointer_cast<pbrt::DiffuseAreaLightBB>(sphere_shape->areaLight);
        }

        if (disk_shape && disk_shape->areaLight) {
            area_light_rgb = std::dynamic_pointer_cast<pbrt::DiffuseAreaLightRGB>(disk_shape->areaLight);
            area_light_bb = std::dynamic_pointer_cast<pbrt::DiffuseAreaLightBB>(disk_shape->areaLight);
        }

        if (area_light_rgb || area_light_bb) {
            Light light = Light::defaultLight();
            light.type = sphere_shape ? LightType::SphereLight : LightType::DiskLight;
            light.radius = sphere_shape ? sphere_shape->radius : disk_shape->radius;
            pbrt::vec3f from = sphere_shape ? sphere_shape->transform.p : disk_shape->transform.p;
            light.from = make_vec3(&from.x);

            if (area_light_rgb) {
                light.L = make_vec3(&area_light_rgb->L.x);
            }

            if (area_light_bb) {
                pbrt::vec3f L = area_light_bb->LinRGB();
                light.L = make_vec3(&L.x);
            }

            m_lights.push_back(light);
            LOG((sphere_shape ? "Parsed sphere area light source" : "Parsed disk area light source"));
        }
    }

    if (obj.geometries.size() > 0) {
        object_map[current] = m_objects.size();
        m_objects.push_back(obj);
    }

    for (auto& instance : current->instances) {
        loadPBRTObjectsRecursive(instance->object, object_map, material_map, texture_index_map);
    }
}

void
PBRTScene::loadPBRTInstancesRecursive(std::shared_ptr<pbrt::Instance> current, const std::map<std::shared_ptr<pbrt::Object>, uint32_t>& object_map) {
    if (!current->object) return;

    if (object_map.find(current->object) != object_map.end()) {
        ObjectInstance instance;
        instance.object_id = object_map.at(current->object);
        auto& xfm = current->xfm;
        instance.instance_to_world = stage_mat4f(
                xfm.l.vx.x, xfm.l.vx.y, xfm.l.vx.z, 0.f,
                xfm.l.vy.x, xfm.l.vy.y, xfm.l.vy.z, 0.f,
                xfm.l.vz.x, xfm.l.vz.y, xfm.l.vz.z, 0.f,
                xfm.p.x, xfm.p.y, xfm.p.z, 1.f);

        m_instances.push_back(instance);
        LOG("Loaded instance of '" + current->object->name + "'");
    }

    for (auto& instance : current->object->instances) {
        loadPBRTInstancesRecursive(instance, object_map);
    }
}

bool 
PBRTScene::loadPBRTMaterial(std::shared_ptr<pbrt::Material> material, OpenPBRMaterial& pbr_material, std::map<std::shared_ptr<pbrt::Texture>, uint32_t>& texture_index_map) {
    if (std::dynamic_pointer_cast<pbrt::DisneyMaterial>(material))
        return loadPBRTMaterialDisney(*std::dynamic_pointer_cast<pbrt::DisneyMaterial>(material), pbr_material, texture_index_map);
    else if (std::dynamic_pointer_cast<pbrt::MetalMaterial>(material))
        return loadPBRTMaterialMetal(*std::dynamic_pointer_cast<pbrt::MetalMaterial>(material), pbr_material, texture_index_map);
    else if (std::dynamic_pointer_cast<pbrt::TranslucentMaterial>(material))
        return loadPBRTMaterialTranslucent(*std::dynamic_pointer_cast<pbrt::TranslucentMaterial>(material), pbr_material, texture_index_map);
    else if (std::dynamic_pointer_cast<pbrt::PlasticMaterial>(material))
        return loadPBRTMaterialPlastic(*std::dynamic_pointer_cast<pbrt::PlasticMaterial>(material), pbr_material, texture_index_map);
    else if (std::dynamic_pointer_cast<pbrt::SubstrateMaterial>(material))
        return loadPBRTMaterialSubstrate(*std::dynamic_pointer_cast<pbrt::SubstrateMaterial>(material), pbr_material, texture_index_map);
    else if (std::dynamic_pointer_cast<pbrt::MirrorMaterial>(material))
        return loadPBRTMaterialMirror(*std::dynamic_pointer_cast<pbrt::MirrorMaterial>(material), pbr_material, texture_index_map);
    else if (std::dynamic_pointer_cast<pbrt::MatteMaterial>(material))
        return loadPBRTMaterialMatte(*std::dynamic_pointer_cast<pbrt::MatteMaterial>(material), pbr_material, texture_index_map);
    else if (std::dynamic_pointer_cast<pbrt::GlassMaterial>(material))
        return loadPBRTMaterialGlass(*std::dynamic_pointer_cast<pbrt::GlassMaterial>(material), pbr_material, texture_index_map);
    else if (std::dynamic_pointer_cast<pbrt::UberMaterial>(material))
        return loadPBRTMaterialUber(*std::dynamic_pointer_cast<pbrt::UberMaterial>(material), pbr_material, texture_index_map);
    else
        return false;
}

bool 
PBRTScene::loadPBRTMaterialDisney(pbrt::DisneyMaterial& material, OpenPBRMaterial& pbr_material, std::map<std::shared_ptr<pbrt::Texture>, uint32_t>& texture_index_map) {
    pbr_material.base_color = make_vec3(&material.color.x);
    pbr_material.base_metalness = material.metallic;
    pbr_material.specular_ior = material.eta;
    pbr_material.specular_roughness = material.roughness;
    pbr_material.transmission_weight = material.specTrans;
    return true;
}

bool 
PBRTScene::loadPBRTMaterialMetal(pbrt::MetalMaterial& material, OpenPBRMaterial& pbr_material, std::map<std::shared_ptr<pbrt::Texture>, uint32_t>& texture_index_map) {
    pbr_material.base_metalness = 1.f;
    // TODO: It is unclear if this mapping for `k` is appropriate as components in both `k` and `eta` regularly exceed 1
    pbr_material.base_color = normalize(make_vec3(&material.k.x));
    pbr_material.specular_color = normalize(make_vec3(&material.k.x));

    stage_vec3f k = loadPBRTSpectrum(material.spectrum_k);
    if (k != stage_vec3f(0.f)) {
        pbr_material.base_color = k;
        pbr_material.specular_color = k;
    }

    if (material.remapRoughness) {
        float roughness = std::max(material.roughness, 1e-5f);
        float x = std::log(roughness);
        pbr_material.specular_roughness = 1.62142f + 0.819955f * x + 0.1734f * x * x +
               0.0171201f * x * x * x + 0.000640711f * x * x * x * x;
    } else {
        pbr_material.specular_roughness = material.roughness;
    }
    return true;
}

bool 
PBRTScene::loadPBRTMaterialTranslucent(pbrt::TranslucentMaterial& material, OpenPBRMaterial& pbr_material, std::map<std::shared_ptr<pbrt::Texture>, uint32_t>& texture_index_map) {
    return false;
}

bool 
PBRTScene::loadPBRTMaterialPlastic(pbrt::PlasticMaterial& material, OpenPBRMaterial& pbr_material, std::map<std::shared_ptr<pbrt::Texture>, uint32_t>& texture_index_map) {
    pbr_material.base_color = make_vec3(&material.kd.x);
    pbr_material.specular_color = make_vec3(&material.ks.x);
    if (material.remapRoughness) {
        float roughness = std::max(material.roughness, 1e-3f);
        float x = std::log(roughness);
        pbr_material.specular_roughness = 1.62142f + 0.819955f * x + 0.1734f * x * x +
               0.0171201f * x * x * x + 0.000640711f * x * x * x * x;
    } else {
        pbr_material.specular_roughness = material.roughness;
    }

    uint32_t texture_idx;
    if (material.map_kd && loadPBRTTexture(material.map_kd, texture_index_map, texture_idx)) {
        pbr_material.base_color_texid = texture_idx;
    }
    return true;
}

bool 
PBRTScene::loadPBRTMaterialSubstrate(pbrt::SubstrateMaterial& material, OpenPBRMaterial& pbr_material, std::map<std::shared_ptr<pbrt::Texture>, uint32_t>& texture_index_map) {
    // TODO: This is similar to the plastic material. For now, treat it the same.
    pbr_material.base_color = make_vec3(&material.kd.x);
    pbr_material.specular_color = make_vec3(&material.ks.x);

    // TODO: Anisotropic roughness is currently not supported. Average both values and use that for now.
    float roughness = (material.uRoughness + material.vRoughness) / 2.f;
    if (material.remapRoughness) {
        roughness = std::max(roughness, 1e-5f);
        float x = std::log(roughness);
        pbr_material.specular_roughness = 1.62142f + 0.819955f * x + 0.1734f * x * x +
               0.0171201f * x * x * x + 0.000640711f * x * x * x * x;
    } else {
        pbr_material.specular_roughness = roughness;
    }

    uint32_t texture_idx;
    if (material.map_kd && loadPBRTTexture(material.map_kd, texture_index_map, texture_idx)) {
        pbr_material.base_color_texid = texture_idx;
    }
    return true;
}

bool 
PBRTScene::loadPBRTMaterialMirror(pbrt::MirrorMaterial& material, OpenPBRMaterial& pbr_material, std::map<std::shared_ptr<pbrt::Texture>, uint32_t>& texture_index_map) {
    pbr_material.base_metalness = 1.f;
    pbr_material.specular_color = make_vec3(&material.kr.x);
    pbr_material.specular_roughness = 1e-5f;
    return true;
}

bool 
PBRTScene::loadPBRTMaterialMatte(pbrt::MatteMaterial& material, OpenPBRMaterial& pbr_material, std::map<std::shared_ptr<pbrt::Texture>, uint32_t>& texture_index_map) {
    pbr_material.base_color = make_vec3(&material.kd.x);
    pbr_material.specular_roughness = 1.f;
    
    uint32_t texture_idx;
    if (material.map_kd && loadPBRTTexture(material.map_kd, texture_index_map, texture_idx)) {
        pbr_material.base_color_texid = texture_idx;
    }
    return true;
}

bool 
PBRTScene::loadPBRTMaterialGlass(pbrt::GlassMaterial& material, OpenPBRMaterial& pbr_material, std::map<std::shared_ptr<pbrt::Texture>, uint32_t>& texture_index_map) {
    //TODO: See metal material. Unclear if mapping `kr` like this makes sense
    pbr_material.specular_color = make_vec3(&material.kr.x);
    pbr_material.specular_roughness = 1e-5f;
    pbr_material.specular_ior = material.index;
    pbr_material.transmission_weight = 1.f;
    return true;
}

bool 
PBRTScene::loadPBRTMaterialUber(pbrt::UberMaterial& material, OpenPBRMaterial& pbr_material, std::map<std::shared_ptr<pbrt::Texture>, uint32_t>& texture_index_map) {
    pbr_material.base_color = make_vec3(&material.kd.x);
    pbr_material.specular_color = make_vec3(&material.ks.x);
    pbr_material.specular_ior = material.index;
    pbr_material.specular_roughness = material.roughness;
    pbr_material.transmission_weight = luminance(make_vec3(&material.kt.x));

    uint32_t texture_idx;
    if (material.map_kd && loadPBRTTexture(material.map_kd, texture_index_map, texture_idx)) {
        pbr_material.base_color_texid = texture_idx;
    }
    return true;
}

bool
PBRTScene::loadPBRTTexture(std::shared_ptr<pbrt::Texture> texture, std::map<std::shared_ptr<pbrt::Texture>, uint32_t>& texture_index_map, uint32_t& texture_index) {
    if (texture_index_map.find(texture) != texture_index_map.end()) {
        texture_index = texture_index_map[texture];
        return true;
    }

    std::shared_ptr<pbrt::ConstantTexture> constant_texture = std::dynamic_pointer_cast<pbrt::ConstantTexture>(texture);
    std::shared_ptr<pbrt::ScaleTexture> scale_texture = std::dynamic_pointer_cast<pbrt::ScaleTexture>(texture);
    std::shared_ptr<pbrt::MixTexture> mix_texture = std::dynamic_pointer_cast<pbrt::MixTexture>(texture);
    std::shared_ptr<pbrt::ImageTexture> image_texture = std::dynamic_pointer_cast<pbrt::ImageTexture>(texture);
    if (!constant_texture && !scale_texture && !mix_texture && !image_texture) {
        WARN("Unsupported texture type '" + texture->toString() + "'");
        return false;
    }

    if (constant_texture) {
        stage_vec3f color = make_vec3(&constant_texture->value.x);
        Image img(color);
        m_textures.push_back(std::move(img));
        LOG("Read constant image (" + std::to_string(color.x) + ", " + std::to_string(color.y) + ", " + std::to_string(color.z) + ")");
        texture_index_map[texture] = m_textures.size() - 1;
        texture_index = m_textures.size() - 1;
        return true;
    }

    if (scale_texture) {
        uint32_t tex1_idx, tex2_idx;
        bool has_tex1 = scale_texture->tex1 && loadPBRTTexture(scale_texture->tex1, texture_index_map, tex1_idx);
        bool has_tex2 = scale_texture->tex2 && loadPBRTTexture(scale_texture->tex2, texture_index_map, tex2_idx);

        if (has_tex1 && !has_tex2) {
            // m_textures[tex1_idx].scale(make_vec3(&scale_texture->scale2.x));
            m_textures[tex1_idx].scale(make_vec3(&scale_texture->scale2.x));
            texture_index = tex1_idx;
        } else if (has_tex2 && !has_tex1) {
            // m_textures[tex2_idx].scale(make_vec3(&scale_texture->scale1.x));
            m_textures[tex2_idx].scale(make_vec3(&scale_texture->scale1.x));
            texture_index = tex2_idx;
        } else if (has_tex1 && has_tex2) {
            auto& tex2 = m_textures[tex2_idx];
            m_textures[tex1_idx].scale(tex2);
            texture_index = tex1_idx;
        } else {
            return false;
        }

        return true;
    }

    if (mix_texture) {
        uint32_t tex1_idx, tex2_idx;
        bool has_tex1 = mix_texture->tex1 && loadPBRTTexture(mix_texture->tex1, texture_index_map, tex1_idx);
        bool has_tex2 = mix_texture->tex2 && loadPBRTTexture(mix_texture->tex2, texture_index_map, tex2_idx);

        if (has_tex1 && !has_tex2) {
            // m_textures[tex1_idx].mix(make_vec3(&mix_texture->scale2.x), 
                                //   make_vec3(&mix_texture->amount.x));
            m_textures[tex1_idx].mix(make_vec3(&mix_texture->scale2.x), 
                                  make_vec3(&mix_texture->amount.x));
            texture_index = tex1_idx;
        } else if (has_tex2 && !has_tex1) {
            // m_textures[tex2_idx].mix(make_vec3(&mix_texture->scale1.x), 
                                //   make_vec3(&mix_texture->amount.x));
            m_textures[tex2_idx].mix(make_vec3(&mix_texture->scale1.x), 
                                  make_vec3(&mix_texture->amount.x));
            texture_index = tex2_idx;
        } else if (has_tex1 && has_tex2) {
            auto& tex2 = m_textures[tex2_idx];
            // m_textures[tex1_idx].mix(tex2, 
                                //   make_vec3(&mix_texture->amount.x));
            m_textures[tex1_idx].mix(tex2, 
                                  make_vec3(&mix_texture->amount.x));
            texture_index = tex1_idx;
        } else {
            return false;
        }

        return true;
    }

    if (image_texture) {
        std::filesystem::path texture_filename = getAbsolutePath(image_texture->fileName);
        Image img(texture_filename.string()); 
        m_textures.push_back(std::move(img));
        LOG("Read texture image '" + image_texture->fileName + "'");
        texture_index_map[texture] = m_textures.size() - 1;
        texture_index = m_textures.size() - 1;
        return true;
    }

    return false;
}

// Adapted from https://github.com/mmp/pbrt-v3/blob/13d871faae88233b327d04cda24022b8bb0093ee/src/core/spectrum.h#L289
stage_vec3f
PBRTScene::loadPBRTSpectrum(pbrt::Spectrum& spectrum) {
    if (spectrum.spd.size() == 0) return stage_vec3f(0.f);
    int sampled_lambda_start = 400, sampled_lambda_end = 700;
    int n_spectral_samples = 60;
    std::vector<float> lambdas, values;
    stage_vec3f xyz(0.f), rgb(0.f);
    std::vector<float> c (n_spectral_samples, 0.f);
    std::vector<float> X (n_spectral_samples, 0.f), Y (n_spectral_samples, 0.f), Z (n_spectral_samples, 0.f);

    for (auto& el : spectrum.spd) {
        lambdas.push_back(el.first);
        values.push_back(el.second);
    }

    auto averageSample = [](const std::vector<float>& lambdas, const std::vector<float>& values, float lambda0, float lambda1) {
        size_t n = lambdas.size();
        if (lambda1 <= lambdas[0]) return values[0];
        if (lambda0 >= lambdas[n - 1]) return values[n - 1];
        if (n == 1) return values[0];
        float sum = 0;

        if (lambda0 < lambdas[0]) sum += values[0] * (lambdas[0] - lambda0);
        if (lambda1 > lambdas[n - 1])
            sum += values[n - 1] * (lambda1 - lambdas[n - 1]);

        size_t i = 0;
        while (lambda0 > lambdas[i + 1]) ++i;
        if (i+1 >= n) return sum;

        auto interp = [lambdas, values](float w, int i) {
            float weight = (w - lambdas[i]) / (lambdas[i+1] - lambdas[i]);
            return (1.f - weight) * values[i] + weight * values[i+1];
        };

        for (; i + 1 < n && lambda1 >= lambdas[i]; ++i) {
            float segLambdaStart = std::max(lambda0, lambdas[i]);
            float segLambdaEnd = std::min(lambda1, lambdas[i + 1]);
            sum += 0.5 * (interp(segLambdaStart, i) + interp(segLambdaEnd, i)) *
                (segLambdaEnd - segLambdaStart);
        }

        return sum / (lambda1 - lambda0);
    };

    for (int i = 0; i < n_spectral_samples; i++) {
        float w0 = (float)i / n_spectral_samples;
        float w1 = (float)(i+1) / n_spectral_samples;
        float lambda0 = (1.f - w0) * sampled_lambda_start + w0 * sampled_lambda_end;
        float lambda1 = (1.f - w1) * sampled_lambda_start + w1 * sampled_lambda_end;
        c[i] = averageSample(lambdas, values, lambda0, lambda1);
        X[i] = averageSample(CIE_lambda, CIE_X, lambda0, lambda1);
        Y[i] = averageSample(CIE_lambda, CIE_Y, lambda0, lambda1);
        Z[i] = averageSample(CIE_lambda, CIE_Z, lambda0, lambda1);
    }

    for (int i = 0; i < n_spectral_samples; ++i) {
        xyz[0] += X[i] * c[i];
        xyz[1] += Y[i] * c[i];
        xyz[2] += Z[i] * c[i];
    }

    float scale = float(CIE_lambda[n_cie_samples-1] - CIE_lambda[0]) /
                    float(CIE_Y_integral * n_cie_samples);
    xyz[0] *= scale;
    xyz[1] *= scale;
    xyz[2] *= scale;

    rgb[0] = 3.240479f * xyz[0] - 1.537150f * xyz[1] - 0.498535f * xyz[2];
    rgb[1] = -0.969256f * xyz[0] + 1.875991f * xyz[1] + 0.041556f * xyz[2];
    rgb[2] = 0.055648f * xyz[0] - 0.204043f * xyz[1] + 1.057311f * xyz[2];

    return clamp(rgb, 0.f, 1.f);
}

void FBXScene::loadFBX() {
    ufbx_load_opts opts = { }; // Optional, pass NULL for defaults
    opts.progress_cb.fn = nullptr;
    opts.progress_cb.user = nullptr;

    ufbx_error error; // Optional, pass NULL if you don't care about errors
    ufbx_scene *fbx_scene = ufbx_load_file(m_scene_path.string().c_str(), &opts, &error);
    if (!fbx_scene) {
        throw std::runtime_error(error.description.data);
    }

    // Parse materials
    std::map<uint32_t, uint32_t> material_map;
    for (size_t materialid = 0; materialid < fbx_scene->materials.count; materialid++) {
        auto* fbx_material = fbx_scene->materials.data[materialid];

        OpenPBRMaterial material = OpenPBRMaterial::defaultMaterial();
        material.base_color.x = fbx_material->pbr.base_color.value_vec3.x;
        material.base_color.y = fbx_material->pbr.base_color.value_vec3.y;
        material.base_color.z = fbx_material->pbr.base_color.value_vec3.z;
        material.base_metalness = fbx_material->pbr.metalness.value_real;
        material.base_roughness = fbx_material->pbr.diffuse_roughness.value_real;
        material.base_weight = fbx_material->pbr.base_factor.value_real;

        material.specular_anisotropy = fbx_material->pbr.specular_anisotropy.value_real;
        material.specular_color.x = fbx_material->pbr.specular_color.value_vec3.x;
        material.specular_color.y = fbx_material->pbr.specular_color.value_vec3.y;
        material.specular_color.z = fbx_material->pbr.specular_color.value_vec3.z;
        material.specular_ior = fbx_material->pbr.specular_ior.value_real;
        material.specular_rotation = fbx_material->pbr.specular_rotation.value_real;
        material.specular_roughness = fbx_material->pbr.roughness.value_real;
        material.specular_weight = fbx_material->pbr.specular_factor.value_real;

        material.transmission_weight = fbx_material->pbr.transmission_factor.value_real;

        material.geometry_opacity = fbx_material->pbr.opacity.value_real;

        // Load textures
        if (fbx_material->pbr.base_color.texture_enabled) {
            if (loadFBXTexture(fbx_material->pbr.base_color.texture)) {
                material.base_color_texid = m_textures.size() - 1;
            }
        }

        if (fbx_material->pbr.opacity.texture_enabled) {
            if (loadFBXTexture(fbx_material->pbr.opacity.texture)) {
                material.geometry_opacity_texid = m_textures.size() - 1;
            }
        }

        m_materials.push_back(material);
        material_map[fbx_material->element_id] = m_materials.size() - 1;
    }
    // Put a default material in case faces have no material assigned
    OpenPBRMaterial material = OpenPBRMaterial::defaultMaterial();
    m_materials.push_back(material);

    // Parse objects
    uint32_t triangulate_indices[1024];
    for (size_t meshid = 0; meshid < fbx_scene->meshes.count; meshid++) {
        auto* fbx_mesh = fbx_scene->meshes[meshid];
        if (fbx_mesh->instances.count == 0) continue;

        Object obj(m_config.layout, m_config.vertex_alignment);

        std::map<uint32_t, uint32_t> index_map;

        std::vector<stage_vec3f> positions;
        std::vector<stage_vec3f> normals;
        std::vector<stage_vec2f> uvs;
        std::vector<uint32_t> material_ids;
        std::vector<uint32_t> indices;

        // Keep track of all the unique indices we use
        uint32_t g_n_unique_idx_cnt = 0;

        for (uint32_t faceid = 0; faceid < fbx_mesh->num_faces; faceid++) {
            size_t num_tris = ufbx_triangulate_face(triangulate_indices, 1024, fbx_mesh, fbx_mesh->faces[faceid]);

            for (uint32_t triangleid = 0; triangleid < num_tris; triangleid++) {
                for (uint32_t vertexid = 0; vertexid < 3; vertexid++) {
                    uint32_t index = triangulate_indices[triangleid*3 + vertexid];
                    uint32_t g_index = 0;
                    if (index_map.find(index) != index_map.end()) {
                        g_index = index_map.at(index);
                    } else {
                        g_index = g_n_unique_idx_cnt++;

                        ufbx_vec3 position = ufbx_get_vertex_vec3(&fbx_mesh->vertex_position, index);
                        ufbx_vec3 normal = ufbx_get_vertex_vec3(&fbx_mesh->vertex_normal, index);
                        ufbx_vec2 uv = fbx_mesh->vertex_uv.exists ? ufbx_get_vertex_vec2(&fbx_mesh->vertex_uv, index) : ufbx_vec2({0, 0});
                        stage_vec3f stage_position;
                        stage_position.x = position.x;
                        stage_position.y = position.y;
                        stage_position.z = position.z;
                        stage_vec3f stage_normal;
                        stage_normal.x = normal.x;
                        stage_normal.y = normal.y;
                        stage_normal.z = normal.z;
                        stage_vec2f stage_uv;
                        stage_uv.x = uv.x;
                        stage_uv.y = uv.y;
                        positions.push_back(stage_position);
                        normals.push_back(stage_normal);
                        uvs.push_back(stage_uv);
                        
                        if (fbx_mesh->face_material.count > 0) {
                            auto* fbx_material = fbx_mesh->materials.data[fbx_mesh->face_material[faceid]];
                            uint32_t materialid = fbx_material->element_id;
                            if (material_map.find(materialid) != material_map.end()) {
                                material_ids.push_back(material_map.at(materialid));
                            } else {
                                material_ids.push_back(m_materials.size() - 1);
                            }
                        } else {
                            material_ids.push_back(m_materials.size() - 1);
                        }

                        index_map[index] = g_index;
                    }
                    indices.push_back(g_index);
                }
            }
        }
        Geometry g(obj, positions, normals, uvs, material_ids, indices);
        obj.geometries.push_back(g);
        LOG("Read geometry (v: " + std::to_string(g.positions.size()) +", i: " + std::to_string(g.indices.size()) + ")");
        m_objects.push_back(obj);

        // Parse instances
        for (uint32_t instanceid = 0; instanceid < fbx_mesh->instances.count; instanceid++) {
            auto* fbx_instance = fbx_mesh->instances[instanceid];

            ObjectInstance instance;
            instance.object_id = m_objects.size() - 1;
            instance.instance_to_world = stage_mat4f(
                stage_vec4f(stage_vec3f(fbx_instance->node_to_world.cols[0].x, fbx_instance->node_to_world.cols[0].y, fbx_instance->node_to_world.cols[0].z), 0.f),
                stage_vec4f(stage_vec3f(fbx_instance->node_to_world.cols[1].x, fbx_instance->node_to_world.cols[1].y, fbx_instance->node_to_world.cols[1].z), 0.f),
                stage_vec4f(stage_vec3f(fbx_instance->node_to_world.cols[2].x, fbx_instance->node_to_world.cols[2].y, fbx_instance->node_to_world.cols[2].z), 0.f),
                stage_vec4f(stage_vec3f(fbx_instance->node_to_world.cols[3].x, fbx_instance->node_to_world.cols[3].y, fbx_instance->node_to_world.cols[3].z), 1.f)
            );

            m_instances.push_back(instance);
        }
    }

    // // Parse camera
    if (fbx_scene->cameras.count > 0) {
        // m_camera = std::make_shared<Camera>();
        // auto* fbx_camera = fbx_scene->cameras[0];
        // m_camera->fovy = glm::radians(fbx_camera->field_of_view_deg.y);
        // m_camera->position = make_vec3(&fbx_camera->element.instances[0]->unscaled_node_to_world.cols[3].x);
    }

    ufbx_free_scene(fbx_scene);
}

bool 
FBXScene::loadFBXTexture(ufbx_texture *texture)
{
    std::unique_ptr<Image> image;
    if (texture->content.size > 0)
    {
        image = std::make_unique<Image>((uint8_t*)texture->content.data, texture->content.size);
        if (image->isValid()) LOG("Read texture image blob");
    }
    else
    {
        std::filesystem::path filepath = getAbsolutePath(texture->relative_filename.data);
        image = std::make_unique<Image>(filepath.string());
        if (image->isValid()) LOG("Read texture image '" + filename + "'");
    }
    if (image->isValid())
    {
        m_textures.push_back(std::move(*image));
        return true;
    }
    return false;
}
}
}
