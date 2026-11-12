#pragma once
#include "../scripts/SceneManager.hpp"
#include "../scripts/Coroutine.hpp"
#include "../scripts/PostFX.hpp"

class MainMenuTop : public Scene
{
public:
    Coroutine blinkCoroutine;
    bool logoVisible = true;

    float angle = 0.0f;
    float speed = 0.0f;
    bool rotating = false;
    bool starting = false;

    MainMenuTop()
    {
        blinkCoroutine.start(0.5f);
    }

    void update() override
    {
        blinkCoroutine.update();
        if (blinkCoroutine.finished())
        {
            logoVisible = !logoVisible;
            blinkCoroutine.start(0.5f);
        }

        touchPosition touch;
        hidTouchRead(&touch);

        if (!starting && (hidKeysDown() & KEY_TOUCH))
        {
            starting = true;
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

                if (starting)
                {
                    goToTop("GameTop");
                    goToBot("GameBot");
                }
            }
        }
    }

    bool usesPostFX() const override { return true; }

    void draw(Renderer &r) override
    {
        r.beginLayer(0.8f);
        C2D_Image bg = r.getImage("background_top", 0);
        if (bg.subtex)
            r.drawImage("background_top", 0, -6, 0, 412.0f / bg.subtex->width, 240.0f / bg.subtex->height);
        r.endLayer();

        r.beginLayer(-0.3f);
        float scaleX = 0.5f * cosf(angle);
        r.drawImageRotated("menu", 0, 135 + 64, 70 + 48, scaleX, 0.4f, 0.0f);
        r.endLayer();

        if (postfx.capturing) return;

        postfx.drawBloom();
        postfx.drawVignette(r);

        r.drawText("The Last Dream", 145, 170, 0.6f, C2D_Color32(255, 255, 255, 255));

        if (logoVisible)
        {
            r.drawText("Touch the screen of your 3DS", 100, 218, 0.6f, C2D_Color32(255, 255, 255, 255));
        }
    }
};