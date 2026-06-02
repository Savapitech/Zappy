#include "Core.hpp"
#include "Logger.hpp"

#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glx.h>

#include <Utils/math.hpp>

#include <fstream>
#include <sstream>
#include <string>

unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    auto compile = [](unsigned int type, const char* source) -> unsigned int {
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

std::string readShaderFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        LOG_FATAL(std::string("Failed to find the shader:") + filepath);
        exit(1);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}


    Core::Core() : _isRunning(true), _shaderProgram(0), _VAO(0), _VBO(0), _EBO(0) {}

    Core::~Core() {}

    void Core::init() {
        _window.open(WIDTH, HEIGHT, "Zappy");

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

        _myTexture = std::make_unique<Zappy::Texture>("/media/data/zappy/Zappy/gui/assets/cute.png");


        const std::string vertexCode = readShaderFile("gui/src/Core/Shader/vertex.vert");
        const std::string fragmentCode = readShaderFile("gui/src/Core/Shader/fragment.frag");

        _shaderProgram = createShaderProgram(vertexCode.c_str(), fragmentCode.c_str());

        float vertices[] = {
             0.5f,  0.5f,   1.0f, 1.0f,
             0.5f, -0.5f,   1.0f, 0.0f,
            -0.5f, -0.5f,   0.0f, 0.0f,
            -0.5f,  0.5f,   0.0f, 1.0f
        };
        unsigned int indices[] = { 0, 1, 3, 1, 2, 3 };

        glGenVertexArrays(1, &_VAO);
        glGenBuffers(1, &_VBO);
        glGenBuffers(1, &_EBO);

        glBindVertexArray(_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, _VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

void Core::run() {
    while (_isRunning) {
        const auto& events = _window.pollEvents();
        for (const auto& event : events) {
            if (event.type == Zappy::EventType::WindowClosed) {
                _isRunning = false;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(_shaderProgram);

        Zappy::Math::mat4 projection = Zappy::Math::perspective(
            Zappy::Math::radians(45.0f), WIDTH / HEIGHT, 0.1f, 1000.0f
        );

        Zappy::Math::mat4 view = Zappy::Math::translate(
            Zappy::Math::mat4(),
            Zappy::Math::vec3(0.0f, 0.0f, -3.0f)
        );

        Zappy::Math::mat4 model = Zappy::Math::translate(
            Zappy::Math::mat4(),
            Zappy::Math::vec3(0.0f, 0.0f, -10.0f)
        );

        float imgWidth = static_cast<float>(_myTexture->getWidth());
        float imgHeight = static_cast<float>(_myTexture->getHeight());

        float ratio = imgWidth / imgHeight;

        float desiredHeight = 5.0f;

        model = Zappy::Math::scale(model, Zappy::Math::vec3(desiredHeight * ratio, desiredHeight, 1.0f));

        Zappy::Math::mat4 mvp = projection * view * model;

        int mvpLoc = glGetUniformLocation(_shaderProgram, "u_MVP");
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, mvp.value_ptr());

        _myTexture->bind();
        glBindVertexArray(_VAO);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        _window.swapBuffers();
    }
}