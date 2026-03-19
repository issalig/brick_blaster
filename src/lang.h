#ifndef LANG_H
#define LANG_H

#include <cpctelera.h>

typedef enum {
    STR_GAME_MODE,
    STR_1_PLAYER,
    STR_2_PLAYERS,
    STR_PRESS_H_HELP,
    STR_CONTROLS,
    STR_KEY_LEFT,
    STR_MOVE_LEFT,
    STR_KEY_RIGHT,
    STR_MOVE_RIGHT,
    STR_KEY_SPACE,
    STR_FIRE,
    STR_KEY_ESC,
    STR_PAUSE,
    STR_KEY_M,
    STR_MUSIC,
    STR_CAPSULE_GUIDE,
    STR_CLUELESS,
    STR_L_DESC,
    STR_S_DESC,
    STR_C_DESC,
    STR_P_DESC,
    STR_B_DESC,
    STR_E_DESC,
    STR_A_DESC,
    STR_M_DESC,
    STR_D_DESC,
    STR_V_DESC,
    STR_T_DESC,
    STR_G_DESC,
    STR_I_DESC,
    STR_U_DESC,
    STR_F_DESC,
    STR_CONGRATS,
    STR_RECOVERED_MOTO,
    STR_NOW_GO_BREAD,
    STR_NOW_GO_BREAD_2,
    STR_EAT_SANDWICH,
    STR_SQUID_BOCATA,
    STR_GAME_OVER,
    STR_NO_MOTO_NO_EAT,
    STR_STORY_1,
    STR_STORY_2,
    STR_STORY_3,
    STR_STORY_4,
    STR_STORY_5,
    STR_STORY_6,
    STR_STORY_7,
    STR_STORY_WIN_1,
    STR_STORY_WIN_2,
    STR_PLAYER_START,
    STR_LEVEL,
    STR_CREDITS_CODE,
    STR_CREDITS_MUSIC,
    STR_CREDITS_POWERED,
    STR_GO_TO_THE_DOOR,
    STR_DEMO,
    STR_DIFFICULTY,
    STR_EASY,
    STR_NORMAL,
    STR_HARD,
    STR_COUNT
} string_id_t;

#ifdef LANG_ES
extern const u8* const lang_es[STR_COUNT];
#endif
#ifdef LANG_EN
extern const u8* const lang_en[STR_COUNT];
#endif
#ifdef LANG_FR
extern const u8* const lang_fr[STR_COUNT];
#endif
#ifdef LANG_GR
extern const u8* const lang_gr[STR_COUNT];
#endif

extern const u8** const lang_strings[4];
extern u8 current_lang;

#define GET_STR(id) lang_strings[current_lang][id]

#endif
