#pragma once

// #define NDEBUG

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <stdexcept>
#include <variant>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

#include "constants.h"
#include "proj.h"

#include "Auxiliary.hpp"
#include "Bounding.hpp"
#include "Camera.hpp"
#include "Geometry.hpp"
#include "Light.hpp"
#include "Shader.hpp"
#include "Transform.hpp"

#define MOUSE_VIEW_ROTATE_SENSITIVITY 0.1f
#define MOUSE_VIEW_TRANSLATE_SENSITIVITY 0.06f

#define TEXT(txt) reinterpret_cast<const char *>(u8##txt)

// 仅用于该文件内的调试
static void BK(int n) { printf("break %d\n", n); }

namespace {
using namespace std;
using glm::mat4;
using glm::quat;
using glm::vec3;
using glm::vec4;
namespace fs = filesystem;

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

class RayBufferObject {
private:
  static inline mt19937 random_generator;

public:
  GLuint vao{0};
  GLuint vbo{0};
  vector<vec3> rays;
  vector<uint32_t> v_nums;
  vector<vec3> colors;

  RayBufferObject() {
    glGenVertexArrays(1, &this->vao);
    glGenBuffers(1, &this->vbo);
    // // 随机生成一个颜色
    // uniform_real_distribution<> distr(0.1f, 0.9f);
    // this->color = vec3(distr(RayObject::random_generator), distr(RayObject::random_generator), distr(RayObject::random_generator));
  }
  void push(shared_ptr<vector<vec3>> vert) {
    this->rays.insert(this->rays.end(), vert->begin(), vert->end());
    this->v_nums.push_back(vert->size());
    uniform_real_distribution<> distr(0.1f, 0.9f);
    vec3 color = vec3(distr(RayBufferObject::random_generator), distr(RayBufferObject::random_generator), distr(RayBufferObject::random_generator));
    this->colors.push_back(color);
  }
  void clear() {
    this->rays.clear();
    this->v_nums.clear();
    this->colors.clear();
  }
  void updateVBO() {
    // for (int i = 0; i < this->rays.size(); i++) {
    //   all_vertices.insert(all_vertices.begin(), this->rays[i].vertices.begin(), this->rays[i].vertices.end());
    //   ray_size.push_back(this->rays[i].vertices.size());
    // }
    glBindVertexArray(this->vao);
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    glBufferData(GL_ARRAY_BUFFER, this->rays.size() * sizeof(vec3), this->rays.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
  }
  void draw(Shader *sd) {
    glBindVertexArray(this->vao);
    uint32_t offset = 0u;
    for (int i = 0; i < this->v_nums.size(); i++) {
      sd->set("lineColor", this->colors[i]);
      glDrawArrays(GL_LINE_STRIP, offset, v_nums[i]);
      offset += v_nums[i]; // 累加上第i段的顶点数
    }
  }
  ~RayBufferObject() {
    glDeleteVertexArrays(1, &this->vao);
    glDeleteBuffers(1, &this->vbo);
  }
};

struct RadiosityResult {
  // float radiant_flux{0.0f};
  // 与GeometryRenderObject::geometry::surfaces的元素一一对应
  vector<float> radiant_flux;
};

Pixel cubemap_sample(PngImage *cubmaps, vec3 dir) {
  dir = glm::normalize(dir);
  // cubmaps顺序 px nx py ny pz nz
  uint8_t map_idx{0};

  float theta = acosf(glm::dot(dir, _up));
  float phi = atan2(-dir.z, dir.x);
  float x{0.0f}, y{0.0f};
  if (theta < PI / 4.0f) {
    // 上面, py
    float r = sqrtf(2) * sin(theta);
    x = r * sqrtf(2) * cos(phi);
    y = -r * sqrtf(2) * sin(phi);
    map_idx = 2;
  } else if (theta >= 3.0f * PI / 4.0f) {
    // 下面, ny
    float r = sqrtf(2) * sin(theta);
    x = r * sqrtf(2) * cos(phi);
    y = r * sqrtf(2) * sin(phi);
    map_idx = 3;
  } else {
    if (phi >= 3.0f * PI / 4.0f || phi < -3.0f * PI / 4.0f) {
      // 左面, nx
      y = sqrtf(2) * cos(theta);
      x = sqrtf(2) * sin(phi);
      map_idx = 1;
    } else if (phi >= -PI / 4.0f && phi < PI / 4.0f) {
      // 右面, px
      y = sqrtf(2) * cos(theta);
      x = -sqrtf(2) * sin(phi);
      map_idx = 0;
    } else if (phi >= PI / 4.0f && phi < 3.0f * PI / 4.0f) {
      // 前面, nz
      y = sqrtf(2) * cos(theta);
      x = sqrtf(2) * cos(phi);
      map_idx = 4;
    } else {
      // 后面, pz
      y = sqrtf(2) * cos(theta);
      x = -sqrtf(2) * cos(phi);
      map_idx = 5;
    }
  }
  x = max(x, -1.0f);
  x = min(x, 1.0f);
  y = max(y, -1.0f);
  y = min(y, 1.0f);

  // 将(x,y)映射到图片像素上
  uint32_t width = cubmaps[map_idx].width;
  uint32_t height = cubmaps[map_idx].height;
  uint32_t row_idx = static_cast<uint32_t>(height * (0.5f - y / 2));
  uint32_t col_idx = static_cast<uint32_t>(width * (0.5f + x / 2));

  row_idx = min(row_idx, height - 1);
  col_idx = min(col_idx, width - 1);

  return cubmaps[map_idx].img[col_idx + row_idx * width];
}

class BoundingBoxRenderObject {
public:
  GLuint vao{0};
  GLuint vbo{0};
  GLuint ebo{0};
  int draw_size{0};

  BoundingBoxRenderObject() = default;
  BoundingBoxRenderObject(const vector<vec3> &vertices,
                          const vector<uint32_t> &indices) {
    glGenVertexArrays(1, &this->vao);
    glBindVertexArray(this->vao);

    glGenBuffers(1, &this->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3),
                 vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &this->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t),
                 indices.data(), GL_STATIC_DRAW);

    this->draw_size = indices.size();
  }
  void update(const vector<vec3> &vertices, const vector<uint32_t> &indices) {
    // 仅更新vbo和ebo

    glBindVertexArray(this->vao);
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(vec3),
                    vertices.data());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0,
                    indices.size() * sizeof(uint32_t), indices.data());
  }
  void update(const vector<Vertex> &vertices) {
    // 仅更新vbo
    glBindVertexArray(this->vao);
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(vec3),
                    vertices.data());
  }
  ~BoundingBoxRenderObject() {
    glDeleteBuffers(1, &this->ebo);
    glDeleteBuffers(1, &this->vbo);
    glDeleteVertexArrays(1, &this->vao);
  }
};

