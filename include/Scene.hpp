#pragma once

// #define NDEBUG

#include "GLFW/glfw3.h"
#include "constants.h"
#include "glm/common.hpp"
#include "glm/ext/quaternion_geometric.hpp"
#include <algorithm>
#include <iostream>

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
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
#include <stb_image.h>

#include "proj.h"

#include "Auxiliary.hpp"
#include "Camera.hpp"
#include "Geometry.hpp"
#include "Shader.hpp"
#include "Transform.hpp"

#define MOUSE_VIEW_ROTATE_SENSITIVITY 0.1f
#define MOUSE_VIEW_TRANSLATE_SENSITIVITY 0.02f

// 仅用于该文件内的调试
static void BK(int n) { printf("break %d\n", n); }

namespace {
using namespace std;

using glm::mat4;
using glm::quat;
using glm::vec3;
using glm::vec4;

namespace fs = filesystem;

const static vec3 anchor = {0.0f, 0.0f, 0.0f};

void framebufferResizeCallback(GLFWwindow *window, int width, int height);

bool hitBoundingBox(const vec3 &min_xyz, const vec3 &max_xyz,
                    const vec3 &origin, const vec3 &dir) {
  vec3 ndir = glm::normalize(dir);
  vec3 t_min_xyz = (min_xyz - origin) / ndir;
  vec3 t_max_xyz = (max_xyz - origin) / ndir;
  float t_enter = glm::max(t_min_xyz.x, glm::max(t_min_xyz.y, t_min_xyz.z));
  float t_exit = glm::min(t_max_xyz.x, glm::min(t_max_xyz.y, t_max_xyz.z));
  // cout << "min_xyz = " << glm::to_string(min_xyz) << endl;
  // cout << "max_xyz = " << glm::to_string(max_xyz) << endl;
  // printf("t_enter=%.4f   t_exit=%.4f\n", t_enter, t_exit);
  if (t_enter < t_exit && t_exit >= 0)
    return true;
  return false;
}

class Light {
  // 简单封装一个点光源
public:
  vec3 position{0.0f, 0.0f, 0.0f};
  vec3 color{1.0f, 1.0f, 1.0f};
  // float intensity{1.0f};

  Light() = default;
  Light(vec3 position, vec3 color) : position(position), color(color) {}
};

class ParallelLight {
public:
  vec3 position{0.0f, 0.0f, 0.0f};
  vec3 direction{0.0f, 0.0f, -1.0f};
  vec3 color{1.0f, 1.0f, 1.0f};
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
  }

  void initBoundingBoxVBO() {
    updateBoundingBox();
    vec3 max_xyz = this->box.max_bound;
    vec3 min_xyz = this->box.min_bound;
    vector<vec3> b_vertices = {min_xyz,
                               {min_xyz.x, min_xyz.y, max_xyz.z},
                               {min_xyz.x, max_xyz.y, min_xyz.z},
                               {min_xyz.x, max_xyz.y, max_xyz.z},
                               {max_xyz.x, min_xyz.y, min_xyz.z},
                               {max_xyz.x, min_xyz.y, max_xyz.z},
                               {max_xyz.x, max_xyz.y, min_xyz.z},
                               max_xyz};
    // cout << "调试输出初始化的包围盒顶点：" << endl;
    // for (auto &vert : b_vertices) {
    //   cout << "(" << vert.x << "," << vert.y << "," << vert.z << ")" << endl;
    // }
    vector<GLuint> b_indices = {0, 1, 0, 2, 0, 4, 1, 3, 1, 5, 2, 3,
                                2, 6, 4, 6, 4, 5, 3, 7, 5, 7, 6, 7};

    glGenVertexArrays(1, &this->box.vao);
    glBindVertexArray(this->box.vao);

    glGenBuffers(1, &this->box.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->box.vbo);
    glBufferData(GL_ARRAY_BUFFER, b_vertices.size() * sizeof(vec3),
                 b_vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &this->box.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->box.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, b_indices.size() * sizeof(GLuint),
                 b_indices.data(), GL_STATIC_DRAW);
  }

