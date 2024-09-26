#pragma once

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <map>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "proj.h"

namespace {
using namespace std;
namespace fs = std::filesystem;


class Shader {
  // 加载当前目录下的"shaders/"子目录的glsl文件，封装为Shader Program
private:
  GLuint program_id{0};

  // 缓存uniform变量名与location
  map<string, GLuint> locations;
  
  const string prefix = "assets\\shaders\\";

  void readFile(string &dst, const string &filename) {
    stringstream ss;
    ifstream fp(filename);
    ss << fp.rdbuf();
    dst = ss.str();
  }

  string shaderType2str(GLuint shader_type) {
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

  GLuint createShader(GLuint shader_type, const string &source) {
    // 根据source编译源文件
    GLuint shader = glCreateShader(shader_type);
    assert(shader != 0);
    const char *ss = source.c_str();
    GLint len = source.size();
    glShaderSource(shader, 1, &ss, &len);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      cout << "failed to compile " << shaderType2str(shader_type) << endl;
#ifndef NDEBUG
      char log[200] = {0};
      int log_num = 0;
      glGetShaderInfoLog(shader, 200, &log_num, log);
      cout << log << endl;
#endif
      return 0;
    }
    return shader;
  }

  GLuint createShader(const string &shader_source_filename) {
    // 读取glsl文件源码创建shader，着色器类型由文件名后缀来标识
    fs::path fname = shader_source_filename;
    string ext = fname.filename().extension().string();
    string source;
    readFile(source, shader_source_filename);
    GLuint shader{0};
    if (ext == ".vert") {
      shader = createShader(GL_VERTEX_SHADER, source);
    } else if (ext == ".frag") {
      shader = createShader(GL_FRAGMENT_SHADER, source);
    } else if (ext == ".geo") {
      shader = createShader(GL_GEOMETRY_SHADER, source);
    } else {
      string msg = "unknown glsl extension!";
      cerr << msg << endl;
      throw runtime_error(msg);
    }
    assert(shader != 0);
    return shader;
  }

public:
  Shader(const string &vertex_filename, const string &geometry_filename,
         const string &fragment_filename) {
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
      cout << "failed to link shader program!" << endl;
#ifndef NDEBUG
      char log[200];
      int log_num;
      glGetProgramInfoLog(this->program_id, 200, &log_num, log);
      cout << log << endl;
#endif
    }
#ifndef NDEBUG
    cout << "secceed to link program!" << endl;
#endif
    glDeleteShader(vshader);
    glDeleteShader(gshader);
    glDeleteShader(fshader);
  }

  Shader(const string &vertex_filename, const string &fragment_filename) {
    GLuint vshader = createShader(prefix + vertex_filename);
    GLuint fshader = createShader(prefix + fragment_filename);

    this->program_id = glCreateProgram();
    glAttachShader(this->program_id, vshader);
    glAttachShader(this->program_id, fshader);
    glLinkProgram(this->program_id);

    GLint success;
    glGetProgramiv(this->program_id, GL_LINK_STATUS, &success);
    if (!success) {
      cout << "failed to link shader program!" << endl;
#ifndef NDEBUG
      char log[200];
      int log_num;
      glGetProgramInfoLog(this->program_id, 200, &log_num, log);
      cout << log << endl;
#endif
    }
#ifndef NDEBUG
    cout << "secceed to link program!" << endl;
#endif
    glDeleteShader(vshader);
    glDeleteShader(fshader);
  }

  Shader(const Shader &sd) = delete;
  ~Shader() { glDeleteProgram(this->program_id); }


  bool has_uniform(const string& name)const{
    return this->locations.find(name) != this->locations.end();
  }

  void register_location(const string& name){
    GLuint loc = glGetUniformLocation(this->program_id, name.c_str());
    if(loc==-1){
      string msg = "unknown uniform name: \"" + name + "\"";
      cerr << msg << endl;
      throw runtime_error(msg);
    }
    this->locations[name] = loc;
  }

  void set(const string &name, bool bool_val){
    if(!this->has_uniform(name))
      register_location(name);
    glUniform1ui(this->locations[name], bool_val);
  }
  void set(const string &name, unsigned int uint_val){
    if(!this->has_uniform(name))
      register_location(name);
    glUniform1ui(this->locations[name], uint_val);
  }
  void set(const string &name,int int_val){
    if(!this->has_uniform(name))
      register_location(name);
    glUniform1i(this->locations[name], int_val);
  }
  void set(const string &name, float float_val){
    if(!this->has_uniform(name))
      register_location(name);
    glUniform1f(this->locations[name], float_val);
  }
  void set(const string &name, const glm::vec3 &vec3_val){
    if(!this->has_uniform(name))
      register_location(name);
    glUniform3fv(this->locations[name], 1, glm::value_ptr(vec3_val));
  }
  void set(const string &name, const glm::vec4 &vec4_val){
    if(!this->has_uniform(name))
      register_location(name);
    glUniform4fv(this->locations[name], 1, glm::value_ptr(vec4_val));
  }
  void set(const string &name, const glm::mat3 &mat3_val){
    if(!this->has_uniform(name))
      register_location(name);
    glUniformMatrix3fv(this->locations[name], 1,GL_FALSE, glm::value_ptr(mat3_val));
  }
  void set(const string &name, const glm::mat4 &mat4_val){
    if(!this->has_uniform(name))
      register_location(name);
    glUniformMatrix4fv(this->locations[name], 1,GL_FALSE, glm::value_ptr(mat4_val));
  }

  void use() { glUseProgram(this->program_id); }

  GLuint program() const { return this->program_id; }
};
} // namespace
