/**
 * Brick Blaster - English Localization
 * License: MIT License (c) 2026 ISSALIG
 */
#include "lang.h"
#ifdef LANG_EN

static const u8 s_en_STR_GAME_MODE[] = "GAME MODE";
static const u8 s_en_STR_1_PLAYER[] = "1. 1 PLAYER";
static const u8 s_en_STR_2_PLAYERS[] = "2. 2 PLAYERS";
static const u8 s_en_STR_PRESS_H_HELP[] = "H. HELP";
static const u8 s_en_STR_CONTROLS[] = "CONTROLS";
static const u8 s_en_STR_KEY_LEFT[] = "LEFT/O:";
static const u8 s_en_STR_MOVE_LEFT[] = "MOVE LEFT";
static const u8 s_en_STR_KEY_RIGHT[] = "RIGHT/P:";
static const u8 s_en_STR_MOVE_RIGHT[] = "MOVE RIGHT";
static const u8 s_en_STR_KEY_SPACE[] = "SPACE:";
static const u8 s_en_STR_FIRE[] = "FIRE";
static const u8 s_en_STR_KEY_ESC[] = "ESC:";
static const u8 s_en_STR_PAUSE[] = "PAUSE";
static const u8 s_en_STR_KEY_M[] = "M:";
static const u8 s_en_STR_MUSIC[] = "MUSIC";
static const u8 s_en_STR_CAPSULE_GUIDE[] = "CAPSULE GUIDE";
static const u8 s_en_STR_CLUELESS[] = "FOR THE CLUELESS";
static const u8 s_en_STR_L_DESC[] = "AGGRESSIVE";
static const u8 s_en_STR_S_DESC[] = "SLOW";
static const u8 s_en_STR_C_DESC[] = "STICKY";
static const u8 s_en_STR_P_DESC[] = "A MIRACLE";
static const u8 s_en_STR_B_DESC[] = "MAGIC DOOR";
static const u8 s_en_STR_E_DESC[] = "XXL PADDLE";
static const u8 s_en_STR_A_DESC[] = "AUTOPILOT";
static const u8 s_en_STR_M_DESC[] = "MULTIBALL!";
static const u8 s_en_STR_D_DESC[] = "DRUNK MODE";
static const u8 s_en_STR_V_DESC[] = "METEOR MODE";
static const u8 s_en_STR_T_DESC[] = "TINY PADDLE";
static const u8 s_en_STR_G_DESC[] = "GRAVITY MODE";
static const u8 s_en_STR_I_DESC[] = "GHOST MODE";
static const u8 s_en_STR_U_DESC[] = "ICE MODE";
static const u8 s_en_STR_F_DESC[] = "GREAT BALL OF FIRE!";

static const u8 s_en_STR_CONGRATS[] = "CONGRATULATIONS!";
static const u8 s_en_STR_RECOVERED_MOTO[] = "YOU GOT YOUR BIKE BACK.";
static const u8 s_en_STR_NOW_GO_BREAD[] = "NOW YOU CAN FINALLY GO";
static const u8 s_en_STR_NOW_GO_BREAD_2[] = "AND GET SOME BREAD";
static const u8 s_en_STR_EAT_SANDWICH[] = "AND EAT A NICE";
static const u8 s_en_STR_SQUID_BOCATA[] = "BACON SANDWICH.";
static const u8 s_en_STR_GAME_OVER[] = "GAME OVER";
static const u8 s_en_STR_NO_MOTO_NO_EAT[] = "NO BIKE, NO DINNER";
static const u8 s_en_STR_STORY_1[] = "IT WAS A NORMAL MORNING";
static const u8 s_en_STR_STORY_2[] = "YOU WENT FOR BREAD, BUT MARTIN";
static const u8 s_en_STR_STORY_3[] = "THE MARTIAN STOLE YOUR SCOOTER.";
static const u8 s_en_STR_STORY_4[] = "NOW YOU ARE TRAPPED IN A";
static const u8 s_en_STR_STORY_5[] = "SPACE PRISON OF BRICKS.";
static const u8 s_en_STR_STORY_6[] = "YOUR WEAPON: AN IRONING BOARD";
static const u8 s_en_STR_STORY_7[] = "AND A NUCLEAR TENNIS BALL.";
static const u8 s_en_STR_STORY_WIN_1[] = "GET YOUR SCOOTER BACK AND";
static const u8 s_en_STR_STORY_WIN_2[] = "RETURN IN TIME FOR DINNER!";
static const u8 s_en_STR_PLAYER_START[] = "PLAYER";
static const u8 s_en_STR_LEVEL[] = "SELECT LEVEL";
static const u8 s_en_STR_CREDITS_CODE[] = "CODE AND GFX: ISSALIG";
static const u8 s_en_STR_CREDITS_MUSIC[] = "MUSIC: ULTRASYD";
static const u8 s_en_STR_CREDITS_POWERED[] = "POWERED BY CPCTELERA";
static const u8 s_en_STR_GO_TO_THE_DOOR[] = "TIME TO HEAD OUT!";

const u8* const lang_en[STR_COUNT] = {
    s_en_STR_GAME_MODE, s_en_STR_1_PLAYER, s_en_STR_2_PLAYERS, s_en_STR_PRESS_H_HELP, s_en_STR_CONTROLS, s_en_STR_KEY_LEFT, s_en_STR_MOVE_LEFT, s_en_STR_KEY_RIGHT, s_en_STR_MOVE_RIGHT,
    s_en_STR_KEY_SPACE, s_en_STR_FIRE, s_en_STR_KEY_ESC, s_en_STR_PAUSE, s_en_STR_KEY_M, s_en_STR_MUSIC, s_en_STR_CAPSULE_GUIDE, s_en_STR_CLUELESS, s_en_STR_L_DESC, s_en_STR_S_DESC,
    s_en_STR_C_DESC, s_en_STR_P_DESC, s_en_STR_B_DESC, s_en_STR_E_DESC, s_en_STR_A_DESC, s_en_STR_M_DESC, s_en_STR_D_DESC, s_en_STR_V_DESC, s_en_STR_T_DESC, s_en_STR_G_DESC,
    s_en_STR_I_DESC, s_en_STR_U_DESC, s_en_STR_F_DESC, s_en_STR_CONGRATS, s_en_STR_RECOVERED_MOTO, s_en_STR_NOW_GO_BREAD,
    s_en_STR_NOW_GO_BREAD_2, s_en_STR_EAT_SANDWICH, s_en_STR_SQUID_BOCATA, s_en_STR_GAME_OVER, s_en_STR_NO_MOTO_NO_EAT, s_en_STR_STORY_1, s_en_STR_STORY_2, s_en_STR_STORY_3, s_en_STR_STORY_4, s_en_STR_STORY_5,
    s_en_STR_STORY_6, s_en_STR_STORY_7, s_en_STR_STORY_WIN_1, s_en_STR_STORY_WIN_2, s_en_STR_PLAYER_START, s_en_STR_LEVEL,
    s_en_STR_CREDITS_CODE, s_en_STR_CREDITS_MUSIC, s_en_STR_CREDITS_POWERED, s_en_STR_GO_TO_THE_DOOR
};
#endif

