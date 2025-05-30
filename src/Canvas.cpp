#include "Canvas.h"

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

Canvas::Canvas(QWidget *parent) : QOpenGLWidget(parent) {
  this->camera = std::make_unique<Camera>(
      glm::vec3(0.0f, 0.0f, 20.0f), glm::vec3(0.0f, 0.0f, -1.0f),
      static_cast<float>(this->width()) / this->height());
}

void Canvas::setBackground(const glm::vec4 &color) {
  glClearColor(color[0], color[1], color[2], color[3]);
}

void Canvas::initializeGL() {
  initializeOpenGLFunctions();
  makeCurrent();
  // =====================
  // 开启MSAA抗锯齿
  glEnable(GL_MULTISAMPLE);
  // 开启深度测试
  glEnable(GL_DEPTH_TEST);
  // 开启面剔除
  // glEnable(GL_CULL_FACE);
  // 顺时针索引顺序为正面
  glFrontFace(GL_CW);
  glLineWidth(1.5f);
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);

  // ==========显卡信息============
  const GLubyte *vendor = glGetString(GL_VENDOR);
  const GLubyte *renderer = glGetString(GL_RENDERER);
  std::printf("Vendor: %s\n", vendor);
  std::printf("Graphics Device: %s\n", renderer);

  // =========================

  load_all_shader();
  load_all_texture();

  init_ubo();
  init_scene_obj();
  init_skybox();
  init_framebuffer();
  init_depthmap();
  init_line_buffer();
}

void Canvas::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // 。。。
}
void Canvas::resizeGL(int w, int h) { glViewport(0, 0, w, h); }


void Canvas::updateGeometryListView() {
  std::vector<std::shared_ptr<Component::GeometryObject>> tmp_item_view;
  for (auto &[obj, ctx] : this->objs) {
    if (obj->flag & ObjectFlag_LISTED)
      tmp_item_view.emplace_back(obj);
  }
  //this->imgui.list_items = tmp_item_view;
  // todo: 替换为qt中列表的更新
}

std::shared_ptr<Component::GeometryObject>
Canvas::findGeometryObjectByName(const std::string &name) {
  auto iter = std::find_if(this->objs.begin(), this->objs.end(),
                      [&](std::pair<const std::shared_ptr<Component::GeometryObject>,
                               std::unique_ptr<GeometryContext>> &obj_pair) {
                        return obj_pair.first->getName().compare(name) == 0;
                      });
  if (iter == this->objs.end())
    return nullptr;
  return iter->first;
}

void Canvas::addSceneObject(
    const std::shared_ptr<Component::GeometryObject> &obj, uint32_t flag,
    GLuint texture) {
  std::shared_ptr<Component::GeometryObject> ptr =
      findGeometryObjectByName(obj->getName());
  if (ptr != nullptr) {
    printf("scene cannot add \"SceneObject aux\" with an existed name \"%s\"\n",
           obj->getName().c_str());
    return;
  }
  obj->flag = flag;

  // 创建Context
  std::unique_ptr<GeometryContext> context = std::make_unique<GeometryContext>(obj.get());
  context->init();
  context->texture = texture;

  this->objs[obj] = std::move(context);

  updateGeometryListView();
}