class GeometryRenderObject {
private:
  void initVBO() {
    glGenVertexArrays(1, &this->vao);
    glBindVertexArray(this->vao);

    glGenBuffers(1, &this->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 this->geometry->vertices.size() * sizeof(Vertex),
                 this->geometry->vertices.data(), GL_STATIC_DRAW);

    size_t stride = sizeof(Vertex);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                          (void *)0); // 位置
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          (void *)(3 * sizeof(float))); // 法向量
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
                          (void *)(6 * sizeof(float))); // 颜色
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride,
                          (void *)(9 * sizeof(float))); // 纹理坐标
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glGenBuffers(1, &this->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 this->geometry->surfaces.size() * sizeof(Surface),
                 this->geometry->surfaces.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);

    // 同时初始化包围盒
    mat4 model = this->transform.getModel();
    vector<vec3> vertices(this->geometry->vertices.size());
    for (int i = 0; i < this->geometry->vertices.size(); i++) {
      vertices[i] = vec3(
          model *
          vec4(glm::make_vec3(this->geometry->vertices[i].position), 1.0f));
    }
    this->box = make_unique<BoundingBox>(vertices);
    vector<vec3> box_vertices;
    vector<uint32_t> box_indices;
    this->box->genOpenGLRenderInfo(box_vertices, box_indices);
    this->box_obj =
        make_unique<BoundingBoxRenderObject>(box_vertices, box_indices);
  }

public:
  shared_ptr<Geometry> geometry;
  Transform transform;

  GLuint vao{0};
  GLuint vbo{0};
  GLuint ebo{0};
  GLuint texture{0};

  RadiosityResult radiosity; // 由compute_radiosity()更新

  // 最外层包围盒
  unique_ptr<BoundingBox> box;
  unique_ptr<BoundingBoxRenderObject> box_obj;

  // 层次包围盒
  unique_ptr<BvhTree> bvhtree{nullptr};
  vector<shared_ptr<BoundingBoxRenderObject>> bvhbox_objs;

  // 与计算着色器之间数据交换的上下文信息
  // struct {
  //   GLuint position_texture; // geometry中所有顶点位置
  //   GLuint index_texture;    // geometry中所有三角索引
  //   GLuint output_texture;   // 计算着色器输出结果
  // } ctx;

  bool isSelected{false};
  bool visible{true};
  // 暂时只是为了处理Ground作为aux元素却需要做光线碰撞的问题
  // 仅仅为了能将Ground从objs的其他对象中区分出来
  bool isAux{false};

  GeometryRenderObject() = default;
  GeometryRenderObject(const shared_ptr<Geometry> &geometry,
                       Transform transform)
      : geometry(geometry), transform(transform) {
    initVBO();
  }
  GeometryRenderObject(const shared_ptr<Geometry> &geometry)
      : GeometryRenderObject(geometry, Transform()) {}

  void constructBvhTree() {
    if (this->bvhtree != nullptr) {
      // 销毁当前的tree，完全重构
      this->bvhtree.reset();
    }
    this->bvhtree = make_unique<BvhTree>(this->geometry, this->transform);
    this->bvhtree->construct();
    // 生成顶点缓冲
    this->bvhbox_objs.clear(); // 释放所有旧的顶点缓冲（智能指针自动析构）
    this->bvhtree->traverse([this](BvhNode *node) {
      vector<vec3> vertices;
      vector<uint32_t> indices;
      node->box->genOpenGLRenderInfo(vertices, indices);
      this->bvhbox_objs.emplace_back(
          make_shared<BoundingBoxRenderObject>(vertices, indices));
    });
  }

  void destroyBvhTree() {
    if (this->bvhtree != nullptr) {
      this->bvhtree.reset();
      this->bvhbox_objs.clear();
    }
  }

  void updateVBO() {
    glBindVertexArray(this->vao);
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 this->geometry->vertices.size() * sizeof(Vertex),
                 this->geometry->vertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 this->geometry->surfaces.size() * sizeof(Surface),
                 this->geometry->surfaces.data(), GL_DYNAMIC_DRAW);
    glBindVertexArray(0);

    // 重新计算包围盒并更新包围盒的VBO
    mat4 model = this->transform.getModel();
    vector<vec3> vertices(this->geometry->vertices.size());
    for (int i = 0; i < this->geometry->vertices.size(); i++) {
      vertices[i] = vec3(
          model *
          vec4(glm::make_vec3(this->geometry->vertices[i].position), 1.0f));
    }
    // 计算包围盒的6个边界值
    this->box->update(vertices);
    // 更新到OpenGL的顶点缓冲区
    vector<vec3> bound_vertices;
    vector<uint32_t> bound_indices;
    this->box->genOpenGLRenderInfo(bound_vertices, bound_indices);
    this->box_obj->update(bound_vertices, bound_indices);

    // 若启用了层次包围盒，则一同更新
    if (this->bvhtree != nullptr) {
      // 更新所有节点的包围盒
      this->bvhtree->loadGeometry(this->geometry, this->transform);
      this->bvhtree->construct();
      // 更新所有包围盒的顶点缓冲
      this->bvhbox_objs.clear(); // 释放所有旧的顶点缓冲（智能指针自动析构）
      this->bvhtree->traverse([this](BvhNode *node) {
        vector<vec3> vertices;
        vector<uint32_t> indices;
        node->box->genOpenGLRenderInfo(vertices, indices);
        this->bvhbox_objs.emplace_back(
            make_shared<BoundingBoxRenderObject>(vertices, indices));
      });
    }
  }
  ~GeometryRenderObject() {
    glDeleteBuffers(1, &this->vbo);
    glDeleteBuffers(1, &this->ebo);
    glDeleteVertexArrays(1, &this->vao);
  }
};

void errorCallback(int code, const char *msg) {
  cerr << "errors occured! error code: " << code << endl;
  cout << msg << endl;
}

class Scene {
private:
  int width = 1600;
  int height = 1200;

  GLFWwindow *window{nullptr};
  ImGuiIO *io{nullptr};

  GLuint ubo{0};
  const GLuint PVM_binding_point = 0;

  mt19937 random_generator;

  // 用于CPU端存储天空盒图片
  PngImage cubemaps[6];

  struct SkyboxInfo {
    GLuint vao{0};
    GLuint vbo{0};
    GLuint ebo{0};
    GLuint texture{0};
  } skybox_obj;

  struct {
    float left{-10.0f};
    float right{10.0f};
    float bottom{-10.0f};
    float top{10.0f};
    float near{0.0f};
    float far{200.0f};
    GLuint texture{0};
    GLuint fbo{0};
  } depthmap;

  bool isShowGround{true};
  bool isShowAxis{true};
  bool isShowLight{true};

  // imgui的状态变量
  struct ImguiInfo {
    shared_ptr<GeometryRenderObject> cur_obj{nullptr};
    int selected_idx = 0;
    int highlighted_idx = 0;
    bool start_record{true};
    ImVec2 mouse_pos{0, 0};
    vector<string> items;
  } imgui;

public:
  map<string, Shader *> shaders;
  map<string, GLuint> textures;
  map<string, shared_ptr<GeometryRenderObject>> objs;
  map<string, shared_ptr<GeometryRenderObject>> aux;

  shared_ptr<RayBufferObject> ray_buffer{nullptr};

  // 开发阶段暂时忽略渲染逻辑，实现lights中光源模拟辐照度计算
  vector<shared_ptr<Light>>
      lights; // 只用于计算的光源，为了能在场景中看到光源实际位置，需要将其加入到aux中使用sphere进行渲染可视化
  PointLight light{{1.0f, 1.0f, 1.0f},
                   {0.0f, 1.0f, 0.0f},
                   1.0f}; // 用于OpenGL可视化渲染的光源

