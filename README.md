# Higher: The Last Dream - 3DS Visit Card
---
## Release Builds
https://github.com/Sp3culo/Higher-The-Last-Dream-3DS/releases

## Project Structure

```
3DS_Visit_Caard/
├── source/
│   ├── main.cpp                    # Entry point, asset loading, render loop
│   ├── scenes/
│   │   ├── MainMenuTop.hpp         # Top screen menu (logo, touch spin)
│   │   ├── MainMenuBot.hpp         # Bottom screen menu (QR codes)
│   │   ├── GameTop.hpp             # Top screen gameplay
│   │   └── GameBot.hpp             # Bottom screen gameplay
│   └── scripts/
│       ├── SceneManager.hpp        # Scene lifecycle & fade system
│       ├── Renderer.hpp            # citro2d drawing wrapper
│       ├── Coroutine.hpp           # Frame-based timer utility
│       ├── PostFX.hpp              # Post-processing (vignette)
│       ├── Background.hpp          # Parallax background layers
│       ├── Player.hpp              # Player controller & animation
│       ├── TileMap.hpp / World.hpp # CSV tile maps & level prefabs
│       └── GameState.hpp           # Shared game state
├── romfs/                          # Packed into the app: only runtime files here
│   ├── assets/                     # .t3x textures
│   ├── fonts/                      # .bcfnt custom font
│   └── maps/                       # Level prefabs (CSV)
├── gfx/                            # Source art (NOT packed into the app)
│   ├── banner/                     # HOME menu banner (png, wav, cgfx, glb)
│   ├── src/ player/ postfx/        # Source PNGs for the .t3x textures
│   └── qr/                         # Generated QR code PNGs
├── shell/
│   └── run.sh                      # Launches the .3dsx in Azahar
├── icon.png                        # HOME menu icon (48x48)
├── app.rsf                         # CIA metadata (title, region, etc.)
└── Makefile                        # devkitARM build system
```

---

## Building

### Prerequisites

- [devkitARM](https://devkitpro.org/wiki/Getting_Started) with 3DS support
- `libctru`, `citro2d`, `citro3d`
- devkitPro packages: `3ds-dev`

Install via devkitPro pacman:
```bash
dkp-pacman -S 3ds-dev
```

### Compile

```bash
make clean && make
```

This generates:
- `Higher_The_Last_Dream.3dsx` — for use with [Homebrew Launcher](https://smealum.github.io/3ds/)
- `Higher_The_Last_Dream.cia` — installable via FBI, built with `make cia` (requires `makerom` and `bannertool`)

The CIA uses `icon.png` as the HOME menu icon, and `gfx/banner/banner.png` (logo, 256x128) + `gfx/banner/banner.wav` (3 s max) as the top-screen banner.

---

### Converting Images

Source PNGs live in `gfx/`; never put them in `romfs/`, everything there is packed into the app. Convert them to `.t3x` with `tex3ds` (devkitPro).

## Links

- **Steam Page** : https://store.steampowered.com/app/3722970/Higher_The_Last_Dream/
- **Discord Community** : https://discord.gg/3qDbjFXFe9 
- **Itch.io Page** : https://sp3culo.itch.io/highergame
- **Official Website** : https://highergame.fr/

---

## License

This project is a promotional homebrew tool for *Higher: The Last Dream*. All game assets are © their respective owners.