void Canvas::load_all_shader() {
  std::printf("compiling shader...\n");
  shaders["default"] = new Shader("default.vert", "default.frag");
  shaders["normal"] = new Shader("normal.vert", "normal.geom", "normal.frag");
  shaders["skybox"] = new Shader("skybox.vert", "skybox.frag");
  shaders["lightDepth"] = new Shader("lightDepth.vert", "lightDepth.frag");
  shaders["line"] = new Shader("line.vert", "line.frag");
  shaders["screen"] = new Shader("screen.vert", "screen.frag");
  std::printf("compile shader done!\n");
}
void Canvas::load_all_texture() {
  fs::path texture_dir =
      QApplication::applicationDirPath().toStdString() + "/assets/textures";
  if (!fs::exists(texture_dir)) {
    printf("\"%s\" not exists!\n", texture_dir.string().c_str());
    exit(1);
  }
  for (auto &file : fs::directory_iterator(texture_dir)) {
    if (file.is_regular_file()) {
      // 加载纹理
      std::string fname = file.path().string();
      // 读取图片文件
      QImage img(QString::fromStdString(fname));
      if (img.isNull()) {
        printf("load texture: \"%s\" failed!\n", fname.c_str());
        exit(2);
      }

      // 创建纹理缓冲
      GLuint new_texture;
      glGenTextures(1, &new_texture);
      glBindTexture(GL_TEXTURE_2D, new_texture);

      glTextureParameteri(new_texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTextureParameteri(new_texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTextureParameteri(new_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTextureParameteri(new_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      img = img.convertToFormat(QImage::Format_RGB32);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width(), img.height(), 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, img.constBits());
      glGenerateTextureMipmap(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, 0);

      this->textures[file.path().filename().stem().string()] = new_texture;
    }
  }
}
void Canvas::init_ubo() {
  glGenBuffers(1, &this->camera_ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, this->camera_ubo);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, nullptr,
               GL_STATIC_DRAW);

  glm::mat4 projecion = this->camera->getProject();
  glm::mat4 view = this->camera->getView();
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
                  glm::value_ptr(projecion));
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4),
                  glm::value_ptr(view));

  glBindBufferBase(GL_UNIFORM_BUFFER, PVM_binding_point, this->camera_ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
void Canvas::init_scene_obj() {
  uint32_t obj_flag{ObjectFlag_None};
  // 光源
  std::shared_ptr<Geometry> lightBall = Mesh::Sphere(0.07f, 36, 18);
  lightBall->update();
  lightBall->setColor(1.0f, 1.0f, 1.0f);
  std::shared_ptr<Component::GeometryObject> obj1 =
      make_shared<Component::GeometryObject>("Light", lightBall);
  obj_flag = this->canvas_flag & CanvasFlag_SHOW_LIGHT ? ObjectFlag_VISIBLE: ObjectFlag_None;
  this->addSceneObject(obj1, obj_flag, 0);

  // 坐标轴
  std::shared_ptr<Geometry> axis = std::make_shared<CoordinateAxis>(0.1, 1.0f);
  std::shared_ptr<Component::GeometryObject> obj2 =
      std::make_shared<Component::GeometryObject>("Axis", axis);
  obj_flag = this->canvas_flag & CanvasFlag_SHOW_AXIS ? ObjectFlag_VISIBLE
                                                      : ObjectFlag_None;
  this->addSceneObject(obj2, obj_flag, 0);

  // 游标
  std::shared_ptr<Geometry> cursor = std::make_shared<CoordinateAxis>(0.02, 0.5f);
  std::shared_ptr<Component::GeometryObject> cursor_obj =
      make_shared<Component::GeometryObject>(
          "Cursor", cursor, Transform({glm::vec3(0.0f, 2.0f, 0.0f)}));
  obj_flag = this->canvas_flag & CanvasFlag_SHOW_CURSOR
                  ? ObjectFlag_VISIBLE | ObjectFlag_LISTED
                 : ObjectFlag_None;
  this->addSceneObject(cursor_obj, obj_flag, 0);

  // 地面
  std::shared_ptr<Geometry> ground = Mesh::Plane(20.0f, 20.0f, 10, 10);
  // 为了让光线不在两个重叠面上抖动进而穿透，将Ground下移一个微小距离
  std::shared_ptr<Component::GeometryObject> obj3 =
      std::make_shared<Component::GeometryObject>(
          "Ground", ground, Transform({0.0f, -0.1f, 0.0f}), true);
  obj_flag = ObjectFlag_VISIBLE | ObjectFlag_LIGHTED | ObjectFlag_COLLIDED;
  this->addSceneObject(obj3, obj_flag, this->textures["fabric"]);

  // 加入的侧面全为可见、光照、碰撞
  obj_flag = ObjectFlag_VISIBLE | ObjectFlag_LIGHTED | ObjectFlag_COLLIDED;
  // 左侧面
  std::shared_ptr<Geometry> side_left = Mesh::Plane(20.0f, 20.0f, 10, 10);
  side_left->setColor(0.0f, 0.0f, 1.0f);
  std::shared_ptr<Component::GeometryObject> side_left_obj =
      make_shared<Component::GeometryObject>(
          "Side_left", side_left,
          Transform({-10.0f, 9.9f, 0.0f}, _front, glm::radians(90.0f)), true);
  this->addSceneObject(side_left_obj, obj_flag, 0);

  // 后侧面
  std::shared_ptr<Geometry> side_back = Mesh::Plane(20.0f, 20.0f, 10, 10);
  side_back->setColor(0.0f, 1.0f, 0.0f);
  std::shared_ptr<Component::GeometryObject> side_back_obj =
      make_shared<Component::GeometryObject>(
          "Side_back", side_back,
          Transform({0.0f, 9.9f, -10.0f}, _right, glm::radians(90.0f)), true);
  this->addSceneObject(side_back_obj, obj_flag, 0);

  // 上侧面
  std::shared_ptr<Geometry> side_top = Mesh::Plane(20.0f, 20.0f, 10, 10);
  side_top->setColor(1.0f, 0.0f, 0.0f);
  std::shared_ptr<Component::GeometryObject> side_top_obj =
      make_shared<Component::GeometryObject>(
          "Side_top", side_top,
          Transform({0.0f, 19.9f, 0.0f}, _right, glm::radians(180.0f)), true);
  this->addSceneObject(side_top_obj, obj_flag, 0);
}
void Canvas::init_skybox() {
  // 与前面的GL_TEXTURE_2D纹理目标不同，天空盒使用GL_TEXTURE_CUBE_MAP_XXX作为纹理目标

  // 1. 加载VBO
  std::vector<glm::vec3> vertices = {
      {-1.0, -1.0, -1.0}, {-1.0, -1.0, 1.0}, {-1.0, 1.0, -1.0},
      {-1.0, 1.0, 1.0},   {1.0, -1.0, -1.0}, {1.0, -1.0, 1.0},
      {1.0, 1.0, -1.0},   {1.0, 1.0, 1.0},
  };
  std::vector<uint32_t> surfaces = {1, 5, 7, 1, 7, 3, 0, 2, 6, 0, 6, 4,
                               5, 4, 6, 5, 6, 7, 0, 1, 3, 0, 3, 2,
                               4, 5, 1, 4, 1, 0, 3, 7, 6, 3, 6, 2};

  glGenVertexArrays(1, &this->skybox.vao);
  glBindVertexArray(this->skybox.vao);

  glGenBuffers(1, &this->skybox.vbo);
  glBindBuffer(GL_ARRAY_BUFFER, this->skybox.vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
               vertices.data(), GL_STATIC_DRAW);

  glGenBuffers(1, &this->skybox.ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->skybox.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, surfaces.size() * sizeof(GLuint),
               surfaces.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glEnableVertexAttribArray(0);

  // 2. 加载纹理
  glGenTextures(1, &this->skybox.texture);
  glBindTexture(GL_TEXTURE_CUBE_MAP, this->skybox.texture);

  std::vector<std::string> skybox_texture_names = {"px", "nx", "py", "ny", "pz", "nz"};
  for (int i = 0; i < skybox_texture_names.size(); i++) {
    std::string fname = "assets/textures/skybox/" + skybox_texture_names[i] + ".png";
    int width, height, channel;
    QImage img(QString::fromStdString(fname));
    uchar *image_data = img.convertToFormat(QImage::Format_RGBA8888).bits();

    if (image_data == 0) {
      printf("load skybox texture failed: \"%s\"\n",fname.c_str());
      continue;
    }
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, img.width(),
                 img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}
void Canvas::init_framebuffer() {
  // 深度缓冲
  glGenFramebuffers(1, &this->framebuffer.fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer.fbo);

  // 创建纹理附件
  glGenTextures(1, &this->framebuffer.texture);
  glBindTexture(GL_TEXTURE_2D, this->framebuffer.texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width(), this->height(), 0,
               GL_RGB,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // 将“颜色附件”附着给fbo
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         this->framebuffer.texture, 0);

  // 创建渲染缓冲用于缓存“深度、模板”
  glGenRenderbuffers(1, &this->framebuffer.rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, this->framebuffer.rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, this->width(),
                        this->height());
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  // 将“深度、模板附件”附着给fbo
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, this->framebuffer.rbo);
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
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
}
void Canvas::init_depthmap() {
  glGenTextures(1, &this->depthmap.texture);
  glBindTexture(GL_TEXTURE_2D, this->depthmap.texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, this->width(), this->height(), 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glGenFramebuffers(1, &this->depthmap.fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, this->depthmap.fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         this->depthmap.texture, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void Canvas::init_line_buffer() {
  this->lines["Ray"] = std::make_shared<LineDrawer>();
  this->lines["Coord"] = std::make_shared<LineDrawer>();
}