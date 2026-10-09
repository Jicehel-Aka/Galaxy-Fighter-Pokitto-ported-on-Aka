#include "pokitto_compat/PokittoCore.h"
#include "pokitto_compat/PokittoCookie.h"
#include "pokitto_compat/PokittoDisplay.h"
#include "pokitto_compat/PokittoSound.h"
#include "pokitto_compat/Font5x7.h"

#include <SDL.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

namespace Pokitto {
namespace {
constexpr int kScale = 3;
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* texture;
SDL_AudioDeviceID audioDevice;
SDL_AudioDeviceID musicDevice;
bool running = true;
uint32_t frameDelay = 5;
uint32_t lastFrame;
bool firstFrame = true;
uint32_t frameNumber;
uint8_t palette[16][3];
uint8_t* frameBuffer;
uint32_t keyboardKeys;
uint32_t currentKeys;
uint32_t previousKeys;
uint32_t lastFrameKeys;
std::unordered_map<SDL_FingerID, uint32_t> touchButtons;
const uint8_t* activeSfx;
const uint8_t* activeSfxEnd;

constexpr uint32_t bit(uint32_t mask) { return mask; }
uint32_t keyMask(SDL_Keycode key) {
    switch (key) {
        case SDLK_UP: case SDLK_w: case SDLK_z: return 0x0200;
        case SDLK_DOWN: case SDLK_s: return 0x0400;
        case SDLK_LEFT: case SDLK_a: case SDLK_q: return 0x0800;
        case SDLK_RIGHT: case SDLK_d: return 0x0100;
        case SDLK_SPACE: case SDLK_RETURN: case SDLK_j: return 0x8000;
        case SDLK_x: case SDLK_LCTRL: return 0x2000;
        case SDLK_c: case SDLK_LSHIFT: return 0x4000;
        case SDLK_ESCAPE: return 0x0004;
        default: return 0;
    }
}

std::string savePath() {
    char* base = SDL_GetPrefPath("Jicehel", "GalaxyFighter");
    if (!base) return "save.dat";
    std::string result(base);
    SDL_free(base);
    result += "save.dat";
    return result;
}

uint32_t touchMask(float normalizedX, float normalizedY) {
    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    if (windowWidth <= 0 || windowHeight <= 0) return 0;
    const float scale = std::min(windowWidth / static_cast<float>(Display::width),
                                 windowHeight / static_cast<float>(Display::height));
    const float offsetX = (windowWidth - Display::width * scale) * 0.5f;
    const float offsetY = (windowHeight - Display::height * scale) * 0.5f;
    const float x = (normalizedX * windowWidth - offsetX) / scale;
    const float y = (normalizedY * windowHeight - offsetY) / scale;

    if (x >= 30 && x < 58 && y >= 91 && y < 119) return 0x0200;
    if (x >= 30 && x < 58 && y >= 147 && y < 175) return 0x0400;
    if (x >= 2 && x < 30 && y >= 119 && y < 147) return 0x0800;
    if (x >= 58 && x < 86 && y >= 119 && y < 147) return 0x0100;
    if ((x - 185) * (x - 185) + (y - 139) * (y - 139) <= 22 * 22) return 0x8000;
    if ((x - 145) * (x - 145) + (y - 145) * (y - 145) <= 17 * 17) return 0x2000;
    if ((x - 165) * (x - 165) + (y - 105) * (y - 105) <= 17 * 17) return 0x4000;
    return 0;
}

void drawTouchControls() {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    const auto circle = [](int cx, int cy, int radius, uint32_t mask) {
        const bool pressed = (currentKeys & mask) != 0;
        SDL_SetRenderDrawColor(renderer, 30, 35, 55, pressed ? 190 : 105);
        for (int y = -radius; y <= radius; ++y) {
            const int halfWidth = static_cast<int>(std::sqrt(radius * radius - y * y));
            SDL_RenderDrawLine(renderer, cx - halfWidth, cy + y, cx + halfWidth, cy + y);
        }
        SDL_SetRenderDrawColor(renderer, 220, 230, 255, 190);
        SDL_Rect outline{cx - radius, cy - radius, radius * 2, radius * 2};
        SDL_RenderDrawRect(renderer, &outline);
    };
    const auto label = [](int x, int y, char letter) {
        static constexpr uint8_t glyphs[3][5] = {
            {0x0e, 0x11, 0x1f, 0x11, 0x11},
            {0x1e, 0x11, 0x1e, 0x11, 0x1e},
            {0x0e, 0x11, 0x10, 0x11, 0x0e}
        };
        const int glyphIndex = letter == 'A' ? 0 : letter == 'B' ? 1 : 2;
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 235);
        for (int row = 0; row < 5; ++row)
            for (int col = 0; col < 5; ++col)
                if (glyphs[glyphIndex][row] & (1 << (4 - col)))
                    SDL_RenderDrawPoint(renderer, x + col, y + row);
    };
    const auto arrow = [](int x, int y, int dx, int dy) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 235);
        SDL_RenderDrawLine(renderer, x - dx * 4, y - dy * 4, x + dx * 4, y + dy * 4);
        SDL_RenderDrawLine(renderer, x + dx * 4, y + dy * 4,
                           x + dx * 4 - dy * 3, y + dy * 4 + dx * 3);
        SDL_RenderDrawLine(renderer, x + dx * 4, y + dy * 4,
                           x + dx * 4 + dy * 3, y + dy * 4 - dx * 3);
    };

    circle(44, 105, 14, 0x0200);
    circle(44, 161, 14, 0x0400);
    circle(16, 133, 14, 0x0800);
    circle(72, 133, 14, 0x0100);
    circle(185, 139, 22, 0x8000);
    circle(145, 145, 17, 0x2000);
    circle(165, 105, 17, 0x4000);
    arrow(44, 105, 0, -1);
    arrow(44, 161, 0, 1);
    arrow(16, 133, -1, 0);
    arrow(72, 133, 1, 0);
    label(183, 137, 'A');
    label(143, 143, 'B');
    label(163, 103, 'C');
}
} // namespace

