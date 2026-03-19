/**
 * Brick Blaster - Greek (Greeklish) Localization
 * License: MIT License (c) 2026 ISSALIG
 */
#include "lang.h"
#ifdef LANG_GR

static const u8 s_gr_STR_GAME_MODE[] = "TROPOS PAIXNIDIOU";
static const u8 s_gr_STR_1_PLAYER[] = "1. 1 PAIKTHS";
static const u8 s_gr_STR_2_PLAYERS[] = "2. 2 PAIKTES";
static const u8 s_gr_STR_PRESS_H_HELP[] = "H. VOHTHEIA";
static const u8 s_gr_STR_CONTROLS[] = "XERISTHRIA";
static const u8 s_gr_STR_KEY_LEFT[] = "ARISTERA/O:";
static const u8 s_gr_STR_MOVE_LEFT[] = "KINHSH ARISTERA";
static const u8 s_gr_STR_KEY_RIGHT[] = "DEXIA/P:";
static const u8 s_gr_STR_MOVE_RIGHT[] = "KINHSH DEXIA";
static const u8 s_gr_STR_KEY_SPACE[] = "SPACE:";
static const u8 s_gr_STR_FIRE[] = "VOLH";
static const u8 s_gr_STR_KEY_ESC[] = "ESC:";
static const u8 s_gr_STR_PAUSE[] = "PAYSH";
static const u8 s_gr_STR_KEY_M[] = "M:";
static const u8 s_gr_STR_MUSIC[] = "MOYSIKH";
static const u8 s_gr_STR_CAPSULE_GUIDE[] = "ODHGOS KAPSULWN";
static const u8 s_gr_STR_CLUELESS[] = "GIA TOUS ASXETOUS";
static const u8 s_gr_STR_L_DESC[] = "EPITHETIKOS";
static const u8 s_gr_STR_S_DESC[] = "ARGOS";
static const u8 s_gr_STR_C_DESC[] = "KOLLWDES";
static const u8 s_gr_STR_P_DESC[] = "ENA THAYMA";
static const u8 s_gr_STR_B_DESC[] = "MAGIKH PORTA";
static const u8 s_gr_STR_E_DESC[] = " XXL RAQUETTA";
static const u8 s_gr_STR_A_DESC[] = "AYTOPILOTOS";
static const u8 s_gr_STR_M_DESC[] = "POLLAPLH BALA!";
static const u8 s_gr_STR_D_DESC[] = "TROPOS MECHHS";
static const u8 s_gr_STR_V_DESC[] = "TROPOS METEWRWN";
static const u8 s_gr_STR_T_DESC[] = "MIKRH RAQUETTA";
static const u8 s_gr_STR_G_DESC[] = "TROPOS BARUTHTAS";
static const u8 s_gr_STR_I_DESC[] = "TROPOS FANTASMA";
static const u8 s_gr_STR_U_DESC[] = "TROPOS PAGOΥ";
static const u8 s_gr_STR_F_DESC[] = "PYRINH BALA";