  // void initComputeShaderTexture(){
  // }

public:
  shared_ptr<Geometry> geometry;
  Transform transform;

  GLuint vao{0};
  GLuint vbo{0};
  GLuint ebo{0};
  GLuint texture{0};

  struct {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    vec3 min_bound{0.0f, 0.0f, 0.0f};
    vec3 max_bound{0.0f, 0.0f, 0.0f};
  } box;

  // 与计算着色器之间数据交换的上下文信息
  // struct {
  //   GLuint position_texture; // geometry中所有顶点位置
  //   GLuint index_texture;    // geometry中所有三角索引
  //   GLuint output_texture;   // 计算着色器输出结果
  // } ctx;

  bool isSelected{false};
  bool visible{true};

  GeometryRenderObject() = default;
  GeometryRenderObject(const shared_ptr<Geometry> &geometry,
                       Transform transform)
      : geometry(geometry), transform(transform) {
    initVBO();
    // initComputeShaderTexture();
    // updateBoundingBox();
    initBoundingBoxVBO();
  }
  GeometryRenderObject(const shared_ptr<Geometry> &geometry)
      : GeometryRenderObject(geometry, Transform()) {}

  GeometryRenderObject(const vector<Vertex> &vertices,
                       const vector<Surface> &surfaces, Transform transform)
      : transform(transform) {
    this->geometry = make_shared<FixedGeometry>(vertices, surfaces);
    initVBO();
    // initComputeShaderTexture();
    // updateBoundingBox();
    initBoundingBoxVBO();
  }

  GeometryRenderObject(const vector<Vertex> &vertices,
                       const vector<Surface> &surfaces)
      : GeometryRenderObject(vertices, surfaces, Transform()) {}

  GeometryRenderObject(GeometryRenderObject &&obj) noexcept { // 实现移动语义
    this->transform = obj.transform;
    this->vao = obj.vao;
    this->vbo = obj.vbo;
    this->ebo = obj.ebo;
    this->texture = obj.texture;
    this->geometry = obj.geometry;
    this->box = obj.box;
    obj.vao = 0;
    obj.vbo = 0;
    obj.ebo = 0;
    obj.geometry = nullptr;
    obj.box.vao = 0;
    obj.box.vbo = 0;
    obj.box.ebo = 0;
  }
  GeometryRenderObject &operator=(GeometryRenderObject &&obj) noexcept {
    this->transform = obj.transform;
    this->vao = obj.vao;
    this->vbo = obj.vbo;
    this->ebo = obj.ebo;
    this->texture = obj.texture;
    this->geometry = obj.geometry;
    this->box = obj.box;
    obj.vao = 0;
    obj.vbo = 0;
    obj.ebo = 0;
    obj.geometry = nullptr;
    obj.box.vao = 0;
    obj.box.vbo = 0;
    obj.box.ebo = 0;
    return *this;
  }

