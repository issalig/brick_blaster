/**
 * Brick Blaster - French Localization
 * License: MIT License (c) 2026 ISSALIG
 */
#include "lang.h"
#ifdef LANG_FR

static const u8 s_fr_STR_GAME_MODE[] = "MODE DE JEU";
static const u8 s_fr_STR_1_PLAYER[] = "1. 1 JOUEUR";
static const u8 s_fr_STR_2_PLAYERS[] = "2. 2 JOUEURS";
static const u8 s_fr_STR_PRESS_H_HELP[] = "H. AIDE";
static const u8 s_fr_STR_CONTROLS[] = "CONTROLES";
static const u8 s_fr_STR_KEY_LEFT[] = "GAUCHE/O:";
static const u8 s_fr_STR_MOVE_LEFT[] = "VERS LA GAUCHE";
static const u8 s_fr_STR_KEY_RIGHT[] = "DROITE/P:";
static const u8 s_fr_STR_MOVE_RIGHT[] = "VERS LA DROITE";
static const u8 s_fr_STR_KEY_SPACE[] = "ESPACIO:";
static const u8 s_fr_STR_FIRE[] = "TIRER";
static const u8 s_fr_STR_KEY_ESC[] = "ESC:";
static const u8 s_fr_STR_PAUSE[] = "PAUSE";
static const u8 s_fr_STR_KEY_M[] = "M:";
static const u8 s_fr_STR_MUSIC[] = "MUSIQUE";
static const u8 s_fr_STR_CAPSULE_GUIDE[] = "GUIDE CAPSULES";
static const u8 s_fr_STR_CLUELESS[] = "POUR LES NULS";
static const u8 s_fr_STR_L_DESC[] = "AGGRESSIF";
static const u8 s_fr_STR_S_DESC[] = "LENT";
static const u8 s_fr_STR_C_DESC[] = "COLLANT";
static const u8 s_fr_STR_P_DESC[] = "UN MIRACLE";
static const u8 s_fr_STR_B_DESC[] = "PORTE MAGIQUE";
static const u8 s_fr_STR_E_DESC[] = "RAQUETTE XXL";
static const u8 s_fr_STR_A_DESC[] = "AUTOPILOTE";
static const u8 s_fr_STR_M_DESC[] = "MULTI-BALLE!";
static const u8 s_fr_STR_D_DESC[] = "MODE IVRE";
static const u8 s_fr_STR_V_DESC[] = "MODE METEORE";
static const u8 s_fr_STR_T_DESC[] = "MINI RAQUETTE";
static const u8 s_fr_STR_G_DESC[] = "MODE GRAVITE";
static const u8 s_fr_STR_I_DESC[] = "MODE FANTOME";
static const u8 s_fr_STR_U_DESC[] = "MODE GLACE";
static const u8 s_fr_STR_F_DESC[] = "BOULE DE FEU";

static const u8 s_fr_STR_CONGRATS[] = "FELICITATIONS!";
static const u8 s_fr_STR_RECOVERED_MOTO[] = "TA MOTO EST TROUVEE.";
static const u8 s_fr_STR_NOW_GO_BREAD[] = "MAINTENANT TU PEUX";
static const u8 s_fr_STR_NOW_GO_BREAD_2[] = "ALLER CHERCHER";
static const u8 s_fr_STR_EAT_SANDWICH[] = "DU PAIN ET MANGER";
static const u8 s_fr_STR_SQUID_BOCATA[] = "UN JAMBON-BEURRE.";
static const u8 s_fr_STR_GAME_OVER[] = "GAME OVER";
static const u8 s_fr_STR_NO_MOTO_NO_EAT[] = "NI MOTO, NI DINER";
static const u8 s_fr_STR_STORY_1[] = "C,ETAIT UN MATIN NORMAL";
static const u8 s_fr_STR_STORY_2[] = "TU ALLAIS AU PAIN, MAIS LUCIEN";
static const u8 s_fr_STR_STORY_3[] = "LE MARTIEN A VOLE TON SCOOTER.";
static const u8 s_fr_STR_STORY_4[] = "TU ES MAINTENANT COINCE";
static const u8 s_fr_STR_STORY_5[] = "DANS UNE PRISON DE BRIQUES.";
static const u8 s_fr_STR_STORY_6[] = "TON ARME: UNE PLANCHE A REPASSER";
static const u8 s_fr_STR_STORY_7[] = "ET UNE BALLE DE TENNIS NUCLEAIRE.";
static const u8 s_fr_STR_STORY_WIN_1[] = "RECUPERE TON SCOOTER";
static const u8 s_fr_STR_STORY_WIN_2[] = "ET RENTRE POUR MANGER!";
static const u8 s_fr_STR_PLAYER_START[] = "JOUEUR";
static const u8 s_fr_STR_LEVEL[] = "CHOISIR NIVEAU";
static const u8 s_fr_STR_CREDITS_CODE[] = "CODE ET GFX : ISSALIG";
static const u8 s_fr_STR_CREDITS_MUSIC[] = "MUSIQUE : ULTRASYD";
static const u8 s_fr_STR_CREDITS_POWERED[] = "PROPULSE PAR CPCTELERA";
static const u8 s_fr_STR_GO_TO_THE_DOOR[] = "L'HEURE DE FILER !";

const u8* const lang_fr[STR_COUNT] = {
    s_fr_STR_GAME_MODE, s_fr_STR_1_PLAYER, s_fr_STR_2_PLAYERS, s_fr_STR_PRESS_H_HELP, s_fr_STR_CONTROLS, s_fr_STR_KEY_LEFT, s_fr_STR_MOVE_LEFT, s_fr_STR_KEY_RIGHT, s_fr_STR_MOVE_RIGHT,
    s_fr_STR_KEY_SPACE, s_fr_STR_FIRE, s_fr_STR_KEY_ESC, s_fr_STR_PAUSE, s_fr_STR_KEY_M, s_fr_STR_MUSIC, s_fr_STR_CAPSULE_GUIDE, s_fr_STR_CLUELESS, s_fr_STR_L_DESC, s_fr_STR_S_DESC,
    s_fr_STR_C_DESC, s_fr_STR_P_DESC, s_fr_STR_B_DESC, s_fr_STR_E_DESC, s_fr_STR_A_DESC, s_fr_STR_M_DESC, s_fr_STR_D_DESC, s_fr_STR_V_DESC, s_fr_STR_T_DESC, s_fr_STR_G_DESC,
    s_fr_STR_I_DESC, s_fr_STR_U_DESC, s_fr_STR_F_DESC, s_fr_STR_CONGRATS, s_fr_STR_RECOVERED_MOTO, s_fr_STR_NOW_GO_BREAD,
    s_fr_STR_NOW_GO_BREAD_2, s_fr_STR_EAT_SANDWICH, s_fr_STR_SQUID_BOCATA, s_fr_STR_GAME_OVER, s_fr_STR_NO_MOTO_NO_EAT, s_fr_STR_STORY_1, s_fr_STR_STORY_2, s_fr_STR_STORY_3, s_fr_STR_STORY_4, s_fr_STR_STORY_5,
    s_fr_STR_STORY_6, s_fr_STR_STORY_7, s_fr_STR_STORY_WIN_1, s_fr_STR_STORY_WIN_2, s_fr_STR_PLAYER_START, s_fr_STR_LEVEL,
    s_fr_STR_CREDITS_CODE, s_fr_STR_CREDITS_MUSIC, s_fr_STR_CREDITS_POWERED, s_fr_STR_GO_TO_THE_DOOR
};
#endif

