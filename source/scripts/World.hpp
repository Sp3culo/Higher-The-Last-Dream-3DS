#pragma once
#include <citro2d.h>
#include <deque>
#include <string>
#include "TileMap.hpp"

struct Prefab
{
    TileMap ground{16, 16};
    TileMap plants{16, 16};
    TileMap spikes{16, 16};

    int plantOX = 0, plantOY = -1;
    int spikeOX = 13, spikeOY = -1;

    bool load(const std::string &folder)
    {
        if (!ground.load((folder + "/ground.csv").c_str()))
            return false;
        plants.load((folder + "/plant.csv").c_str());
        spikes.load((folder + "/spike.csv").c_str());
        return true;
    }

    float widthPx() const { return ground.width * ground.tileW; }
};

struct Chunk
{
    const Prefab *prefab;
    float x;
};

class World
{
public:
    static constexpr int TILE = 16;

    Prefab prefab;
    std::deque<Chunk> chunks;

    bool load()
    {
        return prefab.load("romfs:/maps/prefab_1");
    }

    void reset()
    {
        chunks.clear();
        chunks.push_back({&prefab, 0.0f});
    }

    // Ajoute des chunks devant la caméra et supprime ceux qui sont passés
    void update(float camX, float screenW)
    {
        if (prefab.widthPx() <= 0) return;

        while (chunks.back().x + chunks.back().prefab->widthPx() < camX + screenW * 2)
        {
            const Chunk &last = chunks.back();
            chunks.push_back({&prefab, last.x + last.prefab->widthPx()});
        }

        while (chunks.size() > 1 && chunks.front().x + chunks.front().prefab->widthPx() < camX - screenW)
            chunks.pop_front();
    }

    static constexpr int SLOPE_UP_ID   = 49;
    static constexpr int SLOPE_DOWN_ID = 52;

    enum TileType { EMPTY, SOLID, SLOPE_UP, SLOPE_DOWN };

    TileType tileAt(float px, float py) const
    {
        for (const Chunk &c : chunks)
        {
            const TileMap &g = c.prefab->ground;
            int id = g.get((int)floorf((px - c.x) / TILE), (int)floorf(py / TILE));
            if (id < 0) continue;
            if (id == SLOPE_UP_ID)   return SLOPE_UP;
            if (id == SLOPE_DOWN_ID) return SLOPE_DOWN;
            return SOLID;
        }
        return EMPTY;
    }

    bool isSolidAt(float px, float py) const
    {
        return tileAt(px, py) == SOLID;
    }

    static constexpr float NO_FLOOR = 1e9f;

    // Hauteur du sol sous px entre yFrom et yTo (pentes comprises), NO_FLOOR sinon
    float floorAt(float px, float yFrom, float yTo) const
    {
        int r0 = (int)floorf(yFrom / TILE), r1 = (int)floorf(yTo / TILE);
        float lx = px - floorf(px / TILE) * TILE;

        for (int row = r0; row <= r1; row++)
        {
            float top = row * TILE;
            switch (tileAt(px, top + 0.5f))
            {
            case SOLID:      return top;
            case SLOPE_UP:   return top + (TILE - lx);
            case SLOPE_DOWN: return top + lx;
            default:         break;
            }
        }
        return NO_FLOOR;
    }

    bool isDeadlyAt(float px, float py) const
    {
        for (const Chunk &c : chunks)
        {
            const Prefab &p = *c.prefab;
            if (p.spikes.isSolidAt(px - c.x - p.spikeOX * TILE, py - p.spikeOY * TILE))
                return true;
        }
        return false;
    }

    void draw(C2D_Image tileset, C2D_Image spikeImg, float camX, float camY, float screenW, float screenH)
    {
        for (const Chunk &c : chunks)
        {
            const Prefab &p = *c.prefab;
            float cx = camX - c.x;
            p.ground.draw(tileset, cx, camY, screenW, screenH);
            p.plants.draw(tileset, cx - p.plantOX * TILE, camY - p.plantOY * TILE, screenW, screenH);
            p.spikes.draw(spikeImg, cx - p.spikeOX * TILE, camY - p.spikeOY * TILE, screenW, screenH);
        }
    }
};
