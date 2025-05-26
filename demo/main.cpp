#include <QApplication>

#include <iostream>

#include "Editor.h"
#include "Shader.h"


int main(int argc, char**argv){

    QApplication app(argc, argv);

    Editor editor("editor");

    // ±‡“Îshader
    std::unordered_map<std::string, Shader *> shaders;
    shaders["default"] = new Shader("default.vert", "default.frag");
    shaders["normal"] = new Shader("normal.vert", "normal.geom", "normal.frag");
    shaders["skybox"] = new Shader("skybox.vert", "skybox.frag");
    shaders["lightDepth"] = new Shader("lightDepth.vert", "lightDepth.frag");
    shaders["line"] = new Shader("line.vert", "line.frag");
    shaders["screen"] = new Shader("screen.vert", "screen.frag");
    std::cout << "compile shader success!" << std::endl;

    return app.exec();
}
