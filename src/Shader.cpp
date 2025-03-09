#pragma once

#include "Shader.h"

namespace fs = std::filesystem;

void Shader::readFile(std::string& dst, const std::string& filename) {
  std::stringstream ss;
  std::ifstream     fp(filename);
  ss << fp.rdbuf();
  dst = ss.str();
}

std::string Shader::shaderType2str(GLuint shader_type) {
  switch (shader_type) {
  case GL_VERTEX_SHADER:
    return "Vertex Shader";
  case GL_FRAGMENT_SHADER:
    return "Fragment Shader";
  case GL_COMPUTE_SHADER:
    return "Compute Shader";
  case GL_GEOMETRY_SHADER:
    return "Geometry Shader";
  case GL_TESS_EVALUATION_SHADER:
    return "Tesselation Evaluation Shader";
  case GL_TESS_CONTROL_SHADER:
    return "Tesselation Control Shader";
  default:
    return "Uknown Shader";
  }
}

GLuint Shader::createShader(GLuint shader_type, const std::string& source) {
  // 根据source编译源文件
  GLuint shader = glCreateShader(shader_type);
  assert(shader != 0);
  const char* ss  = source.c_str();
  GLint       len = source.size();
  glShaderSource(shader, 1, &ss, &len);
  glCompileShader(shader);

  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    std::cout << "failed to compile " << shaderType2str(shader_type) << std::endl;
#ifndef NDEBUG
    char log[200] = {0};
    int  log_num  = 0;
    glGetShaderInfoLog(shader, 200, &log_num, log);
    std::cout << log << std::endl;
#endif
    return 0;
  }
  return shader;
}

GLuint Shader::createShader(const std::string& shader_source_filename) {
  // 读取glsl文件源码创建shader，着色器类型由文件名后缀来标识
  fs::path    fname = shader_source_filename;
  std::string ext   = fname.filename().extension().string();
  std::string source;
  readFile(source, shader_source_filename);
  GLuint shader{0};
  if (ext == ".vert") {
    shader = createShader(GL_VERTEX_SHADER, source);
  }
  else if (ext == ".frag") {
    shader = createShader(GL_FRAGMENT_SHADER, source);
  }
  else if (ext == ".geom") {
    shader = createShader(GL_GEOMETRY_SHADER, source);
  }
  else if (ext == ".comp") {
    shader = createShader(GL_COMPUTE_SHADER, source);
  }
  else {
    std::string msg = "unknown glsl extension \"" + ext + "\"";
    std::cerr << msg << std::endl;
    throw std::runtime_error(msg);
  }
  assert(shader != 0);
  return shader;
}

Shader::Shader(const std::string& compute_filename) {
  GLuint cshader = createShader(prefix + compute_filename);

  this->program_id = glCreateProgram();
  glAttachShader(this->program_id, cshader);
  glLinkProgram(this->program_id);

  GLint success;
  glGetProgramiv(this->program_id, GL_LINK_STATUS, &success);
  if (!success) {
    std::cout << "failed to link shader program!" << std::endl;
#ifndef NDEBUG
    char log[200];
    int  log_num;
    glGetProgramInfoLog(this->program_id, 200, &log_num, log);
    std::cout << log << std::endl;
#endif
  }
  glDeleteShader(cshader);
}

Shader::Shader(const std::string& vertex_filename, const std::string& geometry_filename, const std::string& fragment_filename) {
  GLuint vshader = createShader(prefix + vertex_filename);
  GLuint gshader = createShader(prefix + geometry_filename);
  GLuint fshader = createShader(prefix + fragment_filename);

  this->program_id = glCreateProgram();
  glAttachShader(this->program_id, vshader);
  glAttachShader(this->program_id, gshader);
  glAttachShader(this->program_id, fshader);
  glLinkProgram(this->program_id);

  GLint success;
  glGetProgramiv(this->program_id, GL_LINK_STATUS, &success);
  if (!success) {
    std::cout << "failed to link shader program!" << std::endl;
#ifndef NDEBUG
    char log[200];
    int  log_num;
    glGetProgramInfoLog(this->program_id, 200, &log_num, log);
    std::cout << log << std::endl;
#endif
  }
  glDeleteShader(vshader);
  glDeleteShader(gshader);
  glDeleteShader(fshader);
}

Shader::Shader(const std::string& vertex_filename, const std::string& fragment_filename) {
  GLuint vshader = createShader(prefix + vertex_filename);
  GLuint fshader = createShader(prefix + fragment_filename);

  this->program_id = glCreateProgram();
  glAttachShader(this->program_id, vshader);
  glAttachShader(this->program_id, fshader);
  glLinkProgram(this->program_id);

  GLint success;
  glGetProgramiv(this->program_id, GL_LINK_STATUS, &success);
  if (!success) {
    std::cout << "failed to link shader program!" << std::endl;
#ifndef NDEBUG
    char log[200];
    int  log_num;
    glGetProgramInfoLog(this->program_id, 200, &log_num, log);
    std::cout << log << std::endl;
#endif
  }
  glDeleteShader(vshader);
  glDeleteShader(fshader);
}

Shader::~Shader() {
  glDeleteProgram(this->program_id);
}

bool Shader::has_uniform(const std::string& name) const {
  return this->locations.find(name) != this->locations.end();
}

void Shader::register_location(const std::string& name) {
  GLuint loc = glGetUniformLocation(this->program_id, name.c_str());
  if (loc == -1) {
    std::string msg = "unknown uniform name: \"" + name + "\"";
    std::cerr << msg << std::endl;
    throw std::runtime_error(msg);
  }
  this->locations[name] = loc;
}

void Shader::set(const std::string& name, bool bool_val) {
  if (!this->has_uniform(name))
    register_location(name);
  glUniform1ui(this->locations[name], bool_val);
}
void Shader::set(const std::string& name, unsigned int uint_val) {
  if (!this->has_uniform(name))
    register_location(name);
  glUniform1ui(this->locations[name], uint_val);
}
void Shader::set(const std::string& name, int int_val) {
  if (!this->has_uniform(name))
    register_location(name);
  glUniform1i(this->locations[name], int_val);
}
void Shader::set(const std::string& name, float float_val) {
  if (!this->has_uniform(name))
    register_location(name);
  glUniform1f(this->locations[name], float_val);
}
void Shader::set(const std::string& name, const glm::vec3& vec3_val) {
  if (!this->has_uniform(name))
    register_location(name);
  glUniform3fv(this->locations[name], 1, glm::value_ptr(vec3_val));
}
void Shader::set(const std::string& name, const glm::vec4& vec4_val) {
  if (!this->has_uniform(name))
    register_location(name);
  glUniform4fv(this->locations[name], 1, glm::value_ptr(vec4_val));
}
void Shader::set(const std::string& name, const glm::mat3& mat3_val) {
  if (!this->has_uniform(name))
    register_location(name);
  glUniformMatrix3fv(this->locations[name], 1, GL_FALSE, glm::value_ptr(mat3_val));
}
void Shader::set(const std::string& name, const glm::mat4& mat4_val) {
  if (!this->has_uniform(name))
    register_location(name);
  glUniformMatrix4fv(this->locations[name], 1, GL_FALSE, glm::value_ptr(mat4_val));
}

void Shader::use() {
  glUseProgram(this->program_id);
}

GLuint Shader::program() const {
  return this->program_id;
}
