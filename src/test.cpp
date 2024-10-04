#include <exception>
#include <iostream>
#include <memory>

#include "Auxiliary.hpp"
#include "Scene.hpp"
#include "Skeleton.hpp"
using namespace std;

int main(void) {

  try {

    // Skeleton a;
    // SkNode *n1 = new SkNode, *n2 = new SkNode, *n3 = new SkNode,
    //        *n4 = new SkNode, *n5 = new SkNode, *n6 = new SkNode;
    // a.root = n1;
    // n1->addChild(n2);

    // ================================================================================

    Scene scene;
    scene.camera.setPosition({4.0f, 5.0f, 13.0f});
    scene.camera.lookAt({0.0f, 3.0f, 0.0f});
    scene.light.position = {0.0, 8.0f, 0.0f};

    scene.add("Cylinder", make_shared<Cylinder>(0.05f, 1.0f),
              Transform(glm::vec3(2.0f, 0.0f, 2.0f)));

    scene.mainloop();

  } catch (exception e) {
    cerr << e.what() << endl;
  }

  return 0;
}
