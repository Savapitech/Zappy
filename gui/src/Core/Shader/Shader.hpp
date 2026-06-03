#pragma once
#include <string>
#include <Utils/math.hpp>

namespace Zappy {
    class Shader {
    private:
        unsigned int _id;
        std::string readShaderFile(const std::string& filepath);
        unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource);

    public:
        Shader(const std::string& vertexPath, const std::string& fragmentPath);
        ~Shader();

        void bind() const;
        void unbind() const;

        void setInt(const std::string &name, int value) const;
        void setMat4(const std::string& name, const Zappy::Math::mat4& mat) const;
    };
}