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
    scene.camera.setPosition(glm::vec3(19.0f, 12.0f, 11.0f));
    // scene.camera.lookAt({0.0f, 4.0f, 0.0f});
    scene.camera.lookAt(139.5f, 100.0f);
    scene.light.color = {0.6f, 0.6f, 0.6f};
    scene.light.position = {6.0f, 19.0f, -2.0f};

    scene.add(
      "Leaf",
      Mesh::Leaf(LeafParameters{1.9, 20, 0.415, -0.161, 0.311, 4.398, -0.880},
                 5, 60),
      Transform(glm::vec3(0.0f, 0.1f, -4.0f)));

    scene.mainloop();
  } catch (std::exception e) {
    cout << e.what() << endl;
  }
  return 0;
}
