#pragma once
#include "IScene/IScene.hpp"
#include "Sprite/Sprite.hpp"
#include "Sprite/InstancedGrid.hpp"
#include "Texture/TextureManager.hpp"
#include "Utils/math.hpp"

#include "Logger.hpp"

#include <vector>



namespace Zappy {
    class MenuScene : public IScene {
    private:

        unsigned int depthMapFBO;
        unsigned int depthMap;
        TextureManager& _texManager;
        std::vector<std::unique_ptr<Sprite>> _players;
        std::unique_ptr<InstancedGrid> _floor;

        std::unique_ptr<Shader> _instancedShader;
        std::unique_ptr<Shader> _defaultShader;

        unsigned int sceneFBO;
        unsigned int sceneColorTex;
        unsigned int sceneDepthTex;

        unsigned int quadVAO;
        unsigned int quadVBO;
        
        std::unique_ptr<Shader> _postProcessShader;

        Zappy::Math::vec3 cameraPos = {0.0f, 25.0f, 35.0f};


    public:
        MenuScene(TextureManager& tm) : _texManager(tm) {}

        void onEnter() override {
            
            glGenFramebuffers(1, &depthMapFBO);
            glGenTextures(1, &depthMap);
            glBindTexture(GL_TEXTURE_2D, depthMap);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 2048, 2048, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

            glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            Texture& islandTex = _texManager.get("/media/data/zappy/Zappy/gui/assets/island.png");
            Texture& cuteTex = _texManager.get("/media/data/zappy/Zappy/gui/assets/cute.png");

            _instancedShader = std::make_unique<Shader>("gui/src/Core/Shader/instanced.vert", "gui/src/Core/Shader/fragment.frag");
            _defaultShader = std::make_unique<Shader>("gui/src/Core/Shader/vertex.vert", "gui/src/Core/Shader/fragment.frag");

            _floor = std::make_unique<InstancedGrid>(islandTex);

            for (int x = -5; x <= 5; x++) {
                for (int z = -5; z <= 5; z++) {
    
                    _floor->addTile(Zappy::Math::vec3(x * 2.0f, 0.0f, z * 2.0f), 2.0f);

                    if (x == 0 && z == 0)
                    {
                        auto player = std::make_unique<Sprite>(cuteTex);
                        player->position = Zappy::Math::vec3(x * 2.0f, 0.0f, z * 2.0f - 1.0f);
                        player->scale = Zappy::Math::vec3(1.0f, 1.0f, 1.0f);
                        player->isBillboard = true;
                        _players.push_back(std::move(player));
                    }
                    
                }
            }
            _floor->build();
            
            _postProcessShader = std::make_unique<Shader>("gui/src/Core/Shader/processDot.vert", "gui/src/Core/Shader/processDot.frag");

            glGenFramebuffers(1, &sceneFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

            glGenTextures(1, &sceneColorTex);
            glBindTexture(GL_TEXTURE_2D, sceneColorTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneColorTex, 0);
            glGenTextures(1, &sceneDepthTex);

            glBindTexture(GL_TEXTURE_2D, sceneDepthTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, WIDTH, HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, sceneDepthTex, 0);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            float quadVertices[] = {
                -1.0f,  1.0f,  0.0f, 1.0f,
                -1.0f, -1.0f,  0.0f, 0.0f,
                 1.0f, -1.0f,  1.0f, 0.0f,
                -1.0f,  1.0f,  0.0f, 1.0f,
                 1.0f, -1.0f,  1.0f, 0.0f,
                 1.0f,  1.0f,  1.0f, 1.0f
            };
            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
            glBindVertexArray(0);
        }

       void draw(Shader& defaultShader) override {
            glEnable(GL_DEPTH_TEST);

            Zappy::Math::mat4 lightProjection = Zappy::Math::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 1.0f, 100.0f);
            Zappy::Math::vec3 lightPos(-20.0f, 30.0f, 20.0f); 
            Zappy::Math::mat4 lightView = Zappy::Math::lookAt(lightPos, Zappy::Math::vec3(0,0,0), Zappy::Math::vec3(0,1,0));
            Zappy::Math::mat4 lightSpaceMatrix = lightProjection * lightView;

            Zappy::Math::mat4 projection = Zappy::Math::perspective(Zappy::Math::radians(45.0f), static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), 0.1f, 1000.0f);
           
            Zappy::Math::mat4 view = Zappy::Math::lookAt(cameraPos, Zappy::Math::vec3(0,0,0), Zappy::Math::vec3(0,1,0));
            Zappy::Math::mat4 viewProj = projection * view;

            glViewport(0, 0, 2048, 2048);
            glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
            glClear(GL_DEPTH_BUFFER_BIT);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); 

            defaultShader.bind();
            defaultShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
            for (auto& p : _players) {
                p->draw(defaultShader, lightView, lightProjection);
            }

            glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO); 
            glViewport(0, 0, WIDTH, HEIGHT);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, depthMap);
            glActiveTexture(GL_TEXTURE0);

            _instancedShader->bind();
            _instancedShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
            _instancedShader->setInt("shadowMap", 1);
            _instancedShader->setInt("ourTexture", 0);
            _floor->draw(*_instancedShader, viewProj);

            defaultShader.bind();
            defaultShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
            defaultShader.setInt("shadowMap", 1);
            defaultShader.setInt("ourTexture", 0);
            for (auto& p : _players) { 
                p->draw(defaultShader, view, projection);
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0); 
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);

            _postProcessShader->bind();

            float dist = Zappy::Math::length(cameraPos);
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

            glActiveTexture(GL_TEXTURE0);
        }

        SceneState update(const std::vector<Zappy::Event>& events) override {
            float cameraSpeed = 0.5f; 

            for (const auto& event : events) {
                if (event.type == Zappy::EventType::KeyPressed) {
                    if (event.keyCode == Zappy::Key::Z)
                        cameraPos.z -= cameraSpeed; 
                    if (event.keyCode == Zappy::Key::S)
                        cameraPos.z += cameraSpeed; 

                    if (event.keyCode == Zappy::Key::Q)
                        cameraPos.x -= cameraSpeed; 
                    if (event.keyCode == Zappy::Key::D)
                        cameraPos.x += cameraSpeed; 
                }
            }
            return SceneState::NONE;
        }

        void onExit() override {
            _players.clear();
            glDeleteFramebuffers(1, &depthMapFBO);
            glDeleteTextures(1, &depthMap);
            
            glDeleteFramebuffers(1, &sceneFBO);
            glDeleteTextures(1, &sceneColorTex);
            glDeleteTextures(1, &sceneDepthTex);
            
            glDeleteVertexArrays(1, &quadVAO);
            glDeleteBuffers(1, &quadVBO);
        }
    };
}
