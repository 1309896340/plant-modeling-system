#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <QKeyEvent>
#include <QMainWindow>
#include <QSplitter>
#include <QStatusBar>
#include <QVBoxLayout>

#include <cstdint>
#include <memory>
#include <string>

#include "Auxiliary.hpp"
#include "Bounding.h"
#include "Camera.h"
#include "Component.h"
#include "Geometry.h"
#include "LSystem.h"
#include "Renderer.h"
#include "Shader.h"
#include "Skeleton.h"
#include "Transform.h"

#include "Canvas.h"

class Toolbar : public QWidget {
public:
  Toolbar(QWidget *parent = nullptr);
};

class Editor : public QMainWindow {
  Q_OBJECT
public:
  enum ToolMode { CURSOR, PEN };

private:
  ToolMode mode;
  QVBoxLayout *layout{nullptr};
  QSplitter *splitter{nullptr};
  Canvas *canvas{nullptr};
  Toolbar *toolbar{nullptr};
  QStatusBar *statusBar{nullptr};
private slots:
public:
  Editor(std::string title);

  void keyPressEvent(QKeyEvent *event) override;
  Canvas *getCanvas() const;
};
