#pragma once

// #define NDEBUG

// #include "glm/ext/quaternion_common.hpp"
// #include "glm/ext/quaternion_transform.hpp"
// #include "glm/ext/quaternion_trigonometric.hpp"

#include "glm/fwd.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>
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

#define MOUSE_VIEW_ROTATE_SENSITIVITY 0.1f
#define MOUSE_VIEW_TRANSLATE_SENSITIVITY 0.02f

namespace {
using namespace std;
// namespace fs = std::filesystem;

using glm::mat4;
using glm::quat;
using glm::vec3;
using glm::vec4;

void framebufferResizeCallback(GLFWwindow *window, int width, int height);

class Transform {
private:
  vec3 position;
  quat attitude{glm::angleAxis(-PI / 2, glm::vec3(1.0f, 0.0f, 0.0f))};

public:
  // 构造函数中默认做了一个绕x轴-90°旋转，将OpenGL坐标系(y朝上，z朝屏幕外)转换为z朝上，y朝屏幕内
  Transform(vec3 position, vec3 attitude_vec, float attitude_angle)
      : position(position) {
    this->attitude = glm::rotate(this->attitude, attitude_angle, attitude_vec);
  }
  Transform(vec3 position, quat attitude) : position(position) {
    // 未测试
    this->attitude = attitude * this->attitude * glm::conjugate(attitude);
  }

  Transform() : position{0.0f, 0.0f, 0.0f} {}

  void setPosition(vec3 new_position){this->position = new_position;};

  mat4 getModel() const {
    mat4 mvp(1.0f);
    mat4 rot_mat = glm::mat4_cast(this->attitude);
    mvp = glm::translate(mvp, this->position);
    mvp = mvp * rot_mat;
    return mvp;
  }
};

class Light {
  // 简单封装一个点光源
public:
  vec3 position{0.0f, 0.0f, 0.0f};
  vec3 color{1.0f, 1.0f, 1.0f};
  // float intensity{1.0f};

  Light() = default;
  Light(vec3 position, vec3 color) : position(position), color(color) {}
};

class GeometryRenderObject {
private:
  void init_vbo() {

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

    // 绑定纹理(根据子类型)
    Plane *pln = dynamic_cast<Plane *>(this->geometry);
    if (pln != nullptr) {
      // 目前仅考虑为子类型为Plane的Geometry绑定纹理，其他不做处理
      glGenTextures(1, &this->texture);
      glBindTexture(GL_TEXTURE_2D, this->texture);

      glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      int width, height, nChannels;
      uint8_t *data = stbi_load("assets/textures/fabric.jpg", &width, &height,
                                &nChannels, 0);
      if (data == nullptr) {
        string msg = "load texture \"assets/textures/fabric.jpg\" failed!";
        cerr << msg << endl;
        throw runtime_error(msg);
      }
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                   GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
    }

    glBindVertexArray(0);
  }

public:
  Geometry *geometry{nullptr};
  Transform transform;

  GLuint vao{0};
  GLuint vbo{0};
  GLuint ebo{0};
  GLuint texture{0};

  bool isSelected{false};

  GeometryRenderObject() = default;
  GeometryRenderObject(Geometry *geo, Transform trans)
      : geometry(geo), transform(trans) {
    init_vbo();
  }

  GeometryRenderObject(GeometryRenderObject &&geo) noexcept { // 实现移动语义
    this->transform = geo.transform;
    this->vao = geo.vao;
    this->vbo = geo.vbo;
    this->ebo = geo.ebo;
    this->texture = geo.texture;
    this->geometry = geo.geometry;
    geo.vao = 0;
    geo.vbo = 0;
    geo.ebo = 0;
    geo.geometry = nullptr;
  }

  GeometryRenderObject &operator=(GeometryRenderObject &&geo) noexcept {
    this->transform = geo.transform;
    this->vao = geo.vao;
    this->vbo = geo.vbo;
    this->ebo = geo.ebo;
    this->texture = geo.texture;
    this->geometry = geo.geometry;
    geo.vao = 0;
    geo.vbo = 0;
    geo.ebo = 0;
    geo.geometry = nullptr;
    return *this;
  }

