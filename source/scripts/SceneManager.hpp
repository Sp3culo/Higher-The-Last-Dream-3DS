#pragma once
#include "./Renderer.hpp"

class Scene
{
public:
    virtual ~Scene() {}
    virtual void update() = 0;
    virtual void draw(Renderer &r) = 0;
    virtual bool usesPostFX() const { return false; }
};

class SceneManager
{
public:
    Scene *current = nullptr;
    Scene *next = nullptr;
    int generation = 0;
    float fade = 0.0f;
    bool fadingOut = false;

    float screenW = 400.0f;
    float screenH = 240.0f;

    SceneManager(float w = 400.0f, float h = 240.0f)
        : screenW(w), screenH(h) {}

    // Change de scène avec un fondu au noir
    void set(Scene *scene)
    {
        next = scene;
        fadingOut = true;
        fade = 0.0f;
    }

    void update()
    {
        if (fadingOut)
        {
            fade += 0.05f;
            if (fade >= 1.0f)
            {
                fade = 1.0f;
                fadingOut = false;
                delete current;
                current = next;
                next = nullptr;
                generation++;
            }
        }
        else if (fade > 0.0f)
        {
            fade -= 0.05f;
            if (fade < 0.0f)
                fade = 0.0f;
        }

        if (current)
            current->update();
    }

    void draw(Renderer &r)
    {
        if (current)
        {
            current->draw(r);
        }

        if (fade > 0.0f)
        {
            u8 alpha = (u8)(fade * 255);
            C2D_DrawRectSolid(0, 0, 0.5f, screenW, screenH,
                              C2D_Color32(0, 0, 0, alpha));
        }
    }
};

extern SceneManager sceneManagerTop;
extern SceneManager sceneManagerBot;

Scene *createScene(const char *name);
void goToTop(const char *name);
void goToBot(const char *name);