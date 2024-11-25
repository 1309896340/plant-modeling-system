// #define ENABLE_NORMAL_VISUALIZATION
// #define ENBALE_POLYGON_VISUALIZATION
#include "Scene.hpp"

#include "GeometryGenerator.hpp"
#include "LSystem.hpp"
#include "lexy/input/string_input.hpp"
// #include "lexy/visualize.hpp"
#include "lexy_ext/report_error.hpp"

// 这里尝试将LSystem模块嵌入到当前项目中

std::mt19937_64 rdgen;

using namespace std;
int main(int argc, char** argv) {

    Scene scene;

    //   scene.showAxis();
    scene.camera.lookAt({0.0f, 4.0f, 0.0f});
    scene.light.color = {0.6f, 0.6f, 0.6f};


    //   vector<string> productions = {
    //     "S(r, h) -> C(r,h) [RZ(30)RY(90)S(r, h*0.8)] [RZ(-30)RY(90)S(r, h*0.8)]"
    //   };


    //   LSystem::D0L_System lsys("S(0.03, 3)", productions);

    //   string lsys_cmds = lsys.next(4);
    // cout << lsys_cmds << endl;

    // // 生成，渲染
    // auto s_input = lexy::zstring_input(lsys_cmds.c_str());
    // auto res     = lexy::parse<GeometryGenerator::grammar::GraphicsStructure>(s_input, lexy_ext::report_error);
    // assert(res.is_success());
    // const GeometryGenerator::config::GraphicsStructure& gs       = res.value();
    // shared_ptr<Skeleton>                                skeleton = gs.construct();
    // scene.add("skeleton", skeleton, Transform{vec3(0.5f, 0.03f, 0.5f)});

    scene.mainloop();
    return 0;
}
