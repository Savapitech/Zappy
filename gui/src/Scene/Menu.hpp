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

            auto myFirstSprite = std::make_unique<Sprite>(cuteTexture);
            myFirstSprite->position = Zappy::Math::vec3(0.0f, 0.0f, -10.0f);
            myFirstSprite->scale = Zappy::Math::vec3(5.0f, 5.0f, 1.0f);

            auto mySecondSprite = std::make_unique<Sprite>(cuteTexture);
            mySecondSprite->position = Zappy::Math::vec3(6.0f, 0.0f, -10.0f);
            mySecondSprite->scale = Zappy::Math::vec3(5.0f, 5.0f, 1.0f);

            _sprites.push_back(std::move(myFirstSprite));
            _sprites.push_back(std::move(mySecondSprite));
        }

        SceneState update() override {
            return SceneState::NONE;
        }

        void draw(Shader& shader) override {
            Zappy::Math::mat4 projection = Zappy::Math::perspective(
                Zappy::Math::radians(45.0f),
                static_cast<float>(WIDTH) / static_cast<float>(HEIGHT),
                0.1f, 1000.0f
            );

            Zappy::Math::mat4 view = Zappy::Math::translate(
                Zappy::Math::mat4(),
                Zappy::Math::vec3(0.0f, 0.0f, -3.0f)
            );

            Zappy::Math::mat4 viewProj = projection * view;

            for (auto& sprite : _sprites) {
                sprite->draw(shader, viewProj);
            }
        }

        void onExit() override {
            _sprites.clear();
        }
    };
}