  Camera camera{vec3(0.0f, 0.0f, 20.0f), vec3{0.0f, 0.0f, 0.0f},
                static_cast<float>(width) / static_cast<float>(height)};
  Scene() {
    if (glfwInit() == GLFW_FALSE) {
      string msg = "failed to init glfw!";
      cerr << msg << endl;
      throw runtime_error(msg);
    }
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    this->window = glfwCreateWindow(width, height, "demo", 0, 0);
    glfwMakeContextCurrent(this->window);
    glfwSetWindowUserPointer(window, this);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      string msg = "failed to init glad!";
      cerr << msg << endl;
      throw runtime_error(msg);
    }
    glEnable(GL_MULTISAMPLE); // 开启MSAA抗锯齿
    glEnable(GL_DEPTH_TEST);  // 开启深度测试
    glEnable(GL_CULL_FACE);   // 开启面剔除
    glFrontFace(GL_CCW);      // 逆时针索引顺序为正面
#ifdef ENBALE_POLYGON_VISUALIZATION
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif

    glfwSetFramebufferSizeCallback(this->window, framebufferResizeCallback);
    glfwSetErrorCallback(errorCallback);

    show_info();

    loadIcon();
    initImgui();

    load_all_shader();
    load_all_texture();

    init_ubo();
    init_scene_obj();
    init_skybox();

    init_depthmap();

    init_ray_buffer();

    // init_ray_visualze(); // 创建用于可视化光线路径的线条对象

