#include <3ds.h>
#include <citro2d.h>
#include <thread>
#include <chrono>
#include "scripts/SceneManager.hpp"
#include "scenes/MainMenuTop.hpp"
#include "scenes/MainMenuBot.hpp"
#include "scenes/GameTop.hpp"
#include "scenes/GameBot.hpp"
#include <cstring>
#include <cstdio>
#include <vector>
#include "scripts/Coroutine.hpp"
#include "scripts/PostFX.hpp"

C3D_RenderTarget *bot;
C3D_RenderTarget *topLeft;
C3D_RenderTarget *topRight;

constexpr float MAX_EYE_SHIFT = 5.0f;

SceneManager sceneManagerTop(400.0f, 240.0f);
SceneManager sceneManagerBot(320.0f, 240.0f);
PostFX postfx;
PostFX postfxBot;
GameState gameState;

Scene *createSceneTop(const char *name)
{
    if (strcmp(name, "MainMenuTop") == 0) return new MainMenuTop();
    if (strcmp(name, "GameTop") == 0) return new GameTop();
    return nullptr;
}

Scene *createSceneBot(const char *name)
{
    if (strcmp(name, "MainMenuBot") == 0) return new MainMenuBot();
    if (strcmp(name, "GameBot") == 0) return new GameBot();
    return nullptr;
}

void goToTop(const char *name)
{
    sceneManagerTop.set(createSceneTop(name));
}

void goToBot(const char *name)
{
    sceneManagerBot.set(createSceneBot(name));
}

int main(int argc, char **argv)
{
    osSetSpeedupEnable(true);
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    gfxSet3D(true);

    bot = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    topLeft  = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    topRight = C2D_CreateScreenTarget(GFX_TOP, GFX_RIGHT);

    Renderer renderer;
    renderer.loadSheet("gameBackground",      "romfs:/assets/Background1.t3x");
    renderer.loadSheet("menu",       "romfs:/assets/logo.t3x");
    renderer.loadSheet("background_top", "romfs:/assets/output_top.t3x");
    renderer.loadSheet("background_bot", "romfs:/assets/output_bottom.t3x");
    renderer.loadSheet("steam",   "romfs:/assets/steam.t3x");
    renderer.loadSheet("discord", "romfs:/assets/discord.t3x");
    renderer.loadSheet("itchio",  "romfs:/assets/itchio.t3x");
    renderer.loadSheet("website", "romfs:/assets/website.t3x");
    for (const char *k : {"steam", "discord", "itchio", "website"})
        if (C3D_Tex *t = renderer.getImage(k, 0).tex)
            C3D_TexSetFilter(t, GPU_NEAREST, GPU_NEAREST);
    renderer.loadSheet("tileset", "romfs:/assets/tileset.t3x");
    if (C3D_Tex *t = renderer.getImage("tileset", 0).tex)
        C3D_TexSetFilter(t, GPU_NEAREST, GPU_NEAREST);
    renderer.loadSheet("spikes", "romfs:/assets/spikes.t3x");
    renderer.loadSheet("player", "romfs:/assets/player.t3x");
    renderer.loadSheet("trees", "romfs:/assets/trees.t3x");
    renderer.loadSheet("cave",  "romfs:/assets/cave.t3x");
    renderer.loadSheet("rocks", "romfs:/assets/rocks.t3x");
    renderer.loadSheet("vignette", "romfs:/assets/vignette.t3x");
    if (C3D_Tex *t = renderer.getImage("vignette", 0).tex)
        C3D_TexSetFilter(t, GPU_LINEAR, GPU_LINEAR);
    postfx.init(400, 240);
    postfxBot.init(320, 240);
    if (C3D_Tex *t = renderer.getImage("player", 0).tex)
        C3D_TexSetFilter(t, GPU_NEAREST, GPU_NEAREST);
    if (C3D_Tex *t = renderer.getImage("spikes", 0).tex)
        C3D_TexSetFilter(t, GPU_NEAREST, GPU_NEAREST);

    goToTop("MainMenuTop");
    goToBot("MainMenuBot");

    bool showPerf = false;
    int botBloomGen = -1;

    while (aptMainLoop())
    {
        hidScanInput();
        if (hidKeysDown() & KEY_SELECT) showPerf = !showPerf;

        sceneManagerTop.update();
        sceneManagerBot.update();

        float slider = osGet3DSliderState();
        bool stereo = slider > 0.0f;

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        renderer.beginFrame();

        Scene *topScene = sceneManagerTop.current;
        Scene *botScene = sceneManagerBot.current;

        auto drawTopEye = [&](C3D_RenderTarget *target, float eye, bool recapture)
        {
            renderer.eyeShift = eye * slider * MAX_EYE_SHIFT;
            if (recapture && topScene && topScene->usesPostFX())
                postfx.capture([&] { topScene->draw(renderer); });

            C2D_TargetClear(target, C2D_Color32(0, 0, 0, 255));
            C2D_SceneBegin(target);
            sceneManagerTop.draw(renderer);
        };

        drawTopEye(topLeft, -1.0f, true);
        drawTopEye(topRight, +1.0f, stereo);

        renderer.eyeShift = 0.0f;
        if (botScene && botScene->usesPostFX() && botBloomGen != sceneManagerBot.generation)
        {
            postfxBot.capture([&] { botScene->draw(renderer); });
            botBloomGen = sceneManagerBot.generation;
        }

        C2D_TargetClear(bot, C2D_Color32(0, 0, 0, 255));
        C2D_SceneBegin(bot);
        sceneManagerBot.draw(renderer);

        if (showPerf)
        {
            char perf[64];
            snprintf(perf, sizeof(perf), "CPU %.1f ms  GPU %.1f ms",
                     C3D_GetProcessingTime(), C3D_GetDrawingTime());
            renderer.drawRect(0, 0, 320, 16, C2D_Color32(0, 0, 0, 180));
            renderer.drawText(perf, 4, 1, 0.45f, C2D_Color32(255, 255, 0, 255));
        }

        C3D_FrameEnd(0);
    }

    postfx.fini();
    postfxBot.fini();
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}