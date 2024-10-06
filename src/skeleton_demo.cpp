#include <exception>
#include <iostream>
#include <memory>

#include "Auxiliary.hpp"
#include "Scene.hpp"
#include "Skeleton.hpp"

using namespace std;

int main(void) {

  try {

    Skeleton a;
    SkNode *n1 = new SkNode, *n2 = new SkNode, *n3 = new SkNode,
           *n4 = new SkNode, *n5 = new SkNode, *n6 = new SkNode;
    a.root = n1;
    n1->addChild(n2);
    n2->addChild(n3);
    n3->addChild(n4);

    // n1->setPosition({1.2f,0.0f,1.2f});

    n2->setAttitude(glm::radians(30.0f), _front);
    n2->setPosition({0.0f, 1.0f, 0.0f});
    
    n3->setAttitude(glm::radians(45.0f), _right);
    n3->setPosition({0.0f, 1.0f, 0.0f});
    
    n4->setAttitude(glm::radians(45.0f), _front);
    n4->setPosition({0.0f, 1.0f, 0.0f});

    a.update();

    // ================================================================================

    Scene scene;
    scene.hideAxis();
    scene.camera.setPosition({4.0f, 5.0f, 13.0f});
    scene.camera.lookAt({0.0f, 3.0f, 0.0f});
    scene.light.position = {0.0, 8.0f, 0.0f};

    scene.add("Axis1", make_shared<CoordinateAxis>(), n1->getAbsTransform());
    scene.add("Axis2", make_shared<CoordinateAxis>(), n2->getAbsTransform());
    scene.add("Axis3", make_shared<CoordinateAxis>(), n3->getAbsTransform());
    scene.add("Axis4", make_shared<CoordinateAxis>(), n4->getAbsTransform());


    // scene.add("obj", make_shared<Cylinder>(0.4f, 1.0f));


    scene.mainloop();

  } catch (exception e) {
    cerr << e.what() << endl;
  }

  return 0;
}