uint32_t Core::frameCount = 0;
bool Core::suppressPresent = false;
Buttons Core::buttons;

void Core::begin() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0)
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init: %s", SDL_GetError());
    window = SDL_CreateWindow("Galaxy Fighter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        Display::width * kScale, Display::height * kScale, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (renderer) SDL_RenderSetLogicalSize(renderer, Display::width, Display::height);
    texture = renderer ? SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING, Display::width, Display::height) : nullptr;
    SDL_AudioSpec spec{};
    spec.freq = 22050;
    spec.format = AUDIO_S16SYS;
    spec.channels = 1;
    spec.samples = 1024;
    audioDevice = SDL_OpenAudioDevice(nullptr, 0, &spec, nullptr, 0);
    if (audioDevice) SDL_PauseAudioDevice(audioDevice, 0);
    musicDevice = SDL_OpenAudioDevice(nullptr, 0, &spec, nullptr, 0);
    if (musicDevice) SDL_PauseAudioDevice(musicDevice, 0);
    frameBuffer = new uint8_t[(Display::width * Display::height) / 2]();
    Display::screenbuffer = frameBuffer;
    lastFrame = SDL_GetTicks();
    firstFrame = true;
}

void Core::setFrameRate(uint8_t fps) { frameDelay = fps ? 1000u / fps : 16u; }
bool Core::isRunning() { return running; }
uint32_t Core::getTime() { return SDL_GetTicks(); }

