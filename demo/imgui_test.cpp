#include "Scene.hpp"

using namespace std;

int main(void) {
    Scene scene;
    scene.showAxis();
    scene.camera.setPosition(glm::vec3(-12.0f, 30.0f, 20.0f));
    scene.camera.lookAt({0.0f, 4.0f, 0.0f});
    scene.light.color    = {0.6f, 0.6f, 0.6f};
    scene.light.position = {1.0f, 7.0f, 2.0f};

    
    scene.mainloop();
    return 0;
}
