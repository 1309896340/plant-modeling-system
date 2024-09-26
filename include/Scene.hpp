#pragma once

// #define NDEBUG

// #include "glm/ext/quaternion_common.hpp"
// #include "glm/ext/quaternion_transform.hpp"
// #include "glm/ext/quaternion_trigonometric.hpp"

#include "GLFW/glfw3.h"
#include "Scene.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/geometric.hpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <map>
#include <stdexcept>

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
    this->attitude = attitude * this->attitude * glm::conjugate(attitude);
  }

  Transform() : position{0.0f, 0.0f, 0.0f} {}

  mat4 getModel() const {
    mat4 mvp(1.0f);
    mat4 rot_mat = glm::mat4_cast(this->attitude);
    mvp = glm::translate(mvp, this->position);
    mvp = mvp * rot_mat;
    return mvp;
  }
};

class GeometryObj {
private:
  void initBuffer() {

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
      if (data == nullptr)
        throw runtime_error(
            "load texture \"assets/textures/fabric.jpg\" failed!");
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

  GeometryObj() = default;
  GeometryObj(Geometry *geo, Transform trans)
      : geometry(geo), transform(trans) {
    initBuffer();
  }

  GeometryObj(GeometryObj &&geo) noexcept { // 实现移动语义
    this->transform = geo.transform;
    this->vao = geo.vao;
    this->vbo = geo.vbo;
    this->ebo = geo.ebo;
    this->texture = geo.texture;
    this->geometry = geo.geometry;
    geo.geometry = nullptr;
  }

  GeometryObj &operator=(GeometryObj &&geo) noexcept {
    this->transform = geo.transform;
    this->vao = geo.vao;
    this->vbo = geo.vbo;
    this->ebo = geo.ebo;
    this->texture = geo.texture;
    this->geometry = geo.geometry;
    geo.geometry = nullptr;
    return *this;
  }
};

void framebufferResizeCallback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

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

public:
  const int width = 1200;
  const int height = 900;

  map<string, Shader *> shaders;
  map<string, GeometryObj> objs;

  Camera camera{vec3(0.0f, 0.0f, 16.0f), vec3{0.0f, 0.0f, 0.0f},
                static_cast<float>(width) / static_cast<float>(height)};
  Scene() {
    if (glfwInit() == GLFW_FALSE)
      throw runtime_error("failed to init glfw!");
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    this->window = glfwCreateWindow(width, height, "demo", 0, 0);
    glfwMakeContextCurrent(this->window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
      throw runtime_error("failed to init glad!");
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

    loadAllShader();

    init_ubo();
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
  void loadAllShader() {
    // // 读取shaders目录下的所有文件
    // set<string> shader_names;
    // fs::path shader_dir = "shaders";
    // for (auto &file : fs::directory_iterator(shader_dir)) {
    //   fs::path pp = file.path();
    //   if (!fs::is_regular_file(pp))
    //     throw runtime_error("\"/shaders\" cannot contain directory");
    //   fs::path fname = pp.filename();
    //   string ext = fname.extension().string();
    //   if (ext != ".vert" && ext != ".frag" && ext != ".geo") {
    //     stringstream err_msg;
    //     err_msg << "invalid glsl extension: " << fname;
    //     throw err_msg.str();
    //   }
    //   shader_names.insert(fname.stem().string());
    // }
    // for (const string &sdns : shader_names)
    //   shaders[sdns] = new Shader(sdns + ".vert", sdns + ".frag");

    shaders["default"] = new Shader("default.vert", "default.frag");
    shaders["normal"] = new Shader("normal.vert", "normal.geo", "normal.frag");

#ifndef NDEBUG
    cout << "all shaders compile finished!" << endl;
#endif
  }

  void add(const string &name, Geometry *geo, Transform trans) {
    if (this->objs.find(name) != this->objs.end()) {
      cout << "scene cannot add object with an existed name!" << endl;
      return;
    }
    GeometryObj obj(geo, trans);
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

  void imgui_menu() {

    // ImGui::ShowDemoWindow();
    imgui_docking_render();

    ImGui::Begin("相机");

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
          this->camera.rotate({MOUSE_VIEW_ROTATE_SENSITIVITY * io->MouseDelta.x,
                               MOUSE_VIEW_ROTATE_SENSITIVITY * io->MouseDelta.y,
                               0.0});
        }
      }
      if (ImGui::IsMouseDragging(ImGuiMouseButton_Right, 0.0f)) {
        this->camera.move_relative(
            {MOUSE_VIEW_TRANSLATE_SENSITIVITY * io->MouseDelta.x,
             -MOUSE_VIEW_TRANSLATE_SENSITIVITY * io->MouseDelta.y, 0.0f});
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

    ImGui::End();
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
    if (this->camera.project_is_changed) {
      glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
                      glm::value_ptr(projection));
      this->camera.apply_projection_done();
    }
    if (this->camera.view_is_changed) {
      glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4),
                      glm::value_ptr(view));
      this->camera.apply_view_done();
    }

    for (auto &pair_obj : this->objs) {
      // 计算pvm矩阵
      GeometryObj *cur_obj = &pair_obj.second;
      mat4 model = cur_obj->transform.getModel();

      // 1. 渲染本体
      Shader *cur_shader = this->shaders["default"];
      cur_shader->use();

      // 1.1 传入model矩阵
      cur_shader->set("model", model);

      // 1.2 启用材质
      cur_shader->set("useTexture", (cur_obj->texture != 0) ? true : false);
      glBindTexture(GL_TEXTURE_2D, cur_obj->texture);

      // 1.3 绘制
      glBindVertexArray(cur_obj->vao);
      glDrawElements(GL_TRIANGLES, cur_obj->geometry->surfaces.size() * 3,
                     GL_UNSIGNED_INT, (void *)0);

#ifdef ENABLE_NORMAL_VISUALIZATION
      // 2. 渲染法向量
      cur_shader = this->shaders["normal"];
      cur_shader->use();
      cur_shader->set("model", model);

      glBindVertexArray(cur_obj->vao);
      glDrawArrays(GL_POINTS, 0, cur_obj->geometry->vertices.size());
#endif
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

      // imgui_interaction();
      imgui_menu();

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
};

} // namespace
