#include "ui/screen_config.h"

ScreenManager& ScreenManager::getInstance() {
    static ScreenManager instance;
    return instance;
}

ScreenManager::ScreenManager() : currentConfig(LANDSCAPE_CONFIG) {
    // Par défaut : paysage pour optimiser le jeu
}

void ScreenManager::setOrientation(ScreenOrientation orientation) {
    if (orientation == ScreenOrientation::PORTRAIT) {
        currentConfig = PORTRAIT_CONFIG;
    } else {
        currentConfig = LANDSCAPE_CONFIG;
    }
}

ScreenOrientation ScreenManager::getOrientation() const {
    return currentConfig.orientation;
}

const ScreenConfig& ScreenManager::getConfig() const {
    return currentConfig;
}

uint16_t ScreenManager::getScreenWidth() const {
    return currentConfig.width;
}

uint16_t ScreenManager::getScreenHeight() const {
    return currentConfig.height;
}

uint16_t ScreenManager::getGameX() const {
    return currentConfig.gameAreaX;
}

uint16_t ScreenManager::getGameY() const {
    return currentConfig.gameAreaY;
}

uint16_t ScreenManager::getGameWidth() const {
    return currentConfig.gameWidth;
}

uint16_t ScreenManager::getGameHeight() const {
    return currentConfig.gameHeight;
}

int16_t ScreenManager::gameToScreenX(int16_t gameX) const {
    return gameX + currentConfig.gameAreaX;
}

int16_t ScreenManager::gameToScreenY(int16_t gameY) const {
    return gameY + currentConfig.gameAreaY;
}

int16_t ScreenManager::screenToGameX(int16_t screenX) const {
    return screenX - currentConfig.gameAreaX;
}

int16_t ScreenManager::screenToGameY(int16_t screenY) const {
    return screenY - currentConfig.gameAreaY;
}