  // 需要区分是否是顶点数量进行变化
  // 1. 顶点数量不变：只通过glBufferSubData进行更新
  // 2. 顶点数量改变：需要glBufferData进行空间的重分配
  // 先统一用glBufferData做更新
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
  }
  ~GeometryRenderObject() {
    glDeleteBuffers(1, &this->vbo);
    glDeleteBuffers(1, &this->ebo);
    glDeleteVertexArrays(1, &this->vao);
  }
};

class AuxiliaryRenderObject {
  // 使用 auxiliary 着色器程序渲染
  // 只有位置、颜色
private:
public:
  Transform transform;

  GLuint vao{0};
  GLuint vbo{0};
  GLuint ebo{0};

  uint32_t v_size{0};
  AuxiliaryRenderObject() = default;
  AuxiliaryRenderObject(const Geometry &geometry)
      : v_size(geometry.surfaces.size() * 3) {
    glGenVertexArrays(1, &this->vao);
    glBindVertexArray(this->vao);

    glGenBuffers(1, &this->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    glBufferData(GL_ARRAY_BUFFER, geometry.vertices.size() * sizeof(Vertex),
                 geometry.vertices.data(), GL_STATIC_DRAW);

    size_t stride = sizeof(Vertex);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                          (void *)0); // 位置
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
                          (void *)(6 * sizeof(float))); // 颜色
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(2);

    glGenBuffers(1, &this->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 geometry.surfaces.size() * sizeof(Surface),
                 geometry.surfaces.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
  }
  AuxiliaryRenderObject(AuxiliaryRenderObject &&ax) noexcept {
    this->vao = ax.vao;
    this->vbo = ax.vbo;
    this->ebo = ax.ebo;
    this->v_size = ax.v_size;
    ax.vao = 0;
    ax.vbo = 0;
    ax.ebo = 0;
    ax.v_size = 0;
  }
  AuxiliaryRenderObject &operator=(AuxiliaryRenderObject &&ax) noexcept {
    this->vao = ax.vao;
    this->vbo = ax.vbo;
    this->ebo = ax.ebo;
    this->v_size = ax.v_size;
    ax.v_size = 0;
    ax.vao = 0;
    ax.vbo = 0;
    ax.ebo = 0;
    return *this;
  }
  ~AuxiliaryRenderObject() {
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
  GLFWwindow *window{nullptr};
  ImGuiIO *io{nullptr};

  GLuint ubo{0};
  const GLuint PVM_binding_point = 0;

  bool isShowAxis{false};

public:
  const int width = 1600;
  const int height = 1200;

  map<string, Shader *> shaders;
  map<string, GeometryRenderObject> objs;
  map<string, AuxiliaryRenderObject> aux;

  Light light;
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
    this->window = glfwCreateWindow(width, height, "demo", 0, 0);
    glfwMakeContextCurrent(this->window);
    glfwSetWindowUserPointer(window, this);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      string msg = "failed to init glad!";
      cerr << msg << endl;
      throw runtime_error(msg);
    }
    glEnable(GL_DEPTH_TEST); // 开启深度测试
    glEnable(GL_CULL_FACE);  // 开启面剔除
    glFrontFace(GL_CCW);

#ifdef ENBALE_POLYGON_VISUALIZATION
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif

    loadIcon();
    initImgui();

    glfwSetFramebufferSizeCallback(this->window, framebufferResizeCallback);
    glfwSetErrorCallback(errorCallback);

    load_all_shader();

    init_ubo();

    init_axis_display();
    init_light_display();
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

