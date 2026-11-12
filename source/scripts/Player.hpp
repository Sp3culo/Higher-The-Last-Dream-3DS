#pragma once
#include <3ds.h>
#include <cmath>
#include "World.hpp"
#include "Renderer.hpp"

class Player
{
public:
    float x, y;
    float w = 10, h = 21;
    float vx = 0, vy = 0;
    bool onGround = false;
    bool dead = false;

    const float START_SPEED = 3.0f;
    const float MAX_SPEED   = 7.5f;
    const float ACCEL       = 0.0005f;
    const float GRAVITY     = 0.25f;
    const float JUMP        = -5.0f;
    const float MAX_FALL    = 6.0f;
    const float DEATH_Y     = 240.0f;

    float runSpeed;
    float spawnX, spawnY;

    Player(float startX, float startY)
        : x(startX), y(startY), runSpeed(START_SPEED), spawnX(startX), spawnY(startY) {}

    void respawn()
    {
        x = spawnX; y = spawnY;
        vx = vy = 0;
        runSpeed = START_SPEED;
        onGround = false;
        dead = false;
        animTime = 0;
        jumpTime = -1;
    }

    static constexpr int RUN_FIRST = 0,  RUN_COUNT = 12;
    static constexpr int JUMP_FIRST = 12, JUMP_COUNT = 6;
    static constexpr float FRAME_FOOT_Y = 80.0f, FRAME_CENTER_X = 64.0f;
    static constexpr float SPRITE_SCALE = 0.75f;

    float animTime = 0;
    float jumpTime = -1;

    int currentFrame() const
    {
        if (onGround)
            return RUN_FIRST + (int)animTime % RUN_COUNT;
        if (jumpTime < 0)
            return JUMP_FIRST + JUMP_COUNT - 1;
        int f = (int)jumpTime;
        if (f >= JUMP_COUNT) f = JUMP_COUNT - 1;
        return JUMP_FIRST + f;
    }

    void draw(Renderer &r, float camX, float camY) const
    {
        float sx = floorf(x + w / 2 - FRAME_CENTER_X * SPRITE_SCALE - camX);
        float sy = floorf(y + h - FRAME_FOOT_Y * SPRITE_SCALE - camY);
        r.drawImage("player", currentFrame(), sx, sy, SPRITE_SCALE, SPRITE_SCALE);
    }

    void update(const World &world)
    {
        if (dead) return;

        u32 down = hidKeysDown();

        runSpeed += ACCEL;
        if (runSpeed > MAX_SPEED) runSpeed = MAX_SPEED;
        vx = runSpeed;

        if (onGround && (down & KEY_TOUCH))
        {
            vy = JUMP;
            jumpTime = 0;
        }

        vy += GRAVITY;
        if (vy > MAX_FALL) vy = MAX_FALL;

        moveX(world);
        moveY(world);

        if (onGround)
        {
            animTime += 0.1f * runSpeed;
            jumpTime = -1;
        }
        else if (jumpTime >= 0)
            jumpTime += 0.2f;

        if (touchesSpike(world) || y > DEATH_Y)
            dead = true;
    }

private:
    static constexpr int T = World::TILE;

    bool touchesSpike(const World &world) const
    {
        const float m = 2.0f;
        float l = x + m, r = x + w - m, t = y + m, b = y + h - m;
        return world.isDeadlyAt(l, t) || world.isDeadlyAt(r, t) ||
               world.isDeadlyAt(l, b) || world.isDeadlyAt(r, b);
    }

    // Hauteur de marche franchissable sans être bloqué
    float stepHeight() const
    {
        float s = w / 2 + fabsf(vx) + 1.0f;
        return s < 15.0f ? s : 15.0f;
    }

    void moveX(const World &world)
    {
        x += vx;
        float top = y, bottom = y + h - stepHeight();

        if (vx > 0)
        {
            float right = x + w - 0.01f;
            if (world.isSolidAt(right, top) || world.isSolidAt(right, bottom))
            {
                x = floorf(right / T) * T - w;
                vx = 0;
            }
        }
    }

    // Gravité et atterrissage sur le sol ou les pentes
    void moveY(const World &world)
    {
        if (vy >= 0)
        {
            float feet = y + h;
            float footX = x + w / 2;
            float target = feet + vy;
            float snap = onGround ? vx + 2.0f : 0.0f;

            float step = stepHeight();
            float s = world.floorAt(footX, feet - step, target + snap);
            if (s != World::NO_FLOOR && s >= feet - step && s <= target + snap)
            {
                y = s - h;
                vy = 0;
                onGround = true;
            }
            else
            {
                y += vy;
                onGround = false;
            }
            return;
        }

        y += vy;
        onGround = false;
        float left = x, right = x + w - 0.01f;
        {
            if (world.isSolidAt(left, y) || world.isSolidAt(right, y))
            {
                y = (floorf(y / T) + 1) * T;
                vy = 0;
            }
        }
    }
};
