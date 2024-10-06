#include <iostream>

// 计算着色器初步使用

#include "Scene.hpp"

using namespace std;

int main(void) {


  Scene scene;

  Shader *cshader = new Shader("compute.comp");
  cshader->use();

  cout << "hello world" << endl;


  return 0;
}
