/** @file util.c
 *  @brief Various helper functions
 *
 * Copyright (c) 2014 Bartek Fabiszewski
 * http://www.fabiszewski.net
 *
 * This file is part of libmobi.
 * Licensed under LGPL, either version 3, or any later.
 * See <http://www.gnu.org/licenses/>
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "util.h"
#include "parse_rawml.h"
#include "index.h"
#include "debug.h"

#ifdef USE_LIBXML2
#include "opf.h"
#endif

/** @brief Lookup table for cp1252 to utf8 encoding conversion */
static const unsigned char cp1252_to_utf8[32][3] = {
    {0xe2,0x82,0xac},
    {0},
    {0xe2,0x80,0x9a},
    {0xc6,0x92,0},
    {0xe2,0x80,0x9e},
    {0xe2,0x80,0xa6},
    {0xe2,0x80,0xa0},
    {0xe2,0x80,0xa1},
    {0xcb,0x86,0},
    {0xe2,0x80,0xb0},
    {0xc5,0xa0,0},
    {0xe2,0x80,0xb9},
    {0xc5,0x92,0},
    {0},
    {0xc5,0xbd,0},
    {0},
    {0},
    {0xe2,0x80,0x98},
    {0xe2,0x80,0x99},
    {0xe2,0x80,0x9c},
    {0xe2,0x80,0x9d},
    {0xe2,0x80,0xa2},
    {0xe2,0x80,0x93},
    {0xe2,0x80,0x94},
    {0xcb,0x9c,0},
    {0xe2,0x84,0xa2},
    {0xc5,0xa1,0},
    {0xe2,0x80,0xba},
    {0xc5,0x93,0},
    {0},
    {0xc5,0xbe,0},
    {0xc5,0xb8,0},
};

/**
 @brief Get libmobi version

 @return String version
 */
const char * mobi_version(void) {
#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION "0.1"
#endif
    return PACKAGE_VERSION;
}

/**
 @brief Convert cp1252 encoded string to utf-8
 
 Maximum length of output string is 3 * (input string length) + 1
 
 @param[in,out] output Output string
 @param[in,out] input Input string
 @param[in,out] outsize Size of the allocated output buffer, will be set to output string length on return
 @param[in] insize Length of the input string.
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_cp1252_to_utf8(char *output, const char *input, size_t *outsize, const size_t insize) {
    if (!output || !input) {
        return MOBI_PARAM_ERR;
    }
    const unsigned char *in = (unsigned char *) input;
    unsigned char *out = (unsigned char *) output;
    const unsigned char *outend = out + *outsize;
    const unsigned char *inend = in + insize;
    while (*in && in < inend && out < outend) {
        if (*in < 0x80) {
           *out++ = *in++;
        }
        else if (*in < 0xa0) {
            /* table lookup */
            size_t i = 0;
            while (i < 3) {
                unsigned char c = cp1252_to_utf8[*in - 0x80][i];
                if (c == 0) {
                    break;
                }
                *out++ = c;
                i++;
            }
            if (i == 0) {
                /* unassigned character in input */
                return MOBI_DATA_CORRUPT;
            }
            in++;
        }
        else if (*in < 0xc0) {
            *out++ = 0xc2;
            *out++ = *in++;
        }
        else {
            *out++ = 0xc3;
            *out++ = (*in++ & 0x3f) + 0x80;
        }
    }
    *out = '\0';
    *outsize = (size_t) (out - (unsigned char *) output);
    return MOBI_SUCCESS;
}

/** @brief Get text encoding of mobi document
 
 @param[in] m MOBIData structure holding document data and metadata
 @return MOBIEncoding text encoding (MOBI_UTF8 or MOBI_CP1252
 */
MOBIEncoding mobi_get_encoding(const MOBIData *m) {
    if (m && m->mh) {
        if (m->mh->text_encoding) {
            if (*m->mh->text_encoding == MOBI_UTF8) {
                return MOBI_UTF8;
            }
        }
    }
    return MOBI_CP1252;
}

/** @brief Check if document's text is cp1252 encoded
 
 @param[in] m MOBIData structure holding document data and metadata
 @return True or false
 */
bool mobi_is_cp1252(const MOBIData *m) {
    return (mobi_get_encoding(m) == MOBI_CP1252);
}

/**
 @brief strdup replacement
 
 Returned pointer must be freed by caller
 
 @param[in] s Input string
 @return Duplicated string
 */
char * mobi_strdup(const char *s) {
    char *p = malloc(strlen(s) + 1);
    if (p) { strcpy(p, s); }
    return p;
}

#define MOBI_LANG_MAX 99 /**< number of entries in mobi_locale array */
#define MOBI_REGION_MAX 21 /**< maximum number of entries in each language array */

/**< @brief Table of Mobipocket language-region codes
 
 Based on IANA language-subtag registry with some custom Mobipocket modifications.
 http://www.iana.org/assignments/language-subtag-registry/language-subtag-registry
 */
