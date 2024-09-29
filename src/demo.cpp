#include "glm/trigonometric.hpp"
#define ENABLE_NORMAL_VISUALIZATION
// #define ENBALE_POLYGON_VISUALIZATION
#include "Scene.hpp"

uint32_t count = 0;

using namespace std;
int main(int argc, char **argv) {

  Scene scene;

  scene.showAxis();
  scene.camera.setPosition(glm::vec3(-12.0f, 30.0f, 20.0f));
  scene.camera.lookAt({0.0f, 4.0f, 0.0f});
  scene.light.color = {0.6f, 0.6f, 0.6f};
  scene.light.position = {1.0f, 7.0f, 2.0f};

  TruncatedConeEx c1(2.0f, 0.5f, 6.0f, RADIANS(0.0f), RADIANS(0.0f), 8, 16, 36);
  Sphere s1(2.0f, 36, 72);
  Plane ground(24.0f, 24.0f);

  Cone c2(2.0f, 3.0f);
  Cylinder c3(2.5f, 5.0f);

  scene.add("TruncatedConeEx", &c1, {-4.0f, 0.0f, -6.0f});
  scene.add("Sphere", &s1, {4.0f, 2.0f, -6.0f});
  scene.add("Cone", &c2, {0.0f, 0.0f, 6.0f});
  scene.add("Cylinder", &c3, {5.0f, 0.0f, 0.0f});

  scene.add("Ground", &ground);

  scene.mainloop();
  return 0;
}
