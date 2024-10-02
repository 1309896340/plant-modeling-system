﻿#include <algorithm>
#include <exception>
#include <iostream>

#include "Scene.hpp"

#include "Skeleton.hpp"
#include "constants.h"
#include "glm/trigonometric.hpp"

using namespace std;

// 创建坐标系的三个箭头
void add_axis(string prefix, Scene &scene, const Transform &transform) {
  Arrow a_x(0.06f, 1.0f);
  Arrow a_y(0.06f, 1.0f);
  Arrow a_z(0.06f, 1.0f);

  a_x.rotate(glm::radians(90.0f), _front);
  a_z.rotate(glm::radians(90.0f), _right);

  a_x.setColor(1.0f, 0.0f, 0.0f);
  a_y.setColor(0.0f, 1.0f, 0.0f);
  a_z.setColor(0.0f, 0.0f, 1.0f);

  FixedGeometry *res = new FixedGeometry(FixedGeometry(a_x) + a_y + a_z);

  scene.add(prefix, res);
}

int main(void) {

  try {

    Skeleton a;
    SkNode *n1 = new SkNode, *n2 = new SkNode, *n3 = new SkNode,
           *n4 = new SkNode, *n5 = new SkNode, *n6 = new SkNode;
    a.root = n1;
    n1->addChild(n2);

    // 调整姿态，但是姿态不影响偏移
    // n1->transform.rotate(glm::radians(30.0f), {0.0, 0.0f, 1.0f}); //
    // 顺时针30° n1->transform.translate({0.0f, 1.0f, 0.0f}); // 向上1单位

    // n2->transform.rotate(glm::radians(30.0f), {0.0, 0.0f, 1.0f}); //
    // 顺时针30° n2->transform.rotate(glm::radians(90.0f), {0.0, 1.0f, 0.0f});
    // n2->transform.translate({0.0f, 1.0f, 0.0f}); // 向上1单位

    Scene scene;
    // scene.showAxis();
    scene.camera.setPosition({4.0f, 5.0f, 13.0f});
    scene.camera.lookAt({0.0f, 3.0f, 0.0f});
    scene.light.position = {0.0, 8.0f, 0.0f};

    Plane ground(10.0f, 10.0f);
    scene.add("Ground", &ground);

    // Transform transform;
    // transform.translate({0.0f, 1.0f, 0.0f});

    // Arrow arrow1(0.06f, 0.6f);
    // arrow1.setColor(1.0f, 1.0f, 0.0f);
    // scene.add("Arrow1", &arrow1, transform);

    // transform.rotate(glm::radians(45.0f), {0.0f, 0.0f, 1.0f});

    // Arrow arrow2(0.06f, 0.6f);
    // arrow2.setColor(1.0f, 0.0f, 1.0f);
    // scene.add("Arrow2", &arrow2, transform);

    // transform.rotate(glm::radians(90.0f), {0.0f, 1.0f, 0.0f});

    // Arrow arrow3(0.06f, 0.6f);
    // arrow3.setColor(0.0f, 1.0f, 0.0f);
    // scene.add("Arrow3", &arrow3, transform);

    add_axis("axis",scene,Transform());

    scene.mainloop();

  } catch (exception e) {
    cerr << e.what() << endl;
  }

  return 0;
}
