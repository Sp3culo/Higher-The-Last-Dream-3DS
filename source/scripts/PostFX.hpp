#pragma once
#include <3ds.h>
#include <citro2d.h>
#include "Renderer.hpp"

class PostFX
{
public:
    bool  bloomEnabled    = true;
    float bloomIntensity  = 0.35f;
    float bloomSpread     = 3.0f;
    bool  vignetteEnabled = true;
    float vignetteAlpha   = 1.0f;

    bool capturing = false;

    static constexpr float SCALE = 0.25f;
    static constexpr int   TEX_W = 128, TEX_H = 64;

    void init(float w = 400.0f, float h = 240.0f)
    {
        screenW = w;
        screenH = h;
        const int CAP_W = (int)(w * SCALE), CAP_H = (int)(h * SCALE);

        C3D_TexInitVRAM(&tex, TEX_W, TEX_H, GPU_RGBA8);
        C3D_TexSetFilter(&tex, GPU_LINEAR, GPU_LINEAR);
        target = C3D_RenderTargetCreateFromTex(&tex, GPU_TEXFACE_2D, 0, -1);

        sub.width  = CAP_W;
        sub.height = CAP_H;
        sub.left   = 0.0f;
        sub.top    = 1.0f;
        sub.right  = (float)CAP_W / TEX_W;
        sub.bottom = 1.0f - (float)CAP_H / TEX_H;
    }

    void fini()
    {
        if (target) C3D_RenderTargetDelete(target);
        C3D_TexDelete(&tex);
    }

    template <typename DrawFn>
    // Dessine la scène dans une texture réduite, utilisée ensuite par le bloom
    void capture(DrawFn drawScene)
    {
        if (!bloomEnabled || !target) return;
        C2D_TargetClear(target, C2D_Color32(0, 0, 0, 255));
        C2D_SceneBegin(target);
        C2D_ViewReset();
        C2D_ViewScale(SCALE, SCALE);
        capturing = true;
        drawScene();
        capturing = false;
        C2D_ViewReset();
    }

    // Superpose la texture capturée en mode additif pour l'effet de lueur
    void drawBloom() const
    {
        if (!bloomEnabled || !target) return;
        C2D_Image img = {const_cast<C3D_Tex *>(&tex), &sub};

        C2D_Flush();
        C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE, GPU_ZERO, GPU_ONE);

        C2D_ImageTint tint;
        C2D_AlphaImageTint(&tint, bloomIntensity / 4.0f);
        const float s = 1.0f / SCALE, d = bloomSpread;
        const float off[4][2] = {{-d, -d}, {d, -d}, {-d, d}, {d, d}};
        for (auto &o : off)
            C2D_DrawImageAt(img, o[0], o[1], 0.0f, &tint, s, s);

        C2D_Flush();
        C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA,
                       GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
    }

    void drawVignette(Renderer &r) const
    {
        if (!vignetteEnabled) return;
        C2D_Image img = r.getImage("vignette", 0);
        if (!img.subtex) return;
        C2D_ImageTint tint;
        C2D_AlphaImageTint(&tint, vignetteAlpha);
        float sx = screenW / img.subtex->width, sy = screenH / img.subtex->height;
        C2D_DrawImageAt(img, 0, 0, 0.0f, &tint, sx, sy);
    }

private:
    float screenW = 400.0f, screenH = 240.0f;
    C3D_Tex tex{};
    C3D_RenderTarget *target = nullptr;
    Tex3DS_SubTexture sub{};
};

extern PostFX postfx;
extern PostFX postfxBot;
