#include "Scene.hpp"

uint32_t count = 0;

using namespace std;
int main(int argc, char **argv) {

  Scene scene;
  // Sphere s(4.0f, 18, 36);
  // scene.add(&s);

  scene.camera.setPosition(glm::vec3(0.0f, 4.0f, 13.0f));

  // CylinderEx c(1.0f, 0.2f, 8.0f, RADIANS(0.0f), RADIANS(0.0f));
  CylinderEx c(1.0f, 0.2f, 8.0f, RADIANS(0.0f), RADIANS(60.0f));
  c.setHNum(16);
  c.update();

  scene.add("c1", &c,
            Transform({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, RADIANS(0.0f)));

  scene.mainloop();
  return 0;
}
