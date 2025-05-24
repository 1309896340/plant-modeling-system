#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_5_Core>

#include <glm/glm.hpp>

class Canvas : public QOpenGLWidget, protected QOpenGLFunctions_4_5_Core{
private:
protected:
	void initializeGL() override;
	void paintGL() override;
	void resizeGL(int w, int h) override;
public:
	Canvas(QWidget *parent = nullptr);
	void setBackground(const glm::vec4 &color);
};
