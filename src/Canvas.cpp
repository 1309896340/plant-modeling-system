#include "Canvas.h"

Canvas::Canvas(QWidget* parent) :QOpenGLWidget(parent) {
}

void Canvas::setBackground(const glm::vec4& color) {
	glClearColor(color[0], color[1], color[2], color[3]);
}

void Canvas::initializeGL() {
	initializeOpenGLFunctions();

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
}
void Canvas::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// 紙夕
	// 。。。
}
void Canvas::resizeGL(int w, int h) {
	glViewport(0, 0, w, h);
}
