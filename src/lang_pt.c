/**
 * Brick Blaster - Portuguese (Portugal) Localization
 * License: MIT License (c) 2026 issalig
 */
#include "lang.h"

#ifdef LANG_PT
static const u8 s_pt_STR_1_PLAYER[] = "1 JOGADOR";
static const u8 s_pt_STR_2_PLAYERS[] = "2 JOGADORES";
static const u8 s_pt_STR_PRESS_H_HELP[] = "AJUDA";
static const u8 s_pt_STR_CONTROLS[] = "CONTROLOS";
static const u8 s_pt_STR_KEY_LEFT[] = "ESQ:";
static const u8 s_pt_STR_MOVE_LEFT[] = "ESQ/MENU CIMA";
static const u8 s_pt_STR_KEY_RIGHT[] = "DIR:";
static const u8 s_pt_STR_MOVE_RIGHT[] = "DIR/MENU BAIXO";
static const u8 s_pt_STR_KEY_SPACE[] = "ESPACO:";
static const u8 s_pt_STR_FIRE[] = "DISPARO/AVANCAR";
static const u8 s_pt_STR_KEY_ESC[] = "ESC:";
static const u8 s_pt_STR_PAUSE[] = "PAUSA";
static const u8 s_pt_STR_KEY_M[] = "M:";
static const u8 s_pt_STR_MUSIC[] = "MUSICA";
static const u8 s_pt_STR_KEY_P[] = "P:";
static const u8 s_pt_STR_PADDLE[] = "PADDLE";
static const u8 s_pt_STR_CAPSULE_GUIDE[] = "GUIA DE CAPSULAS";
static const u8 s_pt_STR_CLUELESS[] = "PARA DESPISTADOS";
static const u8 s_pt_STR_L_DESC[] = "PARA DAR PORRADA";
static const u8 s_pt_STR_S_DESC[] = "NAS CALMAS";
static const u8 s_pt_STR_C_DESC[] = "COLA DE SAPATEIRO";
static const u8 s_pt_STR_P_DESC[] = "UMA PRENDA, CARALHO";
static const u8 s_pt_STR_B_DESC[] = "SAIDA DE EMERGENCIA";
static const u8 s_pt_STR_E_DESC[] = "PADDLE XXL";
static const u8 s_pt_STR_M_DESC[] = "MUITAS BOLAS";
static const u8 s_pt_STR_I_DESC[] = "CONGELADO NO TEMPO";
static const u8 s_pt_STR_U_DESC[] = "REPELENTE";
static const u8 s_pt_STR_A_DESC[] = "MAOS NOS BOLSOS";
static const u8 s_pt_STR_D_DESC[] = "BEBADO";
static const u8 s_pt_STR_V_DESC[] = "A FUNDO";
static const u8 s_pt_STR_T_DESC[] = "MINI";
static const u8 s_pt_STR_F_DESC[] = "BOLA DE FOGO";

