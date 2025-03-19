#include "Scene.h"

#include "GeometryInterpreter.h"
#include "LSysConfig.h"
#include "constants.h"

#include <filesystem>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

using namespace std;

mt19937_64 rdgen;

namespace fs = filesystem;

Scene::LineDrawer::LineDrawer() {
  glGenVertexArrays(1, &this->vao);
  glGenBuffers(1, &this->vbo);
}

void Scene::LineDrawer::addLine(glm::vec3 pt1, glm::vec3 pt2, glm::vec3 color) {
  vector<glm::vec3> minibuf(2);
  minibuf[0] = pt1;
  minibuf[1] = pt2;
  this->addPolygon(minibuf, color);
}

void Scene::LineDrawer::addLine(glm::vec3 pt1, glm::vec3 pt2) {
  vector<glm::vec3> minibuf(2);
  minibuf[0] = pt1;
  minibuf[1] = pt2;
  this->addPolygon(minibuf);
}

void Scene::LineDrawer::addPolygon(const vector<glm::vec3>& vert) {
  this->rays.insert(this->rays.end(), vert.begin(), vert.end());
  this->v_nums.push_back(vert.size());
  uniform_real_distribution<> distr(0.1f, 0.9f);
  glm::vec3                   color = glm::vec3(distr(rdgen), distr(rdgen), distr(rdgen));
  this->colors.push_back(color);
}

void Scene::LineDrawer::addPolygon(const vector<glm::vec3>& vert, glm::vec3 color) {
  this->rays.insert(this->rays.end(), vert.begin(), vert.end());
  this->v_nums.push_back(vert.size());
  this->colors.push_back(color);
}

void Scene::LineDrawer::clear() {
  this->rays.clear();
  this->v_nums.clear();
  this->colors.clear();
}
void Scene::LineDrawer::update() {
  glBindVertexArray(this->vao);
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glBufferData(GL_ARRAY_BUFFER, this->rays.size() * sizeof(glm::vec3), this->rays.data(), GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glEnableVertexAttribArray(0);
}

size_t Scene::LineDrawer::size() const {
  // 返回线条数量
  return this->v_nums.size();
}

void Scene::LineDrawer::draw(Shader* sd) {
  glBindVertexArray(this->vao);
  uint32_t offset = 0u;
  for (int i = 0; i < this->v_nums.size(); i++) {
    sd->set("lineColor", this->colors[i]);
    glDrawArrays(GL_LINE_STRIP, offset, v_nums[i]);
    offset += v_nums[i];   // 累加上第i段的顶点数
  }
}
Scene::LineDrawer::~LineDrawer() {
  glDeleteVertexArrays(1, &this->vao);
  glDeleteBuffers(1, &this->vbo);
}

Scene::Pixel Scene::cubemap_sample(PngImage* cubmaps, glm::vec3 dir) {
  dir = glm::normalize(dir);
  // cubmaps顺序 px nx py ny pz nz
  uint8_t map_idx{0};

  float theta = acosf(glm::dot(dir, _up));
  float phi   = atan2(-dir.z, dir.x);
  float x{0.0f}, y{0.0f};
  if (theta < PI / 4.0f) {
    // 上面, py
    float r = sqrtf(2) * sin(theta);
    x       = r * sqrtf(2) * cos(phi);
    y       = -r * sqrtf(2) * sin(phi);
    map_idx = 2;
  }
  else if (theta >= 3.0f * PI / 4.0f) {
    // 下面, ny
    float r = sqrtf(2) * sin(theta);
    x       = r * sqrtf(2) * cos(phi);
    y       = r * sqrtf(2) * sin(phi);
    map_idx = 3;
  }
  else {
    if (phi >= 3.0f * PI / 4.0f || phi < -3.0f * PI / 4.0f) {
      // 左面, nx
      y       = sqrtf(2) * cos(theta);
      x       = sqrtf(2) * sin(phi);
      map_idx = 1;
    }
    else if (phi >= -PI / 4.0f && phi < PI / 4.0f) {
      // 右面, px
      y       = sqrtf(2) * cos(theta);
      x       = -sqrtf(2) * sin(phi);
      map_idx = 0;
    }
    else if (phi >= PI / 4.0f && phi < 3.0f * PI / 4.0f) {
      // 前面, nz
      y       = sqrtf(2) * cos(theta);
      x       = sqrtf(2) * cos(phi);
      map_idx = 4;
    }
    else {
      // 后面, pz
      y       = sqrtf(2) * cos(theta);
      x       = -sqrtf(2) * cos(phi);
      map_idx = 5;
    }
  }
  x = max(x, -1.0f);
  x = min(x, 1.0f);
  y = max(y, -1.0f);
  y = min(y, 1.0f);

  // 将(x,y)映射到图片像素上
  uint32_t width   = cubmaps[map_idx].width;
  uint32_t height  = cubmaps[map_idx].height;
  uint32_t row_idx = static_cast<uint32_t>(height * (0.5f - y / 2.0f));
  uint32_t col_idx = static_cast<uint32_t>(width * (0.5f + x / 2.0f));

  row_idx = min(row_idx, height - 1);
  col_idx = min(col_idx, width - 1);

  return cubmaps[map_idx].img[col_idx + row_idx * width];
}

Scene::TriangleSampler::TriangleSampler(const vector<Vertex>& vertices,
                                        const Surface&        surface,
                                        glm::mat4             model) {
  for (int i = 0; i < 3; i++)
    pt[i] = glm::vec3(
      model *
      glm::vec4(glm::make_vec3(vertices[surface.tidx[i]].position), 1.0f));
}
Scene::TriangleSampler::TriangleSampler(const vector<Vertex>& vertices,
                                        const Surface&        surface) {
  for (int i = 0; i < 3; i++)
    pt[i] = glm::make_vec3(vertices[surface.tidx[i]].position);
}

float Scene::TriangleSampler::calcArea() const {
  return glm::length(glm::cross(pt[2] - pt[0], pt[1] - pt[0]));
}

glm::vec3 Scene::TriangleSampler::calcNorm() const {
  return glm::normalize(glm::cross(pt[2] - pt[0], pt[1] - pt[0]));
}

glm::vec3 Scene::TriangleSampler::calcCenter() const {
  return (pt[0] + pt[1] + pt[2]) / 3.0f;
}

// 以法向量为上方向基准，结合pt[2]-pt[1]、pt[1]-pt[0]所成平面构建局部坐标系
tuple<glm::vec3, glm::vec3, glm::vec3>
Scene::TriangleSampler::calcLocalCoord() const {
  glm::vec3 up    = this->calcNorm();
  glm::vec3 right = glm::normalize(glm::cross(pt[1] - pt[0], up));
  glm::vec3 back  = glm::normalize(glm::cross(right, up));
  return make_tuple(right, up, back);
}

// 半球均匀分布随机方向采样
glm::vec3 Scene::TriangleSampler::hemisphereSampleDir() const {

  uniform_real_distribution<> distr(0.0f, 1.0f);

  auto [right, up, back] = this->calcLocalCoord();
  float theta            = acos(1.0f - distr(rdgen));
  float phi              = 2.0f * PI * distr(rdgen);
  float x                = sin(theta) * cos(phi);
  float z                = sin(theta) * sin(phi);
  float y                = cos(theta);
  return x * right + y * up + z * back;
}

Scene::BoundingBoxContext::BoundingBoxContext(BoundingBox* box,
                                              Transform*   transform)
  : OpenGLContext()
  , transform(transform)
  , box(box) {}
tuple<vector<glm::vec3>, vector<uint32_t>>
Scene::BoundingBoxContext::genOpenGLRawData() {
  glm::vec3         max_xyz  = this->box->max_bound;
  glm::vec3         min_xyz  = this->box->min_bound;
  vector<glm::vec3> vertices = {min_xyz,
                                {min_xyz.x, min_xyz.y, max_xyz.z},
                                {min_xyz.x, max_xyz.y, min_xyz.z},
                                {min_xyz.x, max_xyz.y, max_xyz.z},
                                {max_xyz.x, min_xyz.y, min_xyz.z},
                                {max_xyz.x, min_xyz.y, max_xyz.z},
                                {max_xyz.x, max_xyz.y, min_xyz.z},
                                max_xyz};
  glm::mat4         transMat = this->transform->getModel();
  for (uint32_t i = 0; i < vertices.size(); i++)
    vertices[i] = glm::vec3(transMat * glm::vec4(vertices[i], 1.0f));
  vector<uint32_t> indices = {0, 1, 0, 2, 0, 4, 1, 3, 1, 5, 2, 3, 2, 6, 4, 6, 4, 5, 3, 7, 5, 7, 6, 7};
  return make_tuple(vertices, indices);
}
void Scene::BoundingBoxContext::init() {
  auto [vertices, indices] = genOpenGLRawData();

  glBindVertexArray(this->vao);

  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

  this->drawSize = indices.size();
}
void Scene::BoundingBoxContext::update() {
  auto [vertices, indices] = genOpenGLRawData();

  glBindVertexArray(this->vao);
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(glm::vec3), vertices.data());
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(uint32_t), indices.data());
}

string Component::GeometryObject::getName() const {
  return this->name;
}
BvhTree* Component::GeometryObject::getBvhTree() {
  return this->bvhtree.get();
}
BoundingBox* Component::GeometryObject::getBoundingBox() {
  return this->box.get();
}
Geometry* Component::GeometryObject::getGeometry() {
  return this->geometry.get();
}

// Scene::GeometryObject::GeometryObject(string name,
//                                       shared_ptr<Geometry> geometry,
//                                       Transform transform = Transform{},
//                                       bool useBvh = false);

void Component::GeometryObject::update() {
  // 1. 组件更新
  // geometry更新(由imgui激活对应参数的观察者实现)

  // bangdingbox更新
  this->box->update(this->geometry->getVertices());
  // bvhtree更新
  if (this->bvhtree != nullptr) {
    this->bvhtree->update();
  }
}
Component::GeometryObject::~GeometryObject() {
  printf("销毁GeometryObject: \"%s\" geometry: %p  box: %p  bvhtree: %p\n",
         this->name.c_str(),
         this->geometry.get(),
         this->box.get(),
         this->bvhtree.get());
}

Component::GeometryObject::GeometryObject(string               name,
                                      shared_ptr<Geometry> geometry,
                                      Transform            transform,
                                      bool                 useBvh)
  : name(name)
  , geometry(geometry)
  , transform(transform) {
  // 构造普通包围盒
  this->box = make_unique<BoundingBox>(geometry->getVertices());
  if (useBvh) {
    this->bvhtree = make_unique<BvhTree>(geometry.get());
    this->bvhtree->construct();
  }
  printf("创建GeometryObject: \"%s\" geometry: %p  box: %p  bvhtree: %p\n",
         this->name.c_str(),
         this->geometry.get(),
         this->box.get(),
         this->bvhtree.get());
}

