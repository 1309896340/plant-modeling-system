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
    scene.camera.setPosition(glm::vec3(26.28f, 17.37f, 19.27f));
    // scene.camera.lookAt({0.0f, 4.0f, 0.0f});
    scene.camera.lookAt(142.1f, 106.9f);
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
    scene.add(
        "Leaf1",
        Mesh::Leaf(LeafParameters{0.8, 10, 0.415, -0.201, 0.429, 2.89, -0.471},
                   12, 60),
        Transform(glm::vec3(-6.6f, 0.0f, -4.85f)));

    scene.mainloop();
  } catch (std::exception e) {
    cout << e.what() << endl;
  }
  return 0;
}
