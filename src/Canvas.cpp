#include "Canvas.h"

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

Canvas::Canvas(QWidget *parent) : QOpenGLWidget(parent) {}

void Canvas::setBackground(const glm::vec4 &color) {
  glClearColor(color[0], color[1], color[2], color[3]);
}

void Canvas::initializeGL() {
  initializeOpenGLFunctions();
  makeCurrent();
  // =====================
  // 开启MSAA抗锯齿
  glEnable(GL_MULTISAMPLE);
  // 开启深度测试
  glEnable(GL_DEPTH_TEST);
  // 开启面剔除
  // glEnable(GL_CULL_FACE);
  // 顺时针索引顺序为正面
  glFrontFace(GL_CW);
  glLineWidth(1.5f);
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);

  // ==========显卡信息============
  const GLubyte *vendor = glGetString(GL_VENDOR);
  const GLubyte *renderer = glGetString(GL_RENDERER);
  std::printf("Vendor: %s\n", vendor);
  std::printf("Graphics Device: %s\n", renderer);

  // =========================

  load_all_shader();
  load_all_texture();

  init_ubo();
  init_scene_obj();
  init_skybox();
  init_framebuffer();
  init_depthmap();
  init_line_buffer();
}

void Canvas::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // 。。。
}
void Canvas::resizeGL(int w, int h) { glViewport(0, 0, w, h); }

void Canvas::load_all_shader() {
  std::printf("compiling shader...\n");
  shaders["default"] = new Shader("default.vert", "default.frag");
  shaders["normal"] = new Shader("normal.vert", "normal.geom", "normal.frag");
  shaders["skybox"] = new Shader("skybox.vert", "skybox.frag");
  shaders["lightDepth"] = new Shader("lightDepth.vert", "lightDepth.frag");
  shaders["line"] = new Shader("line.vert", "line.frag");
  shaders["screen"] = new Shader("screen.vert", "screen.frag");
  std::printf("compile shader done!\n");
}
void Canvas::load_all_texture() {
  fs::path texture_dir =
      QApplication::applicationDirPath().toStdString() + "/assets/textures";
  if (!fs::exists(texture_dir)) {
    printf("\"%s\" not exists!\n", texture_dir.string().c_str());
    exit(1);
  }
  for (auto &file : fs::directory_iterator(texture_dir)) {
    if (file.is_regular_file()) {
      // 加载纹理
      std::string fname = file.path().string();
      // 读取图片文件
      QImage img(QString::fromStdString(fname));
      if (img.isNull()) {
        printf("load texture: \"%s\" failed!\n", fname.c_str());
        exit(2);
      }

      // 创建纹理缓冲
      GLuint new_texture;
      glGenTextures(1, &new_texture);
      glBindTexture(GL_TEXTURE_2D, new_texture);

      glTextureParameteri(new_texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTextureParameteri(new_texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTextureParameteri(new_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTextureParameteri(new_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      img = img.convertToFormat(QImage::Format_RGB32);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width(), img.height(), 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, img.data_ptr());
      glGenerateTextureMipmap(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
  }
}
void Canvas::init_ubo() {}
void Canvas::init_scene_obj() {}
void Canvas::init_skybox() {}
void Canvas::init_framebuffer() {}
void Canvas::init_depthmap() {}
void Canvas::init_line_buffer() {}