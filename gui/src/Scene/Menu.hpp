#pragma once
#include "IScene/IScene.hpp"
#include "Sprite/Sprite.hpp"
#include "Texture/TextureManager.hpp"
#include "Utils/math.hpp"
#include <vector>



namespace Zappy {
    class MenuScene : public IScene {
    private:
        TextureManager& _texManager;
        std::vector<std::unique_ptr<Sprite>> _sprites;

    public:
        MenuScene(TextureManager& tm) : _texManager(tm) {}

        void onEnter() override {
            Texture& cuteTexture = _texManager.get("/media/data/zappy/Zappy/gui/assets/cute.png");

            for (int x = -5; x <= 5; x++) {
                for (int z = -5; z <= 5; z++) {
                    auto sprite = std::make_unique<Sprite>(cuteTexture);
                    sprite->position = Zappy::Math::vec3(x * 2.0f, 0.0f, z * 2.0f);
                    sprite->scale = Zappy::Math::vec3(1.0f, 1.0f, 1.0f);

                    _sprites.push_back(std::move(sprite));
                }
            }
        }

        void draw(Shader& shader) override {
            Zappy::Math::mat4 projection = Zappy::Math::perspective(
                Zappy::Math::radians(45.0f),
                static_cast<float>(WIDTH) / static_cast<float>(HEIGHT),
                0.1f, 1000.0f
            );

            Zappy::Math::vec3 cameraPos(0.0f, 10.0f, 15.0f);
            Zappy::Math::vec3 targetPos(0.0f, 0.0f, 0.0f);
            Zappy::Math::vec3 upVector(0.0f, 1.0f, 0.0f);
            Zappy::Math::mat4 view = Zappy::Math::lookAt(cameraPos, targetPos, upVector);

            for (auto& sprite : _sprites) {
                sprite->draw(shader, view, projection);
            }
        }

        SceneState update() override {
            return SceneState::NONE;
        }

        void onExit() override {
            _sprites.clear();
        }
    };
}
