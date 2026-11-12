#pragma once
#include <citro2d.h>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>

class TileMap
{
public:
    int width = 0, height = 0;
    int tileW, tileH;
    std::vector<int> tiles;

    TileMap(int tw = 16, int th = 16) : tileW(tw), tileH(th) {}

    // Charge une carte CSV (-1 = case vide)
    bool load(const char *path)
    {
        FILE *f = fopen(path, "r");
        if (!f)
            return false;
        char line[4096];
        tiles.clear();
        width = height = 0;
        while (fgets(line, sizeof(line), f))
        {
            int count = 0;
            for (char *tok = strtok(line, ",\r\n"); tok; tok = strtok(nullptr, ",\r\n"))
            {
                tiles.push_back(atoi(tok));
                count++;
            }
            if (count == 0)
                continue;
            if (width == 0)
                width = count;
            height++;
        }
        fclose(f);
        return true;
    }

    int get(int x, int y) const
    {
        if (x < 0 || y < 0 || x >= width || y >= height)
            return -1;
        return tiles[y * width + x];
    }

    void draw(C2D_Image tileset, float camX, float camY, float screenW, float screenH) const
    {
        const Tex3DS_SubTexture *src = tileset.subtex;
        if (!tileset.tex || !src)
            return;
        int cols = src->width / tileW;
        int rows = src->height / tileH;

        int x0 = (int)(camX / tileW), y0 = (int)(camY / tileH);
        int x1 = (int)((camX + screenW) / tileW) + 1;
        int y1 = (int)((camY + screenH) / tileH) + 1;

        for (int y = y0; y <= y1; y++)
            for (int x = x0; x <= x1; x++)
            {
                int id = get(x, y);
                if (id < 0)
                    continue;

                int tx = id % cols, ty = id / cols;
                if (ty >= rows)
                    continue;

                Tex3DS_SubTexture sub;
                sub.width = tileW;
                sub.height = tileH;
                float du = (src->right - src->left) / src->width * tileW;
                float dv = (src->bottom - src->top) / src->height * tileH;
                sub.left = src->left + du * tx;
                sub.right = sub.left + du;
                sub.top = src->top + dv * ty;
                sub.bottom = sub.top + dv;

                C2D_Image img = {tileset.tex, &sub};
                C2D_DrawImageAt(img, x * tileW - camX, y * tileH - camY, 0.0f);
            }
    }
    bool isSolidAt(float px, float py) const
    {
        int tx = (int)floorf(px / tileW);
        int ty = (int)floorf(py / tileH);
        return get(tx, ty) >= 0;
    }
};