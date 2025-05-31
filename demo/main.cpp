#include <QApplication>

#include <iostream>

#include "Editor.h"
#include "Shader.h"

int main(int argc, char **argv) {

  QApplication app(argc, argv);

  Editor editor("editor");
  //editor.getCanvas()->setBackground(glm::vec4(0.4f, 0.3f, 0.3f, 1.0f));


  return app.exec();
}