  void init_axis_display() {
    Arrow axis_x(0.05f, 1.0f);
    axis_x.setColor(1.0f, 0.0f, 0.0f);
    axis_x.rotate(glm::radians(90.0f), {0.0f, 1.0f, 0.0f});

    Arrow axis_y(0.05f, 1.0f);
    axis_y.setColor(0.0f, 1.0f, 0.0f);
    // axis_y.rotate(glm::radians(90.0f), {-1.0f, 0.0f, 0.0f});

    Arrow axis_z(0.05f, 1.0f);
    axis_z.setColor(0.0f, 0.0f, 1.0f);
    axis_z.rotate(glm::radians(90.0f), {1.0f, 0.0f, 0.0f});

    vector<string> names = {"axis_x", "axis_y", "axis_z"};
    vector<Arrow *> buf{&axis_x, &axis_y, &axis_z};
    for (int i = 0; i < 3; i++) {
      this->aux[names[i]] = std::move(AuxiliaryRenderObject(*buf[i]));
    }
  }

  void init_light_display() {
    Sphere lightBall(0.2f, 36, 72);
    lightBall.setColor(1.0f, 1.0f, 1.0f);
    this->aux["light"] = std::move(AuxiliaryRenderObject(lightBall));
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
      static vec3 anchor = {0.0f, 4.0f, 0.0f};
      // 记录下“相机环绕”时的相机极坐标
      static float theta_c = 0.0f;
      static float phi_c = 0.0f;
      static float dist = 0.0f;
      if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        if (ImGui::IsKeyDown(ImGuiKey_ModShift)) {
          vec3 dist_v = this->camera.position_s - anchor;
          dist = glm::length(dist_v);
          theta_c =
              acos(glm::dot(glm::normalize(dist_v), vec3(0.0f, 1.0f, 0.0f)));
          phi_c = atan2(dist_v.z, dist_v.x);
        }
      }

      if (!io->WantCaptureMouse) {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
          if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
            // 以世界坐标锚点为中心做旋转
            phi_c += MOUSE_VIEW_ROTATE_SENSITIVITY * 0.04 * io->MouseDelta.x;
            theta_c -= MOUSE_VIEW_ROTATE_SENSITIVITY * 0.04 * io->MouseDelta.y;

            vec3 new_pos;
            new_pos.x = dist * sin(theta_c) * cos(phi_c);
            new_pos.y = dist * cos(theta_c);
            new_pos.z = dist * sin(theta_c) * sin(phi_c);
            this->camera.setPosition(new_pos + anchor);
            this->camera.lookAt({0.0f, 4.0f, 0.0f});
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
        // cout << "弹起鼠标左键  " << mouse_left_click_cnt++ << endl;
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
      this->aux["light"].transform.setPosition(light.position);
      ImGui::TreePop();
    }

    // GeometryRenderObject *cur_obj = &this->objs.begin()->second;
    static GeometryRenderObject *cur_obj{&this->objs.begin()->second};
    if (ImGui::TreeNodeEx(u8"几何管理", ImGuiTreeNodeFlags_DefaultOpen)) {
      bool is_hightlight = true;
      vector<string> items;
      static int selected_idx = 0;
      static int highlighted_idx = 0;
      for (auto &obj : this->objs) {
        items.push_back(obj.first);
      }
      if (ImGui::BeginListBox(u8"物体")) {
        for (int i = 0; i < items.size(); i++) {
          const bool is_selected = (selected_idx == i);
          if (ImGui::Selectable(items[i].c_str(), is_selected))
            selected_idx = i;
          if (is_hightlight && ImGui::IsItemHovered())
            highlighted_idx = i;
          if (is_selected) {
            cur_obj = &this->objs[items[i]];
            ImGui::SetItemDefaultFocus();
          }
        }
        // 回传选中状态
        for (int i = 0; i < items.size(); i++) {
          this->objs[items[i]].isSelected = false;
          if (selected_idx == i)
            this->objs[items[i]].isSelected = true;
        }
        ImGui::EndListBox();
      }

      // 显示参数
      ImGui::Text(u8"形体参数");
      if (cur_obj == nullptr) {
        string msg = "cur_obj 为 nullptr";
        cerr << msg << endl;
        throw runtime_error(msg);
      }
      for (auto &param : cur_obj->geometry->parameters) {
        // string param_name = param.first;
        // auto param_val = param.second;
        std::visit(
            [param](auto &&arg) {
              using T = std::decay_t<decltype(arg)>;
              if constexpr (std::is_same_v<T, float>) {
                if (ImGui::SliderFloat(param.first.c_str(), &arg, 0.0f,
                                       10.0f)) {
                  cur_obj->geometry->update();
                  cur_obj->updateVBO();
                }
              } else if constexpr (std::is_same_v<T, uint32_t>) {
                if (ImGui::SliderInt(param.first.c_str(),
                                     reinterpret_cast<int *>(&arg), 2, 50)) {
                  cur_obj->geometry->update();
                  cur_obj->updateVBO();
                }
              }
            },
            param.second);
      }

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
    shaders["normal"] = new Shader("normal.vert", "normal.geo", "normal.frag");
    shaders["auxiliary"] = new Shader("auxiliary.vert", "auxiliary.frag");
  }

