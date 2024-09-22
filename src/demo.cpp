#include <iostream>
#include <cassert>

#include "Scene.hpp"

using namespace std;
int main(int argc, char **argv){

    std::cout << "hello world" <<std::endl;
    Scene scene;
    std::cout << "instantiate" <<std::endl;
    scene.init();
    //// assert( && "scene init error");

    scene.mainloop();

    return 0;
}
