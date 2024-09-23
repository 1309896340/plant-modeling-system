#pragma once

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "proj.h"

namespace {
using namespace std;
class Shader {
  // 封装 shader program
private:
  GLuint program{0};

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
    // GLuint shader = glCreateShader(shader_type);
    GLuint shader = glCreateShader(shader_type);
    assert(shader != 0);
    const char *ss = source.c_str();
    GLint len = source.size();
    glShaderSource(shader, 1, &ss, &len);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      cout << "failed to compile shader" << endl;
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

public:
  Shader(const string &vertex_filename, const string &fragment_filename) {
    string vertex_source, fragment_source;
    string prefix = "shaders\\";
    readFile(vertex_source, prefix + vertex_filename);
    readFile(fragment_source, prefix + fragment_filename);
    GLuint vshader = createShader(GL_VERTEX_SHADER, vertex_source);

    if (vshader == 0)
      throw runtime_error("failed to compile Vertex Shader!");
#ifndef NDEBUG
    cout << "succeed to compile Vertex Shader \"" << vertex_filename << "\""
         << endl;
#endif

    GLuint fshader = createShader(GL_FRAGMENT_SHADER, fragment_source);
    if (fshader == 0)
      throw runtime_error("failed to compile Fragment Shader!");
#ifndef NDEBUG
    cout << "succeed to compile Fragment Shader \"" << fragment_filename << "\""
         << endl;
#endif

    this->program = glCreateProgram();
    glAttachShader(this->program, vshader);
    glAttachShader(this->program, fshader);
    glLinkProgram(this->program);

    GLint success;
    glGetProgramiv(this->program, GL_LINK_STATUS, &success);
    if (!success) {
      cout << "failed to link shader program!" << endl;
#ifndef NDEBUG
      char log[200];
      int log_num;
      glGetProgramInfoLog(this->program, 200, &log_num, log);
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
  ~Shader() {
    glDeleteProgram(this->program);
  }
};
} // namespace
