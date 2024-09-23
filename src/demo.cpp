#include "Scene.hpp"

using namespace std;
int main(int argc, char **argv) {

  Scene scene;
  Sphere s(4.0f, 18, 36);
  scene.add(&s);

  scene.mainloop();
  return 0;
}
