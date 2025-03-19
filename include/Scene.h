#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <random>
#include <regex>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <misc/cpp/imgui_stdlib.h>

#include "Auxiliary.hpp"
#include "Bounding.h"
#include "Camera.h"
#include "Geometry.h"
#include "LSystem.h"
#include "Renderer.h"
#include "Shader.h"
#include "Skeleton.h"
#include "Transform.h"


extern std::mt19937_64 rdgen;

#define MOUSE_VIEW_ROTATE_SENSITIVITY 0.1f
#define MOUSE_VIEW_TRANSLATE_SENSITIVITY 0.06f
#define RAY_LENGTH_TO_CUBEMAP 10.0f
#define SURFACE_NORMAL_OFFSET 0.005f

#define TEXT(txt) reinterpret_cast<const char *>(u8##txt)

namespace Scene {

void framebufferResizeCallback(GLFWwindow *window, int width, int height);

struct Pixel {
  // 用于解析stb_load加载的png图片
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
};

struct PngImage {
  Pixel *img;
  int width;
  int height;
  int channel;
};

struct SkeletonObject {
  std::string name;
  std::shared_ptr<Skeleton> skeleton{nullptr};
};

struct Status {
  bool selected{false};
  bool visible{true};
  bool collided{false};
  bool listed{true};
  bool lighted{true};
};

struct SkyboxInfo {
  GLuint vao{0};
  GLuint vbo{0};
  GLuint ebo{0};
  GLuint texture{0};
};

struct DepthmapInfo {
  float left{-10.0f};
  float right{10.0f};
  float bottom{-10.0f};
  float top{10.0f};
  float near{0.0f};
  float far{200.0f};
  GLuint texture{0};
  GLuint fbo{0};
};

struct FramebufferInfo {
  GLuint fbo{0};
  GLuint texture{0};
  GLuint rbo{0};
  GLuint vbo{0};
  GLuint vao{0};
  GLuint ebo{0};
};

struct RadiosityResult {
  // float radiant_flux{0.0f};
  // 与GeometryContext::geometry::surfaces的元素一一对应
  std::vector<glm::vec3> radiant_flux;
};

class LineDrawer {
private:
  GLuint vao{0};
  GLuint vbo{0};
  std::vector<glm::vec3> rays;
  std::vector<uint32_t> v_nums;
  std::vector<glm::vec3> colors;

public:
  LineDrawer();
  void addLine(glm::vec3 pt1, glm::vec3 pt2, glm::vec3 color);
  void addLine(glm::vec3 pt1, glm::vec3 pt2);
  void addPolygon(const std::vector<glm::vec3> &vert);

  void addPolygon(const std::vector<glm::vec3> &vert, glm::vec3 color);
  void clear();
  void update();
  size_t size() const;

  void draw(Shader *sd);
  ~LineDrawer();
};

Pixel cubemap_sample(PngImage *cubmaps, glm::vec3 dir);

class TriangleSampler {
public:
  glm::vec3 pt[3];
  TriangleSampler(const std::vector<Vertex> &vertices, const Surface &surface,
                  glm::mat4 model);
  TriangleSampler(const std::vector<Vertex> &vertices, const Surface &surface);

  float calcArea() const;

  glm::vec3 calcNorm() const;
  glm::vec3 calcCenter() const;
  // 以法向量为上方向基准，结合pt[2]-pt[1]、pt[1]-pt[0]所成平面构建局部坐标系
  std::tuple<glm::vec3, glm::vec3, glm::vec3> calcLocalCoord() const;

  // 半球均匀分布随机方向采样
  glm::vec3 hemisphereSampleDir() const;
};

class BoundingBoxContext : public OpenGLContext {
private:
  BoundingBox *box{nullptr};
  Transform *transform;

public:
  BoundingBoxContext() = delete;

  BoundingBoxContext(BoundingBox *box, Transform *transform);
  std::tuple<std::vector<glm::vec3>, std::vector<uint32_t>> genOpenGLRawData();

  virtual void init();
  virtual void update();
};

class GeometryObject {
private:
  std::string name;
  std::shared_ptr<Geometry> geometry{nullptr};
  std::unique_ptr<BoundingBox> box{nullptr};
  std::unique_ptr<BvhTree> bvhtree{nullptr};

public:
  Transform transform;
  Status status;

  RadiosityResult radiosity; // 由compute_radiosity()更新

  std::string getName() const;
  BvhTree *getBvhTree();
  BoundingBox *getBoundingBox();
  Geometry *getGeometry();
  GeometryObject(std::string name, std::shared_ptr<Geometry> geometry,
                 Transform transform = Transform{}, bool useBvh = false);

