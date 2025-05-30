#pragma once

#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLWidget>

#include <glm/glm.hpp>

#include "Auxiliary.hpp"
#include "Camera.h"
#include "Component.h"
#include "Geometry.h"
#include "Renderer.h"
#include "Shader.h"
#include "LineDrawer.h"


enum CanvasFlag {
  CanvasFlag_None = 0x0,
  CanvasFlag_SHOW_LIGHT = 0x1,
  CanvasFlag_SHOW_AXIS = 0x2,
  CanvasFlag_SHOW_CURSOR = 0x4,
};

struct SkyboxInfo {
  GLuint vao{0};
  GLuint vbo{0};
  GLuint ebo{0};
  GLuint texture{0};
};

struct Pixel {
  // 用于解析stb_load加载的png图片
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
};

// struct PngImage {
//   Pixel *img;
//   int width;
//   int height;
//   int channel;
// };

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


class Canvas : public QOpenGLWidget, protected QOpenGLFunctions_4_5_Core {
private:
  const uint32_t PVM_binding_point = 0;
  uint32_t camera_ubo{0};
  std::unordered_map<std::string, Shader *> shaders;
  std::map<std::string, uint32_t> textures;
  std::map<std::shared_ptr<Component::GeometryObject>,
           std::unique_ptr<GeometryContext>>
      objs;
  std::unique_ptr<Camera> camera{nullptr};

  std::map<std::string, std::shared_ptr<LineDrawer>> lines;
  SkyboxInfo skybox;
  FramebufferInfo framebuffer;
  DepthmapInfo depthmap;

  uint32_t canvas_flag{CanvasFlag_SHOW_LIGHT | CanvasFlag_SHOW_AXIS};

  void load_all_shader();
  void load_all_texture();
  void init_ubo();
  void init_scene_obj();
  void init_skybox();
  void init_framebuffer();
  void init_depthmap();
  void init_line_buffer();

protected:
  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int w, int h) override;

public:
  Canvas(QWidget *parent = nullptr);
  void setBackground(const glm::vec4 &color);

  void addSceneObject(const std::shared_ptr<Component::GeometryObject> &obj,
                      uint32_t flag, GLuint texture);
  std::shared_ptr<Component::GeometryObject>
  findGeometryObjectByName(const std::string &name);
  void updateGeometryListView();
};
