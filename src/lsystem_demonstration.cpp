// #define ENABLE_NORMAL_VISUALIZATION
// #define ENBALE_POLYGON_VISUALIZATION
#include "Scene.hpp"

#include "GeometryGenerator.hpp"
#include "LSystem.hpp"
#include "lexy/input/string_input.hpp"
// #include "lexy/visualize.hpp"
#include "lexy_ext/report_error.hpp"

// 这里尝试将LSystem模块嵌入到当前项目中

using namespace std;
int main(int argc, char** argv) {

    Scene scene;

    //   scene.showAxis();
    scene.camera.lookAt({0.0f, 4.0f, 0.0f});
    scene.light.color = {0.6f, 0.6f, 0.6f};


    // 调试
    LSystem::D0L_System lsys("F(2)", "F(x) -> F(x*0.5) C(x+1) F(2*x)");
    string a = lsys.next();
    cout << a << endl;

    // scene.mainloop();
    return 0;
}
