// #define ENABLE_NORMAL_VISUALIZATION
// #define ENBALE_POLYGON_VISUALIZATION
#include "Scene.h"
#include <memory>

uint32_t count = 0;

using namespace std;
int main(int argc, char **argv) {

  try {
    Scene::Scene scene;

    scene.showAxis();
    scene.camera.setPosition(glm::vec3(28.0f, 13.0f, 20.0f));
    // scene.camera.lookAt({0.0f, 4.0f, 0.0f});
    scene.camera.lookAt(134.0f, 97.0f);
    scene.light.color = {0.6f, 0.6f, 0.6f};
    scene.light.position = {1.0f, 12.0f, 2.0f};

    scene.add("Sphere", Mesh::Sphere(2.0f, 72, 36),
              Transform({-4.0f, 2.05f, 6.0f}));
    scene.add("Cube", Composition::Cube(10.0f, 7.0f, 4.0f, 10, 7, 4),
              Transform({4.0f, 0.05f, -6.0f}));
    scene.add("Cylinder", Composition::Cylinder(2.5f, 5.0f, 72, 10, 20),
              Transform({5.0f, 0.05f, 0.0f}));
    scene.add("Cone", Composition::Cone(2.0f, 3.0f, 72, 10, 20),
              Transform({0.0f, 0.05f, 6.0f}));

    scene.mainloop();
  } catch (std::exception e) {
    cout << e.what() << endl;
  }
  return 0;
}
