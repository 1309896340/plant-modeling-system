#include "Scene.hpp"
#include "glm/fwd.hpp"



using namespace std;
int main(int argc, char **argv) {

  Scene scene;
  // Sphere s(4.0f, 18, 36);
  // scene.add(&s);

  scene.camera.setPosition(glm::vec3(0.0f, 12.0f, 12.0f));
  scene.camera.lookAt(glm::vec3(0.0f,4.0f,0.0f));
  scene.camera.move_relative(glm::vec3(0.0f,4.0f,-6.0f));
  // scene.camera.lookAt(glm::vec3(0.0f,4.0f,0.0f));


  CylinderEx c(2.0f, 0.5f, 6.0f, ANGLE(30.0f), ANGLE(45.0f));
  scene.add(&c,Transform({0.0f,0.0f,0.0f},{0.0f,0.0f,1.0f},ANGLE(30.0f)));

  scene.mainloop();
  return 0;
}
