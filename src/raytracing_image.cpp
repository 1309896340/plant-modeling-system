// #define ENABLE_NORMAL_VISUALIZATION
// #define ENABLE_BOUNDINGBOX_VISUALIZATION
#include <memory>

#include <stb_image.h>
#include <stb_image_write.h>

#include "Scene.hpp"

std::mt19937_64 rdgen(42);

vec3 screen2world(vec2 pos, Scene &scene) {
  mat4 view = scene.camera.getView();
  auto [fov, near, far, aspect] = scene.camera.getProperties();
  vec4 target_dir = vec4(glm::normalize(vec3(
                             pos.x * near * tanf(fov / 2.0f) * aspect,
                             pos.y * near * tanf(fov / 2.0f),
                             near)),
                         0.0f);
  vec4 world_dir = view * target_dir;
  world_dir.z = -world_dir.z;
  return vec3(world_dir);
}

void screen_ray_cast(Scene &scene) {
  const int width = 1600;
  const int height = 1200;
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      float y = static_cast<float>(i) * 2.0f / height - 1.0f;
      float x = static_cast<float>(j) * 2.0f / width - 1.0f;
      // (x, y) 屏幕坐标，转换为世界坐标
    }
  }
}

using namespace std;
int main(int argc, char **argv) {

  Scene scene;

  scene.showAxis();
  scene.camera.setPosition(glm::vec3(0.0f, 10.0f, 15.0f));
  scene.camera.lookAt({0.0f, 2.0f, 0.0f});
  scene.light.color = {0.6f, 0.6f, 0.6f};
  scene.light.position = {1.0f, 7.0f, 2.0f};

  scene.add("TruncatedConeEx",
            make_shared<TruncatedConeEx>(2.0f, 0.5f, 6.0f, 0.0f, 0.0f),
            {-4.0f, 0.0f, -6.0f});
  scene.add("Sphere", make_shared<Sphere>(2.0f, 18, 36), {0.0f, 2.0f, 0.0f});
  scene.add("Cone", make_shared<Cone>(2.0f, 3.0f), {0.0f, 0.0f, 6.0f});
  scene.add("Cylinder", make_shared<Cylinder>(2.5f, 5.0f), {5.0f, 0.0f, 0.0f});

  // 创建Sphere对象的层次包围盒
  for (auto &obj : scene.objs)
    obj.second->constructBvhTree();

  screen_ray_cast(scene);

  scene.mainloop();
  return 0;
}

// void ray_cast_test(Scene *scene) {
//   // 没有确定是否采用这种方式进行计算

//   // 无关于OpenGL渲染，实现CPU端的仿真计算，仅考虑Sphere形体
//   const shared_ptr<GeometryRenderObject> obj = scene->objs["Sphere"];
//   const shared_ptr<Geometry> geom = obj->geometry;
//   // 注意：这里的geom处于模型空间，未变换到世界坐标系
//   const float sample_width = 20.0f;
//   const float sample_height = 20.0f;
//   const uint32_t sample_w_num = 100;
//   const uint32_t sample_h_num = 100;
//   const float dw = sample_width / sample_w_num;
//   const float dh = sample_height / sample_h_num;
//   vec3 lightPos = {0.0f, 4.0f, 4.0f};
//   vec3 lightDir = glm::normalize(obj->transform.getPosition() - lightPos);
//   vec3 right = glm::cross(lightDir, _up);
//   vec3 up = glm::cross(right, lightDir);
//   for (uint32_t i = 0; i < sample_w_num; i++) {
//     for (uint32_t j = 0; j < sample_h_num; j++) {
//       vec3 lightSamplePos =
//           (-sample_width / 2.0f + dw / 2.0f + i * dw) * right +
//           (sample_height / 2.0f - dh / 2.0f - j * dh) * up;
//       // 以lightSamplePos为起点，以lightDir为方向，对几何目标进行轰击
//     }
//   }
// }