static const u8 s_gr_STR_CONGRATS[] = "SYGXARHTHRIA!";
static const u8 s_gr_STR_RECOVERED_MOTO[] = "VRETHHKE H MOTO SOY.";
static const u8 s_gr_STR_NOW_GO_BREAD[] = "TWRA MPOREIS TELIKA";
static const u8 s_gr_STR_NOW_GO_BREAD_2[] = "NA PAS GIA PSWMI";
static const u8 s_gr_STR_EAT_SANDWICH[] = "KAI NA FAGES ENA";
static const u8 s_gr_STR_SQUID_BOCATA[] = "NOSTIMO PITOGURO.";
static const u8 s_gr_STR_GAME_OVER[] = "GAME OVER";
static const u8 s_gr_STR_NO_MOTO_NO_EAT[] = "OYTE MOTO, OYTE PSWMI";
static const u8 s_gr_STR_STORY_1[] = "HTAN ENA KANONIKO PRWI";
static const u8 s_gr_STR_STORY_2[] = "PHGAINES GIA PSWMI, ALLA O LOYKIANOS";
static const u8 s_gr_STR_STORY_3[] = "O AREIANOS EKLEPHE TO SKOYTER.";
static const u8 s_gr_STR_STORY_4[] = "TWRA EISAI PAGIDEYMENOS";
static const u8 s_gr_STR_STORY_5[] = "SE MIA FYLAKH APO TOYBLA.";
static const u8 s_gr_STR_STORY_6[] = "TO OPLO SOY: MIA SIDHRWSTRA";
static const u8 s_gr_STR_STORY_7[] = "KAI MIA PYRHNIKH BALA TENNIS.";
static const u8 s_gr_STR_STORY_WIN_1[] = "PARE PISW TO SKOYTER SOY";
static const u8 s_gr_STR_STORY_WIN_2[] = "KAI GYRNA GIA FAGHTO!";
static const u8 s_gr_STR_PLAYER_START[] = "PAIKTHS";
static const u8 s_gr_STR_LEVEL[] = "EPILEXTE EPIPEDO";
static const u8 s_gr_STR_CREDITS_CODE[] = "KWDIKAS KAI GFX: ISSALIG";
static const u8 s_gr_STR_CREDITS_MUSIC[] = "MOYSIKH: ULTRASYD";
static const u8 s_gr_STR_CREDITS_POWERED[] = "ME THN CPCTELERA";
static const u8 s_gr_STR_GO_TO_THE_DOOR[] = "ORA NA FEYGOYME!";

const u8* const lang_gr[STR_COUNT] = {
    s_gr_STR_GAME_MODE, s_gr_STR_1_PLAYER, s_gr_STR_2_PLAYERS, s_gr_STR_PRESS_H_HELP, s_gr_STR_CONTROLS, s_gr_STR_KEY_LEFT, s_gr_STR_MOVE_LEFT, s_gr_STR_KEY_RIGHT, s_gr_STR_MOVE_RIGHT,
    s_gr_STR_KEY_SPACE, s_gr_STR_FIRE, s_gr_STR_KEY_ESC, s_gr_STR_PAUSE, s_gr_STR_KEY_M, s_gr_STR_MUSIC, s_gr_STR_CAPSULE_GUIDE, s_gr_STR_CLUELESS, s_gr_STR_L_DESC, s_gr_STR_S_DESC,
    s_gr_STR_C_DESC, s_gr_STR_P_DESC, s_gr_STR_B_DESC, s_gr_STR_E_DESC, s_gr_STR_A_DESC, s_gr_STR_M_DESC, s_gr_STR_D_DESC, s_gr_STR_V_DESC, s_gr_STR_T_DESC, s_gr_STR_G_DESC,
    s_gr_STR_I_DESC, s_gr_STR_U_DESC, s_gr_STR_F_DESC, s_gr_STR_CONGRATS, s_gr_STR_RECOVERED_MOTO, s_gr_STR_NOW_GO_BREAD,
    s_gr_STR_NOW_GO_BREAD_2, s_gr_STR_EAT_SANDWICH, s_gr_STR_SQUID_BOCATA, s_gr_STR_GAME_OVER, s_gr_STR_NO_MOTO_NO_EAT, s_gr_STR_STORY_1, s_gr_STR_STORY_2, s_gr_STR_STORY_3, s_gr_STR_STORY_4, s_gr_STR_STORY_5,
    s_gr_STR_STORY_6, s_gr_STR_STORY_7, s_gr_STR_STORY_WIN_1, s_gr_STR_STORY_WIN_2, s_gr_STR_PLAYER_START, s_gr_STR_LEVEL,
    s_gr_STR_CREDITS_CODE, s_gr_STR_CREDITS_MUSIC, s_gr_STR_CREDITS_POWERED, s_gr_STR_GO_TO_THE_DOOR
};
#endif

