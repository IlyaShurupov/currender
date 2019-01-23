#pragma once

#include <vector>
#include <memory>

#include "nanort/nanort.h"
#include "include/camera.h"
#include "include/mesh.h"

namespace crender {

class RendererOption {
 public:
  bool use_vertex_color{false};
  float depth_scale{1.0f};
  enum ColorInterpolation { NN = 0, BILINEAR = 1 };
  ColorInterpolation interp{BILINEAR};
  bool backface_culling{true};

  RendererOption();
  ~RendererOption();
  void copy_to(RendererOption& dst) const;
};

class Renderer {
  bool mesh_initialized_{false};
  std::shared_ptr<Camera> camera_;
  std::shared_ptr<Mesh> mesh_;
  RendererOption option_;

  std::vector<float> flatten_vertices;
  std::vector<unsigned int> flatten_faces;

  nanort::BVHBuildOptions<float> build_options;
  std::unique_ptr<nanort::TriangleMesh<float>> triangle_mesh;
  std::unique_ptr<nanort::TriangleSAHPred<float>> triangle_pred;
  nanort::BVHAccel<float> accel;
  nanort::BVHBuildStatistics stats;
  float bmin[3], bmax[3];

 public:
  Renderer();
  ~Renderer();
  explicit Renderer(const RendererOption& option);
  void set_option(const RendererOption& option);
  void set_mesh(std::shared_ptr<Mesh> mesh);
  bool prepare_mesh();
  void set_camera(std::shared_ptr<Camera> camera);
  bool render(Image3b& color, Image1w& depth, Image1b& mask);
};

}  // namespace crender