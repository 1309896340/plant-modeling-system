// #define ENABLE_NORMAL_VISUALIZATION
// #define ENBALE_POLYGON_VISUALIZATION
#include "Scene.hpp"

uint32_t count = 0;

using namespace std;
int main(int argc, char **argv) {

  Scene scene;

  scene.camera.setPosition(glm::vec3(0.0f, 4.0f, 13.0f));

  // CylinderEx c(1.0f, 0.2f, 8.0f, RADIANS(0.0f), RADIANS(0.0f));
  CylinderEx c1(1.0f, 0.2f, 12.0f, RADIANS(0.0f), RADIANS(60.0f), 8, 16, 18);
  CylinderEx c2(0.7f, 0.1f, 4.0f, RADIANS(90.0f), RADIANS(45.0f), 6, 12, 18);
  Plane ground(16.0f, 9.0f);

  scene.add("c1", &c1);
  scene.add("c2", &c2, {1.0f, 0.0f, -2.0f});
  scene.add("ground", &ground);

  scene.mainloop();
  return 0;
}
