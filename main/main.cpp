// main.cpp — Point d'entree du portage AKA de Galaxy Fighter (2e jeu de
// reference AKA Port Studio, apres Kong-II). Structure calquee sur le
// main() d'origine (cookie -> setup -> boucle), adaptee pour app_main()
// ESP-IDF et l'init materiel AKA (meme squelette que kong2_aka/main.cpp).
#include "gb_core.h"
#include "gb_graphics.h"
#include "core/input.h"
#include "aka_runtime/aka_runtime.h"
#include "pokitto_compat/Pokitto.h"

#include "game/Game.h"
#include "game/utils/GameCookie.h"
#include "game/utils/Constants.h"

#include <cstdlib>
#include <ctime>

gb_core     g_core;
gb_graphics gfx;

using PC = Pokitto::Core;
using PD = Pokitto::Display;

// Palette PICO-8 (16 x RGB 8-bit) — fournie par le vrai SDK Pokitto ; ni
// Kong-II ni Galaxy Fighter ne la definissent eux-memes dans leurs sources.
const uint8_t palettePico[16 * 3] = {
      0,  0,  0,   29, 43, 83,  126, 37, 83,   0,135, 81,
    171, 82, 54,   95, 87, 79,  194,195,199,  255,241,232,
    255,  0, 77,  255,163,  0,  255,236, 39,    0,228, 54,
     41,173,255,  131,118,156,  255,119,168,  255,204,170,
};

Game       game;
GameCookie cookie;

extern "C" void app_main(void) {
    g_core.init();
    gfx.set_backlight_percent(80);
    gfx.set_refresh_rate(60);

    akaRuntime.begin("galaxy");

    cookie.begin("GALAXY", sizeof(cookie), (char*)&cookie);

    PC::begin();
    PD::loadRGBPalette(palettePico);
    PD::persistence = true;
    PD::setColor(5);
    PC::setFrameRate(200);
    PD::setFont(fontC64);

    srand((unsigned)time(0));

    if (cookie.initialised != COOKIE_INITIALISED)
        cookie.initialise();

    game.setup(&cookie);

    while (PC::isRunning()) {
        if (!PC::update()) continue;
        if (!akaRuntime.update(g_keys)) continue;
        PC::sound.updateStream();
        game.loop();
    }
}
