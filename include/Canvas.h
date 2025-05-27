#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_5_Core>

#include <glm/glm.hpp>

#include "Shader.h"

class Canvas : public QOpenGLWidget, protected QOpenGLFunctions_4_5_Core{
private:
  std::unordered_map<std::string, Shader *> shaders;

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
};
