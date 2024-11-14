﻿// #define ENABLE_NORMAL_VISUALIZATION
// #define ENBALE_POLYGON_VISUALIZATION
#include "Scene.hpp"

#include "GeometryGenerator.hpp"
#include "LSystem.hpp"
#include "lexy/input/string_input.hpp"
#include "lexy/visualize.hpp"
#include "lexy_ext/report_error.hpp"

// 这里尝试将LSystem模块嵌入到当前项目中

std::mt19937_64 rdgen;

uint32_t count = 0;

using namespace std;
int main(int argc, char **argv) {

  Scene scene;

  scene.showAxis();
  scene.camera.setPosition(glm::vec3(-12.0f, 30.0f, 20.0f));
  scene.camera.lookAt({0.0f, 4.0f, 0.0f});
  scene.light.color = {0.6f, 0.6f, 0.6f};
  scene.light.position = {1.0f, 7.0f, 2.0f};

  // scene.add("TruncatedConeEx",
  //           make_shared<TruncatedConeEx>(2.0f, 0.5f, 6.0f, 0.0f, 0.0f),
  //           {-4.0f, 0.0f, -6.0f});
  // scene.add("Sphere", make_shared<Sphere>(2.0f, 36, 72), {4.0f, 2.0f, -6.0f});
  // scene.add("Cone", make_shared<Cone>(2.0f, 3.0f), {0.0f, 0.0f, 6.0f});
  // scene.add("Cylinder", make_shared<Cylinder>(2.5f, 5.0f), {5.0f, 0.0f, 0.0f});

  vector<string> productions = {
    "Cylinder(r, h) -> Cylinder(r,h*0.8) [RZ(30)RY(90)Sphere(r)] [RZ(-30)RY(90)Sphere(r)]"
  };

  LSystem::D0L_System lsys("Cylinder(0.05, 1)", productions);

  string lsys_cmds = lsys.next(1);
  cout << lsys_cmds << endl;

  // 测试结构生成结果，并进行可视化测试
  auto s_input = lexy::zstring_input(lsys_cmds.c_str());
  auto res = lexy::parse<GeometryGenerator::grammar::GraphicsStructure>(s_input, lexy_ext::report_error);
  assert(res.is_success());
  const GeometryGenerator::config::GraphicsStructure &gs = res.value();
  shared_ptr<Skeleton> skeleton = gs.construct();
  scene.add("skeleton", skeleton, Transform{vec3(0.0f,0.0f,0.0f)});


  
  // auto s_input = lexy::zstring_input(lsys_cmds.c_str());
  // lexy::parse_tree_for<decltype(s_input)> tree;
  // auto result = lexy::parse_as_tree<GeometryGenerator::grammar::GraphicsStructure>(tree, s_input, lexy_ext::report_error);
  // lexy::visualize(stdout, tree, {lexy::visualize_fancy});


  scene.mainloop();
  return 0;
}
