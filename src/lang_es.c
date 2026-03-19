/**
 * Brick Blaster - Spanish Localization
 * License: MIT License (c) 2026 ISSALIG
 */
#include "lang.h"

#ifdef LANG_ES
static const u8 s_es_STR_GAME_MODE[] = "MODO DE JUEGO";
static const u8 s_es_STR_1_PLAYER[] = "1. 1 JUGADOR";
static const u8 s_es_STR_2_PLAYERS[] = "2. 2 JUGADORES";
static const u8 s_es_STR_PRESS_H_HELP[] = "H. AYUDA";
static const u8 s_es_STR_CONTROLS[] = "CONTROLES";
static const u8 s_es_STR_KEY_LEFT[] = "IZQ/O:";
static const u8 s_es_STR_MOVE_LEFT[] = "MOVER IZQ";
static const u8 s_es_STR_KEY_RIGHT[] = "DER/P:";
static const u8 s_es_STR_MOVE_RIGHT[] = "MOVER DER";
static const u8 s_es_STR_KEY_SPACE[] = "ESPACIO:";
static const u8 s_es_STR_FIRE[] = "DISPARAR";
static const u8 s_es_STR_KEY_ESC[] = "ESC:";
static const u8 s_es_STR_PAUSE[] = "PAUSA";
static const u8 s_es_STR_KEY_M[] = "M:";
static const u8 s_es_STR_MUSIC[] = "MUSICA";
static const u8 s_es_STR_CAPSULE_GUIDE[] = "GUIA DE CAPSULAS";
static const u8 s_es_STR_CLUELESS[] = "PARA DESPISTADOS";
static const u8 s_es_STR_L_DESC[] = "MODO AGRESIVO";
static const u8 s_es_STR_S_DESC[] = "MODO LEEEENTO";
static const u8 s_es_STR_C_DESC[] = "MODO PEGAJOSO";
static const u8 s_es_STR_P_DESC[] = "UN MILAGRO";
static const u8 s_es_STR_B_DESC[] = "PUERTA MAGICA";
static const u8 s_es_STR_E_DESC[] = "PADDLE XXL";
static const u8 s_es_STR_A_DESC[] = "PILOTO AUTOMATICO";
static const u8 s_es_STR_M_DESC[] = "MULTIBOLA";
static const u8 s_es_STR_D_DESC[] = "MODO MAREADO";
static const u8 s_es_STR_V_DESC[] = "MODO METEORO";
static const u8 s_es_STR_T_DESC[] = "MICRO PADDLE";
static const u8 s_es_STR_G_DESC[] = "MODO GRAVEDAD";
static const u8 s_es_STR_I_DESC[] = "MODO FLASH";
static const u8 s_es_STR_U_DESC[] = "DIRECCION INSISTIDA";
static const u8 s_es_STR_F_DESC[] = "GRAN BOLA DE FUEGO";

static const u8 s_es_STR_CONGRATS[] = "¡ENHORABUENA!";
static const u8 s_es_STR_RECOVERED_MOTO[] = "HAS RECUPERADO TU MOTO.";
static const u8 s_es_STR_NOW_GO_BREAD[] = "AHORA YA PUEDES IR";
static const u8 s_es_STR_NOW_GO_BREAD_2[] = "A POR EL PAN";
static const u8 s_es_STR_EAT_SANDWICH[] = "Y COMERTE UN BOCATA";
static const u8 s_es_STR_SQUID_BOCATA[] = "DE CALAMARES.";
static const u8 s_es_STR_GAME_OVER[] = "GAME OVER";
static const u8 s_es_STR_NO_MOTO_NO_EAT[] = "SIN MOTO Y SIN COMER";
static const u8 s_es_STR_STORY_1[] = "ERA UNA MAÑANA CUALQUIERA";
static const u8 s_es_STR_STORY_2[] = "IBAS A COMPRAR EL PAN, PERO MARIANO";
static const u8 s_es_STR_STORY_3[] = "EL MARCIANO TE ROBO LA VESPA.";
static const u8 s_es_STR_STORY_4[] = "AHORA ESTAS ATRAPADO IN UNA";
static const u8 s_es_STR_STORY_5[] = "PRISION ESPACIAL DE LADRILLOS.";
static const u8 s_es_STR_STORY_6[] = "TU ARMA: UNA TABLA DE PLANCHAR";
static const u8 s_es_STR_STORY_7[] = "Y UNA PELOTA DE TENIS NUCLEAR.";
static const u8 s_es_STR_STORY_WIN_1[] = "¡RECUPERA LA MOTO Y VUELVE";
static const u8 s_es_STR_STORY_WIN_2[] = "A TIEMPO PARA COMER!";
static const u8 s_es_STR_PLAYER_START[] = "JUGADOR";
static const u8 s_es_STR_LEVEL[] = "ELIGE NIVEL";
static const u8 s_es_STR_CREDITS_CODE[] = "CODIGO Y GFX: ISSALIG";
static const u8 s_es_STR_CREDITS_MUSIC[] = "MUSICA: ULTRASYD";
static const u8 s_es_STR_CREDITS_POWERED[] = "SUPERPODERES: CPCTELERA";
static const u8 s_es_STR_GO_TO_THE_DOOR[] = "MOMENTO DE PIRARSE";

const u8* const lang_es[STR_COUNT] = {
    s_es_STR_GAME_MODE, s_es_STR_1_PLAYER, s_es_STR_2_PLAYERS, s_es_STR_PRESS_H_HELP, s_es_STR_CONTROLS, s_es_STR_KEY_LEFT, s_es_STR_MOVE_LEFT, s_es_STR_KEY_RIGHT, s_es_STR_MOVE_RIGHT,
    s_es_STR_KEY_SPACE, s_es_STR_FIRE, s_es_STR_KEY_ESC, s_es_STR_PAUSE, s_es_STR_KEY_M, s_es_STR_MUSIC, s_es_STR_CAPSULE_GUIDE, s_es_STR_CLUELESS, s_es_STR_L_DESC, s_es_STR_S_DESC,
    s_es_STR_C_DESC, s_es_STR_P_DESC, s_es_STR_B_DESC, s_es_STR_E_DESC, s_es_STR_A_DESC, s_es_STR_M_DESC, s_es_STR_D_DESC, s_es_STR_V_DESC, s_es_STR_T_DESC, s_es_STR_G_DESC,
    s_es_STR_I_DESC, s_es_STR_U_DESC, s_es_STR_F_DESC, s_es_STR_CONGRATS, s_es_STR_RECOVERED_MOTO, s_es_STR_NOW_GO_BREAD,
    s_es_STR_NOW_GO_BREAD_2, s_es_STR_EAT_SANDWICH, s_es_STR_SQUID_BOCATA, s_es_STR_GAME_OVER, s_es_STR_NO_MOTO_NO_EAT, s_es_STR_STORY_1, s_es_STR_STORY_2, s_es_STR_STORY_3, s_es_STR_STORY_4, s_es_STR_STORY_5,
    s_es_STR_STORY_6, s_es_STR_STORY_7, s_es_STR_STORY_WIN_1, s_es_STR_STORY_WIN_2, s_es_STR_PLAYER_START, s_es_STR_LEVEL,
    s_es_STR_CREDITS_CODE, s_es_STR_CREDITS_MUSIC, s_es_STR_CREDITS_POWERED, s_es_STR_GO_TO_THE_DOOR
};
#endif

