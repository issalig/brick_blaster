/**
 * Brick Blaster - Valencian Localization
 * License: MIT License (c) 2026 ISSALIG
 */
#include "lang.h"

#ifdef LANG_VA
static const u8 s_va_STR_GAME_MODE[] = "MODE DE JOC";
static const u8 s_va_STR_1_PLAYER[] = "1 JUGADOR";
static const u8 s_va_STR_2_PLAYERS[] = "2 JUGADORS";
static const u8 s_va_STR_PRESS_H_HELP[] = "AJUDA";
static const u8 s_va_STR_CONTROLS[] = "CONTROLS";
static const u8 s_va_STR_KEY_LEFT[] = "ESQ / O:";
static const u8 s_va_STR_MOVE_LEFT[] = "MOURE ESQUERRA";
static const u8 s_va_STR_KEY_RIGHT[] = "DRE / P:";
static const u8 s_va_STR_MOVE_RIGHT[] = "MOURE DRETA";
static const u8 s_va_STR_KEY_SPACE[] = "ESPAI:";
static const u8 s_va_STR_FIRE[] = "DISPARO";
static const u8 s_va_STR_KEY_ESC[] = "ESC:";
static const u8 s_va_STR_PAUSE[] = "PAUSA";
static const u8 s_va_STR_KEY_M[] = "M:";
static const u8 s_va_STR_MUSIC[] = "MUSICA";
static const u8 s_va_STR_CAPSULE_GUIDE[] = "GUIA DE CAPSULES";
static const u8 s_va_STR_CLUELESS[] = "PELS DESPISTATS";
static const u8 s_va_STR_L_DESC[] = "AGRESIU";
static const u8 s_va_STR_S_DESC[] = "A POC A POC";
static const u8 s_va_STR_C_DESC[] = "TOT APEGALOS";
static const u8 s_va_STR_P_DESC[] = "REGALET";
static const u8 s_va_STR_B_DESC[] = "PORTA MAGICA";
static const u8 s_va_STR_E_DESC[] = "PADDLE XXL";
static const u8 s_va_STR_M_DESC[] = "MULTIBOLA";
static const u8 s_va_STR_I_DESC[] = "CONGELACIO";
static const u8 s_va_STR_U_DESC[] = "REPEL.LENT";
static const u8 s_va_STR_A_DESC[] = "PILOT AUTOMATIC";
static const u8 s_va_STR_D_DESC[] = "VAIG BUFAT";
static const u8 s_va_STR_V_DESC[] = "FITIPALDI";
static const u8 s_va_STR_T_DESC[] = "XICOTIU";
static const u8 s_va_STR_F_DESC[] = "GRAN BOLA DE FOC";

