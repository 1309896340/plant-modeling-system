#pragma once

#include <cassert>
#include <map>
#include <string>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


class Shader {
  // 加载当前目录下的"shaders/"子目录的glsl文件，封装为Shader Program
  private:
  GLuint program_id{0};
  // 缓存uniform变量名与location
  std::map<std::string, GLuint> locations;
  const std::string             prefix = "assets\\shaders\\";
  void                          readFile(std::string& dst, const std::string& filename);
  std::string                   shaderType2str(GLuint shader_type);
  GLuint                        createShader(GLuint shader_type, const std::string& source);
  GLuint                        createShader(const std::string& shader_source_filename);

  public:
  Shader(const std::string& compute_filename);
  Shader(const std::string& vertex_filename, const std::string& geometry_filename, const std::string& fragment_filename);
  Shader(const std::string& vertex_filename, const std::string& fragment_filename);
  Shader(const Shader& sd) = delete;
  ~Shader();
  bool has_uniform(const std::string& name) const;
  void register_location(const std::string& name);
  void set(const std::string& name, bool bool_val);
  void set(const std::string& name, unsigned int uint_val);
  void set(const std::string& name, int int_val);
  void set(const std::string& name, float float_val);
  void set(const std::string& name, const glm::vec3& vec3_val);
  void set(const std::string& name, const glm::vec4& vec4_val);
  void set(const std::string& name, const glm::mat3& mat3_val);
  void set(const std::string& name, const glm::mat4& mat4_val);
  void use();

  GLuint program() const;
};