static const u8 s_pt_STR_CONGRATS[] = "PARABENS!";
static const u8 s_pt_STR_RECOVERED_MOTO[] = "RECUPERASTE A TUA MOTA.";
static const u8 s_pt_STR_NOW_GO_BREAD[] = "AGORA JA PODES IR FINALMENTE";
static const u8 s_pt_STR_NOW_GO_BREAD_2[] = "AO PAO DE MAFRA";
static const u8 s_pt_STR_EAT_SANDWICH[] = "E COMER UM BOM";
static const u8 s_pt_STR_SQUID_BOCATA[] = "BACALHAU COM REGA.";
static const u8 s_pt_STR_GAME_OVER[] = "GAME OVER";
static const u8 s_pt_STR_NO_MOTO_NO_EAT[] = "SEM MOTA E SEM COMIDA";
static const u8 s_pt_STR_GAME_OVER_SUB2[] = "JOGA OUTRA VEZ PARA NAO PASSARES FOME";
static const u8 s_pt_STR_STORY_1[] = "ERA UMA MANHA QUALQUER";
static const u8 s_pt_STR_STORY_2[] = "IA AO PAO DE MAFRA, MAS O MANUEL";
static const u8 s_pt_STR_STORY_3[] = "O MARCIANO ROUBOU-TE A SCOOTER.";
static const u8 s_pt_STR_STORY_4[] = "AGORA ESTAS PRESO NUMA";
static const u8 s_pt_STR_STORY_5[] = "PRISAO ESPACIAL DE TIJOLOS.";
static const u8 s_pt_STR_STORY_6[] = "A TUA ARMA: TABUA DE ENGOMAR";
static const u8 s_pt_STR_STORY_7[] = "E UMA BOLA DE TENIS NUCLEAR.";
static const u8 s_pt_STR_STORY_WIN_1[] = "RECUPERA A MOTA E VOLTA";
static const u8 s_pt_STR_STORY_WIN_2[] = "A TEMPO PARA O BACALHAU!";
static const u8 s_pt_STR_PLAYER_START[] = "JOGADOR";
static const u8 s_pt_STR_LEVEL[] = "ESCOLHE NIVEL";
static const u8 s_pt_STR_CREDITS_CODE[] = "CODIGO E GFX: ISSALIG";
static const u8 s_pt_STR_CREDITS_MUSIC[] = "MUSICA: ULTRASYD";
static const u8 s_pt_STR_CREDITS_POWERED[] = "SUPER-PODERES: CPCTELERA";
static const u8 s_pt_STR_GO_TO_THE_DOOR[] = "HORA DE NOS PIRARMOS";
static const u8 s_pt_STR_DEMO[] = "DEMO";
static const u8 s_pt_STR_DIFFICULTY[] = "DIFICULDADE: ";


const u8* const lang_pt[STR_COUNT] = {
    s_pt_STR_1_PLAYER, s_pt_STR_2_PLAYERS, s_pt_STR_PRESS_H_HELP, s_pt_STR_CONTROLS, s_pt_STR_KEY_LEFT, s_pt_STR_MOVE_LEFT, s_pt_STR_KEY_RIGHT, s_pt_STR_MOVE_RIGHT,
    s_pt_STR_KEY_SPACE, s_pt_STR_FIRE, s_pt_STR_KEY_ESC, s_pt_STR_PAUSE, s_pt_STR_KEY_M, s_pt_STR_MUSIC, s_pt_STR_KEY_P, s_pt_STR_PADDLE, s_pt_STR_CAPSULE_GUIDE, s_pt_STR_CLUELESS, s_pt_STR_L_DESC, s_pt_STR_S_DESC,
    s_pt_STR_C_DESC, s_pt_STR_P_DESC, s_pt_STR_B_DESC, s_pt_STR_E_DESC, s_pt_STR_M_DESC, s_pt_STR_I_DESC, s_pt_STR_U_DESC, s_pt_STR_A_DESC, s_pt_STR_D_DESC,
    s_pt_STR_V_DESC, s_pt_STR_T_DESC, s_pt_STR_F_DESC, s_pt_STR_CONGRATS, s_pt_STR_RECOVERED_MOTO, s_pt_STR_NOW_GO_BREAD,
    s_pt_STR_NOW_GO_BREAD_2, s_pt_STR_EAT_SANDWICH, s_pt_STR_SQUID_BOCATA, s_pt_STR_GAME_OVER, s_pt_STR_NO_MOTO_NO_EAT, s_pt_STR_GAME_OVER_SUB2, s_pt_STR_STORY_1, s_pt_STR_STORY_2, s_pt_STR_STORY_3, s_pt_STR_STORY_4, s_pt_STR_STORY_5,
    s_pt_STR_STORY_6, s_pt_STR_STORY_7, s_pt_STR_STORY_WIN_1, s_pt_STR_STORY_WIN_2, s_pt_STR_PLAYER_START, s_pt_STR_LEVEL,
    s_pt_STR_CREDITS_CODE, s_pt_STR_CREDITS_MUSIC, s_pt_STR_CREDITS_POWERED, s_pt_STR_GO_TO_THE_DOOR, s_pt_STR_DEMO,
    s_pt_STR_DIFFICULTY
};
#endif