  void update();
  virtual ~GeometryObject();
};

class GeometryContext : public OpenGLContext {
private:
  GeometryObject *obj{nullptr};
  BvhTree *bvhtree{nullptr};

public:
  std::unique_ptr<BoundingBoxContext> box{nullptr};
  std::vector<std::unique_ptr<BoundingBoxContext>> boxes;
  GeometryContext(GeometryObject *obj);

  virtual void init();
  virtual void update();
};

void errorCallback(int code, const char *msg);

struct ImguiInfo {
  bool start_record{true};
  ImVec2 mouse_pos{0, 0};
  std::shared_ptr<GeometryObject> cur{nullptr};
  int32_t selected_idx = 0;
  std::vector<std::shared_ptr<GeometryObject>> list_items;
  bool changeGeometryListView{true};
};

struct LSystemInfo {
  const size_t LSYSTEM_MAX_LENGTH{1000};
  std::string axiom;
  std::vector<std::string> productions;
  uint32_t iter_n{0};
  std::shared_ptr<LSystem::LSystem> config{nullptr};
  std::shared_ptr<Skeleton> skeleton{nullptr};
};

// 路径上每条光线的信息
struct RayInfo {
  glm::vec3 BRDF{1.0f, 1.0f, 1.0f};
  float cosine{1.0f};
};

class Scene {
private:
  const float FPS_SHOW_SPAN = 1.0f; // 大约每过1秒显示一下fps
  const GLuint PVM_binding_point{0};
  const std::regex check_skeleton_node_name_pattern{"[a-zA-Z]\\w*#\\d+"};

  int width = 1600;
  int height = 1200;

  GLFWwindow *window{nullptr};
  ImGuiIO *io{nullptr};

  GLuint ubo{0};

  PngImage cubemaps[6];

  SkyboxInfo skybox;
  DepthmapInfo depthmap;
  FramebufferInfo framebuffer;

  ImguiInfo imgui;
  LSystemInfo lsystem;

  bool isShowGround{true};
  bool isShowAxis{true};
  bool isShowLight{true};
  bool isShowRay{false};
  bool isShowCoord{false};
  bool isShowWireFrame{false};
  bool isShowCursor{false};
  bool isShowBvhFrame{false};

public:
  std::map<std::string, Shader *> shaders;
  std::map<std::string, GLuint> textures;
  std::map<std::shared_ptr<GeometryObject>, std::unique_ptr<GeometryContext>>
      objs;

  std::map<std::string, std::shared_ptr<LineDrawer>> lines;
  std::map<std::string, SkeletonObject> skeletons;

  // 开发阶段暂时忽略渲染逻辑，实现lights中光源模拟辐照度计算
  std::vector<std::shared_ptr<Light>>
      lights; // 只用于计算的光源，为了能在场景中看到光源实际位置，需要将其加入到aux中使用sphere进行渲染可视化
  PointLight light{{1.0f, 1.0f, 1.0f},
                   {-2.0f, 10.0f, 3.0f},
                   1.0f}; // 用于OpenGL可视化渲染的光源

  Camera camera{glm::vec3(4.0f, 11.0f, 27.0f), glm::vec3{15.0f, 2.0f, 0.0f},
                static_cast<float>(width) / static_cast<float>(height)};
  Scene();

  Scene(const Scene &sc) = delete;
  ~Scene();

  void init_line_buffer();

  void init_ubo();

  void init_lsystem();

  void init_scene_obj();

  void init_skybox();

  void test_cubemap();

  void resize_framebuffer();

  void init_framebuffer();

  void init_depthmap();

  // shared_ptr<RayObject> generate_ray_object() {
  //   shared_ptr<RayObject> obj = make_shared<RayObject>();
  //   this->ray_objs.emplace_back(obj);
  //   return obj;
  // }

  // void update_ray_obj() {
  //   // 根据ray_obj.vertices更新vbo
  //   glBindVertexArray(ray_obj.vao);
  //   glBindBuffer(GL_ARRAY_BUFFER, ray_obj.vbo);
  //   glBufferData(GL_ARRAY_BUFFER, ray_obj.vertices.size() * sizeof(vec3),
  //   ray_obj.vertices.data(), GL_DYNAMIC_DRAW);
  // }

  void showAxis();
  void hideAxis();
  void showGround();
  void hideGround();
  void showCursor();
  void hideCursor();

  void show_info();

  void printRadiosityInfo();

  std::shared_ptr<GeometryObject>
  findGeometryObjectByName(const std::string &name);

  // void setSeed(uint32_t sd) {
  //   this->random_generator.seed(sd);
  // }

  Ray cast_ray_from_mouse();

  glm::vec3 screen2world(glm::vec2 pos);