Scene::GeometryContext::GeometryContext(Component::GeometryObject* obj)
  : OpenGLContext()
  , obj(obj) {
  BoundingBox* bbox = this->obj->getBoundingBox();
  assert(bbox != nullptr);
  this->box     = make_unique<BoundingBoxContext>(bbox, &obj->transform);
  this->bvhtree = this->obj->getBvhTree();
}

void Scene::GeometryContext::init() {
  if (this->obj == nullptr) {
    cerr << "GeometryContext没有初始化有效的GeometryObject实例!" << endl;
    return;
  }
  Geometry* geometry = this->obj->getGeometry();

  assert(geometry != nullptr && "GeometryContext::init() geometry is null!");

  glBindVertexArray(this->vao);
  auto& vertices = geometry->getVertices();
  auto& surfaces = geometry->getSurfaces();
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

  size_t stride = sizeof(Vertex);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                        (void*)0);   // 位置
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                        (void*)(3 * sizeof(float)));   // 法向量
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
                        (void*)(6 * sizeof(float)));   // 颜色
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride,
                        (void*)(9 * sizeof(float)));   // 纹理坐标
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(3);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, surfaces.size() * sizeof(Surface), surfaces.data(), GL_STATIC_DRAW);
  glBindVertexArray(0);

  this->drawSize = surfaces.size() * 3;

  this->box->init();
  if (this->bvhtree != nullptr) {
    this->bvhtree->traverse([this](BvhNode* node) {
      auto box_context = make_unique<BoundingBoxContext>(node->box.get(),
                                                         &this->obj->transform);
      box_context->init();
      this->boxes.emplace_back(std::move(box_context));
    });
  }
}
void Scene::GeometryContext::update() {
  if (this->obj == nullptr) {
    cerr << "GeometryContext没有初始化有效的GeometryObject实例!" << endl;
    return;
  }

  Geometry* geometry = this->obj->getGeometry();
  assert(geometry != nullptr &&
         "GeometryRenderObjecy::update() geometry is null!");
  // 将geometry中的顶点属性更新到顶点缓冲区
  auto& vertices = geometry->getVertices();
  glBindVertexArray(this->vao);
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
  auto& surfaces = geometry->getSurfaces();
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, surfaces.size() * sizeof(Surface), surfaces.data(), GL_DYNAMIC_DRAW);
  glBindVertexArray(0);

  this->drawSize = surfaces.size() * 3;

  this->box->update();
  if (this->bvhtree != nullptr) {
    // 由于bvhtree的更新回重构所有BoundingBox，因此其对应context也只能全部重建
    this->boxes.clear();
    this->bvhtree->traverse([this](BvhNode* node) {
      auto box_context = make_unique<BoundingBoxContext>(node->box.get(),
                                                         &this->obj->transform);
      box_context->init();
      this->boxes.emplace_back(std::move(box_context));
    });
  }
}

void Scene::errorCallback(int code, const char* msg) {
  cerr << "errors occured! error code: " << code << endl;
  cout << msg << endl;
}

Scene::Scene::Scene() {
  if (glfwInit() == GLFW_FALSE) {
    string msg = "failed to init glfw!";
    cerr << msg << endl;
    throw runtime_error(msg);
  }
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  glfwWindowHint(GLFW_SAMPLES, 4);
  this->window = glfwCreateWindow(width, height, "demo", 0, 0);
  glfwMakeContextCurrent(this->window);
  glfwSetWindowUserPointer(window, this);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    string msg = "failed to init glad!";
    cerr << msg << endl;
    throw runtime_error(msg);
  }
  glEnable(GL_MULTISAMPLE);   // 开启MSAA抗锯齿
  glEnable(GL_DEPTH_TEST);    // 开启深度测试
  glEnable(GL_CULL_FACE);     // 开启面剔除
  glFrontFace(GL_CW);         // 顺时针索引顺序为正面
  glLineWidth(1.5f);

  glfwSetFramebufferSizeCallback(this->window, framebufferResizeCallback);
  // glfwSetErrorCallback(errorCallback);

  show_info();

  loadIcon();
  initImgui();

  load_all_shader();
  load_all_texture();

  init_ubo();
  init_scene_obj();
  init_skybox();

  init_framebuffer();

  init_depthmap();

  init_line_buffer();

  init_lsystem();

  // init_ray_visualze(); // 创建用于可视化光线路径的线条对象

  // test_cubemap();
}

Scene::Scene::~Scene() {
  for (const pair<string, Shader*>& sd : this->shaders)
    delete sd.second;
  for (int i = 0; i < 6; i++)
    stbi_image_free(this->cubemaps[i].img);
  this->objs.clear();
  // this->aux.clear();
}

void Scene::Scene::init_line_buffer() {
  this->lines["Ray"]   = make_shared<LineDrawer>();
  this->lines["Coord"] = make_shared<LineDrawer>();
}

