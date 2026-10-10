#include "ui/button_overlay.h"
#include "core/input.h"
#include "gb_graphics.h"
#include "gb_common.h"

extern gb_graphics gfx;
ButtonOverlay buttonOverlay;

// Zones hors-écran du jeu (320x240)
// Gauche : -80 à 0 (dpad + autres)
// Droite : 320 à 400 (A/B/C/D)
// Bas : 240 à 280 (Run/Menu)

void ButtonOverlay::init() {
    // ========== ZONE GAUCHE (DPad et boutons L1/R1) ==========
    
    // Directional Pad (colonne gauche)
    // Haut
    buttons[4] = {
        .x = -60, .y = 200,
        .w = 32, .h = 32,
        .colorBase = 0x4444,      // Gris foncé
        .colorPressed = 0x6666,   // Gris clair
        .label = "↑",
        .isPressed = false
    };
    
    // Bas
    buttons[5] = {
        .x = -60, .y = 240,
        .w = 32, .h = 32,
        .colorBase = 0x4444,
        .colorPressed = 0x6666,
        .label = "↓",
        .isPressed = false
    };
    
    // Gauche
    buttons[6] = {
        .x = -100, .y = 220,
        .w = 32, .h = 32,
        .colorBase = 0x4444,
        .colorPressed = 0x6666,
        .label = "←",
        .isPressed = false
    };
    
    // Droite
    buttons[7] = {
        .x = -20, .y = 220,
        .w = 32, .h = 32,
        .colorBase = 0x4444,
        .colorPressed = 0x6666,
        .label = "→",
        .isPressed = false
    };

    // ========== ZONE DROITE (Boutons A/B/C/D) ==========
    
    // A (Rouge) - bas gauche
    buttons[0] = {
        .x = 328, .y = 200,
        .w = 36, .h = 36,
        .colorBase = 0xF800,      // Rouge
        .colorPressed = 0xFF00,   // Rouge plus clair
        .label = "A",
        .isPressed = false
    };
    
    // B (Vert) - haut droit
    buttons[1] = {
        .x = 378, .y = 150,
        .w = 36, .h = 36,
        .colorBase = 0x07E0,      // Vert
        .colorPressed = 0x0FE0,   // Vert plus clair
        .label = "B",
        .isPressed = false
    };
    
    // C (Bleu) - bas droit
    buttons[2] = {
        .x = 378, .y = 200,
        .w = 36, .h = 36,
        .colorBase = 0x001F,      // Bleu
        .colorPressed = 0x003F,   // Bleu plus clair
        .label = "C",
        .isPressed = false
    };
    
    // D (Jaune) - haut gauche
    buttons[3] = {
        .x = 328, .y = 150,
        .w = 36, .h = 36,
        .colorBase = 0xFFE0,      // Jaune
        .colorPressed = 0xFFFF,   // Blanc/jaune clair
        .label = "D",
        .isPressed = false
    };

    // ========== ZONE INFÉRIEURE (Run/Menu) ==========
    
    // L1 (Marron)
    buttons[8] = {
        .x = 10, .y = 248,
        .w = 48, .h = 28,
        .colorBase = 0x8440,      // Marron
        .colorPressed = 0xA660,   // Marron clair
        .label = "L1",
        .isPressed = false
    };
    
    // R1 (Marron)
    buttons[9] = {
        .x = 262, .y = 248,
        .w = 48, .h = 28,
        .colorBase = 0x8440,
        .colorPressed = 0xA660,
        .label = "R1",
        .isPressed = false
    };
}

uint16_t ButtonOverlay::makeColor(uint8_t r, uint8_t g, uint8_t b) {
    // RGB 565: RRRRRGGGGGGBBBBB
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}

void ButtonOverlay::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    gfx.setColor(color);
    gfx.drawRoundRect(x, y, w, h, r);
}

void ButtonOverlay::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    gfx.setColor(color);
    gfx.fillRoundRect(x, y, w, h, r);
}

void ButtonOverlay::drawText(int16_t x, int16_t y, const char* text, uint16_t color) {
    gfx.setColor(color);
    gfx.move_cursor(x, y);
    gfx.print_str(text);
}

void ButtonOverlay::drawButton(const Button& btn) {
    uint16_t color = btn.isPressed ? btn.colorPressed : btn.colorBase;
    
    // Fond arrondi
    fillRoundRect(btn.x, btn.y, btn.w, btn.h, 6, color);
    
    // Bordure
    gfx.setColor(0xFFFF);  // Blanc
    drawRoundRect(btn.x, btn.y, btn.w, btn.h, 6, 0xFFFF);
    
    // Texte (centré)
    uint16_t textColor = 0x0000;  // Noir
    int16_t textX = btn.x + (btn.w / 2) - 4;
    int16_t textY = btn.y + (btn.h / 2) - 4;
    drawText(textX, textY, btn.label, textColor);
}

void ButtonOverlay::render() {
    // Dessiner tous les boutons
    for (int i = 0; i < 10; i++) {
        drawButton(buttons[i]);
    }
}

void ButtonOverlay::updateState(const Keys& keys) {
    // Mettre à jour l'état des boutons basé sur les entrées
    buttons[0].isPressed = keys.A;      // A
    buttons[1].isPressed = keys.B;      // B
    buttons[2].isPressed = keys.C;      // C
    buttons[3].isPressed = keys.D;      // D
    buttons[4].isPressed = keys.up;     // UP
    buttons[5].isPressed = keys.down;   // DOWN
    buttons[6].isPressed = keys.left;   // LEFT
    buttons[7].isPressed = keys.right;  // RIGHT
    buttons[8].isPressed = keys.L1;     // L1
    buttons[9].isPressed = keys.R1;     // R1
}