  void imgui_interact();

  void updateGeometryListView();
  void addSoftReturnsToText(std::string &str, float multilineWidth);

  bool imgui_autosizingMultilineInput(
      const char *label, std::string *str, const ImVec2 &sizeMin,
      const ImVec2 &sizeMax,
      ImGuiInputTextFlags flags = ImGuiInputTextFlags_None);

  bool imgui_menu();

  void loadIcon();

  void initImgui();
  void load_all_shader();

  void load_all_texture();

  void test_triangle_coord();

  void addSceneObject(const std::shared_ptr<GeometryObject> &obj, bool visible,
                      bool listed, bool collided, bool lighted, GLuint texture);

  void addSceneObject(const std::shared_ptr<GeometryObject> &obj);

  void remove(const std::string &name);

  void removeSkeleton(const std::string &name);

  void add(const std::string &name, std::shared_ptr<Skeleton> skeleton,
           Transform transform);

  void add(const std::string &name, std::shared_ptr<Skeleton> skeleton);

  void add(const std::string &name, const std::shared_ptr<Geometry> &geometry,
           Transform transform, bool visible, bool listed, bool collided,
           bool lighted, bool useBvh);
  void add(const std::string &name, const std::shared_ptr<Geometry> &geometry,
           Transform transform);

  void add(const std::string &name, const std::shared_ptr<Geometry> &geometry);
  //   void add(const string &name, const shared_ptr<Geometry> &geometry,
  //            vec3 position);

  void addLight(const std::string &name, const std::shared_ptr<Light> &light);

  // void compute_radiosity_1() {
  //   // 遍历场景中所有objs并计算每个物体对应接收到的辐射通量

  //   // 实际光源并不只有一个，而且并非只有平行光(后续考虑)
  //   shared_ptr<ParallelLight> light =
  //       dynamic_pointer_cast<ParallelLight>(this->lights[0]);

  //   for (auto &[name, cur_obj] : this->objs) {
  //     shared_ptr<Geometry> geometry = cur_obj->geometry;
  //     cur_obj->radiosity.radiant_flux.resize(geometry->surfaces.size());
  //     for (int i = 0; i < geometry->surfaces.size(); i++) {
  //       Surface triangle = geometry->surfaces[i];
  //       vector<vec3> pt(3);
  //       vec3 norm{0.0f, 0.0f, 0.0f};
  //       float area{0.0f};
  //       for (int j = 0; j < 3; j++)
  //         pt[j] =
  //         glm::make_vec3(geometry->vertices[triangle.tidx[j]].position);
  //       vec3 tri_cross = glm::cross(pt[1] - pt[0], pt[2] - pt[0]);
  //       norm = glm::normalize(tri_cross);
  //       area = glm::length(tri_cross) / 2.0f;
  //       // 计算辐射通量(每个三角面)
  //       cur_obj->radiosity.radiant_flux[i] =
  //           light->irradiance * area *
  //           glm::max(0.0f, glm::dot(norm, -light->direction));
  //     }
  //   }
  // }

  // struct HitGeometryObj {
  //   bool isHit{false};
  //   string obj_name; // 击中物体名称
  //   uint32_t tri_id; // 击中物体所在三角面元的索引
  //   vec3 hit_pos;    // 世界坐标系下的击中位置
  // };

  HitInfo hit(Ray ray);

  // // 递归函数版本
  // vec3 trace_ray(Ray ray, const HitInfo &obj, float PR, uint32_t
  // recursive_depth = 0, shared_ptr<vector<vec3>> vert_buffer = nullptr) {
  //   //
  //   recursive_depth用以记录该函数的递归深度，同时在可视化光线路径时用于记录顶点数量
  //   // recursive_depth=0表示对该函数的“根调用”，其他为递归调用
  //   assert(PR < 1.0f && PR > 0.0f);
  //   uniform_real_distribution<> distr(0.0, 1.0);
  //   vec3 L_dir{0.0f, 0.0f, 0.0f}, L_indir{0.0f, 0.0f, 0.0f};
  //   float PR_D = distr(rdgen);
  //   if (PR > PR_D) { // 俄罗斯赌轮盘
  //     assert(this->objs.find(obj.geometry_name) != this->objs.end() &&
  //     "trace_ray hit object cannot found in scene.objs!");

  //     shared_ptr<GeometryContext> gobj = this->objs[obj.geometry_name];
  //     shared_ptr<Geometry> geometry = gobj->geometry;
  //     mat4 model = gobj->transform.getModel();

  //     TriangleSampler tri(geometry->vertices,
  //     geometry->surfaces[obj.triangle_idx], model); vec3 tri_center =
  //     tri.calcCenter(); vec3 tri_norm = tri.calcNorm(); float tri_area =
  //     tri.calcArea(); vec3 wi = tri.hemisphereSampleDir();