  void updateBoundingBox() {
    // 将this->geometry->vertices变换到世界坐标系
    mat4 model = this->transform.getModel();
    vector<vec3> positions(this->geometry->vertices.size());
    for (int i = 0; i < this->geometry->vertices.size(); i++) {
      positions[i] = vec3(
          model *
          vec4(glm::make_vec3(this->geometry->vertices[i].position), 1.0f));
    }
    // 计算包围盒边界
    this->box.min_bound = positions[0];
    this->box.max_bound = positions[0];
    for (auto &vert : positions) {
      this->box.min_bound.x = glm::min(this->box.min_bound.x, vert.x);
      this->box.min_bound.y = glm::min(this->box.min_bound.y, vert.y);
      this->box.min_bound.z = glm::min(this->box.min_bound.z, vert.z);

      this->box.max_bound.x = glm::max(this->box.max_bound.x, vert.x);
      this->box.max_bound.y = glm::max(this->box.max_bound.y, vert.y);
      this->box.max_bound.z = glm::max(this->box.max_bound.z, vert.z);
    }
  }
  void updateBoundingBoxVBO() {
    updateBoundingBox();

    vec3 max_xyz = this->box.max_bound;
    vec3 min_xyz = this->box.min_bound;
    vector<vec3> b_vertices = {min_xyz,
                               {min_xyz.x, min_xyz.y, max_xyz.z},
                               {min_xyz.x, max_xyz.y, min_xyz.z},
                               {min_xyz.x, max_xyz.y, max_xyz.z},
                               {max_xyz.x, min_xyz.y, min_xyz.z},
                               {max_xyz.x, min_xyz.y, max_xyz.z},
                               {max_xyz.x, max_xyz.y, min_xyz.z},
                               max_xyz};
    glBindVertexArray(this->box.vao);
    glBindBuffer(GL_ARRAY_BUFFER, this->box.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, b_vertices.size() * sizeof(vec3),
                    b_vertices.data());
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
    updateBoundingBoxVBO();
  }
  ~GeometryRenderObject() {
    glDeleteBuffers(1, &this->box.vbo);
    glDeleteBuffers(1, &this->box.ebo);
    glDeleteVertexArrays(1, &this->box.vao);

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

  struct {
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

  struct {
    GeometryRenderObject *cur_obj{nullptr};
    int selected_idx = 0;
    int highlighted_idx = 0;
    float theta_c = 0.0f;
    float phi_c = 0.0f;
    float dist = 0.0f;
    ImVec2 mouse_pos{0, 0};
    vector<string> items;
  } imgui;

public:
  map<string, Shader *> shaders;
  map<string, GLuint> textures;
  map<string, GeometryRenderObject> objs;

  map<string, GeometryRenderObject> aux;

  ParallelLight light;
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
  }
  Scene(const Scene &sc) = delete;
  ~Scene() {
    for (const pair<string, Shader *> &sd : this->shaders)
      delete sd.second;
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
    GeometryRenderObject obj1(lightBall);
    obj1.geometry->setColor(1.0f, 1.0f, 1.0f);
    obj1.updateVBO();
    this->addSceneObject("Light", std::move(obj1));

    // 坐标轴
    shared_ptr<Geometry> axis = make_shared<CoordinateAxis>();
    GeometryRenderObject obj2(axis);
    this->addSceneObject("Axis", std::move(obj2));

    // 地面
    shared_ptr<Geometry> ground = make_shared<Ground>(20.0f, 20.0f);
    GeometryRenderObject obj3(ground);
    obj3.texture = this->textures["fabric"];
    this->addSceneObject("Ground", std::move(obj3));
  }

  void init_skybox() {
    // 与前面的GL_TEXTURE_2D纹理目标不同，天空盒使用GL_TEXTURE_CUBE_MAP_XXX作为纹理目标

    // 1. 加载VBO
    vector<vec3> vertices = {
        {-1.0, -1.0, -1.0}, {-1.0, -1.0, 1.0}, {-1.0, 1.0, -1.0},
        {-1.0, 1.0, 1.0},   {1.0, -1.0, -1.0}, {1.0, -1.0, 1.0},
        {1.0, 1.0, -1.0},   {1.0, 1.0, 1.0},
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
      void *data = stbi_load(fname.c_str(), &width, &height, &channel, 0);
      if (data == 0) {
        cerr << "load skybox texture failed: \"" << fname << "\"" << endl;
        continue;
      }

      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width,
                   height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

      stbi_image_free(data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
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

  void showAxis() { this->aux["Axis"].visible = true; }
  void hideAxis() { this->aux["Axis"].visible = false; }
  void showGround() { this->aux["Ground"].visible = true; }
  void hideGround() { this->aux["Ground"].visible = false; }

  void show_info() {
    const GLubyte *vendor = glGetString(GL_VENDOR);
    const GLubyte *renderer = glGetString(GL_RENDERER);
    cout << "Vendor: " << vendor << endl;
    cout << "Graphics Device: " << renderer << endl;
  }

  void imgui_menu() {

    // ImGui::ShowDemoWindow();
    imgui_docking_render();

    ImGui::Begin(u8"场景");
    if (ImGui::TreeNodeEx(u8"相机", ImGuiTreeNodeFlags_Selected |
                                        ImGuiTreeNodeFlags_DefaultOpen)) {
      bool is_theta_changed = ImGui::SliderFloat(
          u8"天顶角", &this->camera.theta_s, 0.0f, 180.0f, "%.1f");
      bool is_phi_changed = ImGui::SliderFloat(u8"方向角", &this->camera.phi_s,
                                               -180.0f, 180.0f, "%.1f");
      if (is_theta_changed || is_phi_changed)
        this->camera.updateToward();

      if (ImGui::InputFloat3(u8"位置", glm::value_ptr(this->camera.position_s),
                             "%.2f", 0)) {
        this->camera.updatePositionFromShadow();
      }

      // 鼠标交互动作
      // 记录下“相机环绕”时的相机球坐标
      if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        if (ImGui::IsKeyDown(ImGuiKey_ModShift)) {
          vec3 dist_v = this->camera.position_s - anchor;
          imgui.dist = glm::length(dist_v);
          imgui.theta_c =
              acos(glm::dot(glm::normalize(dist_v), vec3(0.0f, 1.0f, 0.0f)));
          imgui.phi_c = atan2(dist_v.z, dist_v.x);
        } else {
          // 鼠标左击选中场景物体，遍历所有物体的包围盒求交
          this->imgui.mouse_pos = io->MousePos;
        }
      }

      if (!io->WantCaptureMouse) {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
          if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
            // 以世界坐标锚点为中心做旋转
            imgui.phi_c +=
                MOUSE_VIEW_ROTATE_SENSITIVITY * 0.04 * io->MouseDelta.x;
            imgui.theta_c -=
                MOUSE_VIEW_ROTATE_SENSITIVITY * 0.04 * io->MouseDelta.y;

            vec3 new_pos;
            new_pos.x = imgui.dist * sin(imgui.theta_c) * cos(imgui.phi_c);
            new_pos.y = imgui.dist * cos(imgui.theta_c);
            new_pos.z = imgui.dist * sin(imgui.theta_c) * sin(imgui.phi_c);
            this->camera.setPosition(new_pos + anchor);
            this->camera.lookAt(anchor);
          } else {
            // 以相机为中心旋转
            this->camera.rotate(
                {MOUSE_VIEW_ROTATE_SENSITIVITY * io->MouseDelta.x,
                 MOUSE_VIEW_ROTATE_SENSITIVITY * io->MouseDelta.y, 0.0});
          }
        }
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right, 0.0f)) {
          this->camera.move_relative(
              {-MOUSE_VIEW_TRANSLATE_SENSITIVITY * io->MouseDelta.x,
               MOUSE_VIEW_TRANSLATE_SENSITIVITY * io->MouseDelta.y, 0.0f});
        }
      }

      if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        ImVec2 mouse_pos = io->MousePos;
        if (mouse_pos.x == this->imgui.mouse_pos.x &&
            mouse_pos.y == this->imgui.mouse_pos.y) {
          // 按下与弹起位置一致，中间没有鼠标拖拽
          // 将屏幕坐标转换到裁剪坐标，然后转换为世界坐标，与包围盒求交

          double xpos, ypos;
          glfwGetCursorPos(this->window, &xpos, &ypos);
          xpos = 2.0f * xpos / this->width - 1.0f;
          ypos = 1.0f - 2.0f * ypos / this->height;
          vec3 pos = vec3(xpos, ypos, 1.0f);
          mat4 clip2world =
              glm::inverse(this->camera.getProject() * this->camera.getView());
          vec4 pos_world = clip2world * vec4(pos, 1.0f);
          vec3 dirr = glm::normalize(vec3(pos_world / pos_world.w) -
                                     this->camera.getPosition());
          printf("(%.2f, %.2f, %.2f)\n", dirr.x, dirr.y, dirr.z);
          printf("(%.2f, %.2f)\n", xpos, ypos);
          vec3 camera_pos = this->camera.getPosition();
          for (auto &[name, oobj] : this->objs) {
            vec3 minbound = oobj.box.min_bound, maxbound = oobj.box.max_bound;
            // 选择就近边界为minbound，就远边界为maxbound
            vec3 mmin = glm::abs(camera_pos - oobj.box.min_bound);
            vec3 mmax = glm::abs(camera_pos - oobj.box.max_bound);
            if(mmax.x < mmin.x){
              minbound.x = oobj.box.max_bound.x;
              maxbound.x = oobj.box.min_bound.x;
            }
            if(mmax.y < mmin.y){
              minbound.y = oobj.box.max_bound.y;
              maxbound.y = oobj.box.min_bound.y;
            }
            if(mmax.z < mmin.z){
              minbound.z = oobj.box.max_bound.z;
              maxbound.z = oobj.box.min_bound.z;
            }
            bool isHit = hitBoundingBox(minbound, maxbound,
                                        this->camera.getPosition(), dirr);
            cout << name << " isHit: " << isHit << endl;
            if (isHit) {
              auto selected_name_ptr = std::find(this->imgui.items.begin(),
                                                 this->imgui.items.end(), name);
              if (selected_name_ptr != this->imgui.items.end()) {
                uint32_t idx = selected_name_ptr - this->imgui.items.begin();
                this->imgui.selected_idx = idx;
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
      ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx(u8"光源", ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::SliderFloat3(u8"位置", glm::value_ptr(this->light.position),
                          -20.0f, 20.f);
      // 暂时这么写
      this->aux["Light"].transform.setPosition(light.position);
      ImGui::TreePop();
    }

    if (this->objs.empty()) {
      ImGui::End();
      return;
    }

    if (ImGui::TreeNodeEx(u8"几何管理", ImGuiTreeNodeFlags_DefaultOpen)) {
      bool is_hightlight = true;
      this->imgui.items.clear();
      for (auto &obj : this->objs) {
        this->imgui.items.push_back(obj.first);
      }
      if (ImGui::BeginListBox(u8"物体")) {
        for (int i = 0; i < this->imgui.items.size(); i++) {
          const bool is_selected = (imgui.selected_idx == i);
          if (ImGui::Selectable(this->imgui.items[i].c_str(), is_selected))
            imgui.selected_idx = i;
          if (is_hightlight && ImGui::IsItemHovered())
            imgui.highlighted_idx = i;
          if (is_selected) {
            imgui.cur_obj = &this->objs[this->imgui.items[i]];
            ImGui::SetItemDefaultFocus();
          }
        }
        // 回传选中状态
        for (int i = 0; i < this->imgui.items.size(); i++) {
          this->objs[this->imgui.items[i]].isSelected = false;
          if (imgui.selected_idx == i)
            this->objs[this->imgui.items[i]].isSelected = true;
        }
        ImGui::EndListBox();
      }

      // 显示参数
      if (imgui.cur_obj == nullptr ||
          imgui.cur_obj->geometry->parameters.empty()) {
        ImGui::TreePop();
        ImGui::End();
        return; // 直接返回
      }
      ImGui::Text(u8"形体参数");
      struct visitor {
        string pname;
        Scene *context{nullptr};
        visitor(string name, Scene *context) : pname(name), context(context) {}
        void operator()(float &arg) {
          if (ImGui::SliderFloat(this->pname.c_str(), &arg, 0.0f, 10.0f)) {
            context->imgui.cur_obj->geometry->update();
            context->imgui.cur_obj->updateVBO();
          }
        }
        void operator()(uint32_t &arg) {
          if (ImGui::SliderInt(this->pname.c_str(),
                               reinterpret_cast<int *>(&arg), 2, 50)) {
            context->imgui.cur_obj->geometry->update();
            context->imgui.cur_obj->updateVBO();
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

    ImGui::End();
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

  void addSceneObject(const string &name, GeometryRenderObject &&obj) {

    if (this->aux.find(name) != this->aux.end()) {
      cout << "scene cannot add object with an existed name!" << endl;
      return;
    }
    this->aux[name] = std::move(obj);
  }
  // void addSceneObject(const string &name,
  //                     const shared_ptr<Geometry> &geometry) {
  //   this->addSceneObject(name, geometry, Transform());
  // }

  void add(const string &name, const shared_ptr<Geometry> &geometry,
           Transform transform) {
    if (this->objs.find(name) != this->objs.end()) {
      cout << "scene cannot add object with an existed name!" << endl;
      return;
    }
    GeometryRenderObject obj(geometry, transform);
    this->objs[name] = std::move(obj);
  }

  void add(const string &name, const shared_ptr<Geometry> &geometry) {
    this->add(name, geometry, Transform());
  }

  void add(const string &name, const shared_ptr<Geometry> &geometry,
           vec3 position) {
    this->add(name, geometry, Transform(position));
  }

  void imgui_docking_render(bool *p_open = nullptr) {
    // Variables to configure the Dockspace example.
    static bool opt_fullscreen = true; // Is the Dockspace full-screen?
    static bool opt_padding = false; // Is there padding (a blank space) between
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
      mat4 model = cur_obj.transform.getModel();
      cur_shader->set("model", model);
      // 启用材质，设置材质属性
      glBindTexture(GL_TEXTURE_2D, cur_obj.texture);
      cur_shader->set("useTexture", (cur_obj.texture != 0) ? true : false);
      cur_shader->set("useLight", true);
      // 绘制
      glBindVertexArray(cur_obj.vao);
      glDrawElements(GL_TRIANGLES, cur_obj.geometry->surfaces.size() * 3,
                     GL_UNSIGNED_INT, (void *)0);
    }
    // 2. 场景辅助元素
    for (auto &[name, obj] : this->aux) {
      if (!obj.visible)
        continue;
      cur_shader->set("model", obj.transform.getModel());
      if (obj.texture != 0) {
        cur_shader->set("useTexture", true);
        cur_shader->set("useLight", true);
      } else {
        cur_shader->set("useTexture", false);
        cur_shader->set("useLight", false);
      }
      glBindTexture(GL_TEXTURE_2D, obj.texture);
      glBindVertexArray(obj.vao);
      glDrawElements(GL_TRIANGLES, obj.geometry->surfaces.size() * 3,
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
      cur_shader->set("model", obj.transform.getModel());
      glBindVertexArray(obj.vao);
      glDrawElements(GL_TRIANGLES, obj.geometry->surfaces.size() * 3,
                     GL_UNSIGNED_INT, nullptr);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

#ifdef ENABLE_NORMAL_VISUALIZATION
    // 4. 渲染法向量
    for (auto &[name, cur_obj] : this->objs) {
      if (cur_obj.isSelected) {
        cur_shader = this->shaders["normal"];
        cur_shader->use();
        cur_shader->set("model", model);

        glBindVertexArray(cur_obj.vao);
        glDrawArrays(GL_POINTS, 0, cur_obj.geometry->vertices.size());
      }
    }
#endif

#ifdef ENABLE_BOUNDINGBOX_VISUALIZATION
    // 5. 渲染包围盒边框
    cur_shader = this->shaders["line"];
    cur_shader->use();
    cur_shader->set("lineColor", glm::vec3(0.0f, 1.0f, 1.0f));
    for (auto &[name, cur_obj] : this->objs) {
      if (cur_obj.isSelected) {
        glBindVertexArray(cur_obj.box.vao);
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, nullptr);
      }
    }
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
