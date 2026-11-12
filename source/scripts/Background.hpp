#pragma once
#include <citro2d.h>
#include <cmath>
#include "Renderer.hpp"

class Background
{
public:
    struct Rect { float x, y, w, h; };

    float frontFadeAlpha = 0.3f;
    float frontFadeDist  = 24.0f;
    struct Layer
    {
        const char *sheet;
        int count;
        float scroll;
        float scale;
        float spacing;
        float baseY;
        float fog;
        u32 fogColor;
        float depth3D;
        float density;
        float visible;
    };

    u32 shadeColor = C2D_Color32(12, 4, 8, 255);

    float caveScroll  = 0.05f;
    float caveDepth3D = 1.0f;
    float caveShade   = 0.45f;

    static constexpr int FRONT_COUNT = 2;
    Layer front[FRONT_COUNT] = {
        {"trees", 3, 1.25f, 2.00f, 700.0f, 340.0f, 0.30f, C2D_Color32(10, 4, 6, 255), -0.25f, 0.35f, 1.0f},
        {"rocks", 3, 1.40f, 0.60f, 170.0f, 262.0f, 0.25f, C2D_Color32(10, 4, 6, 255), -0.40f, 0.60f, 1.0f},
    };

    void drawBack(Renderer &r, float camX, float screenW = 400, float screenH = 240) const
    {
        drawCave(r, camX, screenW);
    }

    void drawFront(Renderer &r, float camX, const Rect &player, float screenW = 400) const
    {
        for (int l = 0; l < FRONT_COUNT; l++)
            drawLayer(r, front[l], 10 + l, camX, screenW, &player);
    }

private:
    void drawCave(Renderer &r, float camX, float screenW) const
    {
        C2D_Image img = r.getImage("cave", 0);
        if (!img.subtex) return;

        float s = 240.0f / img.subtex->height;
        float w = img.subtex->width * s;

        Tex3DS_SubTexture mirrored = *img.subtex;
        mirrored.left  = img.subtex->right;
        mirrored.right = img.subtex->left;
        C2D_Image imgMirror = {img.tex, &mirrored};

        r.beginLayer(caveDepth3D);

        C2D_ImageTint tint;
        C2D_PlainImageTint(&tint, shadeColor, caveShade);

        float offset = camX * caveScroll;
        int first = (int)floorf(offset / w) - 1;
        float sx = (w + 1.0f) / img.subtex->width;
        for (int i = first; i * w - offset < screenW + 20; i++)
        {
            float x = floorf(i * w - offset);
            C2D_DrawImageAt((i & 1) ? imgMirror : img, x, 0, 0.0f, &tint, sx, s);
        }

        r.endLayer();
    }

    // Dessine une couche de décor en parallaxe, placée de façon pseudo-aléatoire
    void drawLayer(Renderer &r, const Layer &L, int seed, float camX, float screenW,
                   const Rect *focus = nullptr) const
    {
        r.beginLayer(L.depth3D);

        C2D_ImageTint tint;
        C2D_PlainImageTint(&tint, L.fogColor, L.fog);

        float offset = camX * L.scroll;
        int first = (int)floorf((offset - 250) / L.spacing);
        int last  = (int)floorf((offset + screenW + 50) / L.spacing) + 1;

        for (int i = first; i <= last; i++)
        {
            u32 h = hash(i, seed);
            if ((float)((h >> 4) % 1000) / 1000.0f >= L.density) continue;

            int variant = h % L.count;
            float jitter = (float)((h >> 8) % 1000) / 1000.0f * L.spacing * 0.6f;
            float s = L.scale * (0.85f + (float)((h >> 20) % 100) / 100.0f * 0.3f);

            C2D_Image img = r.getImage(L.sheet, variant);
            if (!img.subtex) continue;

            Tex3DS_SubTexture part = *img.subtex;
            part.height = (u16)(img.subtex->height * L.visible);
            part.bottom = img.subtex->top + (img.subtex->bottom - img.subtex->top) * L.visible;
            C2D_Image shown = {img.tex, &part};

            float w = part.width * s, ht = part.height * s;
            float sx = floorf(i * L.spacing + jitter - offset - w / 2);
            float sy = floorf(L.baseY - ht);

            if (focus)
            {
                float mx = w * 0.15f, my = ht * 0.10f;
                float ex0 = sx + mx, ex1 = sx + w - mx, ey0 = sy + my, ey1 = sy + ht;
                float dx = fmaxf(0.0f, fmaxf(ex0 - (focus->x + focus->w), focus->x - ex1));
                float dy = fmaxf(0.0f, fmaxf(ey0 - (focus->y + focus->h), focus->y - ey1));
                float t = fminf(1.0f, sqrtf(dx * dx + dy * dy) / frontFadeDist);
                float alpha = frontFadeAlpha + (1.0f - frontFadeAlpha) * t;

                u32 c = (L.fogColor & 0x00FFFFFF) | ((u32)(alpha * 255.0f) << 24);
                C2D_PlainImageTint(&tint, c, L.fog);
            }

            C2D_DrawImageAt(shown, sx, sy, 0.0f, &tint, s, s);
        }

        r.endLayer();
    }

    static u32 hash(int i, int layer)
    {
        u32 h = (u32)i * 73856093u ^ (u32)(layer + 1) * 19349663u;
        h ^= h >> 13; h *= 0x5bd1e995u; h ^= h >> 15;
        return h;
    }
};
