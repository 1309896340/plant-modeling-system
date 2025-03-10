#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


using prop = std::variant<uint32_t, int32_t, float, double, bool, char, glm::vec3>;

struct Vertex {
  union {
    float position[3];
    struct {
      float x;
      float y;
      float z;
    };
  };
  union {
    float normal[3];
    struct {
      float nx;
      float ny;
      float nz;
    };
  };
  union {
    float color[3];
    struct {
      float r;
      float g;
      float b;
    };
  };
  union {
    unsigned int uv[2];
    struct {
      float u;
      float v;
    };
  };
};


struct Surface {
  uint32_t tidx[3];   // 3个Vertex构成
};

class Observer : public std::enable_shared_from_this<Observer> {
  public:
  virtual void notify(const std::string& name, const prop& parameter) = 0;
};

class ReflectValue : public Observer {
  private:
  std::string                          name;
  prop                                 value;
  std::vector<std::weak_ptr<Observer>> observers;
  std::function<float(prop)>           func{nullptr};   // 可能存在函数关系的映射

  bool topoFlag{false};

  public:
  ReflectValue(const std::string& name, prop init_value);
  ReflectValue(const std::string& name, prop init_value, std::weak_ptr<Observer> observer);
  ReflectValue(const std::string& name, prop init_value, bool isTopo);
  ReflectValue(const std::string& name, prop init_value, bool isTopo, std::weak_ptr<Observer> observer);
  prop& getProp();
  bool  isTopo() const;

  virtual void notify(const std::string& name, const prop& param);

  void        notifyAll();
  std::string getName() const;
  void        addObserver(std::weak_ptr<Observer> observer);
};

class Geometry : public Observer {
  protected:
  std::vector<Vertex>  vertices;
  std::vector<Surface> surfaces;

  public:
  std::map<std::string, std::shared_ptr<ReflectValue>> parameters;

  Geometry()                = default;
  Geometry(const Geometry&) = default;
  ~Geometry()               = default;

  std::vector<Vertex>&        getVertices();
  std::vector<Surface>&       getSurfaces();
  const std::vector<Vertex>&  getVertices() const;
  const std::vector<Surface>& getSurfaces() const;

  void translate(float x_offset, float y_offset, float z_offset);

  void rotate(float angle, glm::vec3 axis);

  void setColor(float r, float g, float b);

  virtual void update() = 0;
  virtual void reset();
};

struct FixedGeometry {
  std::vector<Vertex>  vertices;
  std::vector<Surface> surfaces;
  FixedGeometry() = default;
  FixedGeometry(const std::vector<Vertex>& vertices, const std::vector<Surface>& surfaces);
  FixedGeometry(const Geometry& geo);
};

FixedGeometry operator+(const FixedGeometry& a, const FixedGeometry& b);

class Mesh : public Geometry {
  using MeshUpdater = std::function<Vertex(float, float)>;

  private:
  MeshUpdater updater;
  bool        topo_flag{true};

  public:
  Mesh(uint32_t uNum, uint32_t vNum);
  Mesh(uint32_t uNum, uint32_t vNum, MeshUpdater updater);

  virtual void update();

  // 由于updater的参数构建比较复杂，且比较抽象，以下的简单赋值并不足以正确完成updater的替换
  // 下面这种做法是存在问题的，使用时注意lambda内部捕获的变量需要重新绑定到parameters中
  void setUpdater(MeshUpdater updater);

  void resize();

  virtual void notify(const std::string& name, const prop& param);

  static std::shared_ptr<Mesh> Sphere(float radius, uint32_t PNum, uint32_t VNum);

  static std::shared_ptr<Mesh> Disk(float radius, uint32_t PNum, uint32_t RNum);
  static std::shared_ptr<Mesh> ConeSide(float radius, float height, uint32_t PNum, uint32_t HNum);

  static std::shared_ptr<Mesh> CylinderSide(float radius, float height, uint32_t PNum, uint32_t HNum);

  static std::shared_ptr<Mesh> Plane(float width, float height, uint32_t VNum, uint32_t HNum);
};

using TransformUpdater = std::function<glm::mat4()>;

class Composition : public Geometry {
  private:
  std::vector<std::shared_ptr<Geometry>> goemetries;
  std::vector<TransformUpdater>          transforms;

  public:
  Composition() = default;

  void pushGeometry(std::shared_ptr<Geometry> mesh,
                    const TransformUpdater&   transform);

  void popMesh();

  virtual void                        notify(const std::string& name, const prop& param);
  virtual void                        update();
  static std::shared_ptr<Composition> Cube(float xWidth, float yWidth, float zWidth, uint32_t xNum, uint32_t yNum, uint32_t zNum);
  static std::shared_ptr<Composition> Cylinder(float radius, float height, uint32_t PNum, uint32_t RNum, uint32_t HNum);

  static std::shared_ptr<Composition> Cone(float radius, float height, uint32_t PNum, uint32_t RNum, uint32_t HNum);

  static std::shared_ptr<Composition> Arrow(float radius, float length);
};

// } // namespace