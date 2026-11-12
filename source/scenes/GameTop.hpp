#pragma once
#include "../scripts/SceneManager.hpp"
#include "../scripts/Coroutine.hpp"
#include "../scripts/World.hpp"
#include <string>
#include "../scripts/Player.hpp"
#include "../scripts/Background.hpp"
#include "../scripts/PostFX.hpp"
#include "../scripts/GameState.hpp"

class GameTop : public Scene
{
public:
    Coroutine blinkCoroutine;
    bool logoVisible = true;

    float angle = 0.0f;
    float speed = 0.0f;
    bool rotating = false;

    World world;
    Background background;
    Player player{32, -40};
    float camX = 0, camY = -100;

    float gameDepth3D = 0.3f;

    int score = 0, best = 0;
    int deadTimer = 0;
    bool leaving = false;

    GameTop()
    {
        blinkCoroutine.start(0.5f);
        world.load();
        restart();
    }

    // Remet le monde, le joueur et le score à zéro
    void restart()
    {
        world.reset();
        player.respawn();
        camX = 0;
        score = 0;
        deadTimer = 0;
        gameState.gameOver = false;
    }

    void update() override
    {
        player.update(world);

        u32 down = hidKeysDown();

        if (!leaving && (down & KEY_B))
        {
            leaving = true;
            gameState.gameOver = false;
            goToTop("MainMenuTop");
            goToBot("MainMenuBot");
        }

        if (player.dead)
        {
            if (score > best) best = score;
            if (!leaving) gameState.gameOver = true;
            deadTimer++;
            if (!leaving && deadTimer > 30 && (down & KEY_TOUCH))
                restart();
        }
        else
        {
            score = (int)((player.x - player.spawnX) / World::TILE);
        }

        float target = floorf(player.x - 100);
        if (target > camX) camX = target;

        world.update(camX, 400);

        blinkCoroutine.update();
        if (blinkCoroutine.finished())
        {
            logoVisible = !logoVisible;
            blinkCoroutine.start(0.5f);
        }

        touchPosition touch;
        hidTouchRead(&touch);

        if (hidKeysHeld() & KEY_TOUCH)
        {
            rotating = true;
            speed = 0.15f;
        }

        if (rotating)
        {
            float remaining = (2.0f * M_PI) - angle;
            speed = 0.02f + 0.13f * (remaining / (2.0f * M_PI));

            angle += speed;

            if (angle >= 2.0f * M_PI)
            {
                angle = 0.0f;
                speed = 0.0f;
                rotating = false;
            }
        }
    }

    bool usesPostFX() const override { return true; }

    void draw(Renderer &r) override
    {
        background.drawBack(r, camX);

        r.beginLayer(gameDepth3D);
        world.draw(r.getImage("tileset", 0), r.getImage("spikes", 0), camX, camY, 400, 240);
        player.draw(r, camX, camY);
        r.endLayer();
        Background::Rect pr = {player.x - camX, player.y - camY, player.w, player.h};
        background.drawFront(r, camX, pr);

        if (postfx.capturing) return;

        postfx.drawBloom();
        postfx.drawVignette(r);

        u32 white = C2D_Color32(255, 255, 255, 255);
        r.drawText("Score : " + std::to_string(score), 8, 8, 0.5f, white);
        r.drawText("Highscore : " + std::to_string(best), 8, 24, 0.5f, white);
        if (player.dead)
        {
            r.drawTextCentered("GAME OVER", 200, 110, 0.8f, white, true);
        }
    }
};