static const char *mobi_locale[MOBI_LANG_MAX][MOBI_REGION_MAX] = {
    {"neutral"},
    {
    "ar", /**< Arabic >*/
    "ar-sa", /**< Arabic (Saudi Arabia) >*/
    "ar", /**< Arabic (Unknown) */
    "ar-eg", /**< Arabic (Egypt) >*/
    "ar", /**< Arabic (Unknown) */
    "ar-dz", /**< Arabic (Algeria) >*/
    "ar-ma", /**< Arabic (Morocco) >*/
    "ar-tn", /**< Arabic (Tunisia) >*/
    "ar-om", /**< Arabic (Oman) >*/
    "ar-ye", /**< Arabic (Yemen) >*/
    "ar-sy", /**< Arabic (Syria) >*/
    "ar-jo", /**< Arabic (Jordan) >*/
    "ar-lb", /**< Arabic (Lebanon) >*/
    "ar-kw", /**< Arabic (Kuwait) >*/
    "ar-ae", /**< Arabic (UAE) >*/
    "ar-bh", /**< Arabic (Bahrain) >*/
    "ar-qa", /**< Arabic (Qatar) >*/
    },
    {"bg"}, /**< Bulgarian >*/
    {"ca"}, /**< Catalan >*/
    {
    "zh", /**< Chinese >*/
    "zh-tw", /**< Chinese (Taiwan) >*/
    "zh-cn", /**< Chinese (PRC) >*/
    "zh-hk", /**< Chinese (Hong Kong) >*/
    "zh-sg", /**< Chinese (Singapore) >*/
    },
    {"cs"}, /**< Czech >*/
    {"da"}, /**< Danish >*/
    {
    "de", /**< German >*/
    "de-de", /**< German (Germany) >*/
    "de-ch", /**< German (Switzerland) >*/
    "de-at", /**< German (Austria) >*/
    "de-lu", /**< German (Luxembourg) >*/
    "de-li", /**< German (Liechtenstein) >*/
    },
    {"el"}, /**< Greek (modern) >*/
    {
    "en", /**< English >*/
    "en-us", /**< English (United States) >*/
    "en-gb", /**< English (United Kingdom) >*/
    "en-au", /**< English (Australia) >*/
    "en-ca", /**< English (Canada) >*/
    "en-nz", /**< English (New Zealand) >*/
    "en-ie", /**< English (Ireland) >*/
    "en-za", /**< English (South Africa) >*/
    "en-jm", /**< English (Jamaica) >*/
    "en", /**< English (Unknown) >*/
    "en-bz", /**< English (Belize) >*/
    "en-tt", /**< English (Trinidad) >*/
    "en-zw", /**< English (Zimbabwe) >*/
    "en-ph", /**< English (Philippines) >*/
    },
    {
    "es", /**< Spanish >*/
    "es-es", /**< Spanish (Spain) >*/
    "es-mx", /**< Spanish (Mexico) >*/
    "es", /**< Spanish (Unknown) >*/
    "es-gt", /**< Spanish (Guatemala) >*/
    "es-cr", /**< Spanish (Costa Rica) >*/
    "es-pa", /**< Spanish (Panama) >*/
    "es-do", /**< Spanish (Dominican Republic) >*/
    "es-ve", /**< Spanish (Venezuela) >*/
    "es-co", /**< Spanish (Colombia) >*/
    "es-pe", /**< Spanish (Peru) >*/
    "es-ar", /**< Spanish (Argentina) >*/
    "es-ec", /**< Spanish (Ecuador) >*/
    "es-cl", /**< Spanish (Chile) >*/
    "es-uy", /**< Spanish (Uruguay) >*/
    "es-py", /**< Spanish (Paraguay) >*/
    "es-bo", /**< Spanish (Bolivia) >*/
    "es-sv", /**< Spanish (El Salvador) >*/
    "es-hn", /**< Spanish (Honduras) >*/
    "es-ni", /**< Spanish (Nicaragua) >*/
    "es-pr", /**< Spanish (Puerto Rico) >*/
    },
    {"fi"}, /**< Finnish >*/
    {
    "fr", /**< French >*/
    "fr-fr", /**< French (France) >*/
    "fr-be", /**< French (Belgium) >*/
    "fr-ca", /**< French (Canada) >*/
    "fr-ch", /**< French (Switzerland) >*/
    "fr-lu", /**< French (Luxembourg) >*/
    "fr-mc", /**< French (Monaco) >*/
    },
    {"he"}, /**< Hebrew (also code iw) >*/
    {"hu"}, /**< Hungarian >*/
    {"is"}, /**< Icelandic >*/
    {
    "it", /**< Italian >*/
    "it-it", /**< Italian (Italy) >*/
    "it-ch", /**< Italian (Switzerland) >*/
    },
    {"ja"}, /**< Japanese >*/
    {"ko"}, /**< Korean >*/
    {
    "nl", /**< Dutch / Flemish >*/
    "nl-nl", /**< Dutch (Netherlands) >*/
    "nl-be", /**< Dutch (Belgium) >*/
    },
    {"no"}, /**< Norwegian >*/
    {"pl"}, /**< Polish >*/
    {
    "pt", /**< Portuguese >*/
    "pt-br", /**< Portuguese (Brazil) >*/
    "pt-pt", /**< Portuguese (Portugal) >*/
    },
    {"rm"}, /**< Romansh >*/
    {"ro"}, /**< Romanian >*/
    {"ru"}, /**< Russian >*/
    {"hr"}, /**< Croatian >*/
    {
    "sr", /**< Serbian >*/
    "sr", /**< Serbian (Unknown) >*/
    "sr", /**< Serbian (Unknown) >*/
    "sr", /**< Serbian (Serbia) >*/
    },
    {"sk"}, /**< Slovak >*/
    {"sq"}, /**< Albanian >*/
    {
    "sv", /**< Swedish >*/
    "sv-se", /**< Swedish (Sweden) >*/
    "sv-fi", /**< Swedish (Finland) >*/
    },
    {"th"}, /**< Thai >*/
    {"tr"}, /**< Turkish >*/
    {"ur"}, /**< Urdu >*/
    {"id"}, /**< Indonesian >*/
    {"uk"}, /**< Ukrainian >*/
    {"be"}, /**< Belarusian >*/
    {"sl"}, /**< Slovenian >*/
    {"et"}, /**< Estonian >*/
    {"lv"}, /**< Latvian >*/
    {"lt"}, /**< Lithuanian >*/
    [41] = {"fa"}, /**< Farsi / Persian >*/
    {"vi"}, /**< Vietnamese >*/
    {"hy"}, /**< Armenian >*/
    {"az"}, /**< Azerbaijani >*/
    {"eu"}, /**< Basque >*/
    {"sb"}, /**< "Sorbian" >*/
    {"mk"}, /**< Macedonian >*/
    {"sx"}, /**< "Sutu" >*/
    {"ts"}, /**< Tsonga >*/
    {"tn"}, /**< Tswana >*/
    [52] = {"xh"}, /**< Xhosa >*/
    {"zu"}, /**< Zulu >*/
    {"af"}, /**< Afrikaans >*/
    {"ka"}, /**< Georgian >*/
    {"fo"}, /**< Faroese >*/
    {"hi"}, /**< Hindi >*/
    {"mt"}, /**< Maltese >*/
    {"sz"}, /**<"Sami (Lappish)" >*/
    {"ga"}, /**< Irish */
    [62] = {"ms"}, /**< Malay >*/
    {"kk"}, /**< Kazakh >*/
    [65] = {"sw"}, /**< Swahili >*/
    [67] = {
    "uz", /**< Uzbek >*/
    "uz", /**< Uzbek (Unknown) >*/
    "uz-uz", /**< Uzbek (Uzbekistan) >*/
    },
    {"tt"}, /**< Tatar >*/
    {"bn"}, /**< Bengali >*/
    {"pa"}, /**< Punjabi >*/
    {"gu"}, /**< Gujarati >*/
    {"or"}, /**< Oriya >*/
    {"ta"}, /**< Tamil >*/
    {"te"}, /**< Telugu >*/
    {"kn"}, /**< Kannada >*/
    {"ml"}, /**< Malayalam >*/
    {"as"}, /**< Assamese (not accepted in kindlegen >*/
    {"mr"}, /**< Marathi >*/
    {"sa"}, /**< Sanskrit >*/
    [82] = {
    "cy", /**< Welsh */
    "cy-gb" /**< Welsh (UK) */
    },
    {
    "gl", /**< Galician */
    "gl-es" /**< Galician (Spain) */
    },
    [87] = {"x-kok"}, /**< Konkani (real language code is kok) >*/
    [97] = {"ne"}, /**< Nepali >*/
    {"fy"}, /**< Northern Frysian >*/
};

/**
 @brief Get pointer to locale tag for a given Mobipocket locale number
 
 Locale strings are based on IANA language-subtag registry with some custom Mobipocket modifications.
 See mobi_locale array.
 
 @param[in] locale_number Mobipocket locale number (as stored in MOBI header)
 @return Pointer to locale string in mobi_locale array
 */
const char * mobi_get_locale_string(const uint32_t locale_number) {
    uint8_t lang_code = locale_number & 0xffu;
    uint32_t region_code = (locale_number >> 8) / 4;
    if (lang_code >= MOBI_LANG_MAX || region_code >= MOBI_REGION_MAX) {
        return NULL;
    }
    const char *string = mobi_locale[lang_code][region_code];
    if (string == NULL || strlen(string) == 0 ) {
        return NULL;
    }
    return string;
}

/**
 @brief Get Mobipocket locale number for a given string tag
 
 Locale strings are based on IANA language-subtag registry with some custom Mobipocket modifications. 
 See mobi_locale array.
 
 @param[in] locale_string Locale string tag
 @return Mobipocket locale number
 */
size_t mobi_get_locale_number(const char *locale_string) {
    if (locale_string == NULL || strlen(locale_string) < 2) {
        return 0;
    }
    size_t lang_code = 0;
    while (lang_code < MOBI_LANG_MAX) {
        if (mobi_locale[lang_code][0] == NULL) {
            lang_code++;
            continue;
        }
        char lower_locale[strlen(locale_string) + 1];
        int i = 0;
        while (locale_string[i]) {
            lower_locale[i] = (char) tolower(locale_string[i]);
            i++;
        }
        lower_locale[i] = '\0';
        if (strncmp(lower_locale, mobi_locale[lang_code][0], 2) == 0) {
            size_t region_code = 0;
            while (region_code < MOBI_REGION_MAX) {
                if (strcmp(lower_locale, mobi_locale[lang_code][region_code]) == 0) {
                    return (region_code * 4) << 8 | lang_code;
                }
                region_code++;
            }
            return lang_code;
        }
        lang_code++;
    }
    return 0;
}

/**
 @brief Array of known file types, their extensions and mime-types.
 */
