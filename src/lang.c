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
#elif defined(LANG_VA)
#define DEFAULT_LANG lang_va
#else
#define DEFAULT_LANG lang_es
#endif

const u8** const lang_strings[5] = {
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
#ifdef LANG_VA
    lang_va,
#else
    DEFAULT_LANG,
#endif
};