void Scene::Scene::init_ubo() {
  glGenBuffers(1, &this->ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, this->ubo);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, nullptr, GL_STATIC_DRAW);

  glm::mat4 projecion = this->camera.getProject();
  glm::mat4 view      = this->camera.getView();
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projecion));
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));

  glBindBufferBase(GL_UNIFORM_BUFFER, PVM_binding_point, this->ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Scene::Scene::init_lsystem() {
  // 初始化一个示例
  string production = "S(r,h)->C(r,h)[RZ(30)RY(90)S(r,h*0.8)]"
                      "[RZ(-30)RY(90)S(r,h*0.8)]";
  string axiom      = "S(0.03,3)";
  this->lsystem.axiom += axiom;
  this->lsystem.productions.push_back(production);

  this->lsystem.config = make_shared<LSystem::LSystem>(
    this->lsystem.axiom,
    this->lsystem.productions);
}

void Scene::Scene::init_scene_obj() {
  // 光源
  shared_ptr<Geometry> lightBall = Mesh::Sphere(0.07f, 36, 18);
  lightBall->update();
  lightBall->setColor(1.0f, 1.0f, 1.0f);
  shared_ptr<Component::GeometryObject> obj1 =
    make_shared<Component::GeometryObject>("Light", lightBall);
  this->addSceneObject(obj1, this->isShowLight, false, false, false, 0);

  // 坐标轴
  shared_ptr<Geometry>       axis = make_shared<CoordinateAxis>(0.1, 1.0f);
  shared_ptr<Component::GeometryObject> obj2 = make_shared<Component::GeometryObject>("Axis", axis);
  this->addSceneObject(obj2, this->isShowAxis, false, false, false, 0);

  // 游标
  shared_ptr<Geometry> cursor = make_shared<CoordinateAxis>(0.02, 0.5f);
  // shared_ptr<Geometry> cursor = Mesh::Sphere(0.05f, 36, 18);
  // cursor->setColor(1.0f, 1.0f, 0.0f);
  shared_ptr<Component::GeometryObject> cursor_obj = make_shared<Component::GeometryObject>(
    "Cursor",
    cursor,
    Transform({glm::vec3(0.0f, 2.0f, 0.0f)}));
  this->addSceneObject(cursor_obj, this->isShowCursor, false, false, false, 0);

  // 地面
  shared_ptr<Geometry> ground = Mesh::Plane(20.0f, 20.0f, 10, 10);
  // 为了让光线不在两个重叠面上抖动进而穿透，将Ground下移一个微小距离
  shared_ptr<Component::GeometryObject> obj3 = make_shared<Component::GeometryObject>(
    "Ground",
    ground,
    Transform({0.0f, -0.1f, 0.0f}),
    true);
  this->addSceneObject(obj3, true, false, true, true, this->textures["fabric"]);

  // 左侧面
  shared_ptr<Geometry> side_left = Mesh::Plane(20.0f, 20.0f, 10, 10);
  side_left->setColor(0.0f, 0.0f, 1.0f);
  shared_ptr<Component::GeometryObject> side_left_obj = make_shared<Component::GeometryObject>(
    "Side_left",
    side_left,
    Transform({-10.0f, 9.9f, 0.0f}, _front, glm::radians(90.0f)),
    true);
  this->addSceneObject(side_left_obj, true, false, true, true, 0);

  // 后侧面
  shared_ptr<Geometry> side_back = Mesh::Plane(20.0f, 20.0f, 10, 10);
  side_back->setColor(0.0f, 1.0f, 0.0f);
  shared_ptr<Component::GeometryObject> side_back_obj = make_shared<Component::GeometryObject>(
    "Side_back",
    side_back,
    Transform({0.0f, 9.9f, -10.0f}, _right, glm::radians(90.0f)),
    true);
  this->addSceneObject(side_back_obj, true, false, true, true, 0);

  // 上侧面
  shared_ptr<Geometry> side_top = Mesh::Plane(20.0f, 20.0f, 10, 10);
  side_top->setColor(1.0f, 0.0f, 0.0f);
  shared_ptr<Component::GeometryObject> side_top_obj = make_shared<Component::GeometryObject>(
    "Side_top",
    side_top,
    Transform({0.0f, 19.9f, 0.0f}, _right, glm::radians(180.0f)),
    true);
  this->addSceneObject(side_top_obj, true, false, true, true, 0);
}

void Scene::Scene::init_skybox() {
  // 与前面的GL_TEXTURE_2D纹理目标不同，天空盒使用GL_TEXTURE_CUBE_MAP_XXX作为纹理目标

  // 1. 加载VBO
  vector<glm::vec3> vertices = {
    {-1.0, -1.0, -1.0},
    {-1.0, -1.0, 1.0},
    {-1.0, 1.0, -1.0},
    {-1.0, 1.0, 1.0},
    {1.0, -1.0, -1.0},
    {1.0, -1.0, 1.0},
    {1.0, 1.0, -1.0},
    {1.0, 1.0, 1.0},
  };
  vector<uint32_t> surfaces = {1, 5, 7, 1, 7, 3, 0, 2, 6, 0, 6, 4, 5, 4, 6, 5, 6, 7, 0, 1, 3, 0, 3, 2, 4, 5, 1, 4, 1, 0, 3, 7, 6, 3, 6, 2};

  glGenVertexArrays(1, &this->skybox.vao);
  glBindVertexArray(this->skybox.vao);

  glGenBuffers(1, &this->skybox.vbo);
  glBindBuffer(GL_ARRAY_BUFFER, this->skybox.vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

  glGenBuffers(1, &this->skybox.ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->skybox.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, surfaces.size() * sizeof(GLuint), surfaces.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glEnableVertexAttribArray(0);

  // 2. 加载纹理
  glGenTextures(1, &this->skybox.texture);
  glBindTexture(GL_TEXTURE_CUBE_MAP, this->skybox.texture);

  vector<string> skybox_texture_names = {"px", "nx", "py", "ny", "pz", "nz"};
  for (int i = 0; i < skybox_texture_names.size(); i++) {
    string fname = "assets/textures/skybox/" + skybox_texture_names[i] + ".png";
    int    width, height, channel;
    this->cubemaps[i].img = reinterpret_cast<Pixel*>(
      stbi_load(fname.c_str(), &this->cubemaps[i].width, &this->cubemaps[i].height, &this->cubemaps[i].channel, 0));
    assert(this->cubemaps[i].channel && "make sure image channel num is 4!");
    if (this->cubemaps[i].img == 0) {
      cerr << "load skybox texture failed: \"" << fname << "\"" << endl;
      continue;
    }

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, this->cubemaps[i].width, this->cubemaps[i].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, this->cubemaps[i].img);

    // stbi_image_free(this->cubemaps[i]);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void Scene::Scene::test_cubemap() {
  cout << "开始测试" << endl;
  vector<Pixel> buf;

  uint32_t phi_num   = 3600;
  uint32_t theta_num = 1800;
  for (uint32_t v = 0; v < theta_num; v++) {
    for (uint32_t u = 0; u < phi_num; u++) {
      float phi   = static_cast<float>(u) / phi_num * 2.0f * PI;
      float theta = static_cast<float>(v) / theta_num * PI;
      float x     = sin(theta) * cos(phi);
      float y     = cos(theta);
      float z     = -sin(theta) * sin(phi);
      Pixel p     = cubemap_sample(this->cubemaps, {x, y, z});
      buf.emplace_back(p);
    }
  }
  cout << "测试结束，写出图像" << endl;

  stbi_write_png("output.png", phi_num, theta_num, 4, buf.data(), 0);
}

void Scene::Scene::resize_framebuffer() {
  glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer.fbo);
  glBindTexture(GL_TEXTURE_2D, this->framebuffer.texture);
  // 重新分配帧缓冲区大小
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width, this->height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  // 重新分配渲染缓冲区大小
  glBindRenderbuffer(GL_RENDERBUFFER, this->framebuffer.rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, this->width, this->height);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Scene::Scene::init_framebuffer() {
  // 深度缓冲
  glGenFramebuffers(1, &this->framebuffer.fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer.fbo);

  // 创建纹理附件
  glGenTextures(1, &this->framebuffer.texture);
  glBindTexture(GL_TEXTURE_2D, this->framebuffer.texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width, this->height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // 将“颜色附件”附着给fbo
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->framebuffer.texture, 0);

  // 创建渲染缓冲用于缓存“深度、模板”
  glGenRenderbuffers(1, &this->framebuffer.rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, this->framebuffer.rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, this->width, this->height);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  // 将“深度、模板附件”附着给fbo
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->framebuffer.rbo);
  // 检查FBO完整性
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!"
              << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glm::vec4 vertices[4] = {
    {-1.0f, -1.0f, 0.0f, 0.0f},
    {1.0f, -1.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 1.0f},
    {-1.0f, 1.0f, 0.0f, 1.0f},
  };
  GLuint indices[6] = {1, 0, 2, 2, 0, 3};

  glGenVertexArrays(1, &this->framebuffer.vao);
  glBindVertexArray(this->framebuffer.vao);

  glGenBuffers(1, &this->framebuffer.vbo);
  glBindBuffer(GL_ARRAY_BUFFER, this->framebuffer.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glGenBuffers(1, &this->framebuffer.ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->framebuffer.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
}

void Scene::Scene::init_depthmap() {
  glGenTextures(1, &this->depthmap.texture);
  glBindTexture(GL_TEXTURE_2D, this->depthmap.texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glGenFramebuffers(1, &this->depthmap.fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, this->depthmap.fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->depthmap.texture, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// shared_ptr<RayObject> generate_ray_object() {
//   shared_ptr<RayObject> obj = make_shared<RayObject>();
//   this->ray_objs.emplace_back(obj);
//   return obj;
// }

// void update_ray_obj() {
//   // 根据ray_obj.vertices更新vbo
//   glBindVertexArray(ray_obj.vao);
//   glBindBuffer(GL_ARRAY_BUFFER, ray_obj.vbo);
//   glBufferData(GL_ARRAY_BUFFER, ray_obj.vertices.size() * sizeof(glm::vec3),
//   ray_obj.vertices.data(), GL_DYNAMIC_DRAW);
// }

void Scene::Scene::showAxis() {
  shared_ptr<Component::GeometryObject> ptr = findGeometryObjectByName("Axis");
  if (ptr)
    ptr->status.visible = true;
}
void Scene::Scene::hideAxis() {
  shared_ptr<Component::GeometryObject> ptr = findGeometryObjectByName("Axis");
  ptr->status.visible            = false;
}
void Scene::Scene::showGround() {
  shared_ptr<Component::GeometryObject> ptr = findGeometryObjectByName("Ground");
  ptr->status.visible            = true;
}
void Scene::Scene::hideGround() {
  shared_ptr<Component::GeometryObject> ptr = findGeometryObjectByName("Ground");
  ptr->status.visible            = false;
}
void Scene::Scene::showCursor() {
  shared_ptr<Component::GeometryObject> ptr = findGeometryObjectByName("Cursor");
  ptr->status.visible            = true;
};
void Scene::Scene::hideCursor() {
  shared_ptr<Component::GeometryObject> ptr = findGeometryObjectByName("Cursor");
  ptr->status.visible            = false;
};

void Scene::Scene::show_info() {
  const GLubyte* vendor   = glGetString(GL_VENDOR);
  const GLubyte* renderer = glGetString(GL_RENDERER);
  cout << "Vendor: " << vendor << endl;
  cout << "Graphics Device: " << renderer << endl;
}

void Scene::Scene::printRadiosityInfo() {
  printf("============================output============================\n");
  for (auto& [cur_obj, context] : this->objs) {
    if (!cur_obj->status.listed)
      continue;
    glm::vec3 flux_sum{0.0f, 0.0f, 0.0f};
    for (int i = 0; i < cur_obj->radiosity.radiant_flux.size(); i++) {
      flux_sum += cur_obj->radiosity.radiant_flux[i];
    }
    printf("%s : (%.2f, %.2f, %.2f)\n", cur_obj->getName().c_str(), flux_sum.r, flux_sum.g, flux_sum.b);
  }
}

shared_ptr<Component::GeometryObject>
Scene::Scene::findGeometryObjectByName(const string& name) {
  auto iter = find_if(this->objs.begin(), this->objs.end(), [&](pair<const shared_ptr<Component::GeometryObject>, unique_ptr<GeometryContext>>& obj_pair) {
    return obj_pair.first->getName().compare(name) == 0;
  });
  if (iter == this->objs.end())
    return nullptr;
  return iter->first;
}

// void setSeed(uint32_t sd) {
//   this->random_generator.seed(sd);
// }

Ray Scene::Scene::cast_ray_from_mouse() {
  // 捕捉鼠标在窗口中的位置，并将其转换为世界坐标系下的一条射线

  double xpos, ypos;
  glfwGetCursorPos(this->window, &xpos, &ypos);
  xpos = 2.0f * xpos / this->width - 1.0f;
  ypos = 1.0f - 2.0f * ypos / this->height;

  glm::vec3 dir = screen2world({xpos, ypos});

  return {this->camera.getPosition(), dir};
}

glm::vec3 Scene::Scene::screen2world(glm::vec2 pos) {
  pos                           = -pos;
  glm::mat4 view                = this->camera.getView();
  auto [fov, near, far, aspect] = this->camera.getProperties();
  glm::vec4 target_dir =
    glm::vec4(glm::normalize(glm::vec3(pos.x * near * tanf(fov / 2.0f) * aspect,
                                       pos.y * near * tanf(fov / 2.0f),
                                       near)),
              0.0f);
  glm::vec4 world_dir = glm::transpose(view) * target_dir;
  return -glm::vec3(world_dir);
}

void Scene::Scene::imgui_interact() {
  // 鼠标交互动作
  // 记录下“相机环绕”时的相机球坐标
  if (!io->WantCaptureMouse) {
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      if (ImGui::IsKeyDown(ImGuiKey_ModShift)) {
        this->camera.record();
      }
      else {
        // 鼠标左击选中场景物体，遍历所有物体的包围盒求交
        this->imgui.mouse_pos = io->MousePos;
      }
    }

    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
      if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
        // 以世界坐标锚点为中心做旋转
        if (this->imgui.start_record) {
          // 相机记录当前环绕位置和锚点距离
          this->camera.record();
          this->imgui.start_record = false;
        }
        this->camera.surround(
          MOUSE_VIEW_ROTATE_SENSITIVITY * 0.04 * io->MouseDelta.x,
          MOUSE_VIEW_ROTATE_SENSITIVITY * 0.04 * io->MouseDelta.y);
      }
      else {
        // 以相机为中心旋转
        this->camera.rotate({MOUSE_VIEW_ROTATE_SENSITIVITY * io->MouseDelta.x,
                             MOUSE_VIEW_ROTATE_SENSITIVITY * io->MouseDelta.y,
                             0.0});
      }
    }
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Right, 0.0f)) {
      // 沿着相机姿态坐标系的上下左右进行平移
      this->camera.move_relative(
        {-MOUSE_VIEW_TRANSLATE_SENSITIVITY * io->MouseDelta.x,
         MOUSE_VIEW_TRANSLATE_SENSITIVITY * io->MouseDelta.y,
         0.0f});
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
      // 相机准备好下一次环绕的启动
      this->imgui.start_record = true;

      // 鼠标按下不挪动松开，则执行场景物体拾取
      ImVec2 mouse_pos = io->MousePos;
      float  move_offset =
        std::sqrt(std::powf(mouse_pos.x - this->imgui.mouse_pos.x, 2.0f) +
                  std::powf(mouse_pos.y - this->imgui.mouse_pos.y, 2.0f));
      // cout << "鼠标拖拽距离：" << move_offset << endl;
      if (move_offset < 8.0f) {
        // 按下与弹起位置一致，中间没有鼠标大幅拖拽，则进行拾取操作

        Ray ray = cast_ray_from_mouse();

        struct HitInfo_imgui {
          bool      isHit{false};
          glm::vec3 hitPos{0.0f, 0.0f, 0.0f};
          float     distance{FLT_MAX};
          int32_t   id{0};     // 该物体在Scene::imgui::items中的索引位置
          uint32_t  type{0};   // 0为普通包围盒，1为层次包围盒
        };

        HitInfo_imgui target_obj;
        for (auto& [cur_obj, context] : this->objs) {
          if (!cur_obj->status.collided)
            continue;

          HitInfo_imgui tmp_obj;
          if (cur_obj->getBvhTree() != nullptr) {
            // 层次包围盒求交
            HitInfo hit_obj =
              cur_obj->getBvhTree()->hit(ray, cur_obj->transform);
            if (hit_obj.isHit) {
              // 将位置变换回世界坐标系下
              tmp_obj.isHit    = true;
              tmp_obj.hitPos   = hit_obj.hitPos;
              tmp_obj.distance = hit_obj.distance;
              tmp_obj.type     = 1;
              // tmp_obj.id = 0; // 后面统一获取
            }
          }
          else {
            // 普通外层包围盒求交
            assert(cur_obj->getBoundingBox() != nullptr &&
                   "Scene::imgui_interact() hit测试中, 包围盒为空!");
            BoundingBox* box = cur_obj->getBoundingBox();
            if (box->hit(ray, cur_obj->transform)) {
              tmp_obj.isHit    = true;
              tmp_obj.hitPos   = box->getBoxCenter();
              tmp_obj.distance = glm::distance(ray.origin, tmp_obj.hitPos);
              tmp_obj.type     = 0;
              // tmp_obj.id = 0; // 后面统一获取
            }
          }
          if (tmp_obj.isHit &&
              tmp_obj.distance < target_obj.distance) {   // 找到最近对象
            this->imgui.cur = cur_obj;
            auto tmp_ptr    = find(this->imgui.list_items.begin(),
                                this->imgui.list_items.end(),
                                cur_obj);
            if (tmp_ptr != this->imgui.list_items.end())
              tmp_obj.id =
                std::distance(this->imgui.list_items.begin(), tmp_ptr);
            else
              tmp_obj.id = -1;
            target_obj = tmp_obj;
          }
        }
        // 找到最近碰撞目标target_obj
        if (target_obj.isHit) {
          this->imgui.selected_idx = target_obj.id;   // 可能会赋值-1
          if (imgui.cur != nullptr) {
            for (auto& [obj, context] : this->objs)   // 互斥选中
              obj->status.selected = false;
            BoundingBox* box = imgui.cur->getBoundingBox();
            assert(box != nullptr &&
                   "Scene::imgui_interact() boundingbox为空!");
            this->camera.setAnchor(box->getBoxCenter());
          }
          switch (target_obj.type) {
          case 1: {
            shared_ptr<Component::GeometryObject> ptr1 =
              findGeometryObjectByName("Cursor");
            if (ptr1)
              ptr1->transform.setPosition(target_obj.hitPos);
            this->showCursor();
            break;
          }
          case 0: {
            this->hideCursor();
            break;
          default:
            this->hideCursor();
          }
          }
        }
      }
    }
  }

  if (ImGui::IsKeyDown(ImGuiKey_W))
    this->camera.move_relative({0.0f, 0.0f, -0.1f});

  if (ImGui::IsKeyDown(ImGuiKey_S))
    this->camera.move_relative({0.0f, 0.0f, 0.1f});

  if (ImGui::IsKeyDown(ImGuiKey_A))
    this->camera.move_relative({-0.1f, 0.0f, 0.0f});

  if (ImGui::IsKeyDown(ImGuiKey_D))
    this->camera.move_relative({0.1f, 0.0f, 0.0f});

  if (ImGui::IsKeyDown(ImGuiKey_Space))
    this->camera.move_relative({0.0f, 0.1f, 0.0f});

  if (ImGui::IsKeyDown(ImGuiKey_C))
    this->camera.move_relative({0.0f, -0.1f, 0.0f});
}

void Scene::Scene::updateGeometryListView() {
  // auto tmp_item_view =
  //   this->objs | ranges::views::filter([](shared_ptr<GeometryObject> obj) {
  //     return obj->status.listed;
  //   });
  vector<shared_ptr<Component::GeometryObject>> tmp_item_view;
  for (auto& [obj, ctx] : this->objs) {
    if (obj->status.listed)
      tmp_item_view.emplace_back(obj);
  }
  this->imgui.list_items = tmp_item_view;
}
void Scene::Scene::addSoftReturnsToText(std::string& str,
                                        float        multilineWidth) {

  float       textSize = 0;
  std::string tmpStr   = "";
  std::string finalStr = "";
  int         curChr   = 0;
  while (curChr < str.size()) {

    if (str[curChr] == '\n') {
      finalStr += tmpStr + "\n";
      tmpStr = "";
    }

    tmpStr += str[curChr];
    textSize = ImGui::CalcTextSize(tmpStr.c_str()).x;

    if (textSize > multilineWidth) {
      int lastSpace = tmpStr.size() - 1;
      while (tmpStr[lastSpace] != ' ' and lastSpace > 0)
        lastSpace--;
      if (lastSpace == 0)
        lastSpace = tmpStr.size() - 2;
      finalStr += tmpStr.substr(0, lastSpace + 1) + "\r\n";
      if (lastSpace + 1 > tmpStr.size())
        tmpStr = "";
      else
        tmpStr = tmpStr.substr(lastSpace + 1);
    }
    curChr++;
  }
  if (tmpStr.size() > 0)
    finalStr += tmpStr;

  str = finalStr;
};

bool Scene::Scene::imgui_autosizingMultilineInput(const char*         label,
                                                  std::string*        str,
                                                  const ImVec2&       sizeMin,
                                                  const ImVec2&       sizeMax,
                                                  ImGuiInputTextFlags flags) {

  // calculate the maximum y/height
  ImGui::PushTextWrapPos(sizeMax.x);
  auto textSize = ImGui::CalcTextSize(str->c_str());
  if (textSize.x > sizeMax.x) {
    float ratio = textSize.x / sizeMax.x;
    textSize.x  = sizeMax.x;
    textSize.y *= ratio;
    textSize.y += 20;   // add space for an extra line
  }

  textSize.y += 8;   // to compensate for inputbox margins

  if (textSize.x < sizeMin.x)
    textSize.x = sizeMin.x;
  if (textSize.y < sizeMin.y)
    textSize.y = sizeMin.y;
  if (textSize.x > sizeMax.x)
    textSize.x = sizeMax.x;
  if (textSize.y > sizeMax.y)
    textSize.y = sizeMax.y;

  bool value_changed = ImGui::InputTextMultiline(label, str, textSize, flags);
  ImGui::PopTextWrapPos();

  return value_changed;
}

bool Scene::Scene::imgui_menu() {

  // imgui_docking_render();
  // ImGui::ShowExampleAppDockSpace();
  ImGui::ShowDemoWindow();

  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu(TEXT("File"))) {
      ImGui::MenuItem(TEXT("Open"), NULL);
      ImGui::MenuItem(TEXT("Save As"), NULL);
      if (ImGui::MenuItem(TEXT("Exit"), NULL))
        return true;
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu(TEXT("Edit"))) {

      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }

  ImGui::Begin(TEXT("场景"), NULL, ImGuiWindowFlags_AlwaysAutoResize);
  if (ImGui::TreeNodeEx(TEXT("相机"), ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::SliderFloat(TEXT("天顶角"), &this->camera.getTheta(), 0.0f, 180.0f, "%.1f"))
      this->camera.updateAttitudeFromShadow();
    if (ImGui::SliderFloat(TEXT("方位角"), &this->camera.getPhi(), -180.0f, 180.0f, "%.1f"))
      this->camera.updateAttitudeFromShadow();

    ImGui::DragFloat3(TEXT("位置"), glm::value_ptr(this->camera.getPosition()), 0.05f, -FLT_MAX, FLT_MAX);

    ImGui::TreePop();
  }

  if (ImGui::TreeNodeEx(TEXT("光源"), ImGuiTreeNodeFlags_DefaultOpen)) {
    // ImGui::SliderFloat3(TEXT("位置"), glm::value_ptr(this->light.position),
    // -20.0f, 20.f);
    ImGui::DragFloat3(TEXT("位置"), glm::value_ptr(this->light.position), 0.05f, -FLT_MAX, FLT_MAX);
    // 暂时这么写
    shared_ptr<Component::GeometryObject> ptr = findGeometryObjectByName("Light");
    if (ptr)
      ptr->transform.setPosition(light.position);
    ImGui::TreePop();
  }

  if (this->objs.empty()) {
    ImGui::End();
    return false;
  }

  if (this->imgui.changeGeometryListView) {
    updateGeometryListView();   // 初始化列表
    this->imgui.changeGeometryListView = false;
  }

  if (ImGui::TreeNodeEx(TEXT("几何管理"), ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::BeginListBox(TEXT("物体"))) {
      for (int i = 0; i < this->imgui.list_items.size(); i++) {
        const bool is_selected = (imgui.selected_idx == i);
        if (ImGui::Selectable(this->imgui.list_items[i]->getName().c_str(),
                              is_selected))
          imgui.selected_idx = i;
      }
      // 回传选中状态
      if (!this->imgui.list_items.empty()) {
        for (int i = 0; i < this->imgui.list_items.size(); i++)
          this->imgui.list_items[i]->status.selected = false;
        // assert(this->imgui.selected_idx != -1);
        if (this->imgui.selected_idx != -1) {

          this->imgui.cur                  = this->imgui.list_items[this->imgui.selected_idx];
          this->imgui.cur->status.selected = true;
        }
      }

      ImGui::EndListBox();
    }

    // 显示参数
    if (!imgui.list_items.empty() && this->imgui.cur != nullptr &&
        this->imgui.cur->status.listed &&
        !this->imgui.cur->getGeometry()->parameters.empty()) {

      if (ImGui::DragFloat3(
            TEXT("位置"),
            glm::value_ptr(this->imgui.cur->transform.getPosition()),
            0.05f,
            -FLT_MAX,
            FLT_MAX)) {
        // 更新包围盒以及其渲染缓冲
        BoundingBox* box = this->imgui.cur->getBoundingBox();
        if (box != nullptr) {
          box->update(this->imgui.cur->getGeometry()->getVertices());
          // box->context->update();
          // this->objs[this->imgui.cur]->box->update();
          this->objs[this->imgui.cur]->update();
        }
      }
      ImGui::DragFloat4(
        TEXT("姿态"),
        glm::value_ptr(this->imgui.cur->transform.getAttitude()),
        0.05f,
        -FLT_MAX,
        FLT_MAX);

      ImGui::Text(TEXT("形体参数"));
      // 循环两次将topo参数和普通参数分批显示
      // 1. 显示topo属性
      for (auto& [name, arg_val] : imgui.cur->getGeometry()->parameters)
        if (arg_val->isTopo())
          std::visit(
            [name, this](auto&& arg) {
              using T = decay_t<decltype(arg)>;
              if constexpr (is_same_v<T, float>) {

                if (ImGui::SliderFloat(name.c_str(), &arg, 0.0f, 10.0f)) {
                  this->imgui.cur->getGeometry()->parameters[name]->notifyAll();
                  this->imgui.cur->update();
                  this->objs[this->imgui.cur]->update();
                }
              }
              else if constexpr (is_same_v<T, uint32_t>) {

                if (ImGui::SliderInt(name.c_str(),
                                     reinterpret_cast<int*>(&arg),
                                     2,
                                     50)) {
                  this->imgui.cur->getGeometry()->parameters[name]->notifyAll();
                  this->imgui.cur->update();
                  this->objs[this->imgui.cur]->update();
                }
              }
              else {
              }
            },
            arg_val->getProp());
      for (auto& [name, arg_val] : imgui.cur->getGeometry()->parameters)
        if (!arg_val->isTopo())
          std::visit(
            [name, this](auto&& arg) {
              using T = decay_t<decltype(arg)>;
              if constexpr (is_same_v<T, float>) {

                if (ImGui::SliderFloat(name.c_str(), &arg, 0.0f, 10.0f)) {
                  this->imgui.cur->getGeometry()->parameters[name]->notifyAll();
                  this->imgui.cur->update();
                  this->objs[this->imgui.cur]->update();
                }
              }
              else if constexpr (is_same_v<T, uint32_t>) {

                if (ImGui::SliderInt(name.c_str(),
                                     reinterpret_cast<int*>(&arg),
                                     2,
                                     50)) {
                  this->imgui.cur->getGeometry()->parameters[name]->notifyAll();
                  this->imgui.cur->update();
                  this->objs[this->imgui.cur]->update();
                }
              }
              else {
              }
            },
            arg_val->getProp());
    }
    ImGui::TreePop();
  }
  if (ImGui::TreeNodeEx(TEXT("调试"))) {
    ImGui::Checkbox(TEXT("光线追踪可视化"), &this->isShowRay);
    ImGui::SameLine();
    ImGui::PushID(0);
    if (ImGui::Button(TEXT("更新"))) {
      // thread tsk(this->compute_radiosity);
      this->compute_radiosity();
      this->printRadiosityInfo();
    }
    ImGui::PopID();

    ImGui::Checkbox(TEXT("三角局部坐标可视化"), &this->isShowCoord);
    ImGui::SameLine();
    ImGui::PushID(1);
    if (ImGui::Button(TEXT("更新")))
      this->test_triangle_coord();
    ImGui::PopID();

    ImGui::Checkbox(TEXT("线框模式"), &this->isShowWireFrame);
    ImGui::Checkbox(TEXT("显示BVH线框"), &this->isShowBvhFrame);

    ImGui::TreePop();
  }
  ImGui::End();

  // // 用于生成L-System的操作窗口
  // ImGui::Begin(TEXT("L-System"), NULL, ImGuiWindowFlags_AlwaysAutoResize);
  // if (ImGui::TreeNodeEx(TEXT("生成"), ImGuiTreeNodeFlags_DefaultOpen)) {
  //   ImGui::PushItemWidth(300.0f);
  //   if (ImGui::InputText(TEXT("Axiom"), &this->lsystem.axiom, ImGuiInputTextFlags_CallbackEdit)) {
  //     throw runtime_error("Scene::imgui_menu()中，更新this->lsystem的axiom！");
  //     this->lsystem.config->updateAxiom(this->lsystem.axiom);
  //   }
  //   ImGui::PopItemWidth();

  //   for (uint32_t i = 0; i < this->lsystem.productions.size(); i++) {
  //     ImGui::PushItemWidth(900.0f);
  //     if (ImGui::InputText(format("production {}", i + 1).c_str(),
  //                          &this->lsystem.productions[i])) {
  //       // 需要更新lsystem

  //       // throw
  //       // runtime_error("Scene::imgui_menu()中，更新this->lsystem的production！");
  //       if (!this->lsystem.config->updateProduction(
  //             this->lsystem.productions)) {
  //         cout << "产生式规则含可能含有未知错误" << endl;
  //       }
  //     }
  //     ImGui::PopItemWidth();
  //     ImGui::SameLine();
  //     ImGui::PushID(i);
  //     if (ImGui::Button(TEXT("移除"))) {
  //       this->lsystem.productions.erase(this->lsystem.productions.begin() + i);
  //       // 需要更新lsystem
  //       // throw
  //       // runtime_error("Scene::imgui_menu()中，更新this->lsystem的production！");
  //       this->lsystem.config->updateProduction(this->lsystem.productions);
  //     }
  //     ImGui::PopID();
  //   }
  //   if (ImGui::Button(TEXT("增加")))
  //     this->lsystem.productions.push_back("");

  //   ImGui::Text(TEXT("iter: %u"), this->lsystem.iter_n);
  //   if (ImGui::Button(TEXT("迭代"))) {
  //     // throw runtime_error("Scene::imgui_menu()中，更新迭代符号串！");
  //     string lsys_cmds = this->lsystem.config->iterate();
  //     cout << "调试迭代字符串：" << endl;
  //     cout << lsys_cmds << endl;

  //     // 更新目标骨骼系统
  //     if (this->skeletons.find("skeleton") != this->skeletons.end())
  //       this->removeSkeleton("skeleton");

  //     // throw
  //     // runtime_error("Scene::imgui_menu()中，解析新字符串，更新骨架！");
  //     shared_ptr<LSysConfig::SymSeq> symSeq =
  //       this->lsystem.config->parseInput(lsys_cmds);
  //     assert(symSeq && "praseInput() return nullptr!");

  //     const GeometryInterpreter::GraphicsStructure gs(symSeq);
  //     this->lsystem.skeleton = gs.construct();
  //     this->add("skeleton", this->lsystem.skeleton, Transform{glm::vec3(0.5f, 0.03f, 0.5f)});
  //     this->lsystem.iter_n++;
  //   }
  //   ImGui::SameLine();
  //   if (ImGui::Button(TEXT("复位"))) {
  //     // 删除目标骨骼系统
  //     if (this->skeletons.find("skeleton") != this->skeletons.end())
  //       this->removeSkeleton("skeleton");
  //     // throw
  //     // runtime_error("Scene::imgui_menu()中删除骨骼系统时，重置this->lsystem.lsys");
  //     this->lsystem.config->reset();
  //     this->lsystem.iter_n = 0;
  //   }
  //   ImGui::TreePop();
  // }
  // ImGui::End();

  return false;
}

void Scene::Scene::loadIcon() {
  GLFWimage image;
  image.pixels =
    stbi_load("assets/favicon/favicon1.png", &image.width, &image.height, nullptr, 4);
  glfwSetWindowIcon(this->window, 1, &image);
  stbi_image_free(image.pixels);
}
void Scene::Scene::initImgui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  this->io = &ImGui::GetIO();
  io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io->ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
  io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  io->Fonts->AddFontFromFileTTF("C:/Windows/Fonts/simhei.ttf", 24.0f, nullptr, io->Fonts->GetGlyphRangesChineseFull());
  io->Fonts->Build();

  ImGui::StyleColorsDark();

  ImGuiStyle& style = ImGui::GetStyle();
  if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding              = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  ImGui_ImplGlfw_InitForOpenGL(this->window, true);
  ImGui_ImplOpenGL3_Init();
}
void Scene::Scene::load_all_shader() {
  shaders["default"]    = new Shader("default.vert", "default.frag");
  shaders["normal"]     = new Shader("normal.vert", "normal.geom", "normal.frag");
  shaders["skybox"]     = new Shader("skybox.vert", "skybox.frag");
  shaders["lightDepth"] = new Shader("lightDepth.vert", "lightDepth.frag");
  // shaders["compute"] = new Shader("compute.comp");
  shaders["line"]   = new Shader("line.vert", "line.frag");
  shaders["screen"] = new Shader("screen.vert", "screen.frag");
}

