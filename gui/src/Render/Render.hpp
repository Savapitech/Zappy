#pragma once
#include "Core/Shader/Shader.hpp"
#include "Render/Camera.hpp"
#include "Sprite/InstancedGrid.hpp"
#include "Sprite/Sprite.hpp"
#include "Render/Skybox.hpp"
#include <memory>
#include <vector>

namespace Zappy {

  struct DivineLight {
    int x;
    int y;
    float worldX;
    float worldZ;
    float timeActive;
};

class Renderer {
private:
  unsigned int depthMapFBO;
  unsigned int depthMap;
  unsigned int sceneFBO;
  unsigned int sceneColorTex;
  unsigned int sceneDepthTex;
  unsigned int quadVAO;
  unsigned int quadVBO;

  unsigned int _cylinderVAO;
  unsigned int _cylinderVBO;
  unsigned int _cylinderVertexCount = 12;

  std::unique_ptr<Shader> _instancedShader;
  std::unique_ptr<Shader> _defaultShader;
  std::unique_ptr<Shader> _postProcessShader;
  std::unique_ptr<Shader> _skyboxShader;
  std::unique_ptr<Shader> _divineLightShader;
  
  std::unique_ptr<Skybox> _skybox;

  unsigned int _width;
  unsigned int _height;

public:
  Renderer(unsigned int width, unsigned int height)
      : _width(width), _height(height) {
    _instancedShader =
        std::make_unique<Shader>("gui/src/Core/Shader/instanced.vert",
                                 "gui/src/Core/Shader/fragment.frag");
    _defaultShader = std::make_unique<Shader>(
        "gui/src/Core/Shader/vertex.vert", "gui/src/Core/Shader/fragment.frag");
    _postProcessShader =
        std::make_unique<Shader>("gui/src/Core/Shader/processDot.vert",
                                 "gui/src/Core/Shader/processDot.frag");

    _skyboxShader = std::make_unique<Shader>("gui/src/Core/Shader/skybox.vert", "gui/src/Core/Shader/skybox.frag");

    _divineLightShader = std::make_unique<Shader>("gui/src/Core/Shader/divineLight.vert", "gui/src/Core/Shader/divineLight.frag");

    std::vector<std::string> faces = {
      "gui/assets/skybox/right.png",
      "gui/assets/skybox/left.png",
      "gui/assets/skybox/top.png",
      "gui/assets/skybox/bottom.png",
      "gui/assets/skybox/front.png",
      "gui/assets/skybox/back.png"
    };
    _skybox = std::make_unique<Skybox>(faces);

    float crossQuads[] = {
        -0.4f, 0.0f,  0.0f,   0.0f, 0.0f,
         0.4f, 0.0f,  0.0f,   1.0f, 0.0f,
         0.4f, 10.0f, 0.0f,   1.0f, 1.0f,
         0.4f, 10.0f, 0.0f,   1.0f, 1.0f,
        -0.4f, 10.0f, 0.0f,   0.0f, 1.0f,
        -0.4f, 0.0f,  0.0f,   0.0f, 0.0f,

         0.0f, 0.0f, -0.4f,   0.0f, 0.0f,
         0.0f, 0.0f,  0.4f,   1.0f, 0.0f,
         0.0f, 10.0f, 0.4f,   1.0f, 1.0f,
         0.0f, 10.0f, 0.4f,   1.0f, 1.0f,
         0.0f, 10.0f,-0.4f,   0.0f, 1.0f,
         0.0f, 0.0f, -0.4f,   0.0f, 0.0f
    };
    glGenVertexArrays(1, &_cylinderVAO);
    glGenBuffers(1, &_cylinderVBO);
    glBindVertexArray(_cylinderVAO);
    glBindBuffer(GL_ARRAY_BUFFER, _cylinderVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crossQuads), &crossQuads, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glBindVertexArray(0);

    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 2048, 2048, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &sceneFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

    glGenTextures(1, &sceneColorTex);
    glBindTexture(GL_TEXTURE_2D, sceneColorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _width, _height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           sceneColorTex, 0);

    glGenTextures(1, &sceneDepthTex);
    glBindTexture(GL_TEXTURE_2D, sceneDepthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, _width, _height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           sceneDepthTex, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    float quadVertices[] = {-1.0f, 1.0f,  0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
                            1.0f,  -1.0f, 1.0f, 0.0f, -1.0f, 1.0f,  0.0f, 1.0f,
                            1.0f,  -1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  1.0f, 1.0f};
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices,
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)(2 * sizeof(float)));
    glBindVertexArray(0);
  }

  ~Renderer() {
    glDeleteFramebuffers(1, &depthMapFBO);
    glDeleteTextures(1, &depthMap);
    glDeleteFramebuffers(1, &sceneFBO);
    glDeleteTextures(1, &sceneColorTex);
    glDeleteTextures(1, &sceneDepthTex);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);

