// #define ENABLE_NORMAL_VISUALIZATION
// #define ENBALE_POLYGON_VISUALIZATION
#include "Scene.hpp"
#include <memory>


uint32_t count = 0;

using namespace std;
int main(int argc, char** argv) {

  try {
    Scene scene;

    scene.showAxis();
    scene.camera.setPosition(glm::vec3(-12.0f, 30.0f, 20.0f));
    scene.camera.lookAt({0.0f, 4.0f, 0.0f});
    scene.light.color    = {0.6f, 0.6f, 0.6f};
    scene.light.position = {1.0f, 7.0f, 2.0f};

    // scene.add("TruncatedConeEx",
    //           make_shared<TruncatedConeEx>(2.0f, 0.5f, 6.0f, 0.0f, 0.0f),
    //           {-4.0f, 0.0f, -6.0f});


    // scene.add("Sphere", make_shared<Sphere>(2.0f, 36, 72), {4.0f, 2.0f, -6.0f});
    // scene.add("Cone", make_shared<Cone>(2.0f, 3.0f), {0.0f, 0.0f, 6.0f});
    // scene.add("Cylinder", make_shared<Cylinder>(2.5f, 5.0f), {5.0f, 0.0f, 0.0f});


    scene.add("Sphere", Mesh::Sphere(2.0f, 72, 36), {-4.0f, 2.0f, 6.0f});
    scene.add("Cube", CompositeMesh::Cube(10.0f, 7.0f, 4.0f, 10, 7, 4), {4.0f, 2.0f, -6.0f});
    scene.add("Cylinder", CompositeMesh::Cylinder(2.5f, 5.0f, 72, 10, 20), {5.0f, 0.0f, 0.0f});
    scene.add("Cone", CompositeMesh::Cone(2.0f, 3.0f, 72, 10, 20), {0.0f, 0.0f, 6.0f});

    // for(auto &[name, obj]: scene.objs){
    //   obj->constructBvhTree();
    // }

    scene.mainloop();
  }
  catch (std::exception e) {
    cout << e.what() << endl;
  }
  return 0;
}