void Scene::Scene::load_all_texture() {
  fs::path texture_dir = "assets/textures";
  for (auto& file : fs::directory_iterator(texture_dir)) {
    if (file.is_regular_file()) {
      // 加载纹理
      string fname = file.path().string();
      // 读取图片文件
      int   img_width, img_height, img_chn;
      void* img_data =
        stbi_load(fname.c_str(), &img_width, &img_height, &img_chn, 0);
      if (img_data == nullptr) {
        string msg = "load texture image \"" + fname + "\" failed!";
        cerr << msg << endl;
        throw runtime_error(msg);
      }

      // 创建纹理缓冲
      GLuint new_texture;
      glGenTextures(1, &new_texture);
      glBindTexture(GL_TEXTURE_2D, new_texture);

      glTextureParameteri(new_texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTextureParameteri(new_texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTextureParameteri(new_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTextureParameteri(new_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_width, img_height, 0, GL_RGB, GL_UNSIGNED_BYTE, img_data);
      glGenerateTextureMipmap(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, 0);

      this->textures[file.path().filename().stem().string()] = new_texture;
      stbi_image_free(img_data);

      // cout << "load texture: \"" << file.path().filename().string() << "\""
      //      << endl;
    }
  }
}

void Scene::Scene::test_triangle_coord() {
  lines["Coord"]->clear();
  for (auto& [cur_obj, context] : this->objs) {
    glm::mat4 model = cur_obj->transform.getModel();
    for (auto& triangle : cur_obj->getGeometry()->getSurfaces()) {
      TriangleSampler tri(cur_obj->getGeometry()->getVertices(), triangle, model);
      glm::vec3       tri_center = tri.calcCenter();
      // glm::vec3 tri_norm = tri.calcNorm();
      // printf("三角中心： (%.2f,%.2f,%.2f)\n", tri_center.x, tri_center.y,
      // tri_center.z); printf("三角法方向： (%.2f,%.2f,%.2f)\n", tri_norm.x,
      // tri_norm.y, tri_norm.z);
      auto [right, up, back] = tri.calcLocalCoord();
      lines["Coord"]->addLine(tri_center, tri_center + 0.15f * right, {1.0f, 0.0f, 0.0f});
      lines["Coord"]->addLine(tri_center, tri_center + 0.15f * up, {0.0f, 1.0f, 0.0f});
      lines["Coord"]->addLine(tri_center, tri_center + 0.15f * back, {0.0f, 0.0f, 1.0f});
    }
  }
  lines["Coord"]->update();
}

void Scene::Scene::addSceneObject(const shared_ptr<Component::GeometryObject>& obj,
                                  bool                              visible,
                                  bool                              listed,
                                  bool                              collided,
                                  bool                              lighted,
                                  GLuint                            texture) {

  shared_ptr<Component::GeometryObject> ptr = findGeometryObjectByName(obj->getName());
  if (ptr != nullptr) {
    cout << "scene cannot add \"SceneObject aux\" with an existed name \""
         << obj->getName() << "\"" << endl;
    return;
  }
  obj->status.visible  = visible;
  obj->status.listed   = listed;
  obj->status.collided = collided;
  obj->status.lighted  = lighted;
  // this->objs.emplace_back(obj);

  // 创建Context
  unique_ptr<GeometryContext> context = make_unique<GeometryContext>(obj.get());
  context->init();
  context->texture = texture;

  this->objs[obj] = std::move(context);

  updateGeometryListView();
}

void Scene::Scene::remove(const string& name) {
  // 移除objs中的物体，同时销毁相应的GeometryContext，自动释放OpenGL缓冲区
  auto obj_ptr =
    find_if(this->objs.begin(), this->objs.end(), [name](auto& obj) {
      return obj.first->getName().compare(name) == 0;
    });
  if (obj_ptr != this->objs.end())
    this->objs.erase(obj_ptr);
}

// void Scene::Scene::removeSkeleton(const string& name) {
//   auto skptr = this->skeletons.find(name);
//   if (skptr == this->skeletons.end()) {
//     cout << format("warnning: skeleton \"{}\" not found in removeSkeleton()",
//                    name)
//          << endl;
//     return;
//   }
//   erase_if(this->objs, [&](auto& obj) {
//     return regex_match(obj.first->getName(),
//                        this->check_skeleton_node_name_pattern);
//   });
//   this->skeletons.erase(skptr);
// }

// void Scene::Scene::add(const string& name, shared_ptr<Skeleton> skeleton, Transform transform) {
//   // 加入骨架对象，考虑遍历Skeleton的所有节点并将其中的Geometry加入到this->objs中
//   if (this->skeletons.find(name) != this->skeletons.end()) {
//     cout << "warnning: scene cannot add \"Skeleton\" with an existed name \""
//          << name << "\"" << endl;
//     return;
//   }
//   // 将transform的影响加入到skeleton->root节点的transform中，然后更新整个skeleton

//   skeleton->root->setTransform(skeleton->root->getTransform() * transform);
//   skeleton->update();

//   SkeletonObject sk{name, skeleton};

//   uint32_t geo_id = 1;
//   // 遍历skeleton的所有节点并加入到this->objs中
//   sk.skeleton->traverse([=, this, &geo_id](SkNode* node) {
//     stringstream node_geom_name;
//     node_geom_name << name << "#" << geo_id;

//     this->add(node_geom_name.str(), node->obj, node->getAbsTransform(), true, true, true, true, false);

//     // if (!node->children.empty()) {
//     //   // 调试，给每个节点加入一个Axis
//     //   stringstream ass;
//     //   ass << name << "_Axis_" << geo_id;
//     //   shared_ptr<GeometryContext> robj =
//     //   GeometryContext::getInstance(make_shared<CoordinateAxis>(0.067,
//     //   0.6), node->getAbsTransform()); this->addSceneObject(ass.str(),
//     //   robj);
//     // }

//     geo_id++;
//   });
//   this->skeletons[name] = sk;
//   updateGeometryListView();
// }

// void Scene::Scene::add(const string& name, shared_ptr<Skeleton> skeleton) {
//   this->add(name, skeleton, Transform{});
// }

void Scene::Scene::add(const string& name, const shared_ptr<Geometry>& geometry, Transform transform, bool visible, bool listed, bool collided, bool lighted, bool useBvh) {

  shared_ptr<Component::GeometryObject> ptr = findGeometryObjectByName(name);
  if (ptr != nullptr) {
    cout << "scene cannot add \"SceneObject aux\" with an existed name \""
         << name << "\"" << endl;
    return;
  }

  ptr                  = make_shared<Component::GeometryObject>(name, geometry, transform, useBvh);
  ptr->status.visible  = visible;
  ptr->status.listed   = listed;
  ptr->status.collided = collided;
  ptr->status.lighted  = lighted;

  // 创建Context
  unique_ptr<GeometryContext> context = make_unique<GeometryContext>(ptr.get());
  context->init();

  this->objs[ptr] = std::move(context);

  // 加入对场景元素的默认选择，总是选中最后加入的物体，并取消选择其他物体
  for (auto& [cur_obj, context] : this->objs)
    cur_obj->status.selected = false;
  ptr->status.selected = true;

  // 初始化Scene::imgui::selected_idx
  updateGeometryListView();
  auto fptr = find_if(
    this->imgui.list_items.begin(),
    this->imgui.list_items.end(),
    [](shared_ptr<Component::GeometryObject> obj) { return obj->status.selected; });
  if (fptr != this->imgui.list_items.end())
    this->imgui.selected_idx =
      std::distance(this->imgui.list_items.begin(), fptr);
  else
    this->imgui.selected_idx = -1;
}

void Scene::Scene::add(const string& name, const shared_ptr<Geometry>& geometry, Transform transform) {
  this->add(name, geometry, transform, true, true, true, true, true);
}

void Scene::Scene::add(const string&               name,
                       const shared_ptr<Geometry>& geometry) {
  this->add(name, geometry, Transform(), true, true, true, true, true);
}

// void Scene::Scene::add(const string &name, const shared_ptr<Geometry>
// &geometry,
//                        glm::vec3 position) {
//   this->add(name, geometry, Transform(position),true,true,true,true,true);
// }

void Scene::Scene::addLight(const string&            name,
                            const shared_ptr<Light>& light) {
  shared_ptr<Component::GeometryObject> ptr = findGeometryObjectByName(name);
  if (ptr != nullptr) {
    cerr << "scene cannot add object with an existed name!" << endl;
    return;
  }
  this->lights.emplace_back(light);

  // 建立渲染对象，加入到aux中，不同子类建立的可视化对象不一样
  shared_ptr<Component::GeometryObject> render_obj = nullptr;
  switch (light->type) {
  case Light::LightType::POINT: {
    render_obj =
      make_shared<Component::GeometryObject>("Light", Mesh::Sphere(0.03, 72, 36));
    break;
  }
  case Light::LightType::PARALLEL: {
    // 未实现
    break;
  }
  case Light::LightType::SPOT: {
    // 未实现
    break;
  }
  }

  if (render_obj != nullptr) {
    // this->objs.emplace_back(render_obj);
    this->addSceneObject(render_obj, true, false, false, false, 0);
  }
  else
    cerr << "该光源类型的渲染对象未在Scene::addLight()中实现！" << endl;
}

// void compute_radiosity_1() {
//   // 遍历场景中所有objs并计算每个物体对应接收到的辐射通量

//   // 实际光源并不只有一个，而且并非只有平行光(后续考虑)
//   shared_ptr<ParallelLight> light =
//       dynamic_pointer_cast<ParallelLight>(this->lights[0]);

//   for (auto &[name, cur_obj] : this->objs) {
//     shared_ptr<Geometry> geometry = cur_obj->geometry;
//     cur_obj->radiosity.radiant_flux.resize(geometry->surfaces.size());
//     for (int i = 0; i < geometry->surfaces.size(); i++) {
//       Surface triangle = geometry->surfaces[i];
//       vector<glm::vec3> pt(3);
//       glm::vec3 norm{0.0f, 0.0f, 0.0f};
//       float area{0.0f};
//       for (int j = 0; j < 3; j++)
//         pt[j] =
//         glm::make_vec3(geometry->vertices[triangle.tidx[j]].position);
//       glm::vec3 tri_cross = glm::cross(pt[1] - pt[0], pt[2] - pt[0]);
//       norm = glm::normalize(tri_cross);
//       area = glm::length(tri_cross) / 2.0f;
//       // 计算辐射通量(每个三角面)
//       cur_obj->radiosity.radiant_flux[i] =
//           light->irradiance * area *
//           glm::max(0.0f, glm::dot(norm, -light->direction));
//     }
//   }
// }

// struct HitGeometryObj {
//   bool isHit{false};
//   string obj_name; // 击中物体名称
//   uint32_t tri_id; // 击中物体所在三角面元的索引
//   glm::vec3 hit_pos;    // 世界坐标系下的击中位置
// };

HitInfo Scene::Scene::hit(Ray ray) {
  // 场景物体全局求交
  // 遍历this->objs所有元素，找到距离最近的相交物体
  // 返回击中物体名称、击中对象信息

  HitInfo target_obj;
  // string target_name;
  for (auto& [cur_obj, context] : this->objs) {
    BvhTree* tree = cur_obj->getBvhTree();
    assert(tree != nullptr && "element in scene.objs must construct "
                              "bvh-tree!");   // 假定都生成了bvh树

    HitInfo tmp_obj = tree->hit(ray, cur_obj->transform);
    if (tmp_obj.isHit && tmp_obj.distance < target_obj.distance) {
      tmp_obj.geometryName = cur_obj->getName();
      target_obj           = tmp_obj;
    }
  }

  // if (target_obj.isHit) {
  //   cout << "光线击中物体 \"" << target_obj.geometry_name << "\"  tri_id="
  //   << target_obj.triangle_idx << " hit_pos=(" << target_obj.hitPos.x << ",
  //   " << target_obj.hitPos.y << ", " << target_obj.hitPos.z << ")" << endl;
  // }

  return target_obj;
}

// // 递归函数版本
// glm::vec3 trace_ray(Ray ray, const HitInfo &obj, float PR, uint32_t
// recursive_depth = 0, shared_ptr<vector<glm::vec3>> vert_buffer = nullptr) {
//   //
//   recursive_depth用以记录该函数的递归深度，同时在可视化光线路径时用于记录顶点数量
//   // recursive_depth=0表示对该函数的“根调用”，其他为递归调用
//   assert(PR < 1.0f && PR > 0.0f);
//   uniform_real_distribution<> distr(0.0, 1.0);
//   glm::vec3 L_dir{0.0f, 0.0f, 0.0f}, L_indir{0.0f, 0.0f, 0.0f};
//   float PR_D = distr(rdgen);
//   if (PR > PR_D) { // 俄罗斯赌轮盘
//     assert(this->objs.find(obj.geometry_name) != this->objs.end() &&
//     "trace_ray hit object cannot found in scene.objs!");

//     shared_ptr<GeometryContext> gobj = this->objs[obj.geometry_name];
//     shared_ptr<Geometry> geometry = gobj->geometry;
//     mat4 model = gobj->transform.getModel();

//     TriangleSampler tri(geometry->vertices,
//     geometry->surfaces[obj.triangle_idx], model); glm::vec3 tri_center =
//     tri.calcCenter(); glm::vec3 tri_norm = tri.calcNorm(); float tri_area =
//     tri.calcArea(); glm::vec3 wi = tri.hemisphereSampleDir();

//     if (tri_area == 0.0f)
//       // cerr << "Warning : zero area triangle exists! return zero vector."
//       << endl; return {0.0f, 0.0f, 0.0f};

//     float cosine = glm::dot(ray.dir, tri_norm); //
//     出射余弦量(三角面元法线与出射光夹角) float BRDF = 1.0f; //
//     应当根据gobj的材质计算出来，这里先假定为1.0，即光滑镜面反射

//     HitInfo new_obj = hit_obj({obj.hitPos + 0.01f * tri_norm, wi});
//     if (new_obj.isHit) {
//       if (vert_buffer != nullptr)
//         vert_buffer->push_back(new_obj.hitPos);
//       L_indir += trace_ray({new_obj.hitPos, -wi}, new_obj, PR,
//       recursive_depth + 1, vert_buffer) * BRDF * cosine / PR;
//     } else {
//       Pixel p = cubemap_sample(this->cubemaps, wi);
//       L_dir += glm::vec3(p.r, p.g, p.b) / 255.0f;
//       if (vert_buffer != nullptr)
//         vert_buffer->push_back(obj.hitPos + 10.0f * wi);
//     }
//   }

//   return L_dir + L_indir;
// }

// void compute_radiosity(uint32_t sample_N = 100) {
//   uniform_real_distribution<> distr(0.0, 1.0);
//   rdgen.seed(42);

//   lines["Ray"]->clear();
//   // 遍历每个场景(非aux)物体
//   for (auto &[name, cur_obj] : this->objs) {
//     if (cur_obj->isAux)
//       continue;
//     // if (!cur_obj->isSelected) // 暂时只计算被选中目标
//     //   continue;

//     mat4 model = cur_obj->transform.getModel();
//     shared_ptr<Geometry> geometry = cur_obj->geometry;
//     cur_obj->radiosity.radiant_flux.resize(geometry->surfaces.size());

//     for (int i = 0; i < geometry->surfaces.size(); i++) {
//       TriangleSampler tri(geometry->vertices, geometry->surfaces[i],
//       model); glm::vec3 tri_center = tri.calcCenter(); glm::vec3 tri_norm =
//       tri.calcNorm();

//       glm::vec3 L_dir{0.0f, 0.0f, 0.0f};
//       glm::vec3 L_indir{0.0f, 0.0f, 0.0f};
//       //
//       这里只计算irradiance，假定这个三角为黑体，吸收全部的辐射。否则，需要结合三角表面材质纹理来计算对irradiance的吸收率
//       // 调用前初始化this->ray_obj->vertices可视化光线路径

//       glm::vec3 wi = tri.hemisphereSampleDir();

//       shared_ptr<vector<glm::vec3>> vec_buffer = make_shared<vector<glm::vec3>>();
//       vec_buffer->push_back(tri_center + 0.005f * tri_norm);

//       HitInfo new_obj = hit_obj({tri_center + 0.005f * tri_norm, wi});
//       if (new_obj.isHit) {
//         vec_buffer->push_back(new_obj.hitPos);
//         // L_indir += trace_ray({new_obj.hitPos, -wi}, new_obj, 0.95, 0,
//         vec_buffer); L_indir += trace_ray({new_obj.hitPos, -wi}, new_obj,
//         0.95, vec_buffer);
//       } else {
//         vec_buffer->push_back(tri_center + 10.0f * wi);
//         Pixel p = cubemap_sample(this->cubemaps, wi);
//         L_dir += glm::vec3(p.r, p.g, p.b) / 255.0f;
//       }
//       lines["Ray"]->addPolygon(*vec_buffer);

//       // 将计算的irradiance的结果存储
//       glm::vec3 Lo = L_dir + L_indir;
//       cur_obj->radiosity.radiant_flux[i] = Lo;
//       // printf("物体\"%s\"第(%d/%llu)面元 radiance: %.4f\n", name.c_str(),
//       i, geometry->surfaces.size(), Lo);
//     }
//   }
//   lines["Ray"]->update(); // 调试
// }

void Scene::Scene::compute_radiosity(uint32_t sample_N) {
  uniform_real_distribution<> distr(0.0, 1.0);
  // rdgen.seed(42);

  lines["Ray"]->clear();
  // 遍历每个场景(非aux)物体
  for (auto& [cur_obj, context] : this->objs) {
    if (!cur_obj->status.listed)
      continue;
    // if (!cur_obj->isSelected) // 暂时只计算被选中目标
    //   continue;

    glm::mat4 model    = cur_obj->transform.getModel();
    Geometry* geometry = cur_obj->getGeometry();

    cur_obj->radiosity.radiant_flux.resize(geometry->getSurfaces().size());

    for (int i = 0; i < geometry->getSurfaces().size(); i++) {
      TriangleSampler tri(geometry->getVertices(), geometry->getSurfaces()[i], model);
      glm::vec3       tri_center = tri.calcCenter();
      glm::vec3       tri_norm   = tri.calcNorm();
      float           tri_area   = tri.calcArea();

      if (tri.calcArea() == 0.0f)   // 跳过无面积三角
        continue;

      // 可视化光线颜色
      uniform_real_distribution<> distr(0.1f, 0.9f);
      glm::vec3                   color = glm::vec3(distr(rdgen), distr(rdgen), distr(rdgen));
      // 可视化光线颜色

      glm::vec3 radiance_sum{0.0f, 0.0f, 0.0f};   // 蒙特卡洛积分，累加所有采样点的Radiance
      for (int k = 0; k < sample_N; k++) {
        glm::vec3         radiance{0.0f, 0.0f, 0.0f};
        vector<glm::vec3> vec_buffer;
        glm::vec3         wi = tri.hemisphereSampleDir();
        vec_buffer.push_back(tri_center +
                             SURFACE_NORMAL_OFFSET * tri_norm);   // 三角起点
        HitInfo new_obj =
          this->hit({tri_center + SURFACE_NORMAL_OFFSET * tri_norm, wi});
        if (new_obj.isHit) {
          radiance =
            trace_ray({new_obj.hitPos, -wi}, new_obj, 0.85, &vec_buffer);
        }
        else {
          vec_buffer.push_back(tri_center +
                               RAY_LENGTH_TO_CUBEMAP * wi);   // 直接射到cubemap
          Pixel p  = cubemap_sample(this->cubemaps, wi);
          radiance = glm::vec3(p.r, p.g, p.b) / 255.0f;
        }
        radiance_sum += radiance * glm::max(0.0f, glm::dot(tri_norm, wi));
        lines["Ray"]->addPolygon(vec_buffer, color);
      }
      glm::vec3 irradiance = radiance_sum * (2.0f * PI) /
                             static_cast<float>(sample_N);   // 蒙特卡洛积分
      cur_obj->radiosity.radiant_flux[i] = irradiance * tri_area;
    }
  }
  lines["Ray"]->update();   // 调试
}

// 迭代版本
glm::vec3 Scene::Scene::trace_ray(Ray ray, const HitInfo& obj, float PR, vector<glm::vec3>* vert_buffer) {
  // 参数：1. 光线  2. 光线源头三角信息  3. 光线存活率  4.
  // 可视化光线的顶点缓存

  assert(PR < 1.0f && PR > 0.0f);
  assert(obj.isHit && "trace_ray() has no target!");

  uniform_real_distribution<> distr(0.0, 1.0);
  // glm::vec3 L_dir{0.0f, 0.0f, 0.0f}, L_indir{0.0f, 0.0f, 0.0f};
  glm::vec3 L_sample{0.0f, 0.0f, 0.0f};

  Ray     cur_ray = ray;   // 由cur_obj发出的光线cur_ray
  HitInfo cur_obj = obj;

  // 需要记录下每条光线的信息
  vector<RayInfo> ray_stack;

  while (true) {
    float PR_D = distr(rdgen);   // 决定新光线的存活概率，若PR>PR_D则存活
    if (PR_D >= PR)
      break;   // 光线死亡，不再弹射

    shared_ptr<Component::GeometryObject> gobj =
      findGeometryObjectByName(cur_obj.geometryName);
    assert(gobj != nullptr && "hit geometry not found!");

    Geometry* geometry = gobj->getGeometry();
    glm::mat4 model    = gobj->transform.getModel();

    TriangleSampler tri(geometry->getVertices(),
                        geometry->getSurfaces()[cur_obj.triangleIdx],
                        model);
    glm::vec3       tri_norm = tri.calcNorm();
    glm::vec3       wi       = tri.hemisphereSampleDir();

    if (vert_buffer)
      vert_buffer->push_back(cur_obj.hitPos + SURFACE_NORMAL_OFFSET *
                                                tri_norm);   // 可视化光线轨迹

    // ====================计算直接光照========================
    // （这里算的不正确，应当对光源进行直接采样）
    // 暂时不考虑该分量
    // Pixel p = cubemap_sample(this->cubemaps, wi);
    // cur_L_dir += glm::vec3(p.r, p.g, p.b) / 255.0f;

    // ====================计算间接光照========================
    Ray     new_ray{cur_obj.hitPos + SURFACE_NORMAL_OFFSET * tri_norm,
                wi};   // 反向追踪上一根入射光线的发出对象
    HitInfo new_obj = this->hit(new_ray);
    if (new_obj.isHit) {
      // printf("击中物体: %s\n", new_obj.geometry_name.c_str());
      // 找到光源物体
      // 累加计算间接辐射率
      RayInfo ray_info;
      ray_info.BRDF = glm::vec3(1.0f, 1.0f, 1.0f) / PI;   // 假定理想朗伯体，完全漫反射
      ray_info.cosine =
        glm::dot(cur_ray.dir,
                 tri_norm);         // 出射余弦量(三角面元法线与出射光夹角)
      if (ray_info.cosine < 0.0f)   // 跳过本次wi的半球采样
        continue;
      ray_stack.emplace_back(ray_info);
    }
    else {
      if (vert_buffer)
        vert_buffer->push_back(cur_obj.hitPos +
                               RAY_LENGTH_TO_CUBEMAP * wi);   // 可视化光线轨迹

      // 未找到光源物体，说明光线由cubemap发出，直接采样
      Pixel p  = cubemap_sample(this->cubemaps, wi);
      L_sample = glm::vec3(p.r, p.g, p.b) / 255.0f;
      break;   // 找到光源，光线不再弹射
    }

    cur_ray = Ray{new_obj.hitPos, -wi};
    cur_obj = new_obj;
  }

  // 反过来，从光线发出位置计算对三角第i(总共投射出N)根光线的radiance的贡献
  while (!ray_stack.empty()) {
    RayInfo info = ray_stack.back();
    ray_stack.pop_back();
    L_sample *= info.BRDF * info.cosine * (2.0f * PI) / PR;
  }

  // printf("L_sample = (%.2f, %.2f, %.2f)\n", L_sample.x, L_sample.y,
  // L_sample.z);

  return L_sample;
}

void Scene::Scene::imgui_docking_render(bool* p_open) {
  // Variables to configure the Dockspace example.
  static bool opt_padding = false;   // Is there padding (a blank space) between
                                     // the window edge and the Dockspace?
  static ImGuiDockNodeFlags dockspace_flags =
    ImGuiDockNodeFlags_None;   // Config flags for the Dockspace
  dockspace_flags |= ImGuiDockNodeFlags_PassthruCentralNode |
                     ImGuiDockNodeFlags_AutoHideTabBar;

  // ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

  const ImGuiViewport* viewport = ImGui::GetMainViewport();

  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGui::SetNextWindowViewport(viewport->ID);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

  window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                  ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
  window_flags |=
    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

  if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
    window_flags |= ImGuiWindowFlags_NoBackground;

  if (!opt_padding)
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

  ImGui::Begin("DockSpace Demo", p_open, window_flags);

  if (!opt_padding)
    ImGui::PopStyleVar();

  ImGui::PopStyleVar(2);

  ImGuiIO& io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    ImGuiID dockspace_id = ImGui::GetID("Docking Space");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
  }
  else {
    cout << "warning: docking is disabled!" << endl;
  }

  ImGui::End();
}

void Scene::Scene::imgui_docking_config() {
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  // 声明一个DockSpace ID
  ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
  // 拆分这个dockspace
}

void Scene::Scene::render() {
  glm::mat4 view       = this->camera.getView();
  glm::mat4 projection = this->camera.getProject();

  // 启用帧缓冲进行预渲染
  glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer.fbo);
  glEnable(GL_DEPTH_TEST);

  if (this->isShowWireFrame) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }

  // 更新P,V矩阵
  glBindBuffer(GL_UNIFORM_BUFFER, this->ubo);
  if (this->camera.isProjectionChanged()) {
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    this->camera.apply_projection_done();
  }
  if (this->camera.isViewChanged()) {
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
    this->camera.apply_view_done();
  }

  Shader* cur_shader{nullptr};

  glViewport(0, 0, width, height);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // 场景辅助元素（天空盒）
  glDepthMask(GL_FALSE);
  cur_shader = this->shaders["skybox"];
  cur_shader->use();
  glBindTexture(GL_TEXTURE_CUBE_MAP, this->skybox.texture);
  glBindVertexArray(this->skybox.vao);
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
  glDepthMask(GL_TRUE);

  // 1. 常规物体渲染
  cur_shader = this->shaders["default"];
  cur_shader->use();
  for (auto& [cur_obj, context] : this->objs) {
    glViewport(0, 0, width, height);
    cur_shader->set("disable_view", false);
    if (!cur_obj->status.visible)
      continue;
    if (cur_obj->status.lighted) {
      cur_shader->set("useLight", true);
      cur_shader->set("lightPos", this->light.position);
      cur_shader->set("lightColor", this->light.color);
      cur_shader->set("eyePos", this->camera.getPosition());
      cur_shader->set("ambientStrength", 0.2f);
      cur_shader->set("diffuseStrength", 1.0f);
      cur_shader->set("specularStrength", 1.0f);
    }
    else {
      cur_shader->set("useLight", false);
    }
    if (context->texture != 0) {
      cur_shader->set("useTexture", true);
    }
    else {
      cur_shader->set("useTexture", false);
    }
    glm::mat4 model = cur_obj->transform.getModel();
    if (cur_obj->getName() == "Axis") {
      // 只记录view中的姿态，不记录位置偏移
      // 因此需要关闭view，在这里转为model表示
      cur_shader->set("disable_view", true);
      float     phi          = this->camera.getPhi() / 180.0f * PI;
      float     theta        = this->camera.getTheta() / 180.0f * PI;
      glm::vec3 from         = {1.5f * sinf(PI - theta) * cosf(PI - phi),
                                1.5f * cosf(PI - theta),
                                1.5f * sinf(PI - theta) * sinf(PI - phi)};
      model                  = glm::lookAt(from, glm::vec3(0.0f, 0.0f, 0.0f), _up);
      glm::mat4 orth_project = glm::ortho(-1.5f, 1.5f, -1.5f, 1.5f, 0.0f, 10.0f);
      glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
                      glm::value_ptr(orth_project));   // 使用正交投影矩阵
      glViewport(5, this->height - 210, 200, 200);
    }

    cur_shader->set("model", model);
    // 启用材质，设置材质属性
    glBindTexture(GL_TEXTURE_2D, context->texture);
    // 绘制
    glBindVertexArray(context->getVAO());
    glDrawElements(GL_TRIANGLES, context->getSize(), GL_UNSIGNED_INT, nullptr);

    if (cur_obj->getName() == "Axis") {   // 恢复透视矩阵
      glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(this->camera.getProject()));
    }
  }