bool Core::update() {
    uint32_t nextTouchKeys = 0;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) running = false;
        if (event.type == SDL_FINGERDOWN || event.type == SDL_FINGERMOTION) {
            touchButtons[event.tfinger.fingerId] = touchMask(event.tfinger.x, event.tfinger.y);
        } else if (event.type == SDL_FINGERUP) {
            touchButtons.erase(event.tfinger.fingerId);
        }
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
                continue;
            }
            const uint32_t mask = keyMask(event.key.keysym.sym);
            if (event.type == SDL_KEYDOWN) keyboardKeys |= mask;
            else keyboardKeys &= ~mask;
        }
    }
    for (const auto& [finger, mask] : touchButtons) {
        (void)finger;
        nextTouchKeys |= mask;
    }
    currentKeys = keyboardKeys | nextTouchKeys;
    if (!running) return false;
    const uint32_t now = SDL_GetTicks();
    if (!firstFrame && now - lastFrame < frameDelay) return false;
    lastFrame = now;
    firstFrame = false;
    previousKeys = lastFrameKeys;
    lastFrameKeys = currentKeys;
    if (frameNumber && !suppressPresent) Display::present();
    ++frameNumber;
    ++Core::frameCount;
    Sound::poll();
    return true;
}

bool Buttons::pressed(uint8_t btn) const {
    const uint32_t masks[] = {0x0200, 0x0400, 0x0800, 0x0100, 0x8000, 0x2000, 0x4000};
    return btn < 7 && (currentKeys & masks[btn]) && !(previousKeys & masks[btn]);
}
bool Buttons::released(uint8_t btn) const {
    const uint32_t masks[] = {0x0200, 0x0400, 0x0800, 0x0100, 0x8000, 0x2000, 0x4000};
    return btn < 7 && !(currentKeys & masks[btn]) && (previousKeys & masks[btn]);
}
bool Buttons::down(uint8_t btn) const {
    const uint32_t masks[] = {0x0200, 0x0400, 0x0800, 0x0100, 0x8000, 0x2000, 0x4000};
    return btn < 7 && (currentKeys & masks[btn]);
}
bool Buttons::repeat(uint8_t btn, uint8_t interval) const {
    static uint32_t nextFrame[7]{};
    if (!down(btn)) { if (btn < 7) nextFrame[btn] = 0; return false; }
    if (pressed(btn)) { nextFrame[btn] = Core::frameCount + std::max<uint8_t>(interval, 1); return false; }
    if (btn < 7 && Core::frameCount >= nextFrame[btn]) {
        nextFrame[btn] = Core::frameCount + std::max<uint8_t>(interval, 1);
        return true;
    }
    return false;
}
bool Buttons::aBtn() const { return down(BTN_A); }
bool Buttons::bBtn() const { return down(BTN_B); }
bool Buttons::cBtn() const { return down(BTN_C); }
bool Buttons::upBtn() const { return down(BTN_UP); }
bool Buttons::downBtn() const { return down(BTN_DOWN); }
bool Buttons::leftBtn() const { return down(BTN_LEFT); }
bool Buttons::rightBtn() const { return down(BTN_RIGHT); }

uint8_t Display::invisiblecolor = 255;
uint8_t* Display::screenbuffer = nullptr;
bool Display::persistence = true;
uint8_t Display::s_penColor = 0;
uint16_t Display::s_palette[16]{};
const uint8_t* Display::s_font = nullptr;
int16_t Display::s_cursorX = 0;
int16_t Display::s_cursorY = 0;

