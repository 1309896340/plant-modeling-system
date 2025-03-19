// 计算着色器初步使用

#include "Scene.h"
#include "Shader.h"


using namespace std;

GLuint buffer{0};

void init_ubo() {
  float fdata[8] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * 8, fdata,
               GL_DYNAMIC_COPY);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);
}

int main(void) {

  Scene::Scene scene;

  init_ubo();

  Shader *cshader = new Shader("compute.comp");
  cshader->use();

  glDispatchCompute(2, 4, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  // 读取计算结果
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
  float *map_data = (float *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 8*sizeof(float), GL_MAP_READ_BIT);
  for(int i=0;i<8;i++)
    printf("%.2f\n", map_data[i]);
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

  return 0;
}
