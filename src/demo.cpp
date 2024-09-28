#define ENABLE_NORMAL_VISUALIZATION
// #define ENBALE_POLYGON_VISUALIZATION
#include "Scene.hpp"

uint32_t count = 0;

using namespace std;
int main(int argc, char **argv) {

  Scene scene;

  scene.camera.setPosition(glm::vec3(0.0f, 4.0f, 13.0f));
  scene.light.color = {1.0f, 0.3f, 0.3f};
  scene.light.position = {1.0f, 7.0f, 2.0f};

  // CylinderEx c(1.0f, 0.2f, 8.0f, RADIANS(0.0f), RADIANS(0.0f));
  CylinderEx c1(2.0f, 0.5f, 6.0f, RADIANS(0.0f), RADIANS(00.0f), 8, 16, 36);
  Sphere s1(2.0f, 36, 72);
  Plane ground(24.0f, 24.0f);

  // CylinderSide cs1(3.2f,4.0f);
  // ConeSide cs2(2.2f,6.0f);

  Cone c2(2.0f, 3.0f);
  Cylinder c3(2.5f, 5.0f);

  scene.add("CylinderEx", &c1, {-4.0f, 0.0f, -6.0f});
  scene.add("Sphere", &s1, {4.0f, 2.0f, -6.0f});
  // scene.add("cs1", &cs1, {4.0f, 0.0f, 6.0f});
  // scene.add("cs2", &cs2, {-6.0f, 0.0f, 6.0f});
  scene.add("Cone", &c2, {0.0f, 0.0f, 6.0f});
  scene.add("Cylinder", &c3);
  scene.add("ground", &ground);

  scene.mainloop();
  return 0;
}
