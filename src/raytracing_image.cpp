// #define ENABLE_NORMAL_VISUALIZATION
// #define ENABLE_BOUNDINGBOX_VISUALIZATION
#include <cstdint>
#include <memory>

#include <stb_image.h>
#include <stb_image_write.h>

#include "Scene.hpp"


vec3 screen2world(vec2 pos, Scene &scene) {
    pos = - pos;
    mat4 view = scene.camera.getView();
    auto [fov, near, far, aspect] = scene.camera.getProperties();
    vec4 target_dir = vec4(glm::normalize(vec3(
                               pos.x * near * tanf(fov / 2.0f) * aspect,
                               pos.y * near * tanf(fov / 2.0f),
                               near)),
                           0.0f);
    vec4 world_dir = glm::transpose(view) * target_dir;
    return -vec3(world_dir);
}

struct RayCastFrame {
  vector<vec3> radiance;
  uint32_t width{0};
  uint32_t height{0};
};

RayCastFrame screen_ray_cast_init(Scene &scene, int width = 1600, int height = 1200) {
  RayCastFrame frame;
  frame.radiance.resize(width * height, vec3(0.0f, 0.0f, 0.0f));
  frame.width = width;
  frame.height = height;
  return frame;
}

void screen_ray_cast(Scene &scene, RayCastFrame &frame, float PR, uint32_t count) {
  for (int i = 0; i < frame.height; i++) {
    for (int j = 0; j < frame.width; j++) {
      float y = 1.0f - static_cast<float>(i) * 2.0f / frame.height;
      float x = static_cast<float>(j) * 2.0f / frame.width - 1.0f;
      // (x, y) 屏幕坐标，转换为世界坐标系下的方向向量
      vec3 dir = screen2world({x, y}, scene);
      HitInfo obj = scene.hit_obj({scene.camera.getPosition(), dir});
      if (!obj.isHit)
        continue;
      vec3 res = scene.trace_ray({obj.hitPos, -dir}, obj, PR);
      vec3 out = (static_cast<float>(count) * frame.radiance[i * frame.width + j] + res) / static_cast<float>(count + 1);
      out = glm::clamp(out, 0.0f, 1.0f);
      frame.radiance[i * frame.width + j] = out;
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
    obj->constructBvhTree();

  RayCastFrame frame = screen_ray_cast_init(scene, 1600, 1200);
  for (int k = 0; k < 30; k++) {
    screen_ray_cast(scene, frame, 0.92f, k);
    printf("epoch=%d\n", k);
  }
  vector<uint8_t> pixels(frame.width * frame.height * 3);
  for (int k = 0; k < frame.radiance.size(); k++) {
    pixels[k * 3] = static_cast<uint8_t>(frame.radiance[k].r * 256.0f);
    pixels[k * 3 + 1] = static_cast<uint8_t>(frame.radiance[k].g * 256.0f);
    pixels[k * 3 + 2] = static_cast<uint8_t>(frame.radiance[k].b * 256.0f);
  }
  stbi_write_jpg("output.jpg", frame.width, frame.height, 3, pixels.data(), 0);

  printf("渲染完毕\n");

  // scene.mainloop();
  return 0;
}
