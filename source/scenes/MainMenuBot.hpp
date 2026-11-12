#pragma once
#include "../scripts/SceneManager.hpp"
#include "../scripts/Coroutine.hpp"
#include "../scripts/PostFX.hpp"

class MainMenuBot : public Scene
{
public:
    Coroutine blinkCoroutine;
    bool logoVisible = true;

    int chunk = 0;
    static constexpr int CHUNK_COUNT = 2;

    float chunkAlpha = 1.0f;
    bool fadingOut = false;
    static constexpr float FADE_STEP = 1.0f / (0.25f * 60.0f);

    bool showCredits = false;
    bool pendingCredits = false;
    float creditsScroll = 0.0f;
    static constexpr float CREDITS_SPEED = 0.75f;

    MainMenuBot()
    {
        blinkCoroutine.start(0.5f);
    }

    void update() override
    {
        u32 kDown = hidKeysDown();
        if (kDown & KEY_X)
        {
            fadingOut = true;
            pendingCredits = true;
        }
        else if ((kDown & KEY_A) && !showCredits && !pendingCredits)
        {
            fadingOut = true;
        }

        if (fadingOut)
        {
            chunkAlpha -= FADE_STEP;
            if (chunkAlpha <= 0.0f)
            {
                chunkAlpha = 0.0f;
                fadingOut = false;
                if (pendingCredits)
                {
                    pendingCredits = false;
                    showCredits = !showCredits;
                    creditsScroll = 0.0f;
                }
                else
                {
                    chunk = (chunk + 1) % CHUNK_COUNT;
                }
            }
        }
        else if (chunkAlpha < 1.0f)
        {
            chunkAlpha = fminf(1.0f, chunkAlpha + FADE_STEP);
        }

        if (showCredits)
        {
            creditsScroll += CREDITS_SPEED;
            if (creditsScroll > SCREEN_H + CREDITS_COUNT * CREDITS_LINE_H)
                creditsScroll = 0.0f;
        }

        blinkCoroutine.update();

        if (blinkCoroutine.finished())
        {
            logoVisible = !logoVisible;
            blinkCoroutine.start(0.5f);
        }
    }

    bool usesPostFX() const override { return true; }

    void draw(Renderer &r) override
    {
        r.drawImage("background_bot", 0, 0, 0, 1.0f, 1.0f);

        if (postfxBot.capturing)
            return;

        postfxBot.drawBloom();
        postfxBot.drawVignette(r);

        if (showCredits)
        {
            r.drawRect(0, 0, SCREEN_W, SCREEN_H, C2D_Color32(0, 0, 0, (u8)(CREDITS_DIM * chunkAlpha * 255.0f)));
            Credits(r);
            return;
        }

        if (chunk == 0)
            FirstQrChunck(r);
        else
            SecondQrChunck(r);

        u32 hint = C2D_Color32(255, 255, 255, logoVisible ? 255 : 120);
        r.drawTextCentered(std::to_string(chunk + 1) + "/" + std::to_string(CHUNK_COUNT),
                           SCREEN_W / 2, 214, 0.45f, hint);
    }

    void Credits(Renderer &r)
    {
        for (int i = 0; i < CREDITS_COUNT; i++)
        {
            float y = SCREEN_H - creditsScroll + i * CREDITS_LINE_H;
            if (y < -CREDITS_LINE_H || y > SCREEN_H)
                continue;

            float edge = fminf(y + CREDITS_LINE_H, SCREEN_H - y) / CREDITS_EDGE_FADE;
            float alpha = fmaxf(0.0f, fminf(1.0f, edge)) * chunkAlpha;

            const CreditLine &line = CREDITS[i];
            if (line.kind == BLANK)
                continue;
            u8 a = (u8)(alpha * 255.0f);
            switch (line.kind)
            {
            case SECTION:
                r.drawTextCentered(line.text, SCREEN_W / 2, y, 0.6f, C2D_Color32(255, 200, 120, a));
                break;
            case ROLE:
                r.drawTextCentered(line.text, SCREEN_W / 2, y, 0.5f, C2D_Color32(170, 200, 255, a));
                break;
            default:
                r.drawTextCentered(line.text, SCREEN_W / 2, y, 0.45f, C2D_Color32(255, 255, 255, a));
                break;
            }
        }
    }

    void FirstQrChunck(Renderer &r)
    {
        drawQr(r, "steam", "Steam Page", SCREEN_W / 4);
        drawQr(r, "discord", "Discord Server", SCREEN_W * 3 / 4);
    }

    void SecondQrChunck(Renderer &r)
    {
        drawQr(r, "itchio", "Itch.io Page", SCREEN_W / 4);
        drawQr(r, "website", "Official Website", SCREEN_W * 3 / 4);
    }

private:
    static constexpr float SCREEN_W = 320.0f;
    static constexpr float SCREEN_H = 240.0f;