void Display::clear() { if (screenbuffer) std::memset(screenbuffer, 0, Display::width * Display::height / 2); }
void Display::fillScreen(uint16_t color) { if (screenbuffer) std::memset(screenbuffer, ((color & 15) << 4) | (color & 15), Display::width * Display::height / 2); }
void Display::setColor(uint8_t idx) { s_penColor = idx & 15; }
void Display::setInvisibleColor(uint8_t idx) { invisiblecolor = idx; }
void Display::loadRGBPalette(const uint8_t* p) {
    for (int i = 0; i < 16; ++i) {
        palette[i][0] = p[i * 3];
        palette[i][1] = p[i * 3 + 1];
        palette[i][2] = p[i * 3 + 2];
        s_palette[i] = static_cast<uint16_t>(((p[i * 3] >> 3) << 11) | ((p[i * 3 + 1] >> 2) << 5) | (p[i * 3 + 2] >> 3));
    }
}
void Display::drawPixel(int16_t x, int16_t y) {
    if (!screenbuffer || x < 0 || y < 0 || x >= width || y >= height) return;
    uint8_t& b = screenbuffer[y * (width / 2) + x / 2];
    if (x & 1) b = (b & 0xf0) | s_penColor; else b = (b & 0x0f) | (s_penColor << 4);
}
uint8_t Display::getNibble(int16_t x, int16_t y) {
    if (!screenbuffer || x < 0 || y < 0 || x >= width || y >= height) return 0;
    uint8_t b = screenbuffer[y * (width / 2) + x / 2];
    return x & 1 ? (b & 15) : (b >> 4);
}
void Display::drawColumn(int16_t x, int16_t y0, int16_t y1) { if (y0 > y1) std::swap(y0, y1); for (int y = y0; y <= y1; ++y) drawPixel(x, y); }
void Display::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1, dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1, error = dx + dy;
    for (;;) { drawPixel(x0, y0); if (x0 == x1 && y0 == y1) break; int e2 = 2 * error; if (e2 >= dy) { error += dy; x0 += sx; } if (e2 <= dx) { error += dx; y0 += sy; } }
}
void Display::drawRect(int16_t x, int16_t y, int16_t w, int16_t h) { for (int i = 0; i < w; ++i) { drawPixel(x+i,y); drawPixel(x+i,y+h-1); } for (int i = 0; i < h; ++i) { drawPixel(x,y+i); drawPixel(x+w-1,y+i); } }
void Display::fillRect(int16_t x, int16_t y, int16_t w, int16_t h) { for (int j = 0; j < h; ++j) for (int i = 0; i < w; ++i) drawPixel(x+i,y+j); }
void Display::drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap) {
    int w = bitmap[0], h = bitmap[1], rowBytes = (w + 1) / 2;
    for (int row = 0; row < h; ++row) for (int col = 0; col < w; ++col) {
        uint8_t b = bitmap[2 + row * rowBytes + col / 2], color = (col & 1) ? b & 15 : b >> 4;
        if (color != invisiblecolor) { uint8_t previous = s_penColor; s_penColor = color; drawPixel(x + col, y + row); s_penColor = previous; }
    }
}
void Display::drawBitmap(int16_t x, int16_t y, const uint8_t* b, bool, bool) { drawBitmap(x, y, b); }
void Display::setCursor(int16_t x, int16_t y) { s_cursorX = x; s_cursorY = y; }
void Display::setFont(const uint8_t* font) { s_font = font; }
void Display::print(char c) {
    const uint8_t* font = s_font ? s_font : font5x7;
    int index = (c >= FONT5X7_FIRST && c <= FONT5X7_LAST) ? c - FONT5X7_FIRST : 0;
    for (int col = 0; col < FONT5X7_WIDTH; ++col) for (int row = 0; row < FONT5X7_HEIGHT; ++row)
        if (font[index * FONT5X7_WIDTH + col] & (1 << row)) drawPixel(s_cursorX + col, s_cursorY + row);
    s_cursorX += FONT5X7_WIDTH + 1;
}
void Display::print(const char* text) { while (*text) print(*text++); }
void Display::println(const char* text) { print(text); s_cursorX = 0; s_cursorY += FONT5X7_HEIGHT + 1; }
void Display::present() {
    if (!renderer || !texture || !screenbuffer) return;
    std::vector<uint8_t> rgb(static_cast<size_t>(width) * height * 3);
    for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) {
        const uint8_t index = getNibble(x, y);
        const size_t offset = (static_cast<size_t>(y) * width + x) * 3;
        rgb[offset] = palette[index][0]; rgb[offset + 1] = palette[index][1]; rgb[offset + 2] = palette[index][2];
    }
    SDL_UpdateTexture(texture, nullptr, rgb.data(), width * 3);
    SDL_RenderClear(renderer); SDL_RenderCopy(renderer, texture, nullptr, nullptr);