static const u8 s_va_STR_CONGRATS[] = "ENHORABONA!";
static const u8 s_va_STR_RECOVERED_MOTO[] = "HAS RECUPERAT LA MOTO.";
static const u8 s_va_STR_NOW_GO_BREAD[] = "ARA JA POTS ANAR";
static const u8 s_va_STR_NOW_GO_BREAD_2[] = "A PEL PA";
static const u8 s_va_STR_EAT_SANDWICH[] = "I MENJAR-TE UN BOCATA";
static const u8 s_va_STR_SQUID_BOCATA[] = "DE CALAMARS EN ALIOLI.";
static const u8 s_va_STR_GAME_OVER[] = "GAME OVER";
static const u8 s_va_STR_NO_MOTO_NO_EAT[] = "SENSE MOTO I SENSE DINAR";
static const u8 s_va_STR_GAME_OVER_SUB2[] = "TORNA A JUGAR PER A CONSEGUIR PAPEO";
static const u8 s_va_STR_STORY_1[] = "ERA UN MATI QUALSEVOL";
static const u8 s_va_STR_STORY_2[] = "ANAVES A COMPRAR EL PA, PERO MARIA";
static const u8 s_va_STR_STORY_3[] = "EL MARCIA ET VA ROBAR LA VESPA.";
static const u8 s_va_STR_STORY_4[] = "ARA ESTAS ATRAPAT EN UNA";
static const u8 s_va_STR_STORY_5[] = "PRESO ESPACIAL DE RAJOLES.";
static const u8 s_va_STR_STORY_6[] = "LA TEUA ARMA: UNA TAULA DE PLANXAR";
static const u8 s_va_STR_STORY_7[] = "I UNA PILOTA DE TENIS NUCLEAR.";
static const u8 s_va_STR_STORY_WIN_1[] = "RECUPERA LA MOTO I TORNA";
static const u8 s_va_STR_STORY_WIN_2[] = "A TEMPS PER A DINAR!";
static const u8 s_va_STR_PLAYER_START[] = "JUGADOR";
static const u8 s_va_STR_LEVEL[] = "TRIA NIVELL";
static const u8 s_va_STR_CREDITS_CODE[] = "CODI I GFX: ISSALIG";
static const u8 s_va_STR_CREDITS_MUSIC[] = "MUSICA: ULTRASYD";
static const u8 s_va_STR_CREDITS_POWERED[] = "SUPERPODERS: CPCTELERA";
static const u8 s_va_STR_GO_TO_THE_DOOR[] = "ANEM-NOS-EN";
static const u8 s_va_STR_DEMO[] = "DEMO";
static const u8 s_va_STR_DIFFICULTY[] = "DIFICULTAT: ";
static const u8 s_va_STR_EASY[] = "FACIL";
static const u8 s_va_STR_NORMAL[] = "NORMAL";
static const u8 s_va_STR_HARD[] = "DIFICIL";

const u8* const lang_va[STR_COUNT] = {
    s_va_STR_GAME_MODE, s_va_STR_1_PLAYER, s_va_STR_2_PLAYERS, s_va_STR_PRESS_H_HELP, s_va_STR_CONTROLS, s_va_STR_KEY_LEFT, s_va_STR_MOVE_LEFT, s_va_STR_KEY_RIGHT, s_va_STR_MOVE_RIGHT,
    s_va_STR_KEY_SPACE, s_va_STR_FIRE, s_va_STR_KEY_ESC, s_va_STR_PAUSE, s_va_STR_KEY_M, s_va_STR_MUSIC, s_va_STR_CAPSULE_GUIDE, s_va_STR_CLUELESS, s_va_STR_L_DESC, s_va_STR_S_DESC,
    s_va_STR_C_DESC, s_va_STR_P_DESC, s_va_STR_B_DESC, s_va_STR_E_DESC, s_va_STR_M_DESC, s_va_STR_I_DESC, s_va_STR_U_DESC, s_va_STR_A_DESC, s_va_STR_D_DESC,
    s_va_STR_V_DESC, s_va_STR_T_DESC, s_va_STR_F_DESC, s_va_STR_CONGRATS, s_va_STR_RECOVERED_MOTO, s_va_STR_NOW_GO_BREAD,
    s_va_STR_NOW_GO_BREAD_2, s_va_STR_EAT_SANDWICH, s_va_STR_SQUID_BOCATA, s_va_STR_GAME_OVER, s_va_STR_NO_MOTO_NO_EAT, s_va_STR_GAME_OVER_SUB2, s_va_STR_STORY_1, s_va_STR_STORY_2, s_va_STR_STORY_3, s_va_STR_STORY_4, s_va_STR_STORY_5,
    s_va_STR_STORY_6, s_va_STR_STORY_7, s_va_STR_STORY_WIN_1, s_va_STR_STORY_WIN_2, s_va_STR_PLAYER_START, s_va_STR_LEVEL,
    s_va_STR_CREDITS_CODE, s_va_STR_CREDITS_MUSIC, s_va_STR_CREDITS_POWERED, s_va_STR_GO_TO_THE_DOOR, s_va_STR_DEMO,
    s_va_STR_DIFFICULTY, s_va_STR_EASY, s_va_STR_NORMAL, s_va_STR_HARD
};
#endif