    // test_cubemap();
  }
  Scene(const Scene &sc) = delete;
  ~Scene() {
    for (const pair<string, Shader *> &sd : this->shaders)
      delete sd.second;
    for (int i = 0; i < 6; i++)
      stbi_image_free(this->cubemaps[i].img);
    this->objs.clear();
    // this->aux.clear();
  }

  void init_ray_buffer() {
    this->ray_buffer = make_shared<RayBufferObject>();
  }

  void init_ubo() {
    glGenBuffers(1, &this->ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, this->ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, nullptr,
                 GL_STATIC_DRAW);

    mat4 projecion = this->camera.getProject();
    mat4 view = this->camera.getView();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
                    glm::value_ptr(projecion));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4),
                    glm::value_ptr(view));

    glBindBufferBase(GL_UNIFORM_BUFFER, PVM_binding_point, this->ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }

  void init_scene_obj() {
    // 光源
    shared_ptr<Geometry> lightBall = make_shared<Sphere>(0.07f, 36, 72);
    shared_ptr<GeometryRenderObject> obj1 =
        make_shared<GeometryRenderObject>(lightBall);
    obj1->geometry->setColor(1.0f, 1.0f, 1.0f);
    obj1->updateVBO();
    this->addSceneObject("Light", obj1);

    // 坐标轴
    shared_ptr<Geometry> axis = make_shared<CoordinateAxis>();
    shared_ptr<GeometryRenderObject> obj2 =
        make_shared<GeometryRenderObject>(axis);
    this->addSceneObject("Axis", obj2);

    // 游标
    shared_ptr<Geometry> cursor = make_shared<Sphere>(0.05, 36, 72);
    cursor->setColor(1.0f, 1.0f, 0.0f);
    shared_ptr<GeometryRenderObject> cursor_obj =
        make_shared<GeometryRenderObject>(cursor,
                                          Transform{vec3(0.0f, 2.0f, 0.0f)});
    this->addSceneObject("Cursor", cursor_obj);

    // 地面
    shared_ptr<Geometry> ground = make_shared<Ground>(20.0f, 20.0f);
    // 为了让光线不在两个重叠面上抖动进而穿透，将Ground下移一个微小距离
    shared_ptr<GeometryRenderObject> obj3 = make_shared<GeometryRenderObject>(ground, Transform({0.0f, -0.1f, 0.0f}));
    obj3->texture = this->textures["fabric"];
    this->addSceneObject("Ground", obj3, true); // 第三个参数表示是否作为特殊对象加入到this->objs中

    // 左侧面
    shared_ptr<Geometry> side_left = make_shared<Plane>(20.0f, 20.0f);
    side_left->setColor(0.0f, 0.0f, 1.0f);
    shared_ptr<GeometryRenderObject> side_left_obj = make_shared<GeometryRenderObject>(side_left, Transform({-10.0f, 9.9f, 0.0f}, _front, glm::radians(90.0f)));
    this->addSceneObject("Side_left", side_left_obj, true);

    // 后侧面
    shared_ptr<Geometry> side_back = make_shared<Plane>(20.0f, 20.0f);
    side_back->setColor(0.0f, 1.0f, 0.0f);
    shared_ptr<GeometryRenderObject> side_back_obj = make_shared<GeometryRenderObject>(side_back, Transform({0.0f, 9.9f, -10.0f}, _right, glm::radians(90.0f)));
    this->addSceneObject("Side_back", side_back_obj, true);

    // 上侧面
    shared_ptr<Geometry> side_top = make_shared<Plane>(20.0f, 20.0f);
    side_top->setColor(1.0f, 0.0f, 0.0f);
    shared_ptr<GeometryRenderObject> side_top_obj = make_shared<GeometryRenderObject>(side_top, Transform({0.0f, 19.9f, 0.0f}, _right, glm::radians(180.0f)));
    this->addSceneObject("Side_top", side_top_obj, true);
  }

  void init_skybox() {
    // 与前面的GL_TEXTURE_2D纹理目标不同，天空盒使用GL_TEXTURE_CUBE_MAP_XXX作为纹理目标

    // 1. 加载VBO
    vector<vec3> vertices = {
        {-1.0, -1.0, -1.0},
        {-1.0, -1.0, 1.0},
        {-1.0, 1.0, -1.0},
        {-1.0, 1.0, 1.0},
        {1.0, -1.0, -1.0},
        {1.0, -1.0, 1.0},
        {1.0, 1.0, -1.0},
        {1.0, 1.0, 1.0},
    };
    vector<uint32_t> surfaces = {1, 7, 5, 1, 3, 7, 0, 6, 2, 0, 4, 6,
                                 5, 6, 4, 5, 7, 6, 0, 3, 1, 0, 2, 3,
                                 4, 1, 5, 4, 0, 1, 3, 6, 7, 3, 2, 6};
    glGenVertexArrays(1, &this->skybox_obj.vao);
    glBindVertexArray(this->skybox_obj.vao);

    glGenBuffers(1, &this->skybox_obj.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->skybox_obj.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3),
                 vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &this->skybox_obj.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->skybox_obj.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, surfaces.size() * sizeof(GLuint),
                 surfaces.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    // 2. 加载纹理
    glGenTextures(1, &this->skybox_obj.texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->skybox_obj.texture);

    vector<string> skybox_texture_names = {"px", "nx", "py", "ny", "pz", "nz"};
    for (int i = 0; i < skybox_texture_names.size(); i++) {
      string fname =
          "assets/textures/skybox/" + skybox_texture_names[i] + ".png";
      int width, height, channel;
      this->cubemaps[i].img = reinterpret_cast<Pixel *>(
          stbi_load(fname.c_str(), &this->cubemaps[i].width,
                    &this->cubemaps[i].height, &this->cubemaps[i].channel, 0));
      assert(this->cubemaps[i].channel && "make sure image channel num is 4!");
      if (this->cubemaps[i].img == 0) {
        cerr << "load skybox texture failed: \"" << fname << "\"" << endl;
        continue;
      }

      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA,
                   this->cubemaps[i].width, this->cubemaps[i].height, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, this->cubemaps[i].img);

      // stbi_image_free(this->cubemaps[i]);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  }

  void test_cubemap() {
    cout << "开始测试" << endl;
    vector<Pixel> buf;

    uint32_t phi_num = 3600;
    uint32_t theta_num = 1800;
    for (uint32_t v = 0; v < theta_num; v++) {
      for (uint32_t u = 0; u < phi_num; u++) {
        float phi = static_cast<float>(u) / phi_num * 2.0f * PI;
        float theta = static_cast<float>(v) / theta_num * PI;
        float x = sin(theta) * cos(phi);
        float y = cos(theta);
        float z = -sin(theta) * sin(phi);
        Pixel p = cubemap_sample(this->cubemaps, {x, y, z});
        buf.emplace_back(p);
      }
    }
    cout << "测试结束，写出图像" << endl;

    stbi_write_png("output.png", phi_num, theta_num, 4, buf.data(), 0);
  }

  void init_depthmap() {
    glGenTextures(1, &this->depthmap.texture);
    glBindTexture(GL_TEXTURE_2D, this->depthmap.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenFramebuffers(1, &this->depthmap.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, this->depthmap.fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           this->depthmap.texture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  // shared_ptr<RayObject> generate_ray_object() {
  //   shared_ptr<RayObject> obj = make_shared<RayObject>();
  //   this->ray_objs.emplace_back(obj);
  //   return obj;
  // }

  // void update_ray_obj() {
  //   // 根据ray_obj.vertices更新vbo
  //   glBindVertexArray(ray_obj.vao);
  //   glBindBuffer(GL_ARRAY_BUFFER, ray_obj.vbo);
  //   glBufferData(GL_ARRAY_BUFFER, ray_obj.vertices.size() * sizeof(vec3), ray_obj.vertices.data(), GL_DYNAMIC_DRAW);
  // }

  void showAxis() { this->aux["Axis"]->visible = true; }
  void hideAxis() { this->aux["Axis"]->visible = false; }
  void showGround() { this->aux["Ground"]->visible = true; }
  void hideGround() { this->aux["Ground"]->visible = false; }

  void show_info() {
    const GLubyte *vendor = glGetString(GL_VENDOR);
    const GLubyte *renderer = glGetString(GL_RENDERER);
    cout << "Vendor: " << vendor << endl;
    cout << "Graphics Device: " << renderer << endl;
  }

  void setSeed(uint32_t sd) {
    this->random_generator.seed(sd);
  }

  Ray cast_ray_from_mouse() {
    // 捕捉鼠标在窗口中的位置，并将其转换为世界坐标系下的一条射线

    double xpos, ypos;
    glfwGetCursorPos(this->window, &xpos, &ypos);
    xpos = 2.0f * xpos / this->width - 1.0f;
    ypos = 1.0f - 2.0f * ypos / this->height;
    vec3 pos = vec3(xpos, ypos, 1.0f);
    mat4 clip2world =
        glm::inverse(this->camera.getProject() * this->camera.getView());
    vec4 pos_world = clip2world * vec4(pos, 1.0f);
    vec3 dir = glm::normalize(vec3(pos_world / pos_world.w) -
                              this->camera.getPosition());
    return {this->camera.getPosition(), dir};
  }

  void imgui_interact(){
          // 鼠标交互动作
      // 记录下“相机环绕”时的相机球坐标
      if (!io->WantCaptureMouse) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
          if (ImGui::IsKeyDown(ImGuiKey_ModShift)) {
            this->camera.record();
          } else {
            // 鼠标左击选中场景物体，遍历所有物体的包围盒求交
            this->imgui.mouse_pos = io->MousePos;
          }
        }

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
          if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
            // 以世界坐标锚点为中心做旋转
            if (this->imgui.start_record) {
              // 相机记录当前环绕位置和锚点距离
              this->camera.record();
              this->imgui.start_record = false;
            }
            this->camera.surround(
                MOUSE_VIEW_ROTATE_SENSITIVITY * 0.04 * io->MouseDelta.x,
                MOUSE_VIEW_ROTATE_SENSITIVITY * 0.04 * io->MouseDelta.y);
          } else {
            // 以相机为中心旋转
            this->camera.rotate(
                {MOUSE_VIEW_ROTATE_SENSITIVITY * io->MouseDelta.x,
                 MOUSE_VIEW_ROTATE_SENSITIVITY * io->MouseDelta.y, 0.0});
          }
        }
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right, 0.0f)) {
          // 沿着相机姿态坐标系的上下左右进行平移
          this->camera.move_relative(
              {-MOUSE_VIEW_TRANSLATE_SENSITIVITY * io->MouseDelta.x,
               MOUSE_VIEW_TRANSLATE_SENSITIVITY * io->MouseDelta.y, 0.0f});
        }

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
          // 相机准备好下一次环绕的启动
          this->imgui.start_record = true;

          // 鼠标按下不挪动松开，则执行场景物体拾取
          ImVec2 mouse_pos = io->MousePos;
          float move_offset =
              std::sqrt(std::powf(mouse_pos.x - this->imgui.mouse_pos.x, 2.0f) +
                        std::powf(mouse_pos.y - this->imgui.mouse_pos.y, 2.0f));
          // cout << "鼠标拖拽距离：" << move_offset << endl;
          if (move_offset < 8.0f) {
            // 按下与弹起位置一致，中间没有鼠标大幅拖拽，则进行拾取操作

            Ray ray = cast_ray_from_mouse();

            struct HitObj {
              uint32_t id{0};   // 该物体在Scene::imgui::items中的索引位置
              uint32_t type{0}; // 0为普通包围盒，1为层次包围盒
              vec3 hitPos{0.0f, 0.0f, 0.0f};
            };

            float min_distance = FLT_MAX;
            HitObj target_obj;
            bool isisHit = false;
            for (auto &[name, oobj] : this->objs) {
              if (oobj->isAux) {
                // 排除该对象
                continue;
              }
              bool isHit = false;
              float distance = 0.0f;
              HitObj obj;
              if (oobj->bvhtree != nullptr) {
                // 层次包围盒求交
                uint32_t triangle_idx;
                if (oobj->bvhtree->intersect(ray.origin, ray.dir, obj.hitPos, distance, triangle_idx)) {
                  isHit = true;
                  obj.type = 1;
                }
              } else {
                // 普通外层包围盒求交
                if (oobj->box->hit(this->camera.getPosition(), ray.dir)) {
                  isHit = true;
                  obj.type = 0;
                  distance = glm::distance(this->camera.getPosition(), oobj->box->getBoxCenter());
                }
              }
              if (isHit && distance < min_distance) {
                obj.id = std::distance(this->imgui.items.begin(), std::find(this->imgui.items.begin(), this->imgui.items.end(), name));
                target_obj = obj;
                min_distance = distance;
                isisHit = true;
              }
            }
            if (isisHit) {
              this->imgui.selected_idx = target_obj.id;
              this->camera.setAnchor(
                  this->objs[this->imgui.items[this->imgui.selected_idx]]
                      ->box->getBoxCenter());
              if (target_obj.type == 1) {
                this->aux["Cursor"]->transform.setPosition(target_obj.hitPos);
                printf("选中点位置：(%.2f, %.2f, %.2f)\n", target_obj.hitPos.x, target_obj.hitPos.y, target_obj.hitPos.z);
              }
            }
          }
        }
      }

      if (ImGui::IsKeyDown(ImGuiKey_W)) {
        this->camera.move_relative({0.0f, 0.0f, -0.1f});
      }
      if (ImGui::IsKeyDown(ImGuiKey_S)) {
        this->camera.move_relative({0.0f, 0.0f, 0.1f});
      }
      if (ImGui::IsKeyDown(ImGuiKey_A)) {
        this->camera.move_relative({-0.1f, 0.0f, 0.0f});
      }
      if (ImGui::IsKeyDown(ImGuiKey_D)) {
        this->camera.move_relative({0.1f, 0.0f, 0.0f});
      }
  }

  void imgui_menu() {

    ImGui::ShowDemoWindow();
    imgui_docking_render();

    ImGui::Begin(TEXT("场景"));
    if (ImGui::TreeNodeEx(TEXT("相机"), ImGuiTreeNodeFlags_Selected |
                                            ImGuiTreeNodeFlags_DefaultOpen)) {
      bool is_theta_changed = ImGui::SliderFloat(
          TEXT("天顶角"), &this->camera.theta_s, 0.0f, 180.0f, "%.1f");
      bool is_phi_changed = ImGui::SliderFloat(
          TEXT("方向角"), &this->camera.phi_s, -180.0f, 180.0f, "%.1f");
      if (is_theta_changed || is_phi_changed)
        this->camera.updateToward();

      if (ImGui::InputFloat3(TEXT("位置"),
                             glm::value_ptr(this->camera.position_s), "%.2f",
                             0)) {
        this->camera.updatePositionFromShadow();
      }

      ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx(TEXT("光源"), ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::SliderFloat3(TEXT("位置"), glm::value_ptr(this->light.position),
                          -20.0f, 20.f);
      // 暂时这么写
      this->aux["Light"]->transform.setPosition(light.position);
      ImGui::TreePop();
    }

    if (this->objs.empty()) {
      ImGui::End();
      return;
    }

    if (ImGui::TreeNodeEx(TEXT("几何管理"), ImGuiTreeNodeFlags_DefaultOpen)) {
      bool is_hightlight = true;
      this->imgui.items.clear();
      for (auto &obj : this->objs) {
        if (obj.second->isAux) {
          // 排除该对象
          continue;
        }
        this->imgui.items.push_back(obj.first);
      }
      if (ImGui::BeginListBox(TEXT("物体"))) {
        for (int i = 0; i < this->imgui.items.size(); i++) {
          const bool is_selected = (imgui.selected_idx == i);
          if (ImGui::Selectable(this->imgui.items[i].c_str(), is_selected))
            imgui.selected_idx = i;
          if (is_hightlight && ImGui::IsItemHovered())
            imgui.highlighted_idx = i;
          if (is_selected) {
            imgui.cur_obj = this->objs[this->imgui.items[i]];
            ImGui::SetItemDefaultFocus();
          }
        }
        // 回传选中状态
        for (int i = 0; i < this->imgui.items.size(); i++) {
          this->objs[this->imgui.items[i]]->isSelected = false;
          if (imgui.selected_idx == i)
            this->objs[this->imgui.items[i]]->isSelected = true;
        }
        ImGui::EndListBox();
      }

      // 显示参数
      if (imgui.cur_obj != nullptr && !imgui.cur_obj->geometry->parameters.empty()) {

        ImGui::Text(TEXT("形体参数"));
        struct visitor {
          string pname;
          Scene *context{nullptr};
          visitor(string name, Scene *context) : pname(name), context(context) {}
          void operator()(float &arg) {
            if (ImGui::SliderFloat(this->pname.c_str(), &arg, 0.0f, 10.0f)) {
              context->imgui.cur_obj->geometry->update();
              context->imgui.cur_obj->updateVBO();
              // context->compute_radiosity();
            }
          }
          void operator()(uint32_t &arg) {
            if (ImGui::SliderInt(this->pname.c_str(),
                                 reinterpret_cast<int *>(&arg), 2, 50)) {
              context->imgui.cur_obj->geometry->update();
              context->imgui.cur_obj->updateVBO();
              // context->compute_radiosity();
            }
          }
          void operator()(bool &arg) {}
          void operator()(int &arg) {}
          void operator()(vec3 &arg) {}
        };
        for (auto &[name, arg_val] : imgui.cur_obj->geometry->parameters)
          std::visit(visitor(name, this), arg_val);

        ImGui::TreePop();
      }

      if(ImGui::Button(TEXT("重新投射光线"))){
        this->compute_radiosity();
      }

      ImGui::End();
    }
  }

  void loadIcon() {
    GLFWimage image;
    image.pixels =
        stbi_load("favicon.png", &image.width, &image.height, nullptr, 4);
    glfwSetWindowIcon(this->window, 1, &image);
    stbi_image_free(image.pixels);
  }
  void initImgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    this->io = &ImGui::GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
    io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    io->Fonts->AddFontFromFileTTF(
        "C:/Windows/Fonts/simhei.ttf", 24.0f, nullptr,
        io->Fonts->GetGlyphRangesChineseSimplifiedCommon());

    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    ImGuiStyle &style = ImGui::GetStyle();
    if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      style.WindowRounding = 0.0f;
      style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForOpenGL(this->window, true);
    ImGui_ImplOpenGL3_Init();
  }
  void load_all_shader() {
    shaders["default"] = new Shader("default.vert", "default.frag");
    shaders["normal"] = new Shader("normal.vert", "normal.geom", "normal.frag");
    shaders["skybox"] = new Shader("skybox.vert", "skybox.frag");
    shaders["lightDepth"] = new Shader("lightDepth.vert", "lightDepth.frag");
    // shaders["compute"] = new Shader("compute.comp");
    shaders["line"] = new Shader("line.vert", "line.frag");
  }

  void load_all_texture() {
    fs::path texture_dir = "assets/textures";
    for (auto &file : fs::directory_iterator(texture_dir)) {
      if (file.is_regular_file()) {
        // 加载纹理
        string fname = file.path().string();
        // 读取图片文件
        int img_width, img_height, img_chn;
        void *img_data =
            stbi_load(fname.c_str(), &img_width, &img_height, &img_chn, 0);
        if (img_data == nullptr) {
          string msg = "load texture image \"" + fname + "\" failed!";
          cerr << msg << endl;
          throw runtime_error(msg);
        }

        // 创建纹理缓冲
        GLuint new_texture;
        glGenTextures(1, &new_texture);
        glBindTexture(GL_TEXTURE_2D, new_texture);

        glTextureParameteri(new_texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(new_texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTextureParameteri(new_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(new_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_width, img_height, 0, GL_RGB,
                     GL_UNSIGNED_BYTE, img_data);
        glGenerateTextureMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        this->textures[file.path().filename().stem().string()] = new_texture;
        stbi_image_free(img_data);

        // cout << "load texture: \"" << file.path().filename().string() << "\""
        //      << endl;
      }
    }
  }

  void addSceneObject(const string &name, const shared_ptr<GeometryRenderObject> &obj, bool flag = false) {
    // 第三个参数为true时，将obj加入到this->objs中，并将其设置isAux为true，其他对象默认isAux为false
    if (!flag) {
      if (this->aux.find(name) != this->aux.end()) {
        cout << "scene cannot add object with an existed name \"" << name << "\"" << endl;
        return;
      }
      this->aux[name] = obj;
    } else {
      if (this->objs.find(name) != this->objs.end()) {
        cout << "scene cannot add object with an existed name \"" << name << "\"" << endl;
        return;
      }
      obj->isAux = true;
      this->objs[name] = obj;
    }
  }

  void add(const string &name, const shared_ptr<Geometry> &geometry,
           Transform transform) {
    if (this->objs.find(name) != this->objs.end()) {
      cout << "scene cannot add object with an existed name \"" << name << "\"" << endl;
      return;
    }
    this->objs[name] = make_shared<GeometryRenderObject>(geometry, transform);

    // 加入对场景元素的默认选择，总是选中最后加入的物体，并取消选择其他物体
    for (auto &[name, obj] : this->objs) {
      if (obj->isAux)
        continue;
      obj->isSelected = false;
    }
    this->objs[name]->isSelected = true;
    // 初始化Scene::imgui::selected_idx
    this->imgui.items.clear();
    for (auto &[name, obj] : this->objs) {
      if (obj->isAux)
        continue;
      this->imgui.items.emplace_back(name); // 初始化 this->imgui.items
    }
    for (int i = 0; i < this->imgui.items.size(); i++) {
      if (this->objs[this->imgui.items[i]]->isSelected)
        this->imgui.selected_idx = i; // 初始化 this->imgui.selected_idx
    }
  }

  void add(const string &name, const shared_ptr<Geometry> &geometry) {
    this->add(name, geometry, Transform());
  }

  void add(const string &name, const shared_ptr<Geometry> &geometry,
           vec3 position) {
    this->add(name, geometry, Transform(position));
  }

  void addLight(const string &name, const shared_ptr<Light> &light) {
    if (this->objs.find(name) != this->objs.end()) {
      cerr << "scene cannot add object with an existed name!" << endl;
      return;
    }
    this->lights.emplace_back(light);
    // 建立渲染对象，加入到aux中，不同子类建立的可视化对象不一样
    shared_ptr<GeometryRenderObject> render_obj = nullptr;
    switch (light->type) {
    case Light::LightType::POINT: {
      render_obj =
          make_shared<GeometryRenderObject>(make_shared<Sphere>(0.03, 36, 72));
      break;
    }
    case Light::LightType::PARALLEL: {
      // 未实现
      break;
    }
    case Light::LightType::SPOT: {
      // 未实现
      break;
    }
    }

    if (render_obj != nullptr) {
      this->aux[name] = render_obj;
    } else
      cerr << "该光源类型的渲染对象未在Scene::addLight()中实现！" << endl;

    // 即使无法可视化，也会将对象加入
    this->lights.emplace_back(light);
  }

  void compute_radiosity_1() {
    // 遍历场景中所有objs并计算每个物体对应接收到的辐射通量

    // 实际光源并不只有一个，而且并非只有平行光(后续考虑)
    shared_ptr<ParallelLight> light =
        dynamic_pointer_cast<ParallelLight>(this->lights[0]);

    for (auto &[name, cur_obj] : this->objs) {
      shared_ptr<Geometry> geometry = cur_obj->geometry;
      cur_obj->radiosity.radiant_flux.resize(geometry->surfaces.size());
      for (int i = 0; i < geometry->surfaces.size(); i++) {
        Surface triangle = geometry->surfaces[i];
        vector<vec3> pt(3);
        vec3 norm{0.0f, 0.0f, 0.0f};
        float area{0.0f};
        for (int j = 0; j < 3; j++)
          pt[j] = glm::make_vec3(geometry->vertices[triangle.tidx[j]].position);
        vec3 tri_cross = glm::cross(pt[1] - pt[0], pt[2] - pt[0]);
        norm = glm::normalize(tri_cross);
        area = glm::length(tri_cross) / 2.0f;
        // 计算辐射通量(每个三角面)
        cur_obj->radiosity.radiant_flux[i] =
            light->irradiance * area *
            glm::max(0.0f, glm::dot(norm, -light->direction));
      }
    }
  }

  struct HitGeometryObj {
    string obj_name; // 击中物体名称
    uint32_t tri_id; // 击中物体所在三角面元的索引
    vec3 hit_pos;    // 世界坐标系下的击中位置
  };

  bool hit_obj(Ray ray, HitGeometryObj &hit_obj) {
    // 场景物体全局求交
    // 遍历this->objs所有元素，找到距离最近的相交物体，从引用参数回传 HitGeometryObj
    // 返回击中物体名称、三角面元索引、击中位置
    bool isHit = false;

    float min_distance = FLT_MAX;
    HitGeometryObj target_obj;
    for (auto &[name, obj] : this->objs) {
      // 假定都生成了bvh树
      assert(obj->bvhtree != nullptr && "element in scene.objs must construct bvh-tree!");
      float distance;
      HitGeometryObj hobj;
      if (obj->bvhtree->intersect(ray.origin, ray.dir, hobj.hit_pos, distance, hobj.tri_id)) {
        hobj.obj_name = name;
        if (distance < min_distance) {
          min_distance = distance;
          target_obj = hobj;
        }
        isHit = true;
      }
    }
    if (isHit) {
      hit_obj = target_obj;
      // cout << "光线击中物体 \"" << hit_obj.obj_name << "\"  tri_id=" << hit_obj.tri_id << " hit_pos=(" << hit_obj.hit_pos.x << ", " << hit_obj.hit_pos.y << ", " << hit_obj.hit_pos.z << ")" << endl;
    }

    return isHit;
  }

  float trace_ray(Ray ray, HitGeometryObj obj, float PR, uint32_t recursive_depth = 0, shared_ptr<vector<vec3>> vert_buffer = nullptr) {
    // recursive_depth用以记录该函数的递归深度，同时在可视化光线路径时用于记录顶点数量
    // recursive_depth=0表示对该函数的“根调用”，其他为递归调用

    // ray.origin 为当前物体表面，ray.dir为从物体表面的出射光方向
    // 这应当是三角面元接收到的其他物体的光，ray.origin为从三角面元出发沿着-ray.dir击中的物体表面
    // 所以应当在调用该函数之前，在处理三角面元时，就要进行一个wi的采样，wi就是-ray.dir，击中的物体的位置就是ray.origin
    // 因此，该函数还需要传入“击中的三角面元所属的几何体”与“击中的三角面元的索引”，这里描述为HitGeometryObj结构体

    assert(PR < 1.0f && PR > 0.0f);
    // 计算光源贡献
    float L_dir = 0.0f;
    // 。。。       // 需要做与光源之间的遮挡检测

    // 由于使用了cubemap作为光源，因此对于L_inder中光线没有碰撞的到的物体，仅需在那个方向返回一个对cubemap颜色值的采样
    // 将这个颜色采样值换算为radiance即可，这里假设cubemap在某一点上的radiance为该像素点的灰度，即将rgb取平均

    // 计算间接光贡献
    float L_indir = 0.0f;
    uniform_real_distribution<> distr(0.0, 1.0);

    float PR_D = distr(this->random_generator);
    if (PR > PR_D) { // 俄罗斯赌轮盘
      assert(this->objs.find(obj.obj_name) != this->objs.end() && "trace_ray hit object cannot found in scene.objs!");
      shared_ptr<GeometryRenderObject> gobj = this->objs[obj.obj_name];
      shared_ptr<Geometry> geom = gobj->geometry;
      mat4 model = gobj->transform.getModel();
      Surface triangle = geom->surfaces[obj.tri_id];
      vec3 tri_center{0.0f, 0.0f, 0.0f};
      vec3 pt[3];
      for (int j = 0; j < 3; j++) { // 取出击中三角面元的三个顶点，同时变换到世界坐标系
        pt[j] = vec3(model * vec4(glm::make_vec3(geom->vertices[triangle.tidx[j]].position), 1.0f));
        tri_center += pt[j] / 3.0f; // 计算三角形质心
      }
      vec3 tri_cross = glm::cross(pt[1] - pt[0], pt[2] - pt[0]);
      vec3 norm = glm::normalize(tri_cross);
      float area = glm::length(tri_cross);
      assert(area > 0.0f && "triangle area is equal to zero!");

      // 以norm为上，pt[1]-pt[0]与pt[2]-pt[0]的一组正交基，构成的局部半球上采样出一点wi
      // 计算pt[1]-pt[0]与pt[2]-pt[0]的一组正交基
      vec3 &tri_up = norm;
      vec3 tri_right = glm::normalize(glm::cross(pt[1] - pt[0], tri_up));
      vec3 tri_front = glm::normalize(glm::cross(tri_up, tri_right));
      //  在半球上均匀分布随机做一次采样
      float theta = acosf(1.0f - distr(this->random_generator));
      float phi = 2.0f * PI * distr(this->random_generator);
      vec3 wi = glm::normalize(sinf(theta) * cosf(phi) * tri_front + sinf(theta) * sinf(phi) * tri_right + cosf(theta) * tri_up);

      float cosine = glm::dot(ray.dir, norm);
      float BRDF = 1.0f; // 应当根据gobj的材质计算出来，这里先假定为1.0，即光滑镜面反射
      // 以obj.hit_pos为起点，wi为方向，发射光线，与scene.objs中所有物体，包括scene.aux["Ground"]进行求交测试
      // 将求交结果整理为新的HitGeometryObj，即"击中的GeometryRenderObject的name" "对应三角面元的索引" "世界坐标系下的击中位置"
      HitGeometryObj new_obj;
      if (hit_obj({obj.hit_pos + 0.01f * norm, wi}, new_obj)) {
        if (vert_buffer != nullptr)
          vert_buffer->push_back(new_obj.hit_pos + 0.01f * norm);
        L_indir += trace_ray({new_obj.hit_pos + 0.01f * norm, -wi}, new_obj, PR, recursive_depth + 1, vert_buffer) * BRDF * cosine / PR;
      } else {
        Pixel p = cubemap_sample(this->cubemaps, wi);
        L_dir += (p.r + p.g + p.b) / 3.0f / 255.0f;
        if (vert_buffer != nullptr)
          vert_buffer->push_back(obj.hit_pos + 10.0f * wi);
        // cout << "" << recursive_depth << endl;
        // printf("cubemap采样辐射率: %.4f  光线递归深度: %d\n", L_dir, recursive_depth);
      }
    }

    return L_dir + L_indir;
  }

  void compute_radiosity() {
    uniform_real_distribution<> distr(0.0, 1.0);

    this->ray_buffer->clear();
    // 遍历每个场景(非aux)物体
    for (auto &[name, cur_obj] : this->objs) {
      if (cur_obj->isAux)
        continue;
      // if (!cur_obj->isSelected) // 暂时只计算被选中目标
      //   continue;

      mat4 model = cur_obj->transform.getModel();
      shared_ptr<Geometry> geometry = cur_obj->geometry;
      cur_obj->radiosity.radiant_flux.resize(geometry->surfaces.size());

      for (int i = 0; i < geometry->surfaces.size(); i++) {
        Surface triangle = geometry->surfaces[i];
        vec3 tri_center{0.0f, 0.0f, 0.0f};
        vector<vec3> pt(3);
        for (int j = 0; j < 3; j++) {
          pt[j] = vec3(model * vec4(glm::make_vec3(geometry->vertices[triangle.tidx[j]].position), 1.0f));
          tri_center += pt[j] / 3.0f;
        }
        vec3 tri_cross = glm::cross(pt[1] - pt[0], pt[2] - pt[0]);
        vec3 norm = glm::normalize(tri_cross);
        float area = glm::length(tri_cross);
        // 生成一个随机半球采样方向，与场景物体进行全局求交
        vec3 &tri_up = norm;
        vec3 tri_right = glm::normalize(glm::cross(pt[1] - pt[0], tri_up));
        vec3 tri_front = glm::normalize(glm::cross(tri_up, tri_right));
        float theta = acosf(1.0f - distr(this->random_generator));
        float phi = 2.0f * PI * distr(this->random_generator);
        vec3 wi = glm::normalize(sinf(theta) * cosf(phi) * tri_front + sinf(theta) * sinf(phi) * tri_right + cosf(theta) * tri_up);

        HitGeometryObj new_obj;
        float L_dir = 0.0f;
        float L_indir = 0.0f;
        // 这里只计算irradiance，假定这个三角为黑体，吸收全部的辐射。否则，需要结合三角表面材质纹理来计算对irradiance的吸收率
        // 调用前初始化this->ray_obj->vertices可视化光线路径

        // shared_ptr<RayObject> obj = generate_ray_object();
        shared_ptr<vector<vec3>> vec_buffer = make_shared<vector<vec3>>();
        vec_buffer->push_back(tri_center + 0.01f * norm);
        if (hit_obj({tri_center + 0.01f * norm, wi}, new_obj)) {
          vec_buffer->push_back(new_obj.hit_pos);
          L_indir += trace_ray({new_obj.hit_pos, -wi}, new_obj, 0.95, 0, vec_buffer);
          // L_indir += trace_ray({new_obj.hit_pos, -wi}, new_obj, 0.95, 0, nullptr);
        } else {
          vec_buffer->push_back(tri_center + 10.0f * wi); // 没击中就朝着采样方向延伸10单位距离
          // 检索cubemap返回颜色值作为radiance
          Pixel p = cubemap_sample(this->cubemaps, wi);
          L_dir += (p.r + p.g + p.b) / 3.0f / 255.0f;
        }
        // 将vec_buffer加入到RayBufferObject中进行渲染

        this->ray_buffer->push(vec_buffer);

        // 将计算的irradiance的结果存储
        float Lo = L_dir + L_indir;
        cur_obj->radiosity.radiant_flux[i] = Lo;
        // printf("物体\"%s\"第(%d/%llu)面元 radiance: %.4f\n", name.c_str(), i, geometry->surfaces.size(), Lo);
      }
    }
    this->ray_buffer->updateVBO();
  }

  void imgui_docking_render(bool *p_open = nullptr) {
    // Variables to configure the Dockspace example.
    static bool opt_fullscreen = true; // Is the Dockspace full-screen?
    static bool opt_padding = false;   // Is there padding (a blank space) between
                                       // the window edge and the Dockspace?
    static ImGuiDockNodeFlags dockspace_flags =
        ImGuiDockNodeFlags_None; // Config flags for the Dockspace
    dockspace_flags |= ImGuiDockNodeFlags_PassthruCentralNode;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;

    if (opt_fullscreen) {
      const ImGuiViewport *viewport = ImGui::GetMainViewport();

      ImGui::SetNextWindowPos(viewport->WorkPos);
      ImGui::SetNextWindowSize(viewport->WorkSize);
      ImGui::SetNextWindowViewport(viewport->ID);

      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

      window_flags |= ImGuiWindowFlags_NoTitleBar |
                      ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                      ImGuiWindowFlags_NoMove;
      window_flags |=
          ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    } else {
      dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
      window_flags |= ImGuiWindowFlags_NoBackground;

    if (!opt_padding)
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpace Demo", p_open, window_flags);

    if (!opt_padding)
      ImGui::PopStyleVar();

    if (opt_fullscreen)
      ImGui::PopStyleVar(2);

    ImGuiIO &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
      ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
      ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    } else {
      cout << "warning: docking is disabled!" << endl;
    }

    ImGui::End();
  }

  void render() {

    mat4 view = this->camera.getView();
    mat4 projection = this->camera.getProject();

    // 更新P,V矩阵
    glBindBuffer(GL_UNIFORM_BUFFER, this->ubo);
    if (this->camera.isProjectionChanged()) {
      glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
                      glm::value_ptr(projection));
      this->camera.apply_projection_done();
    }
    if (this->camera.isViewChanged()) {
      glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4),
                      glm::value_ptr(view));
      this->camera.apply_view_done();
    }

    Shader *cur_shader{nullptr};

    glViewport(0, 0, width, height);

    // 场景辅助元素（天空盒）
    glDepthMask(GL_FALSE);
    cur_shader = this->shaders["skybox"];
    cur_shader->use();
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->skybox_obj.texture);
    glBindVertexArray(this->skybox_obj.vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    glDepthMask(GL_TRUE);

    // 1. 常规物体渲染
    cur_shader = this->shaders["default"];
    cur_shader->use();
    cur_shader->set("lightPos", this->light.position);
    cur_shader->set("lightColor", this->light.color);
    cur_shader->set("eyePos", this->camera.position_s);
    cur_shader->set("ambientStrength", 0.2f);
    cur_shader->set("diffuseStrength", 1.0f);
    cur_shader->set("specularStrength", 1.0f);
    for (auto &[name, cur_obj] : this->objs) {
      mat4 model = cur_obj->transform.getModel();
      cur_shader->set("model", model);
      // 启用材质，设置材质属性
      glBindTexture(GL_TEXTURE_2D, cur_obj->texture);
      cur_shader->set("useTexture", (cur_obj->texture != 0) ? true : false);
      cur_shader->set("useLight", true);
      // 绘制
      glBindVertexArray(cur_obj->vao);
      glDrawElements(GL_TRIANGLES, cur_obj->geometry->surfaces.size() * 3,
                     GL_UNSIGNED_INT, (void *)0);
    }
    // 2. 场景辅助元素
    for (auto &[name, obj] : this->aux) {
      if (!obj->visible)
        continue;
      cur_shader->set("model", obj->transform.getModel());
      if (obj->texture != 0) {
        cur_shader->set("useTexture", true);
        cur_shader->set("useLight", true);
      } else {
        cur_shader->set("useTexture", false);
        cur_shader->set("useLight", false);
      }
      glBindTexture(GL_TEXTURE_2D, obj->texture);
      glBindVertexArray(obj->vao);
      glDrawElements(GL_TRIANGLES, obj->geometry->surfaces.size() * 3,
                     GL_UNSIGNED_INT, nullptr);
      glBindVertexArray(0);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
    // 3. 从有向光视角生成深度缓存
    cur_shader = this->shaders["lightDepth"];
    cur_shader->use();
    cur_shader->set("projection",
                    glm::ortho(this->depthmap.left, this->depthmap.right,
                               this->depthmap.bottom, this->depthmap.top,
                               this->depthmap.near, this->depthmap.far));
    cur_shader->set("view",
                    glm::lookAt(this->light.position, {0.0f, 0.0f, 0.0f}, _up));
    glBindFramebuffer(GL_FRAMEBUFFER, this->depthmap.fbo);
    glClear(GL_DEPTH_BUFFER_BIT);
    for (auto &[name, obj] : this->objs) {
      cur_shader->set("model", obj->transform.getModel());
      glBindVertexArray(obj->vao);
      glDrawElements(GL_TRIANGLES, obj->geometry->surfaces.size() * 3,
                     GL_UNSIGNED_INT, nullptr);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

#ifdef ENABLE_NORMAL_VISUALIZATION
    // 4. 渲染法向量
    for (auto &[name, cur_obj] : this->objs) {
      if (cur_obj->isSelected) {
        cur_shader = this->shaders["normal"];
        cur_shader->use();
        cur_shader->set("model", cur_obj->transform.getModel());

        glBindVertexArray(cur_obj->vao);
        glDrawArrays(GL_POINTS, 0, cur_obj->geometry->vertices.size());
      }
    }
#endif

#ifdef ENABLE_BOUNDINGBOX_VISUALIZATION
    // 5. 渲染包围盒边框
    cur_shader = this->shaders["line"];
    cur_shader->use();
    cur_shader->set("lineColor", glm::vec3(0.0f, 1.0f, 1.0f));
    for (auto &[name, cur_obj] : this->objs) {
      if (cur_obj->isSelected) {
        if (cur_obj->bvhtree != nullptr) {
          // 渲染层次包围盒
          for (const shared_ptr<BoundingBoxRenderObject> &bb_obj :
               cur_obj->bvhbox_objs) {
            glBindVertexArray(bb_obj->vao);
            glDrawElements(GL_LINES, bb_obj->draw_size, GL_UNSIGNED_INT,
                           nullptr);
          }
        } else {
          // 渲染整体包围盒
          glBindVertexArray(cur_obj->box_obj->vao);
          glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, nullptr);
        }
      }
    }
#endif
#ifdef ENBALE_RAY_VISUALIZATION
    // 6. 可视化光线追踪
    cur_shader = this->shaders["line"];
    cur_shader->use();
    this->ray_buffer->draw(cur_shader);
#endif
  }
  void setWindowSize(float width, float height) {
    this->width = width;
    this->height = height;
  }

  void mainloop() {
    glfwShowWindow(this->window);
    while (!glfwWindowShouldClose(this->window)) {
      glfwPollEvents();
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      render();

      imgui_interact();
      imgui_menu();

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      // imgui实现外部窗口的必要配置
      if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(this->window);
      }

      glfwSwapBuffers(window);
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
  }

  // void showSceneObject() { this->isShowSceneObject = true; }
};

void framebufferResizeCallback(GLFWwindow *window, int width, int height) {
  Scene *scene = (Scene *)glfwGetWindowUserPointer(window);
  assert(scene != nullptr && "scene is nullptr");
  scene->setWindowSize(width, height);
  scene->camera.setAspect(static_cast<float>(width) / height);
  glViewport(0, 0, width, height);
}

} // namespace