#if defined(__ANDROID__)
    drawTouchControls();
#endif
    SDL_RenderPresent(renderer);
}

const uint8_t* Sound::sfxDataPtr = nullptr;
const uint8_t* Sound::sfxEndPtr = nullptr;
void Sound::begin() {}
void Sound::playSFX(const uint8_t* data, uint32_t length) {
    if (!audioDevice || !data || !length) return;
    std::vector<int16_t> samples(length);
    for (uint32_t i = 0; i < length; ++i) samples[i] = static_cast<int16_t>((static_cast<int>(data[i]) - 128) << 8);
    SDL_ClearQueuedAudio(audioDevice);
    SDL_QueueAudio(audioDevice, samples.data(), static_cast<Uint32>(samples.size() * sizeof(int16_t)));
    activeSfx = data; activeSfxEnd = data + length; sfxDataPtr = data; sfxEndPtr = data + length;
}
void Sound::poll() { if (activeSfx && !SDL_GetQueuedAudioSize(audioDevice)) { sfxDataPtr = activeSfxEnd; activeSfx = nullptr; } }
void Sound::playMusicStream(const char* path, uint8_t loop) {
    (void)loop;
    if (!musicDevice || !path) return;
    std::string filename = std::string("assets/galaxy/") + path;
    if (filename.size() >= 4 && filename.substr(filename.size() - 4) == ".raw") filename.replace(filename.size() - 4, 4, ".wav");
    SDL_AudioSpec source{};
    Uint8* bytes = nullptr;
    Uint32 length = 0;
    if (!SDL_LoadWAV(filename.c_str(), &source, &bytes, &length)) {
        filename = std::string("sdcard_files/galaxy/") + path;
        if (filename.size() >= 4 && filename.substr(filename.size() - 4) == ".raw") filename.replace(filename.size() - 4, 4, ".wav");
        if (!SDL_LoadWAV(filename.c_str(), &source, &bytes, &length)) return;
    }
    SDL_AudioCVT conversion;
    if (SDL_BuildAudioCVT(&conversion, source.format, source.channels, source.freq, AUDIO_S16SYS, 1, 22050) < 0) { SDL_FreeWAV(bytes); return; }
    conversion.len = static_cast<int>(length);
    conversion.buf = static_cast<Uint8*>(SDL_malloc(conversion.len * conversion.len_mult));
    if (!conversion.buf) { SDL_FreeWAV(bytes); return; }
    std::memcpy(conversion.buf, bytes, length); SDL_FreeWAV(bytes);
    if (SDL_ConvertAudio(&conversion) == 0) {
        SDL_ClearQueuedAudio(musicDevice);
        SDL_QueueAudio(musicDevice, conversion.buf, static_cast<Uint32>(conversion.len_cvt));
    }
    SDL_free(conversion.buf);
}
void Sound::stopMusic() { if (musicDevice) SDL_ClearQueuedAudio(musicDevice); }
void Sound::playTone(uint16_t, uint16_t) {}
void Sound::stopTone() {}

bool Cookie::begin(const char*, int size, char* data) { m_size = size; m_data = data; return true; }
bool Cookie::loadCookie() {
    if (!m_data) return false;
    FILE* f = std::fopen(savePath().c_str(), "rb"); if (!f) return false;
    bool ok = std::fread(m_data, 1, static_cast<size_t>(m_size), f) == static_cast<size_t>(m_size);
    std::fclose(f); return ok;
}
bool Cookie::saveCookie() {
    if (!m_data) return false;
    FILE* f = std::fopen(savePath().c_str(), "wb"); if (!f) return false;
    bool ok = std::fwrite(m_data, 1, static_cast<size_t>(m_size), f) == static_cast<size_t>(m_size);
    std::fclose(f); return ok;
}
} // namespace Pokitto
