#include "lang.h"

u8 current_lang = 0;

#ifdef LANG_ES
#define DEFAULT_LANG lang_es
#elif defined(LANG_EN)
#define DEFAULT_LANG lang_en
#elif defined(LANG_FR)
#define DEFAULT_LANG lang_fr
#elif defined(LANG_GR)
#define DEFAULT_LANG lang_gr
#endif

const u8** const lang_strings[4] = {
#ifdef LANG_ES
    lang_es,
#else
    DEFAULT_LANG,
#endif
#ifdef LANG_EN
    lang_en,
#else
    DEFAULT_LANG,
#endif
#ifdef LANG_FR
    lang_fr,
#else
    DEFAULT_LANG,
#endif
#ifdef LANG_GR
    lang_gr,
#else
    DEFAULT_LANG,
#endif
};