    glDeleteVertexArrays(1, &_cylinderVAO);
    glDeleteBuffers(1, &_cylinderVBO);
  }

  void drawSkybox(const Camera &camera) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);  
    _skyboxShader->bind();

    Zappy::Math::mat4 projection = Zappy::Math::perspective(
        Zappy::Math::radians(45.0f),
        static_cast<float>(_width) / static_cast<float>(_height), 0.1f,
        1000.0f);

    Zappy::Math::mat4 skyboxView = camera.getViewMatrix();
    skyboxView.m[12] = 0.0f;
    skyboxView.m[13] = 0.0f;
    skyboxView.m[14] = 0.0f;

    _skyboxShader->setMat4("view", skyboxView);
    _skyboxShader->setMat4("projection", projection);
    _skybox->draw();
    glDepthFunc(GL_LESS);
  }

  void renderIncantations(const Camera &camera, const std::vector<DivineLight> &incantations) {
      if (incantations.empty())
        return;

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      glDepthMask(GL_FALSE);

      _divineLightShader->bind();

      Zappy::Math::mat4 projection = Zappy::Math::perspective(
        Zappy::Math::radians(45.0f),
        static_cast<float>(_width) / static_cast<float>(_height), 0.1f,
        1000.0f);

      _divineLightShader->setMat4("u_Projection", projection);
      _divineLightShader->setMat4("u_View", camera.getViewMatrix());
      _divineLightShader->setFloat("u_MaxHeight", 10.0f);

      glBindVertexArray(_cylinderVAO);

      for (const auto& light : incantations) {
          Zappy::Math::mat4 model;
          model = Zappy::Math::translate(model, Zappy::Math::vec3(light.worldX, 0.0f, light.worldZ));

          _divineLightShader->setMat4("u_Model", model);
          _divineLightShader->setFloat("u_Time", light.timeActive);
          _divineLightShader->setVec3("u_Color", Zappy::Math::vec3(1.0f, 0.8f, 0.2f));

          glDrawArrays(GL_TRIANGLES, 0, _cylinderVertexCount); 
      }
    
      glDepthMask(GL_TRUE);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glBindVertexArray(0);
  }

  void render(const Camera &camera, InstancedGrid &floor,
              const std::vector<std::unique_ptr<Sprite>> &players,
              const std::vector<DivineLight> &incantations = {}) {
              
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glEnable(GL_DEPTH_TEST);

    Zappy::Math::mat4 lightProjection =
        Zappy::Math::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 1.0f, 100.0f);
    Zappy::Math::vec3 lightPos(-20.0f, 30.0f, 20.0f);
    Zappy::Math::mat4 lightView = Zappy::Math::lookAt(
        lightPos, Zappy::Math::vec3(0, 0, 0), Zappy::Math::vec3(0, 1, 0));
    Zappy::Math::mat4 lightSpaceMatrix = lightProjection * lightView;

    Zappy::Math::mat4 projection = Zappy::Math::perspective(
        Zappy::Math::radians(45.0f),
        static_cast<float>(_width) / static_cast<float>(_height), 0.1f,
        1000.0f);
    Zappy::Math::mat4 view = camera.getViewMatrix();
    Zappy::Math::mat4 viewProj = projection * view;

    glViewport(0, 0, 2048, 2048);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    _defaultShader->bind();
    _defaultShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
    _defaultShader->setInt("ourTexture", 0);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);

    for (auto &p : players)
      p->draw(*_defaultShader, lightView, lightProjection);

    glDisable(GL_POLYGON_OFFSET_FILL);

    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
    glViewport(0, 0, _width, _height);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glActiveTexture(GL_TEXTURE0);

    _instancedShader->bind();
    _instancedShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
    _instancedShader->setInt("shadowMap", 1);
    _instancedShader->setInt("ourTexture", 0);
    _instancedShader->setVec3("lightPos", lightPos);
    _instancedShader->setVec3("viewPos", camera.position);
    floor.draw(*_instancedShader, viewProj);

    _defaultShader->bind();
    _defaultShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
    _defaultShader->setInt("shadowMap", 1);
    _defaultShader->setInt("ourTexture", 0);
    _defaultShader->setVec3("lightPos", lightPos);
    _defaultShader->setVec3("viewPos", camera.position);
    for (auto &p : players)
      p->draw(*_defaultShader, view, projection);

    drawSkybox(camera);

    renderIncantations(camera, incantations);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    _postProcessShader->bind();
    float dist = Zappy::Math::length(camera.position);
    _postProcessShader->setFloat("u_focusDistance", dist);
    _postProcessShader->setFloat("u_focusRange", 15.0f);
    _postProcessShader->setInt("screenTexture", 0);
    _postProcessShader->setInt("depthTexture", 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneColorTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, sceneDepthTex);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
  }
};
} // namespace Zappy