const MOBIFileMeta mobi_file_meta[] = {
    {T_HTML, "html", "application/xhtml+xml"},
    {T_CSS, "css", "text/css"},
    {T_SVG, "svg", "image/svg+xml"},
    {T_JPG, "jpg", "image/jpeg"},
    {T_GIF, "gif", "image/gif"},
    {T_PNG, "png", "image/png"},
    {T_BMP, "bmp", "image/bmp"},
    {T_OTF, "otf", "application/vnd.ms-opentype"},
    {T_TTF, "ttf", "application/x-font-truetype"},
    {T_MP3, "mp3", "audio/mpeg"},
    {T_MPG, "mpg", "video/mpeg"},
    {T_PDF, "pdf", "application/pdf"},
    {T_OPF, "opf", "application/oebps-package+xml"},
    {T_NCX, "ncx", "application/x-dtbncx+xml"},
    /* termination struct */
    {T_UNKNOWN, "dat", "application/unknown"}
};

/**
 @brief Get MOBIFileMeta tag structure by MOBIFiletype type
 
 @param[in] type MOBIFiletype type
 @return MOBIExthMeta structure for given type, .type = T_UNKNOWN on failure
 */
MOBIFileMeta mobi_get_filemeta_by_type(const MOBIFiletype type) {
    size_t i = 0;
    while (mobi_file_meta[i].type != T_UNKNOWN) {
        if (mobi_file_meta[i].type == type) {
            return mobi_file_meta[i];
        }
        i++;
    }
    return mobi_file_meta[i];
}

/**
 @brief Get ebook full name stored in Record 0 at offset given in MOBI header
 
 @param[in] m MOBIData structure with loaded data
 @param[in,out] fullname Memory area to be filled with zero terminated full name string
 @param[in] len Length of memory area allocated for the string
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_get_fullname(const MOBIData *m, char *fullname, const size_t len) {
    if (fullname == NULL || len == 0) {
        return MOBI_PARAM_ERR;
    }
    fullname[0] = '\0';
    if (m == NULL) {
        debug_print("%s", "Mobi structure not initialized\n");
        return MOBI_INIT_FAILED;
    }
    const size_t offset = mobi_get_kf8offset(m);
    MOBIPdbRecord *record0 = mobi_get_record_by_seqnumber(m, offset);
    if (m->mh == NULL ||
        m->mh->full_name_offset == NULL ||
        m->mh->full_name_length == NULL ||
        record0 == NULL) {
        return MOBI_INIT_FAILED;
    }
    size_t size = min(len, *m->mh->full_name_length);
    memcpy(fullname, record0->data + *m->mh->full_name_offset, size);
    fullname[size] = '\0';
    return MOBI_SUCCESS;
}

/**
 @brief Get palm database record with given unique id
 
 @param[in] m MOBIData structure with loaded data
 @param[in] uid Unique id
 @return Pointer to MOBIPdbRecord record structure, NULL on failure
 */
