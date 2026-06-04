#define GL_GLEXT_PROTOTYPES 1

#include "Shader.hpp"
#include "Logger.hpp"

#include <GL/gl.h>
#include <GL/glx.h>

#include <fstream>
#include <sstream>
#include <string>

namespace Zappy {

Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath) {
  std::string vertexCode = readShaderFile(vertexPath);
  std::string fragmentCode = readShaderFile(fragmentPath);

  _id = createShaderProgram(vertexCode.c_str(), fragmentCode.c_str());
  LOG_INFO("Shader Load: " + vertexPath + " & " + fragmentPath);
}

Shader::~Shader() {
  if (_id != 0) {
    glDeleteProgram(_id);
  }
}

void Shader::bind() const { glUseProgram(_id); }

void Shader::unbind() const { glUseProgram(0); }

void Shader::setMat4(const std::string &name,
                     const Zappy::Math::mat4 &mat) const {

  int location = glGetUniformLocation(_id, name.c_str());
  glUniformMatrix4fv(location, 1, GL_FALSE, mat.value_ptr());
}

void Shader::setInt(const std::string &name, int value) const {
  glUniform1i(glGetUniformLocation(_id, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, float value) const {
  glUniform1f(glGetUniformLocation(_id, name.c_str()), value);
}

std::string Shader::readShaderFile(const std::string &filepath) {
  std::ifstream file(filepath);
  if (!file.is_open()) {
    LOG_FATAL(std::string("Failed to find the shader:") + filepath);
    exit(1);
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

unsigned int Shader::createShaderProgram(const char *vertexSource,
                                         const char *fragmentSource) {
  auto compile = [](unsigned int type, const char *source) -> unsigned int {
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &source, nullptr);
    glCompileShader(id);

    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
      char infoLog[512];
      glGetShaderInfoLog(id, 512, nullptr, infoLog);
      LOG_FATAL(std::string("Shader Compilation :") + infoLog);
    }
    return id;
  };

  unsigned int vs = compile(GL_VERTEX_SHADER, vertexSource);
  unsigned int fs = compile(GL_FRAGMENT_SHADER, fragmentSource);

  unsigned int program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);

  glDeleteShader(vs);
  glDeleteShader(fs);

  return program;
}

} // namespace Zappy