// #define ENABLE_NORMAL_VISUALIZATION
// #define ENBALE_POLYGON_VISUALIZATION
#include "Scene.h"

// 此处演示的示例为
// string production = "S(r, h) -> C(r,h) [RZ(30)RY(90)S(r, h*0.8)] "
// "[RZ(-30)RY(90)S(r, h*0.8)]";
// string axiom      = "S(0.03, 3)";

using namespace std;
int main(int argc, char** argv) {
  try {
    Scene::Scene scene;

    //   scene.showAxis();
    scene.camera.lookAt({0.0f, 4.0f, 0.0f});
    scene.light.color = {0.6f, 0.6f, 0.6f};


    

    scene.mainloop();
  }
  catch (const exception& e) {
    cout << e.what() << endl;
  }

  return 0;
}