#ifdef ENABLE_NORMAL_VISUALIZATION
  // 2. 渲染法向量
  for (auto& [name, cur_obj] : this->objs) {
    if (cur_obj->isSelected) {
      cur_shader = this->shaders["normal"];
      cur_shader->use();
      cur_shader->set("model", cur_obj->transform.getModel());

      glBindVertexArray(cur_obj->vao);
      glDrawArrays(GL_POINTS, 0, cur_obj->geometry->getVertices().size());
    }
  }
#endif

  // 3. 渲染包围盒边框
  cur_shader = this->shaders["line"];
  cur_shader->use();
  cur_shader->set("lineColor", glm::vec3(0.0f, 1.0f, 1.0f));
  for (auto& [cur_obj, context] : this->objs) {
    if (cur_obj->status.selected) {
      BvhTree* tree = cur_obj->getBvhTree();
      // assert(tree != nullptr && "Scene::render() bvhtree is null!");
      if (tree != nullptr && this->isShowBvhFrame) {
        // 渲染层次包围盒
        // tree->traverse([](BvhNode* node) {
        //   assert(node->box->context != nullptr);
        //   glBindVertexArray(node->box->context->getVAO());
        //   glDrawElements(GL_LINES, node->box->context->getSize(),
        //   GL_UNSIGNED_INT, nullptr);
        // });
        for (auto& box_context : context->boxes) {
          glBindVertexArray(box_context->getVAO());
          glDrawElements(GL_LINES, box_context->getSize(), GL_UNSIGNED_INT, nullptr);
        }
      }
      else {
        // 渲染整体包围盒
        // BoundingBox* box = cur_obj->getBoundingBox();
        // assert(box != nullptr && "Scene::render() Boundingbox失效!");
        glBindVertexArray(context->box->getVAO());
        glDrawElements(GL_LINES, context->box->getSize(), GL_UNSIGNED_INT, nullptr);
      }
    }
  }

  // 4. 可视化光线追踪
  if (this->isShowRay)
    lines["Ray"]->draw(cur_shader);

  // 5. 可视化三角局部坐标系
  if (this->isShowCoord)
    lines["Coord"]->draw(cur_shader);

  // 6. 进行最后的帧缓冲处理

  if (this->isShowWireFrame) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
  // 7. 解绑自定义帧缓冲，返回默认帧缓冲进行绘制
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, width, height);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  this->shaders["screen"]->use();
  glBindVertexArray(this->framebuffer.vao);
  glDisable(GL_DEPTH_TEST);
  glBindTexture(GL_TEXTURE_2D, this->framebuffer.texture);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

  // 8. 从有向光视角生成深度缓存
  glBindFramebuffer(GL_FRAMEBUFFER, this->depthmap.fbo);
  glViewport(0, 0, width, height);
  cur_shader = this->shaders["lightDepth"];
  cur_shader->use();
  cur_shader->set("projection",
                  glm::ortho(this->depthmap.left, this->depthmap.right, this->depthmap.bottom, this->depthmap.top, this->depthmap.near, this->depthmap.far));
  cur_shader->set("view",
                  glm::lookAt(this->light.position, {0.0f, 0.0f, 0.0f}, _up));
  glClear(GL_DEPTH_BUFFER_BIT);
  for (auto& [cur_obj, context] : this->objs) {
    cur_shader->set("model", cur_obj->transform.getModel());
    glBindVertexArray(context->getVAO());
    glDrawElements(GL_TRIANGLES, context->getSize(), GL_UNSIGNED_INT, nullptr);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void Scene::Scene::setWindowSize(float width, float height) {
  this->width  = width;
  this->height = height;
}

void Scene::Scene::mainloop() {
  chrono::time_point<chrono::system_clock> start       = chrono::system_clock::now();
  uint32_t                                 frame_count = 0;
  float                                    duration_sec;
  // imgui_docking_config();
  glfwShowWindow(this->window);
  while (!glfwWindowShouldClose(this->window)) {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    duration_sec =
      static_cast<float>(chrono::duration_cast<chrono::milliseconds>(
                           chrono::system_clock::now() - start)
                           .count()) *
      chrono::milliseconds::period::num / chrono::milliseconds::period::den;
    if (duration_sec > FPS_SHOW_SPAN) {
      auto duration_milisec = chrono::duration_cast<chrono::milliseconds>(
        chrono::system_clock::now() - start);
      float fps = static_cast<float>(frame_count) / duration_sec;
      printf("fps: %.2f\n", fps);
      start        = chrono::system_clock::now();
      duration_sec = 0;
      frame_count  = 0;
    }

    render();

    imgui_interact();
    imgui_docking_render();
    if (imgui_menu())
      break;

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // imgui实现外部窗口的必要配置
    if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(this->window);
    }
    glfwSwapBuffers(window);
    frame_count++;
  }
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
}

void Scene::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
  Scene* scene = (Scene*)glfwGetWindowUserPointer(window);
  assert(scene != nullptr && "scene is nullptr");
  scene->setWindowSize(width, height);
  scene->camera.getAspect() = static_cast<float>(width) / height;
  glViewport(0, 0, width, height);

  // 同时会更新scene的framebuffer
  scene->resize_framebuffer();
}
