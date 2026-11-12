#pragma once
#include "../scripts/SceneManager.hpp"
#include "../scripts/Coroutine.hpp"
#include "../scripts/PostFX.hpp"
#include "../scripts/GameState.hpp"

class GameBot : public Scene
{
public:
    Coroutine blinkCoroutine;
    bool logoVisible = true;

    float angle = 0.0f;
    float speed = 0.0f;
    bool rotating = false;

    GameBot()
    {
        blinkCoroutine.start(0.5f);
    }

    void update() override
    {
        fadeTime += 1.0f / 60.0f;

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

    float fadeTime = 0.0f;
    float fadePeriod = 1.6f;

    float bgShade = 0.7f;

    bool usesPostFX() const override { return true; }

    void draw(Renderer &r) override
    {
        C2D_Image bg = r.getImage("gameBackground", 0);
        if (bg.subtex)
        {
            float s = fmaxf(320.0f / bg.subtex->width, 240.0f / bg.subtex->height);
            float w = bg.subtex->width * s, h = bg.subtex->height * s;
            C2D_DrawImageAt(bg, floorf((320.0f - w) / 2), floorf((240.0f - h) / 2), 0.0f, nullptr, s, s);
        }

        C2D_DrawRectSolid(0, 0, 0.0f, 320, 240, C2D_Color32(0, 0, 0, (u8)(bgShade * 255.0f)));

        if (postfxBot.capturing) return;

        postfxBot.drawBloom();
        postfxBot.drawVignette(r);

        u32 white = C2D_Color32(255, 255, 255, 255);
        if (gameState.gameOver)
        {
            r.drawTextCentered("Touch the screen to Try Again", 160, 105, 0.6f, white, true);
            r.drawTextCentered("B : Back to Menu", 160, 135, 0.6f, white, true);
        }
        else
        {
            float wave = 0.5f + 0.5f * sinf(fadeTime * 2.0f * (float)M_PI / fadePeriod);
            u8 alpha = (u8)(255.0f * (0.15f + 0.85f * wave));
            r.drawTextCentered("Touch the screen to Jump", 160, 120, 0.6f, C2D_Color32(255, 255, 255, alpha), true);
        }
    }
};