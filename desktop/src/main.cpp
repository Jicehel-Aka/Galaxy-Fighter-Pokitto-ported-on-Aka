#include "pokitto_compat/Pokitto.h"
#include "game/Game.h"
#include "game/utils/GameCookie.h"
#include "game/utils/Constants.h"
#include <cstdlib>
#include <ctime>

using PC = Pokitto::Core;
using PD = Pokitto::Display;

const uint8_t palettePico[16 * 3] = {
    0,0,0, 29,43,83, 126,37,83, 0,135,81, 171,82,54, 95,87,79,
    194,195,199, 255,241,232, 255,0,77, 255,163,0, 255,236,39, 0,228,54,
    41,173,255, 131,118,156, 255,119,168, 255,204,170
};

#if defined(__ANDROID__) || defined(_WIN32)
extern "C" int SDL_main(int, char**) {
#else
int main(int, char**) {
#endif
    PC::begin();
    PD::loadRGBPalette(palettePico);
    PD::setColor(5);
    PD::setFont(fontC64);
    PC::setFrameRate(60);
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    GameCookie cookie;
    cookie.begin("GALAXY", sizeof(cookie), reinterpret_cast<char*>(&cookie));
    if (!cookie.loadCookie() || cookie.initialised != COOKIE_INITIALISED) cookie.initialise();

    Game game;
    game.setup(&cookie);
    while (PC::isRunning()) {
        if (!PC::update()) continue;
        game.loop();
    }
    return 0;
}