  void add(const string &name, Geometry *geo, Transform trans) {
    if (this->objs.find(name) != this->objs.end()) {
      cout << "scene cannot add object with an existed name!" << endl;
      return;
    }
    GeometryRenderObject obj(geo, trans);
    this->objs[name] = std::move(obj);
  }

  void add(const string &name, Geometry *geo, vec3 position) {
    add(name, geo, Transform(position, glm::identity<glm::quat>()));
  }
  void add(const string &name, Geometry *geo, vec3 position, vec3 rot_axis,
           float rot_angle) {
    add(name, geo, Transform(position, glm::normalize(rot_axis), rot_angle));
  }

  void add(const string &name, Geometry *geo) { add(name, geo, Transform()); }

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

    for (auto &pair_obj : this->objs) {
      // 计算pvm矩阵
      GeometryRenderObject *cur_obj = &pair_obj.second;
      mat4 model = cur_obj->transform.getModel();

      // 1. 渲染本体
      Shader *cur_shader = this->shaders["default"];
      cur_shader->use();

      // 1.1 传入model矩阵
      cur_shader->set("model", model);

      // 1.2 传入灯光属性
      cur_shader->set("lightPos", this->light.position);
      cur_shader->set("lightColor", this->light.color);
      // cur_shader->set("lightIntensity", this->light.intensity);

      // 1.3 传入相机位置
      cur_shader->set("eyePos", this->camera.position_s);

      // 1.4 启用材质，设置材质属性
      glBindTexture(GL_TEXTURE_2D, cur_obj->texture);
      cur_shader->set("useTexture", (cur_obj->texture != 0) ? true : false);
      cur_shader->set("ambientStrength", 0.2f);
      cur_shader->set("diffuseStrength", 1.0f);
      cur_shader->set("specularStrength", 1.0f);

      // 1.5 绘制
      glBindVertexArray(cur_obj->vao);
      glDrawElements(GL_TRIANGLES, cur_obj->geometry->surfaces.size() * 3,
                     GL_UNSIGNED_INT, (void *)0);

#ifdef ENABLE_NORMAL_VISUALIZATION
      // 2. 渲染法向量

      if (cur_obj->isSelected) {
        cur_shader = this->shaders["normal"];
        cur_shader->use();
        cur_shader->set("model", model);

        glBindVertexArray(cur_obj->vao);
        glDrawArrays(GL_POINTS, 0, cur_obj->geometry->vertices.size());
      }
#endif
      // 3. 渲染场景辅助元素
      if (this->isShowAxis) {
        cur_shader = this->shaders["auxiliary"];
        cur_shader->use();
        for (auto &[objName, auxObj] : this->aux) {
          cur_shader->set("model", auxObj.transform.getModel());
          // cur_shader->set("model", glm::mat4(1.0f));
          glBindVertexArray(auxObj.vao);
          glDrawElements(GL_TRIANGLES, auxObj.v_size, GL_UNSIGNED_INT, nullptr);
        }
      }
    }
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

  void showAxis() { this->isShowAxis = true; }
};

void framebufferResizeCallback(GLFWwindow *window, int width, int height) {
  Scene *scene = (Scene *)glfwGetWindowUserPointer(window);
  assert(scene != nullptr && "scene is nullptr");
  scene->camera.setAspect(static_cast<float>(width) / height);
  glViewport(0, 0, width, height);
}

} // namespace