    enum CreditKind
    {
        BLANK,
        SECTION,
        ROLE,
        NAME
    };
    struct CreditLine
    {
        const char *text;
        CreditKind kind;
    };
    static constexpr CreditLine CREDITS[] = {
        {"Game Director / Original Creator", SECTION},
        {"", BLANK},
        {"Clément Arnaud | Sp3culo", NAME},
        {"", BLANK},
        {"", BLANK},
        {"Development", SECTION},
        {"", BLANK},
        {"Conception", ROLE},
        {"Clément Arnaud | Sp3culo", NAME},
        {"", BLANK},
        {"Programmation", ROLE},
        {"Clément Arnaud | Sp3culo", NAME},
        {"YitHash", NAME},
        {"", BLANK},
        {"Game & Level Design", ROLE},
        {"Clément Arnaud | Sp3culo", NAME},
        {"", BLANK},
        {"Scenario", ROLE},
        {"Clément Arnaud | Sp3culo", NAME},
        {"", BLANK},
        {"", BLANK},
        {"Assets & Pixels Arts", SECTION},
        {"", BLANK},
        {"General Assets", ROLE},
        {"Clément Arnaud | Sp3culo", NAME},
        {"", BLANK},
        {"Charadesigns & Concept Art", ROLE},
        {"Clément Arnaud | Sp3culo", NAME},
        {"L'Avis de Dany", NAME},
        {"GeelComics", NAME},
        {"Simon Ducos | Le_Sims", NAME},
        {"Roscoe", NAME},
        {"", BLANK},
        {"Commercial Illustrations", ROLE},
        {"Atto.ink", NAME},
        {"GeelComincs", NAME},
        {"MarineSakuga", NAME},
        {"", BLANK},
        {"Pixel Artists", ROLE},
        {"Clément Arnaud | Sp3culo", NAME},
        {"Tsuny", NAME},
        {"Evil A", NAME},
        {"Roscoe", NAME},
        {"Axoloto", NAME},
        {"Celist", NAME},
        {"", BLANK},
        {"UI & UX Designs", ROLE},
        {"Clément Arnaud | Sp3culo", NAME},
        {"Tsuny", NAME},
        {"", BLANK},
        {"Monsters & Mobs Concepts", ROLE},
        {"Clément Arnaud | Sp3culo", NAME},
        {"Evil A", NAME},
        {"Roscoe", NAME},
        {"Axoloto", NAME},
        {"Celist", NAME},
        {"", BLANK},
        {"Logo", ROLE},
        {"Manon Rochard | Lookyart", NAME},
        {"", BLANK},
        {"Other Assets", ROLE},
        {"Louve", NAME},
        {"Celist", NAME},
        {"", BLANK},
        {"", BLANK},
        {"Music & Audio", SECTION},
        {"", BLANK},
        {"Clément Arnaud | Sp3culo", NAME},
        {"VLADVulpes", NAME},
        {"sillypankeki", NAME},
        {"", BLANK},
        {"", BLANK},
        {"Play Testers", SECTION},
        {"", BLANK},
        {"Alexandre RJF", NAME},
        {"Paul Maurel", NAME},
        {"Labuche", NAME},
        {"omegasomeone", NAME},
        {"Teka", NAME},
        {"Rumpel", NAME},
        {"JustYozora_", NAME},
        {"", BLANK},
        {"", BLANK},
        {"Special Thanks", SECTION},
        {"", BLANK},
        {"Le Garage des Gamers", NAME},
        {"Toulouse Game Dev", NAME},
        {"Yo_Lan", NAME},
        {"Abiding Agora", NAME},
    };
    static constexpr int CREDITS_COUNT = sizeof(CREDITS) / sizeof(CREDITS[0]);
    static constexpr float CREDITS_LINE_H = 20.0f;
    static constexpr float CREDITS_DIM       = 0.65f;
    static constexpr float CREDITS_EDGE_FADE = 24.0f;
    static constexpr float QR_Y = 38.0f;
    static constexpr float LABEL_GAP = 8.0f;

    void drawQr(Renderer &r, const char *key, const char *label, float cx)
    {
        C2D_Image img = r.getImage(key, 0);
        float size = img.subtex ? img.subtex->width : 123.0f;
        r.drawImage(key, 0, floorf(cx - size / 2), QR_Y, 1.0f, 1.0f, chunkAlpha);
        r.drawTextCentered(label, cx, QR_Y + size + LABEL_GAP, 0.5f,
                           C2D_Color32(255, 255, 255, (u8)(chunkAlpha * 255.0f)));
    }
};

constexpr MainMenuBot::CreditLine MainMenuBot::CREDITS[];