  //     if (tri_area == 0.0f)
  //       // cerr << "Warning : zero area triangle exists! return zero vector."
  //       << endl; return {0.0f, 0.0f, 0.0f};

  //     float cosine = glm::dot(ray.dir, tri_norm); //
  //     出射余弦量(三角面元法线与出射光夹角) float BRDF = 1.0f; //
  //     应当根据gobj的材质计算出来，这里先假定为1.0，即光滑镜面反射

  //     HitInfo new_obj = hit_obj({obj.hitPos + 0.01f * tri_norm, wi});
  //     if (new_obj.isHit) {
  //       if (vert_buffer != nullptr)
  //         vert_buffer->push_back(new_obj.hitPos);
  //       L_indir += trace_ray({new_obj.hitPos, -wi}, new_obj, PR,
  //       recursive_depth + 1, vert_buffer) * BRDF * cosine / PR;
  //     } else {
  //       Pixel p = cubemap_sample(this->cubemaps, wi);
  //       L_dir += vec3(p.r, p.g, p.b) / 255.0f;
  //       if (vert_buffer != nullptr)
  //         vert_buffer->push_back(obj.hitPos + 10.0f * wi);
  //     }
  //   }

  //   return L_dir + L_indir;
  // }

  // void compute_radiosity(uint32_t sample_N = 100) {
  //   uniform_real_distribution<> distr(0.0, 1.0);
  //   rdgen.seed(42);

  //   lines["Ray"]->clear();
  //   // 遍历每个场景(非aux)物体
  //   for (auto &[name, cur_obj] : this->objs) {
  //     if (cur_obj->isAux)
  //       continue;
  //     // if (!cur_obj->isSelected) // 暂时只计算被选中目标
  //     //   continue;

  //     mat4 model = cur_obj->transform.getModel();
  //     shared_ptr<Geometry> geometry = cur_obj->geometry;
  //     cur_obj->radiosity.radiant_flux.resize(geometry->surfaces.size());

  //     for (int i = 0; i < geometry->surfaces.size(); i++) {
  //       TriangleSampler tri(geometry->vertices, geometry->surfaces[i],
  //       model); vec3 tri_center = tri.calcCenter(); vec3 tri_norm =
  //       tri.calcNorm();

  //       vec3 L_dir{0.0f, 0.0f, 0.0f};
  //       vec3 L_indir{0.0f, 0.0f, 0.0f};
  //       //
  //       这里只计算irradiance，假定这个三角为黑体，吸收全部的辐射。否则，需要结合三角表面材质纹理来计算对irradiance的吸收率
  //       // 调用前初始化this->ray_obj->vertices可视化光线路径

  //       vec3 wi = tri.hemisphereSampleDir();

  //       shared_ptr<vector<vec3>> vec_buffer = make_shared<vector<vec3>>();
  //       vec_buffer->push_back(tri_center + 0.005f * tri_norm);

  //       HitInfo new_obj = hit_obj({tri_center + 0.005f * tri_norm, wi});
  //       if (new_obj.isHit) {
  //         vec_buffer->push_back(new_obj.hitPos);
  //         // L_indir += trace_ray({new_obj.hitPos, -wi}, new_obj, 0.95, 0,
  //         vec_buffer); L_indir += trace_ray({new_obj.hitPos, -wi}, new_obj,
  //         0.95, vec_buffer);
  //       } else {
  //         vec_buffer->push_back(tri_center + 10.0f * wi);
  //         Pixel p = cubemap_sample(this->cubemaps, wi);
  //         L_dir += vec3(p.r, p.g, p.b) / 255.0f;
  //       }
  //       lines["Ray"]->addPolygon(*vec_buffer);

  //       // 将计算的irradiance的结果存储
  //       vec3 Lo = L_dir + L_indir;
  //       cur_obj->radiosity.radiant_flux[i] = Lo;
  //       // printf("物体\"%s\"第(%d/%llu)面元 radiance: %.4f\n", name.c_str(),
  //       i, geometry->surfaces.size(), Lo);
  //     }
  //   }
  //   lines["Ray"]->update(); // 调试
  // }

  void compute_radiosity(uint32_t sample_N = 1);

  // 迭代版本
  glm::vec3 trace_ray(Ray ray, const HitInfo &obj, float PR,
                      std::vector<glm::vec3> *vert_buffer = nullptr);

  void imgui_docking_render(bool *p_open = nullptr);
  void imgui_docking_config();

  void render();
  void setWindowSize(float width, float height);
  void mainloop();
};

}; // namespace Scene
