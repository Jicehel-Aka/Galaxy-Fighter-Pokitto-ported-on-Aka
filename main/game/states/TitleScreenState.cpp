#include "TitleScreenState.h"

#include "../images/Images.h"
#include "../sounds/Sounds.h"
#include "../utils/Utils.h"
#include <cstring>   // memcpy

using PC = Pokitto::Core;
using PD = Pokitto::Display;
using PS = Pokitto::Sound;

constexpr const static uint8_t UPLOAD_DELAY = 16;




// ----------------------------------------------------------------------------
//  Initialise state ..
//
void TitleScreenState::activate() {

    this->counter = 20;
    this->stateToggle = 300;
    this->viewState = ViewState::Normal;
	this->initStarField();
    PD::invisiblecolor = 4;
    
}


// ----------------------------------------------------------------------------
//  Handle state updates .. 
//
GameStateType TitleScreenState::update(GameStateType currentState, GameCookie *cookie) {

    this->updateStarField(1);
  
    if (this->counter > 0) {
        
        this->counter--;
        
    }
    else {
     
        if (viewState == ViewState::StartGame) {

            currentState = GameStateType::PlayGame_Activate; 
            
        }
        
    }
  


    // Update highlight ..
    
    if (Utils::getFrameCount(5) == 0) {

        this->titleSeq++;

        if (this->titleSeq > 16) this->titleSeq = 0;

    }


    // Update states ..
    
    switch (this->viewState) {

        case ViewState::Marquee:

            if (Utils::getFrameCount(2) == 0) {

                this->marquee++;
                if (this->marquee > 950) {
                    
                    this->marquee = 0;
                    this->stateToggle = 300;
                    this->viewState = ViewState::Normal;

                }

            }

            break;

        case ViewState::Normal:

            this->stateToggle--;

            if (this->stateToggle == 0) {
                
                this->marquee = 0;
                this->viewState = ViewState::Marquee;

            }

            break;

    }


    
	// Handle other input ..
	// AKA : A lance aussi la partie (en plus de C, l'"insert credit" arcade
	// d'origine) -- convention plus naturelle (A=lancer), et evite de
	// dependre uniquement de C tant que le conflit C/retour-loader n'est
	// pas isole (cf. investigation aka_runtime).
	if (PC::buttons.pressed(BTN_C) || PC::buttons.pressed(BTN_A)) {
	    
	    if (this->viewState == ViewState::Normal || this->viewState == ViewState::Marquee) {
	        
            this->viewState = ViewState::StartGame;
            this->counter = 79;
            PS::playSFX(Sounds::sfx_Coin, Sounds::sfx_Coin_length);
            
	    }
	    else {

            currentState = GameStateType::PlayGame_Activate; 
            
	    }

	}

	if (PC::buttons.pressed(BTN_B)) {

        cookie->setLastScore(0);
        currentState = GameStateType::HighScore_Activate; 
        
    }

    return currentState;

}


// ----------------------------------------------------------------------------
//  Render the state .. 
//
void TitleScreenState::render(GameCookie *cookie) {

    PD::clear();

    
    // Render star field ..
    
    this->renderStarField(1);
    
    PD::drawBitmap(20, 30, Images::Title_Full);

    switch (this->titleSeq) {

        case 1:
            PD::drawBitmap(24, 38, Images::Title_00);
            break;

        case 2:
            PD::drawBitmap(30, 44, Images::Title_01);
            break;

        case 3:
            PD::drawBitmap(55, 39, Images::Title_02);
            break;

        case 4:
            PD::drawBitmap(72, 53, Images::Title_03);
            break;

        case 5:
            PD::drawBitmap(113, 54, Images::Title_04);
            break;

    }

    if ((this->viewState == ViewState::Normal && this->counter == 0) || (this->viewState == ViewState::StartGame && ((this->counter / 20) % 2) == 1)) {
        
        PD::setCursor(47, 130);
        PD::setColor(6);
        PD::print("INSERT CREDITS");
        
    }

    if (this->viewState == ViewState::Marquee) {

        const char highScores[] = { 'H', 'I', 'G', 'H', ' ', 'S', 'C', 'O', 'R', 'E', 'S' };
        uint8_t x = 0;

        uint8_t marqueeOffset = this->marquee % 10;
        uint8_t marquee10 = this->marquee / 10;

        PD::setCursor(47 - (this->marquee % 10), 130);
        PD::setColor(6);

        for (uint8_t i = marquee10; i < marquee10 + 16; i++) {

            if ((i >= 16 && i <= 19) || (i >= 21 && i <= 26)) {
                PD::setColor(6);
                this->printSingleChar(highScores[i - 16]);
            }
            else if (i >= 29 && i <= 92) {
                bool printed = false;
                for (uint8_t player = 0; player < 5 && !printed; ++player) {
                    const uint8_t initialsStart = 29 + player * 13;
                    const uint8_t scoreStart = initialsStart + 4;
                    if (i >= initialsStart && i <= initialsStart + 2) {
                        PD::setColor(6);
                        this->printChar(cookie->initials[player][i - initialsStart]);
                        printed = true;
                    }
                    else if (i >= scoreStart && i <= scoreStart + 7) {
                        PD::setColor(8);
                        uint8_t digits[8] = {};
                        Utils::extractDigits(digits, cookie->score[player]);
                        this->printNumber(digits[scoreStart + 7 - i]);
                        printed = true;
                    }
                }
                if (!printed) {
                    PD::setColor(0);
                    PD::print('-');
                }
            }
            else {
                PD::setColor(0);
                PD::print('-');
            }

        }


        PD::setColor(0);
        PD::fillRect(36, 130, 10, 10);
        PD::fillRect(174, 130, 15, 10);
    }
    
}


void TitleScreenState::printSingleChar(char theChar) {

    char output[] = { ' ', '\0' };
    memcpy(&output, &theChar, 1);
    PD::print(&output[0]);
    
}

void TitleScreenState::printChar(uint8_t charIndex) {

    if (charIndex >= 1 && charIndex <= 26) {
        charIndex += 64;
    }
    else if (charIndex >= 27 && charIndex <= 36) {
        charIndex += 21;
    }
    else if (charIndex == 37) {
        charIndex = 46;
    }

    char output[] = { ' ', '\0' };
    memcpy(&output, &charIndex, 1);
    PD::print(&output[0]);
    
}


void TitleScreenState::printNumber(uint8_t number) {

    number = number + 48;

    char output[] = { ' ', '\0' };
    memcpy(&output, &number, 1);
    PD::print(&output[0]);
    
}
