#define ENABLE_NORMAL_VISUALIZATION
// #define ENBALE_POLYGON_VISUALIZATION
#include "Scene.hpp"

uint32_t count = 0;

using namespace std;
int main(int argc, char **argv) {

  Scene scene;

  scene.camera.setPosition(glm::vec3(0.0f, 4.0f, 13.0f));
  scene.light.color = {1.0f,0.3f,0.3f};
  scene.light.position = {1.0f, 7.0f, 2.0f};

  // CylinderEx c(1.0f, 0.2f, 8.0f, RADIANS(0.0f), RADIANS(0.0f));
  CylinderEx c1(2.0f, 0.5f, 6.0f, RADIANS(0.0f), RADIANS(00.0f), 8, 16, 36);
  Sphere s1(2.0f, 36, 72);
  Plane ground(20.0f, 15.0f);

  scene.add("c1", &c1);
  scene.add("s1", &s1, {4.0f, 2.0f, -6.0f});
  scene.add("ground", &ground);

  scene.mainloop();
  return 0;
}