MOBIPdbRecord * mobi_get_record_by_uid(const MOBIData *m, const size_t uid) {
    if (m == NULL) {
        debug_print("%s", "Mobi structure not initialized\n");
        return NULL;
    }
    if (m->rec == NULL) {
        return NULL;
    }
    MOBIPdbRecord *curr = m->rec;
    while (curr != NULL) {
        if (curr->uid == uid) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

/**
 @brief Get rawml->markup MOBIPart part by uid
 
 @param[in] rawml MOBIRawml structure with loaded data
 @param[in] uid Unique id
 @return Pointer to MOBIPart structure, NULL on failure
 */
MOBIPart * mobi_get_part_by_uid(const MOBIRawml *rawml, const size_t uid) {
    if (rawml == NULL) {
        debug_print("%s", "Mobi structure not initialized\n");
        return NULL;
    }
    if (rawml->markup == NULL) {
        return NULL;
    }
    MOBIPart *part = rawml->markup;
    while (part != NULL) {
        if (part->uid == uid) {
            return part;
        }
        part = part->next;
    }
    return NULL;
}

/**
 @brief Get rawml->flow MOBIPart part by uid
 
 @param[in] rawml MOBIRawml structure with loaded data
 @param[in] uid Unique id
 @return Pointer to MOBIPart structure, NULL on failure
 */
MOBIPart * mobi_get_flow_by_uid(const MOBIRawml *rawml, const size_t uid) {
    if (rawml == NULL) {
        debug_print("%s", "Mobi structure not initialized\n");
        return NULL;
    }
    if (rawml->flow == NULL) {
        return NULL;
    }
    MOBIPart *part = rawml->flow;
    while (part != NULL) {
        if (part->uid == uid) {
            return part;
        }
        part = part->next;
    }
    return NULL;
}

/**
 @brief Get MOBIPart resource record with given unique id
 
 @param[in] rawml MOBIRawml structure with loaded data
 @param[in] uid Unique id
 @return Pointer to MOBIPart resource structure, NULL on failure
 */
MOBIPart * mobi_get_resource_by_uid(const MOBIRawml *rawml, const size_t uid) {
    if (rawml == NULL) {
        debug_print("%s", "Rawml structure not initialized\n");
        return NULL;
    }
    if (rawml->resources == NULL) {
        debug_print("%s", "Rawml structure not initialized\n");
        return NULL;
    }
    MOBIPart *curr = rawml->resources;
    while (curr != NULL) {
        if (curr->uid == uid) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

/**
 @brief Get MOBIFiletype type of MOBIPart resource record with given unique id
 
 @param[in] rawml MOBIRawml structure with loaded data
 @param[in] uid Unique id
 @return Pointer to MOBIPart resource structure, NULL on failure
 */
MOBIFiletype mobi_get_resourcetype_by_uid(const MOBIRawml *rawml, const size_t uid) {
    if (rawml == NULL) {
        debug_print("%s", "Rawml structure not initialized\n");
        return T_UNKNOWN;
    }
    if (rawml->resources == NULL) {
        debug_print("%s", "Rawml structure not initialized\n");
        return T_UNKNOWN;
    }
    MOBIPart *curr = rawml->resources;
    while (curr != NULL) {
        if (curr->uid == uid) {
            return curr->type;
        }
        curr = curr->next;
    }
    return T_UNKNOWN;
}

/**
 @brief Get palm database record with given sequential number (first record has number 0)
 
 @param[in] m MOBIData structure with loaded data
 @param[in] num Sequential number
 @return Pointer to MOBIPdbRecord record structure, NULL on failure
 */
MOBIPdbRecord * mobi_get_record_by_seqnumber(const MOBIData *m, const size_t num) {
    if (m == NULL) {
        debug_print("%s", "Mobi structure not initialized\n");
        return NULL;
    }
    if (m->rec == NULL) {
        return NULL;
    }
    MOBIPdbRecord *curr = m->rec;
    size_t i = 0;
    while (curr != NULL) {
        if (i++ == num) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

/**
 @brief Delete palm database record with given sequential number from MOBIData structure
 
 @param[in,out] m MOBIData structure with loaded data
 @param[in] num Sequential number
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_delete_record_by_seqnumber(MOBIData *m, const size_t num) {
    if (m == NULL) {
        debug_print("%s", "Mobi structure not initialized\n");
        return MOBI_INIT_FAILED;
    }
    if (m->rec == NULL) {
        return MOBI_INIT_FAILED;
    }
    size_t i = 0;
    MOBIPdbRecord *curr = m->rec;
    MOBIPdbRecord *prev = NULL;
    while (curr != NULL) {
        if (i++ == num) {
            if (prev == NULL) {
                m->rec = curr->next;
            } else {
                prev->next = curr->next;
            }
            free(curr->data);
            curr->data = NULL;
            free(curr);
            curr = NULL;
            return MOBI_SUCCESS;
        }
        prev = curr;
        curr = curr->next;
    }
    return MOBI_SUCCESS;
}

/**
 @brief Get EXTH record with given MOBIExthTag tag
 
 @param[in] m MOBIData structure with loaded data
 @param[in] tag MOBIExthTag EXTH record tag
 @return Pointer to MOBIExthHeader record structure
 */
MOBIExthHeader * mobi_get_exthrecord_by_tag(const MOBIData *m, const MOBIExthTag tag) {
    if (m == NULL) {
        debug_print("%s", "Mobi structure not initialized\n");
        return NULL;
    }
    if (m->eh == NULL) {
        return NULL;
    }
    MOBIExthHeader *curr = m->eh;
    while (curr != NULL) {
        if (curr->tag == tag) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

/**
 @brief Array of known EXTH tags.
 Name strings shamelessly copied from KindleUnpack
 */
const MOBIExthMeta mobi_exth_tags[] = {
    /* numeric */
    {EXTH_SAMPLE, EXTH_NUMERIC, "Sample"},
    {EXTH_STARTREADING, EXTH_NUMERIC, "Start offset"},
    {EXTH_KF8BOUNDARY, EXTH_NUMERIC, "K8 boundary offset"},
    {EXTH_COUNTRESOURCES, EXTH_NUMERIC, "K8 count of resources, fonts, images"},
    {EXTH_RESCOFFSET, EXTH_NUMERIC, "RESC offset"},
    {EXTH_COVEROFFSET, EXTH_NUMERIC, "Cover offset"},
    {EXTH_THUMBOFFSET, EXTH_NUMERIC, "Thumbnail offset"},
    {EXTH_HASFAKECOVER, EXTH_NUMERIC, "Has fake cover"},
    {EXTH_CREATORSOFT, EXTH_NUMERIC, "Creator software"},
    {EXTH_CREATORMAJOR, EXTH_NUMERIC, "Creator major version"},
    {EXTH_CREATORMINOR, EXTH_NUMERIC, "Creator minor version"},
    {EXTH_CREATORBUILD, EXTH_NUMERIC, "Creator build number"},
    {EXTH_CLIPPINGLIMIT, EXTH_NUMERIC, "Clipping limit"},
    {EXTH_PUBLISHERLIMIT, EXTH_NUMERIC, "Publisher limit"},
    {EXTH_TTSDISABLE, EXTH_NUMERIC, "Text to Speech disabled"},
    {EXTH_RENTAL, EXTH_NUMERIC, "Rental indicator"},
    /* strings */
    {EXTH_DRMSERVER, EXTH_STRING, "Drm server id"},
    {EXTH_DRMCOMMERCE, EXTH_STRING, "Drm commerce id"},
    {EXTH_DRMEBOOKBASE, EXTH_STRING, "Drm Ebookbase book id"},
    {EXTH_TITLE, EXTH_STRING, "Title"},
    {EXTH_AUTHOR, EXTH_STRING, "Creator"},
    {EXTH_PUBLISHER, EXTH_STRING, "Publisher"},
    {EXTH_IMPRINT, EXTH_STRING, "Imprint"},
    {EXTH_DESCRIPTION, EXTH_STRING, "Description"},
    {EXTH_ISBN, EXTH_STRING, "ISBN"},
    {EXTH_SUBJECT, EXTH_STRING, "Subject"},
    {EXTH_PUBLISHINGDATE, EXTH_STRING, "Published"},
    {EXTH_REVIEW, EXTH_STRING, "Review"},
    {EXTH_CONTRIBUTOR, EXTH_STRING, "Contributor"},
    {EXTH_RIGHTS, EXTH_STRING, "Rights"},
    {EXTH_SUBJECTCODE, EXTH_STRING, "Subject code"},
    {EXTH_TYPE, EXTH_STRING, "Type"},
    {EXTH_SOURCE, EXTH_STRING, "Source"},
    {EXTH_ASIN, EXTH_STRING, "ASIN"},
    {EXTH_VERSION, EXTH_STRING, "Version number"},
    {EXTH_ADULT, EXTH_STRING, "Adult"},
    {EXTH_PRICE, EXTH_STRING, "Price"},
    {EXTH_CURRENCY, EXTH_STRING, "Currency"},
    {EXTH_FIXEDLAYOUT, EXTH_STRING, "Fixed layout"},
    {EXTH_BOOKTYPE, EXTH_STRING, "Book type"},
    {EXTH_ORIENTATIONLOCK, EXTH_STRING, "Orientation lock"},
    {EXTH_ORIGRESOLUTION, EXTH_STRING, "Original resolution"},
    {EXTH_ZEROGUTTER, EXTH_STRING, "Zero gutter"},
    {EXTH_ZEROMARGIN, EXTH_STRING, "Zero margin"},
    {EXTH_KF8COVERURI, EXTH_STRING, "K8 masthead/cover image"},
    {EXTH_REGIONMAGNI, EXTH_STRING, "Region magnification"},
    {EXTH_DICTNAME, EXTH_STRING, "Dictionary short name"},
    {EXTH_WATERMARK, EXTH_STRING, "Watermark"},
    {EXTH_DOCTYPE, EXTH_STRING, "Document type"},
    {EXTH_LASTUPDATE, EXTH_STRING, "Last update time"},
    {EXTH_UPDATEDTITLE, EXTH_STRING, "Updated title"},
    {EXTH_ASIN504, EXTH_STRING, "ASIN (504)"},
    {EXTH_TITLEFILEAS, EXTH_STRING, "Title file as"},
    {EXTH_CREATORFILEAS, EXTH_STRING, "Creator file as"},
    {EXTH_PUBLISHERFILEAS, EXTH_STRING, "Publisher file as"},
    {EXTH_LANGUAGE, EXTH_STRING, "Language"},
    {EXTH_ALIGNMENT, EXTH_STRING, "Primary writing mode"},
    {EXTH_PAGEDIR, EXTH_STRING, "Page progression direction"},
    {EXTH_OVERRIDEFONTS, EXTH_STRING, "Override Kindle fonts"},
    {EXTH_SORCEDESC, EXTH_STRING, "Original source description"},
    {EXTH_UNK534, EXTH_STRING, "Unknown (534)"},
    {EXTH_CREATORBUILDREV, EXTH_STRING, "Kindlegen BuildRev number"},
    /* binary */
    {EXTH_TAMPERKEYS, EXTH_BINARY, "Tamper proof keys"},
    {EXTH_FONTSIGNATURE, EXTH_BINARY, "Font signature"},
    {EXTH_UNK403, EXTH_BINARY, "Unknown (403)"},
    {EXTH_UNK405, EXTH_BINARY, "Unknown (405)"},
    {EXTH_UNK407, EXTH_BINARY, "Unknown (407)"},
    {EXTH_UNK450, EXTH_BINARY, "Unknown (450)"},
    {EXTH_UNK451, EXTH_BINARY, "Unknown (451)"},
    {EXTH_UNK452, EXTH_BINARY, "Unknown (452)"},
    {EXTH_UNK453, EXTH_BINARY, "Unknown (453)"},
    /* end */
    {0, 0, NULL},
};

/**
 @brief Get MOBIExthMeta tag structure by MOBIExthTag tag id
 
 @param[in] tag Tag id
 @return MOBIExthMeta structure for given tag id, zeroed structure on failure
 */
MOBIExthMeta mobi_get_exthtagmeta_by_tag(const MOBIExthTag tag) {
    size_t i = 0;
    while (mobi_exth_tags[i].tag > 0) {
        if (mobi_exth_tags[i].tag == tag) {
            return mobi_exth_tags[i];
        }
        i++;
    }
    return (MOBIExthMeta) {0, 0, NULL};
}

/**
 @brief Decode big-endian value stored in EXTH record
 
 Only for EXTH records storing numeric values
 
 @param[in] data Memory area storing EXTH record data
 @param[in] size Size of EXTH record data
 @return 32-bit value
 */
uint32_t mobi_decode_exthvalue(const unsigned char *data, const size_t size) {
    /* FIXME: EXTH numeric data is max 32-bit? */
    uint32_t val = 0;
    size_t i = min(size, 4);
    while (i--) {
        val |= (uint32_t) *data++ << (i * 8);
    }
    return val;
}

/**
 @brief Decode string stored in EXTH record
 
 Only for EXTH records storing string values
 
 @param[in] m MOBIData structure loaded with MOBI data
 @param[in] data Memory area storing EXTH record data
 @param[in] size Size of EXTH record data
 @return String from EXTH record in utf-8 encoding
 */
char * mobi_decode_exthstring(const MOBIData *m, const unsigned char *data, const size_t size) {
    if (!m || !data) {
        return NULL;
    }
    size_t out_length = 3 * size + 1;
    size_t in_length = size;
    char string[out_length];
    if (mobi_is_cp1252(m)) {
        MOBI_RET ret = mobi_cp1252_to_utf8(string, (const char *) data, &out_length, in_length);
        if (ret != MOBI_SUCCESS) {
            return NULL;
        }
    } else {
        memcpy(string, data, size);
        out_length = size;
    }
    string[out_length] = '\0';
    char *exth_string = strdup(string);
    return exth_string;
}

/**
 @brief Convert time values from palmdoc header to time tm struct
 
 Older files set time in mac format. Newer ones in unix time.
 
 @param[in] pdb_time Time value from PDB header
 @return Time structure struct tm of time.h
 */
struct tm * mobi_pdbtime_to_time(const long pdb_time) {
    time_t time = pdb_time;
    const uint32_t mactime_flag = (uint32_t) (1 << 31);
    if (time & mactime_flag) {
        printf("MAC TIME\n");
        time += EPOCH_MAC_DIFF;
    }
    return localtime(&time);
}

/**
 @brief Lookup table for number of bits set in a single byte
 */
static const char setbits[256] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
};

/**
 @brief Get number of bits set in a given byte
 
 @param[in] byte A byte
 @return Number of bits set
 */
int mobi_bitcount(const uint8_t byte) {
    return setbits[byte];
}

/**
 @brief Decompress text record (internal).
 
 Internal function for mobi_get_rawml and mobi_dump_rawml. 
 Decompressed output is stored either in a file or in a text string
 
 @param[in] m MOBIData structure loaded with MOBI data
 @param[in,out] text Memory area to be filled with decompressed output
 @param[in,out] file If not NULL output is written to the file, otherwise to text string
 @param[in,out] len Length of the memory allocated for the text string, on return set to decompressed text length
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
static MOBI_RET mobi_decompress_content(const MOBIData *m, char *text, FILE *file, size_t *len) {
    if (mobi_is_encrypted(m)) {
        debug_print("%s", "Document is encrypted\n");
        return MOBI_FILE_ENCRYPTED;
    }
    int dump = false;
    if (file != NULL) {
        dump = true;
    }
    if (m == NULL) {
        debug_print("%s", "Mobi structure not initialized\n");
        return MOBI_INIT_FAILED;
    }
    const size_t offset = mobi_get_kf8offset(m);
    if (m->rh == NULL || m->rh->text_record_count == 0) {
        debug_print("%s", "Text records not found in MOBI header\n");
        return MOBI_DATA_CORRUPT;
    }
    const size_t text_rec_index = 1 + offset;
    size_t text_rec_count = m->rh->text_record_count;
    const uint16_t compression_type = m->rh->compression_type;
    /* check for extra data at the end of text files */
    uint16_t extra_flags = 0;
    if (m->mh && m->mh->extra_flags) {
        extra_flags = *m->mh->extra_flags;
    }
    /* get first text record */
    const MOBIPdbRecord *curr = mobi_get_record_by_seqnumber(m, text_rec_index);
    MOBIHuffCdic *huffcdic = NULL;
    if (compression_type == RECORD0_HUFF_COMPRESSION) {
        /* load huff/cdic tables */
        huffcdic = mobi_init_huffcdic();
        if (huffcdic == NULL) {
            return MOBI_MALLOC_FAILED;
        }
        MOBI_RET ret = mobi_parse_huffdic(m, huffcdic);
        if (ret != MOBI_SUCCESS) {
            free(huffcdic);
            return ret;
        }
    }
    /* get following CDIC records */
    size_t text_length = 0;
    while (text_rec_count-- && curr) {
        size_t extra_size = 0;
        if (extra_flags) {
            extra_size = mobi_get_record_extrasize(curr, extra_flags);
            if (extra_size == MOBI_NOTSET || extra_size >= curr->size) {
                return MOBI_DATA_CORRUPT;
            }
        }
        const size_t record_size = curr->size - extra_size;
        unsigned char decompressed[RECORD0_TEXT_SIZE_MAX];
        /* FIXME: RECORD0_TEXT_SIZE_MAX should be enough */
        size_t decompressed_size = RECORD0_TEXT_SIZE_MAX;
        switch (compression_type) {
            case RECORD0_NO_COMPRESSION:
                /* no compression */
                memcpy(decompressed, curr->data, curr->size);
                decompressed_size = curr->size;
                break;
            case RECORD0_PALMDOC_COMPRESSION:
                /* palmdoc lz77 compression */
                mobi_decompress_lz77(decompressed, curr->data, &decompressed_size, record_size);
                break;
            case RECORD0_HUFF_COMPRESSION:
                /* mobi huffman compression */
                mobi_decompress_huffman(decompressed, curr->data, &decompressed_size, record_size, huffcdic);
                break;
            default:
                debug_print("%s", "Unknown compression type\n");
                return MOBI_DATA_CORRUPT;
        }
        curr = curr->next;
        if (dump) {
            fwrite(decompressed, 1, decompressed_size, file);
        } else {
            if (text_length > *len) {
                debug_print("%s", "Text buffer too small\n");
                /* free huff/cdic tables */
                if (compression_type == RECORD0_HUFF_COMPRESSION) {
                    mobi_free_huffcdic(huffcdic);
                }
                return MOBI_PARAM_ERR;
            }
            memcpy(text + text_length, decompressed, decompressed_size);
            text_length += decompressed_size;
            text[text_length] = '\0';
        }

    }
    /* free huff/cdic tables */
    if (compression_type == RECORD0_HUFF_COMPRESSION) {
        mobi_free_huffcdic(huffcdic);
    }
    if (len) {
        *len = text_length;
    }
    return MOBI_SUCCESS;
}

/**
 @brief Decompress text to a text buffer.
 
 @param[in] m MOBIData structure loaded with MOBI data
 @param[in,out] text Memory area to be filled with decompressed output
 @param[in,out] len Length of the memory allocated for the text string, on return will be set to decompressed text length
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_get_rawml(const MOBIData *m, char *text, size_t *len) {
    if (m->rh->text_length > *len) {
        debug_print("%s", "Text buffer smaller then text size declared in record0 header\n");
        return MOBI_PARAM_ERR;
    }
    text[0] = '\0';
    return mobi_decompress_content(m, text, NULL, len);
}

/**
 @brief Decompress text record to an open file descriptor.
 
 Internal function for mobi_get_rawml and mobi_dump_rawml.
 Decompressed output is stored either in a file or in a text string
 
 @param[in] m MOBIData structure loaded with MOBI data
 @param[in,out] file File descriptor
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_dump_rawml(const MOBIData *m, FILE *file) {
    if (file == NULL) {
        debug_print("%s", "File descriptor is NULL\n");
        return MOBI_FILE_NOT_FOUND;
    }
    return mobi_decompress_content(m, NULL, file, NULL);
}

/**
 @brief Check if MOBI header is loaded / present in the loaded file
 
 @param[in] m MOBIData structure loaded with MOBI data
 @return true on success, false otherwise
 */
bool mobi_exists_mobiheader(const MOBIData *m) {
    if (m == NULL || m->mh == NULL) {
        return false;
    }
    return true;
}

/**
 @brief Check if skeleton INDX is present in the loaded file
 
 @param[in] m MOBIData structure loaded with MOBI data
 @return true on success, false otherwise
 */
bool mobi_exists_skel_indx(const MOBIData *m) {
    if (!mobi_exists_mobiheader(m)) {
        return false;
    }
    if (m->mh->skeleton_index == NULL || *m->mh->skeleton_index == MOBI_NOTSET) {
        debug_print("%s", "SKEL INDX record not found\n");
        return false;
    }
    return true;
}

/**
 @brief Check if FDST record is present in the loaded file
 
 @param[in] m MOBIData structure loaded with MOBI data
 @return true on success, false otherwise
 */
bool mobi_exists_fdst(const MOBIData *m) {
    if (!mobi_exists_mobiheader(m)) {
        return false;
    }
    if (mobi_get_fileversion(m) >= 8) {
        if (m->mh->fdst_index && *m->mh->fdst_index != MOBI_NOTSET) {
            return true;
        }
    } else {
        if (m->mh->fdst_section_count && *m->mh->fdst_section_count > 1) {
            return true;
        }
    }
    debug_print("%s", "FDST record not found\n");
    return false;
}

/**
 @brief Get sequential number of FDST record
 
 @param[in] m MOBIData structure loaded with MOBI data
 @return Record number on success, MOBI_NOTSET otherwise
 */
size_t mobi_get_fdst_record_number(const MOBIData *m) {
    const size_t offset = mobi_get_kf8offset(m);
    if (m->mh->fdst_index && *m->mh->fdst_index != MOBI_NOTSET) {
        if (m->mh->fdst_section_count && *m->mh->fdst_section_count > 1) {
            return *m->mh->fdst_index + offset;
        }
    }
    if (m->mh->fdst_section_count && *m->mh->fdst_section_count > 1) {
        /* FIXME: if KF7, is it safe to asume last_text_index has fdst index */
        return *m->mh->last_text_index;
    }
    return MOBI_NOTSET;
}

/**
 @brief Check if fragments INDX is present in the loaded file
 
 @param[in] m MOBIData structure loaded with MOBI data
 @return true on success, false otherwise
 */
bool mobi_exists_frag_indx(const MOBIData *m) {
    if (!mobi_exists_mobiheader(m)) {
        return false;
    }
    if (m->mh->fragment_index == NULL || *m->mh->fragment_index == MOBI_NOTSET) {
        debug_print("%s", "Fragments INDX not found\n");
        return false;
    }
    return true;
}

/**
 @brief Check if guide INDX is present in the loaded file
 
 @param[in] m MOBIData structure loaded with MOBI data
 @return true on success, false otherwise
 */
bool mobi_exists_guide_indx(const MOBIData *m) {
    if (!mobi_exists_mobiheader(m)) {
        return false;
    }
    if (m->mh->guide_index == NULL || *m->mh->guide_index == MOBI_NOTSET) {
        debug_print("%s", "Guide INDX not found\n");
        return false;
    }
    return true;
}

/**
 @brief Check if ncx INDX is present in the loaded file
 
 @param[in] m MOBIData structure loaded with MOBI data
 @return true on success, false otherwise
 */
bool mobi_exists_ncx(const MOBIData *m) {
    if (!mobi_exists_mobiheader(m)) {
        return false;
    }
    if (m->mh->ncx_index == NULL || *m->mh->ncx_index == MOBI_NOTSET) {
        debug_print("%s", "NCX INDX not found\n");
        return false;
    }
    return true;
}

/**
 @brief Check if orth INDX is present in the loaded file
 
 @param[in] m MOBIData structure loaded with MOBI data
 @return true on success, false otherwise
 */
bool mobi_exists_orth(const MOBIData *m) {
    if (!mobi_exists_mobiheader(m)) {
        return false;
    }
    if (m->mh->orth_index == NULL || *m->mh->orth_index == MOBI_NOTSET) {
        debug_print("%s", "ORTH INDX not found\n");
        return false;
    }
    return true;
}

/**
 @brief Get file type of given part with number [part_number]
 
 @param[in] rawml MOBIRawml parsed records structure
 @param[in] part_number Sequential number of the part within rawml structure
 @return MOBIFiletype file type
 */
MOBIFiletype mobi_determine_flowpart_type(const MOBIRawml *rawml, const size_t part_number) {
    if (part_number == 0 || rawml->version == MOBI_NOTSET || rawml->version < 8) {
        return T_HTML;
    }
    char target[24];
    sprintf(target, "\"kindle:flow:%04zu?mime=", part_number);
    unsigned char *data_start = rawml->flow->data;
    unsigned char *data_end = data_start + rawml->flow->size;
    MOBIResult result;
    MOBI_RET ret = mobi_search_markup(&result, data_start, data_end, T_HTML, target);
    if (ret == MOBI_SUCCESS && result.start) {
        if (strstr(result.value, "text/css")) {
            return T_CSS;
        } else if (strstr(result.value, "image/svg+xml")) {
            return T_SVG;
        }
    }
    return T_UNKNOWN;
}

/**
 @brief Get font type of given font resource
 
 @param[in] font_data Font resource data
 @return MOBIFiletype file type
 */
MOBIFiletype mobi_determine_font_type(const unsigned char *font_data) {
    const char otf_magic[] = "OTTO";
    const char ttf_magic[] = "\0\1\0\0";
    const char ttf2_magic[] = "true";

    if (memcmp(font_data, otf_magic, 4) == 0) {
        return T_OTF;
    } else if (memcmp(font_data, ttf_magic, 4) == 0) {
        return T_TTF;
    } else if (memcmp(font_data, ttf2_magic, 4) == 0) {
        return T_TTF;
    }
    return T_UNKNOWN;
}

/**
 @brief Replace part data with decoded audio data
 
 @param[in,out] part MOBIPart structure containing font resource, decoded part type will be set in the structure
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_add_audio_resource(MOBIPart *part) {
    unsigned char *data = NULL;
    size_t size = 0;
    MOBI_RET ret = mobi_decode_audio_resource(&data, &size, part);
    if (ret != MOBI_SUCCESS) {
        return ret;
    }
    part->data = data;
    part->size = size;
    /* FIXME: the only possible audio type is mp3 */
    part->type = T_MP3;
    ;
    return MOBI_SUCCESS;
}

/**
 @brief Decode audio resource
 
 @param[in,out] decoded_resource Pointer to data offset in mobipocket record.
 @param[in,out] decoded_size Decoded resource data size
 @param[in,out] part MOBIPart structure containing resource, decoded part type will be set in the structure
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_decode_audio_resource(unsigned char **decoded_resource, size_t *decoded_size, MOBIPart *part) {
    if (part->size < MEDIA_HEADER_LEN) {
        debug_print("Audio resource record too short (%zu)\n", part->size);
        return MOBI_DATA_CORRUPT;
    }
    MOBIBuffer *buf = buffer_init_null(part->size);
    buf->data = part->data;
    char magic[5];
    buffer_getstring(magic, buf, 4);
    if (strncmp(magic, AUDI_MAGIC, 4) != 0) {
        debug_print("Wrong magic for audio resource: %s\n", magic);
        buffer_free_null(buf);
        return MOBI_DATA_CORRUPT;
    }
    uint32_t offset = buffer_get32(buf);
    buf->offset = offset;
    *decoded_size = buf->maxlen - buf->offset;
    *decoded_resource = buf->data;
    buffer_free_null(buf);
    return MOBI_SUCCESS;
}

/**
 @brief Replace part data with decoded video data
 
 @param[in,out] part MOBIPart structure containing font resource, decoded part type will be set in the structure
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_add_video_resource(MOBIPart *part) {
    unsigned char *data = NULL;
    size_t size = 0;
    MOBI_RET ret = mobi_decode_video_resource(&data, &size, part);
    if (ret != MOBI_SUCCESS) {
        return ret;
    }
    part->data = data;
    part->size = size;
    part->type = T_MPG; /* FIXME: other types? */
;
    return MOBI_SUCCESS;
}

/**
 @brief Decode video resource
 
 @param[in,out] decoded_resource Pointer to data offset in mobipocket record.
 @param[in,out] decoded_size Decoded resource data size
 @param[in,out] part MOBIPart structure containing resource, decoded part type will be set in the structure
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_decode_video_resource(unsigned char **decoded_resource, size_t *decoded_size, MOBIPart *part) {
    if (part->size < MEDIA_HEADER_LEN) {
        debug_print("Video resource record too short (%zu)\n", part->size);
        return MOBI_DATA_CORRUPT;
    }
    MOBIBuffer *buf = buffer_init_null(part->size);
    buf->data = part->data;
    char magic[5];
    buffer_getstring(magic, buf, 4);
    if (strncmp(magic, VIDE_MAGIC, 4) != 0) {
        debug_print("Wrong magic for audio resource: %s\n", magic);
        buffer_free_null(buf);
        return MOBI_DATA_CORRUPT;
    }
    uint32_t offset = buffer_get32(buf);
    /* offset is always(?) 12, next four bytes are unknown */
    buf->offset = offset;
    *decoded_size = buf->maxlen - buf->offset;
    *decoded_resource = buf->data;
    buffer_free_null(buf);
    return MOBI_SUCCESS;
}

/**
 @brief Replace part data with decoded font data
 
 @param[in,out] part MOBIPart structure containing font resource, decoded part type will be set in the structure
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_add_font_resource(MOBIPart *part) {
    unsigned char *data = NULL;
    size_t size = 0;
    MOBI_RET ret = mobi_decode_font_resource(&data, &size, part);
    if (ret != MOBI_SUCCESS) {
        return ret;
    }
    part->data = data;
    part->size = size;
    part->type = mobi_determine_font_type(data);
    return MOBI_SUCCESS;
}

/**
 @brief Deobfuscator and decompressor for font resources
 
 @param[in,out] decoded_font Pointer to memory to write to. Will be allocated. Must be freed by caller
 @param[in,out] decoded_size Decoded font data size
 @param[in,out] part MOBIPart structure containing font resource, decoded part type will be set in the structure
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_decode_font_resource(unsigned char **decoded_font, size_t *decoded_size, MOBIPart *part) {
    if (part->size < FONT_HEADER_LEN) {
        debug_print("Font resource record too short (%zu)\n", part->size);
        return MOBI_DATA_CORRUPT;
    }
    MOBIBuffer *buf = buffer_init(part->size);
    memcpy(buf->data, part->data, part->size);
    struct header {
        char magic[5];
        uint32_t decoded_size;
        uint32_t flags;
        uint32_t data_offset;
        uint32_t xor_key_len;
        uint32_t xor_data_off;
    };
    struct header h;
    buffer_getstring(h.magic, buf, 4);
    if (strncmp(h.magic, FONT_MAGIC, 4) != 0) {
        debug_print("Wrong magic for font resource: %s\n", h.magic);
        buffer_free(buf);
        return MOBI_DATA_CORRUPT;
    }
    h.decoded_size = buffer_get32(buf);
    h.flags = buffer_get32(buf);
    h.data_offset = buffer_get32(buf);
    h.xor_key_len = buffer_get32(buf);
    h.xor_data_off = buffer_get32(buf);
    const uint32_t zlib_flag = 1; /* bit 0 */
    const uint32_t xor_flag = 2; /* bit 1 */
    if (h.flags & xor_flag) {
        /* deobfuscate */
        buf->offset = h.data_offset;
        const unsigned char *xor_key = buf->data + h.xor_data_off;
        size_t i = 0;
        /* only xor first 1040 bytes */
        while (buf->offset < buf->maxlen && i < 1040) {
            buf->data[buf->offset++] ^= xor_key[i % h.xor_key_len];
            i++;
        }
    }
    buf->offset = h.data_offset;
    *decoded_size = h.decoded_size;
    *decoded_font = malloc(h.decoded_size);
    const unsigned char *encoded_font = buf->data + buf->offset;
    const unsigned long encoded_size = buf->maxlen - buf->offset;
    if (h.flags & zlib_flag) {
        /* unpack */
        int ret = m_uncompress(*decoded_font, (unsigned long *) decoded_size, encoded_font, encoded_size);
        if (ret != M_OK) {
            buffer_free(buf);
            free(*decoded_font);
            debug_print("%s", "Font resource decompression failed\n");
            return MOBI_DATA_CORRUPT;
        }
        if (*decoded_size != h.decoded_size) {
            buffer_free(buf);
            free(*decoded_font);
            debug_print("Decompressed font size (%zu) differs from declared (%i)\n", *decoded_size, h.decoded_size);
            return MOBI_DATA_CORRUPT;
        }
    } else {
        memcpy(*decoded_font, encoded_font, encoded_size);
    }

    buffer_free(buf);
    return MOBI_SUCCESS;
}

/**
 @brief Get resource type (image, font) by checking its magic header
 
 @param[in] record MOBIPdbRecord structure containing unknown record type
 @return MOBIFiletype file type, T_UNKNOWN if not determined, T_BREAK if end of records mark found
 */
MOBIFiletype mobi_determine_resource_type(const MOBIPdbRecord *record) {
    /* Kindle supports GIF, BMP, JPG, PNG, SVG images. */
    /* GIF: 47 49 46 38 37 61 (GIF87a), 47 49 46 38 39 61 (GIF89a) */
    /* BMP: 42 4D (BM) + 4 byte file length le */
    /* JPG: FF D8 FF (header) + FF D9 (trailer) */
    /* PNG: 89 50 4E 47 0D 0A 1A 0A */
    /* SVG is XML-based format, so stored in flow parts */
    /* FONT: must be decoded */
    const unsigned char jpg_magic[] = "\xff\xd8\xff";
    const unsigned char gif_magic[] = "\x47\x49\x46\x38";
    const unsigned char png_magic[] = "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a";
    const unsigned char bmp_magic[] = "\x42\x4d";
    const unsigned char font_magic[] = FONT_MAGIC;
    const unsigned char audio_magic[] = AUDI_MAGIC;
    const unsigned char video_magic[] = VIDE_MAGIC;
    const unsigned char boundary_magic[] = BOUNDARY_MAGIC;
    const unsigned char eof_magic[] = EOF_MAGIC;
    if (memcmp(record->data, jpg_magic, 3) == 0) {
        return T_JPG;
    } else if (memcmp(record->data, gif_magic, 4) == 0) {
        return T_GIF;
    } else if (memcmp(record->data, png_magic, 8) == 0) {
        return T_PNG;
    } else if (memcmp(record->data, font_magic, 4) == 0) {
        return T_FONT;
    } else if (memcmp(record->data, boundary_magic, 8) == 0) {
        return T_BREAK;
    } else if (memcmp(record->data, eof_magic, 4) == 0) {
        return T_BREAK;
    } else if (memcmp(record->data, bmp_magic, 2) == 0) {
        const size_t bmp_size = (uint32_t) record->data[2] | (uint32_t) record->data[3] << 8 | (uint32_t) record->data[4] << 16 | (uint32_t) record->data[5] << 24;
        if (record->size == bmp_size) {
            return T_BMP;
        }
    } else if (memcmp(record->data, audio_magic, 4) == 0) {
        return T_AUDIO;
    } else if (memcmp(record->data, video_magic, 4) == 0) {
        return T_VIDEO;
    }
    return T_UNKNOWN;
}

/**
 @brief Check if loaded MOBI data is KF7/KF8 hybrid file
 
 @param[in] m MOBIData structure with loaded Record(s) 0 headers
 @return true or false
 */
bool mobi_is_hybrid(const MOBIData *m) {
    if (m == NULL) {
        debug_print("%s", "Mobi structure not initialized\n");
        return false;
    }
    if (m->kf8_boundary_offset != MOBI_NOTSET) {
        return true;
    }
    return false;
}

/**
 @brief Check if loaded document is MOBI/BOOK Mobipocket format
 
 @param[in] m MOBIData structure with loaded Record(s) 0 headers
 @return true or false
 */
bool mobi_is_mobipocket(const MOBIData *m) {
    if (m == NULL || m->ph == NULL) {
        debug_print("%s", "Mobi structure not initialized\n");
        return false;
    }
    if (strcmp(m->ph->type, "BOOK") == 0 &&
        strcmp(m->ph->creator, "MOBI") == 0) {
        return true;
    }
    return false;
}

/**
 @brief Check if loaded document is encrypted
 
 @param[in] m MOBIData structure with loaded Record(s) 0 headers
 @return true or false
 */
bool mobi_is_encrypted(const MOBIData *m) {
    if (m == NULL) {
        debug_print("%s", "Mobi structure not initialized\n");
        return false;
    }
    if (mobi_is_mobipocket(m) && m->rh &&
        (m->rh->encryption_type == RECORD0_OLD_ENCRYPTION ||
         m->rh->encryption_type == RECORD0_MOBI_ENCRYPTION)) {
        return true;
    }
    return false;
}

/**
 @brief Get mobi file version
 
 @param[in] m MOBIData structure with loaded Record(s) 0 headers
 @return MOBI document version, 1 if ancient version (no MOBI header) or MOBI_NOTSET if error
 */
size_t mobi_get_fileversion(const MOBIData *m) {
    if (m == NULL) {
        debug_print("%s", "Mobi structure not initialized\n");
        return MOBI_NOTSET;
    }
    if (m && m->mh && m->mh->version) {
        return *m->mh->version;
    }
    return 1;
}

/**
 @brief Get maximal size of uncompessed text records
 
 @param[in] m MOBIData structure with loaded Record(s) 0 headers
 @return Size of text or MOBI_NOTSET if error
 */
size_t mobi_get_text_maxsize(const MOBIData *m) {
    if (m && m->rh) {
        /* FIXME: is it safe to use data from Record 0 header? */
        if (m->rh->text_record_count > 0) {
            return (m->rh->text_record_count * RECORD0_TEXT_SIZE_MAX);
        }
    }
    return MOBI_NOTSET;
}

/**
 @brief Get sequential number of first resource record (image/font etc)
 
 @param[in] m MOBIData structure with loaded Record(s) 0 headers
 @return Record number or MOBI_NOTSET if not set
 */
size_t mobi_get_first_resource_record(const MOBIData *m) {
    /* is it hybrid file? */
    if (mobi_is_hybrid(m) && m->use_kf8) {
        /* get first image index from KF7 mobi header */
        if (m->next->mh->image_index) {
            return *m->next->mh->image_index;
        }
    }
    /* try to get it from currently set mobi header */
    if (m->mh && m->mh->image_index) {
        return *m->mh->image_index;
    }
    return MOBI_NOTSET;
}


/**
 @brief Calculate exponentiation for unsigned base and exponent
 
 @param[in] base Base
 @param[in] exp Exponent
 @return Result of base raised by the exponent exp
 */
size_t mobi_pow(unsigned base, unsigned exp) {
    size_t result = 1;
    while(exp) {
        if (exp & 1) {
            result *= base;
        }
        exp >>= 1;
        base *= base;
    }
    return result;
}

/**
 @brief Decode positive number from base 32 to base 10.
 
 Base 32 characters must be upper case.
 Maximal supported value is VVVVVV.
 
 @param[in,out] decoded Base 10 output number
 @param[in] encoded Base 32 input number
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_base32_decode(uint32_t *decoded, const char *encoded) {
    if (!encoded || !decoded) {
        debug_print("Error, null parameter (decoded: %p, encoded: %p)\n", (void *) decoded, (void *) encoded)
        return MOBI_PARAM_ERR;
    }
    /* strip leading zeroes */
    while (*encoded == '0') {
        encoded++;
    }
    size_t encoded_length = strlen(encoded);
    /* Let's limit input to 6 chars. VVVVVV(32) is 0x3FFFFFFF */
    if (encoded_length > 6) {
        debug_print("Base 32 number too big: %s\n", encoded);
        return MOBI_PARAM_ERR;
    }
    const unsigned char *c =  (unsigned char *) encoded;
    unsigned len = (unsigned) encoded_length;
    const unsigned base = 32;
    *decoded = 0;
    unsigned value;
    while (*c) {
        /* FIXME: not portable, should we care? */
        if (*c >= 'A' && *c <= 'V') {
            value = *c - 'A' + 10;
        }
        else if (*c >= '0' && *c <= '9') {
            value = *c - '0';
        }
        else {
            debug_print("Illegal character: \"%c\"\n", *c);
            return MOBI_DATA_CORRUPT;
        }
        *decoded += value * mobi_pow(base, --len);
        c++;
    }
    return MOBI_SUCCESS;
}


/**
 @brief Get offset of KF8 Boundary for KF7/KF8 hybrid file cached in MOBIData structure
 
 @param[in] m MOBIData structure
 @return KF8 Boundary sequential number or zero if not found
 */
size_t mobi_get_kf8offset(const MOBIData *m) {
    /* check if we want to parse KF8 part of joint file */
    if (m->use_kf8 && m->kf8_boundary_offset != MOBI_NOTSET) {
        return m->kf8_boundary_offset + 1;
    }
    return 0;
}

/**
 @brief Get sequential number of KF8 Boundary record for KF7/KF8 hybrid file
 
 This function gets KF8 boundary offset from EXTH header
 
 @param[in] m MOBIData structure
 @return KF8 Boundary record sequential number or MOBI_NOTSET if not found
 */
size_t mobi_get_kf8boundary_seqnumber(const MOBIData *m) {
    if (m == NULL) {
        debug_print("%s", "Mobi structure not initialized\n");
        return MOBI_NOTSET;
    }
    const MOBIExthHeader *exth_tag = mobi_get_exthrecord_by_tag(m, EXTH_KF8BOUNDARY);
    if (exth_tag != NULL) {
        uint32_t rec_number = mobi_decode_exthvalue(exth_tag->data, exth_tag->size);
        rec_number--;
        const MOBIPdbRecord *record = mobi_get_record_by_seqnumber(m, rec_number);
        if (record) {
            if(memcmp(record->data, "BOUNDARY", 8) == 0) {
                return rec_number;
            }
        }
    }
    return MOBI_NOTSET;
}

/**
 @brief Loader will parse KF7 part of hybrid file
 
 @param[in,out] m MOBIData structure
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_parse_kf7(MOBIData *m) {
    if (m == NULL) {
        return MOBI_INIT_FAILED;
    }
    m->use_kf8 = false;
    return MOBI_SUCCESS;
}

/**
 @brief Loader will parse KF8 part of hybrid file
 
 This is the default option.
 
 @param[in,out] m MOBIData structure
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_parse_kf8(MOBIData *m) {
    if (m == NULL) {
        return MOBI_INIT_FAILED;
    }
    m->use_kf8 = true;
    return MOBI_SUCCESS;
}

/**
 @brief Swap KF7 and KF8 MOBIData structures in a hybrid file
 
 MOBIData structures form a circular linked list in case of hybrid files.
 By default KF8 structure is first one in the list.
 This function puts KF7 structure on the first place, so that it starts to be used by default.
 
 @param[in,out] m MOBIData structure
 @return MOBI_RET status code (on success MOBI_SUCCESS)
 */
MOBI_RET mobi_swap_mobidata(MOBIData *m) {
    MOBIData *tmp = malloc(sizeof(MOBIData));
    if (tmp == NULL) {
        debug_print("%s", "Memory allocation failed while swaping data\n");
        return MOBI_MALLOC_FAILED;
    }
    tmp->rh = m->rh;
    tmp->mh = m->mh;
    tmp->eh = m->eh;
    m->rh = m->next->rh;
    m->mh = m->next->mh;
    m->eh = m->next->eh;
    m->next->rh = tmp->rh;
    m->next->mh = tmp->mh;
    m->next->eh = tmp->eh;
    free(tmp);
    tmp = NULL;
    return MOBI_SUCCESS;
}
