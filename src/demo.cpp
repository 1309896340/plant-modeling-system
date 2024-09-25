// #define ENABLE_NORMAL_VISUALIZATION
// #define ENBALE_POLYGON_VISUALIZATION
#include "Scene.hpp"

uint32_t count = 0;

using namespace std;
int main(int argc, char **argv) {

  Scene scene;
  // Sphere s(4.0f, 18, 36);
  // scene.add(&s);

  scene.camera.setPosition(glm::vec3(0.0f, 4.0f, 13.0f));

  // CylinderEx c(1.0f, 0.2f, 8.0f, RADIANS(0.0f), RADIANS(0.0f));
  CylinderEx c1(1.0f, 0.2f, 8.0f, RADIANS(0.0f), RADIANS(60.0f), 8, 16, 18);
  CylinderEx c2(0.7f, 0.1f, 4.0f, RADIANS(70.0f), RADIANS(40.0f), 6, 12, 18);
  // CylinderEx c3(1.0f, 0.2f, 8.0f, RADIANS(0.0f), RADIANS(60.0f),8,16,18);

  scene.add("c1", &c1,
            Transform({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, RADIANS(0.0f)));
  scene.add("c2", &c2,
            Transform({2.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, RADIANS(0.0f)));

  scene.mainloop();
  return 0;
}
