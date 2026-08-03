idf_component_register(
    SRCS "main.cpp" "game/Game.cpp" "game/entities/Bullet.cpp" "game/entities/Bullets.cpp" "game/entities/CaptureController.cpp" "game/entities/Enemies.cpp" "game/entities/Enemy.cpp" "game/entities/EnemyBullet.cpp" "game/entities/EnemyBullets.cpp" "game/entities/Player.cpp" "game/states/BaseState.cpp" "game/states/HighScoreState.cpp" "game/states/PlayGameState_Activate.cpp" "game/states/PlayGameState_Render.cpp" "game/states/PlayGameState_Update.cpp" "game/states/PlayGameState_Utils.cpp" "game/states/SplashScreenState.cpp" "game/states/TitleScreenState.cpp"
    INCLUDE_DIRS "." 
    REQUIRES gamebuino aka_runtime pokitto_compat
    PRIV_REQUIRES app_update)

target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-error=reorder -Wno-error=switch -Wno-error=unused-variable
    -Wno-error=type-limits -Wno-error=narrowing -Wno-error=parentheses
    -Wno-error=maybe-uninitialized -Wno-error=unused-function -Wno-error=missing-field-initializers
    -Wno-error=unused-value)

# My_settings.h du jeu (non repris tel quel) definit INCLUDE_SOUND et
# INCLUDE_SOUND_FROM_SD -- retrouve lors du portage Kong-II (macro non
# documentee au depart, cf compatibility_db). On les definit directement
# ici, comme pour Kong-II.
target_compile_definitions(${COMPONENT_LIB} PRIVATE INCLUDE_SOUND_FROM_SD INCLUDE_SOUND)
