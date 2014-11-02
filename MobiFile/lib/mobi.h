/** @file mobi.h
 *  @brief Libmobi main header file
 *
 * This file is installed with the library.
 * Include it in your project with "#include <mobi.h>".
 * See example of usage in mobitool.c.
 *
 * Copyright (c) 2014 Bartek Fabiszewski
 * http://www.fabiszewski.net
 *
 * This file is part of libmobi.
 * Licensed under LGPL, either version 3, or any later.
 * See <http://www.gnu.org/licenses/>
 */

#ifndef libmobi_mobi_h
#define libmobi_mobi_h

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

/** @brief Visibility attributes for symbol export */
#if defined (__CYGWIN__) || defined (__MINGW32__)
#define MOBI_EXPORT __attribute__((visibility("default"))) __declspec(dllexport) extern
#else
#define MOBI_EXPORT __attribute__((__visibility__("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    /**
     @defgroup mobi_enums Exported enums
     @{
     */
    
    /**
     @brief Error codes returned by functions
     */
    typedef enum {
        MOBI_SUCCESS = 0, /**< Generic success return value */
        MOBI_ERROR = 1, /**< Generic error return value */
        MOBI_PARAM_ERR = 2, /**< Wrong function parameter */
        MOBI_DATA_CORRUPT = 3, /**< Corrupted data */
        MOBI_FILE_NOT_FOUND = 4, /**< File not found */
        MOBI_FILE_ENCRYPTED = 5, /**< Unsupported encrypted data */
        MOBI_FILE_UNSUPPORTED = 6, /**< Unsupported document type */
        MOBI_MALLOC_FAILED = 7, /**< Memory allocation error */
        MOBI_INIT_FAILED = 8, /**< Initialization error */
        MOBI_BUFFER_END = 9, /**< Out of buffer error */
        MOBI_XML_ERR = 10, /**< LibXML2 error */
    } MOBI_RET;
    
    /**
     @brief EXTH record types
     */
    typedef enum {
        EXTH_NUMERIC = 0,
        EXTH_STRING = 1,
        EXTH_BINARY = 2
    } MOBIExthType;
    
    /**
     @brief EXTH record tags
     */
    typedef enum {
        EXTH_DRMSERVER = 1,
        EXTH_DRMCOMMERCE = 2,
        EXTH_DRMEBOOKBASE = 3,
        
        EXTH_TITLE = 99, /**< <dc:title> */
        EXTH_AUTHOR = 100, /**< <dc:creator> */
        EXTH_PUBLISHER = 101, /**< <dc:publisher> */
        EXTH_IMPRINT = 102, /**< <imprint> */
        EXTH_DESCRIPTION = 103, /**< <dc:description> */
        EXTH_ISBN = 104, /**< <dc:identifier opf:scheme="ISBN"> */
        EXTH_SUBJECT = 105, /**< <dc:subject> */
        EXTH_PUBLISHINGDATE = 106, /**< <dc:date> */
        EXTH_REVIEW = 107, /**< <review> */
        EXTH_CONTRIBUTOR = 108, /**< <dc:contributor> */
        EXTH_RIGHTS = 109, /**< <dc:rights> */
        EXTH_SUBJECTCODE = 110, /**< <dc:subject BASICCode="subjectcode"> */
        EXTH_TYPE = 111, /**< <dc:type> */
        EXTH_SOURCE = 112, /**< <dc:source> */
        EXTH_ASIN = 113,
        EXTH_VERSION = 114,
        EXTH_SAMPLE = 115,
        EXTH_STARTREADING = 116, /**< Start reading */
        EXTH_ADULT = 117, /**< <adult> */
        EXTH_PRICE = 118, /**< <srp> */
        EXTH_CURRENCY = 119, /**< <srp currency="currency"> */
        EXTH_KF8BOUNDARY = 121,
        EXTH_FIXEDLAYOUT = 122, /**< <fixed-layout> */
        EXTH_BOOKTYPE = 123, /**< <book-type> */
        EXTH_ORIENTATIONLOCK = 124, /**< <orientation-lock> */
        EXTH_COUNTRESOURCES = 125,
        EXTH_ORIGRESOLUTION = 126, /**< <original-resolution> */
        EXTH_ZEROGUTTER = 127, /**< <zero-gutter> */
        EXTH_ZEROMARGIN = 128, /**< <zero-margin> */
        EXTH_KF8COVERURI = 129,
        EXTH_RESCOFFSET = 131,
        EXTH_REGIONMAGNI = 132, /**< <region-mag> */
        
        EXTH_DICTNAME = 200, /**< <DictionaryVeryShortName> */
        EXTH_COVEROFFSET = 201, /**< <EmbeddedCover> */
        EXTH_THUMBOFFSET = 202,
        EXTH_HASFAKECOVER = 203,
        EXTH_CREATORSOFT = 204,
        EXTH_CREATORMAJOR = 205,
        EXTH_CREATORMINOR = 206,
        EXTH_CREATORBUILD = 207,
        EXTH_WATERMARK = 208,
        EXTH_TAMPERKEYS = 209,
        
        EXTH_FONTSIGNATURE = 300,
        
        EXTH_CLIPPINGLIMIT = 401,
        EXTH_PUBLISHERLIMIT = 402,
        EXTH_UNK403 = 403,
        EXTH_TTSDISABLE = 404,
        EXTH_UNK405 = 405,
        EXTH_RENTAL = 406,
        EXTH_UNK407 = 407,
        EXTH_UNK450 = 450,
        EXTH_UNK451 = 451,
        EXTH_UNK452 = 452,
        EXTH_UNK453 = 453,
        
        EXTH_DOCTYPE = 501, /**< PDOC - Personal Doc; EBOK - ebook; EBSP - ebook sample; */
        EXTH_LASTUPDATE = 502,
        EXTH_UPDATEDTITLE = 503,
        EXTH_ASIN504 = 504,
        EXTH_TITLEFILEAS = 508,
        EXTH_CREATORFILEAS = 517,
        EXTH_PUBLISHERFILEAS = 522,
        EXTH_LANGUAGE = 524, /**< <dc:language> */
        EXTH_ALIGNMENT = 525, /**< <primary-writing-mode> */
        EXTH_PAGEDIR = 527,
        EXTH_OVERRIDEFONTS = 528, /**< <override-kindle-fonts> */
        EXTH_SORCEDESC = 529,
        EXTH_UNK534 = 534,
        EXTH_CREATORBUILDREV = 535,
    } MOBIExthTag;
    
    /**
     @brief Types of files stored in database records
     */
    typedef enum {
        T_UNKNOWN, /**< unknown */
        /* markup */
        T_HTML, /**< html */
        T_CSS, /**< css */
        T_SVG, /**< svg */
        T_OPF, /**< opf */
        T_NCX, /**< ncx */
        /* images */
        T_JPG, /**< jpg */
        T_GIF, /**< gif */
        T_PNG, /**< png */
        T_BMP, /**< bmp */
        /* fonts */
        T_OTF, /**< otf */
        T_TTF, /**< ttf */
        /* media */
        T_MP3, /**< mp3 */
        T_MPG, /**< mp3 */
        T_PDF, /**< pdf */
        /* generic types */
        T_FONT, /**< encoded font */
        T_AUDIO, /**< audio resource */
        T_VIDEO, /**< video resource */
        T_BREAK /**< end of file */
    } MOBIFiletype;
    
    /**
     @brief Metadata of file types
     */
    typedef struct {
        MOBIFiletype type; /**< MOBIFiletype type */
        char extension[5]; /**< file extension */
        char mime_type[30]; /**< mime-type */
    } MOBIFileMeta;
    
    /** @} */
    
    /**
     @defgroup raw_structs Exported structures for the raw, unparsed records metadata and data
     @{
     */
    
    /**
     @brief Parsed data from HUFF and CDIC records needed to unpack huffman compressed text
     */
    typedef struct {
        size_t index_count; /**< Total number of indices in all CDIC records, stored in each CDIC record header */
        size_t index_read; /**< Number of indices parsed, used by parser */
        size_t code_length; /**< Code length value stored in CDIC record header */
        uint32_t table1[256]; /**< Table of big-endian indices from HUFF record data1 */
        uint32_t mincode_table[33]; /**< Table of big-endian mincodes from HUFF record data2 */
        uint32_t maxcode_table[33]; /**< Table of big-endian maxcodes from HUFF record data2 */
        uint16_t *symbol_offsets; /**< Index of symbol offsets parsed from CDIC records (index_count entries) */
        unsigned char **symbols; /**< Array of pointers to start of symbols data in each CDIC record (index = number of CDIC record) */
    } MOBIHuffCdic;

    /**
     @brief Header of palmdoc database file
     */
    typedef struct  {
        char name[33]; /**< 0: Database name, zero terminated, trimmed title (+author) */
        uint16_t attributes; /**< 32: Attributes bitfield, PALMDB_ATTRIBUTE_DEFAULT */
        uint16_t version; /**< 34: File version, PALMDB_VERSION_DEFAULT */
        uint32_t ctime; /**< 36: Creation time */
        uint32_t mtime; /**< 40: Modification time */
        uint32_t btime; /**< 44: Backup time */
        uint32_t mod_num; /**< 48: Modification number, PALMDB_MODNUM_DEFAULT */
        uint32_t appinfo_offset; /**< 52: Offset to application info (if present) or zero, PALMDB_APPINFO_DEFAULT */
        uint32_t sortinfo_offset; /**< 56: Offset to sort info (if present) or zero, PALMDB_SORTINFO_DEFAULT */
        char type[5]; /**< 60: Database type, zero terminated, PALMDB_TYPE_DEFAULT */
        char creator[5]; /**< 64: Creator type, zero terminated, PALMDB_CREATOR_DEFAULT */
        uint32_t uid; /**< 68: Used internally to identify record */
        uint32_t next_rec; /**< 72: Used only when database is loaded into memory, PALMDB_NEXTREC_DEFAULT */
        uint16_t rec_count; /**< 76: Number of records in the file */
    } MOBIPdbHeader;

    /**
     @brief Metadata and data of a record. All records form a linked list.
     */
    typedef struct MOBIPdbRecord {
        uint32_t offset; /**< Offset of the record data from the start of the database */
        size_t size; /**< Calculated size of the record data */
        uint8_t attributes; /**< Record attributes */
        uint32_t uid; /**< Record unique id, usually sequential even numbers */
        unsigned char *data; /**< Record data */
        struct MOBIPdbRecord *next; /**< Pointer to the next record or NULL */
    } MOBIPdbRecord;

    /**
     @brief Metadata and data of a EXTH record. All records form a linked list.
     */
    typedef struct MOBIExthHeader {
        uint32_t tag; /**< Record tag */
        uint32_t size; /**< Data size */
        void *data; /**< Record data */
        struct MOBIExthHeader *next; /**< Pointer to the next record or NULL */
    } MOBIExthHeader;
    
    /** 
     @brief EXTH tag metadata
     */
    typedef struct {
        MOBIExthTag tag; /**< Record tag id */
        MOBIExthType type; /**< EXTH_NUMERIC, EXTH_STRING or EXTH_BINARY */
        char *name; /**< Tag name */
    } MOBIExthMeta;
    
    /**
     @brief Header of the Record 0 meta-record
     */
    typedef struct {
        /* PalmDOC header (extended), offset 0, length 16 */
        uint16_t compression_type; /**< 0; 1 == no compression, 2 = PalmDOC compression, 17480 = HUFF/CDIC compression */
        /* uint16_t unused; // 2; 0 */
        uint32_t text_length; /**< 4; uncompressed length of the entire text of the book */
        uint16_t text_record_count; /**< 8; number of PDB records used for the text of the book */
        uint16_t text_record_size; /**< 10; maximum size of each record containing text, always 4096 */
        uint16_t encryption_type; /**< 12; 0 == no encryption, 1 = Old Mobipocket Encryption, 2 = Mobipocket Encryption */
        uint16_t unknown1; /**< 14; usually 0 */
    } MOBIRecord0Header;

    /**
     @brief MOBI header which follows Record 0 header
     
     All MOBI header fields are pointers. Some fields are not present in the header, then the pointer is NULL.
     */
    typedef struct {
        /* MOBI header, offset 16 */
        char mobi_magic[5]; /**< 16: M O B I { 77, 79, 66, 73 }, zero terminated */
        uint32_t *header_length; /**< 20: the length of the MOBI header, including the previous 4 bytes */
        uint32_t *mobi_type; /**< 24: mobipocket file type */
        uint32_t *text_encoding; /**< 28: 1252 = CP1252, 65001 = UTF-8 */
        uint32_t *uid; /**< 32: unique id */
        uint32_t *version; /**< 36: mobipocket format */
        uint32_t *orth_index; /**< 40: section number of orthographic meta index. MOBI_NOTSET if index is not available. */
        uint32_t *infl_index; /**< 44: section number of inflection meta index. MOBI_NOTSET if index is not available. */
        uint32_t *names_index; /**< 48: section number of names meta index. MOBI_NOTSET if index is not available. */
        uint32_t *keys_index; /**< 52: section number of keys meta index. MOBI_NOTSET if index is not available. */
        uint32_t *extra0_index; /**< 56: section number of extra 0 meta index. MOBI_NOTSET if index is not available. */
        uint32_t *extra1_index; /**< 60: section number of extra 1 meta index. MOBI_NOTSET if index is not available. */
        uint32_t *extra2_index; /**< 64: section number of extra 2 meta index. MOBI_NOTSET if index is not available. */
        uint32_t *extra3_index; /**< 68: section number of extra 3 meta index. MOBI_NOTSET if index is not available. */
        uint32_t *extra4_index; /**< 72: section number of extra 4 meta index. MOBI_NOTSET if index is not available. */
        uint32_t *extra5_index; /**< 76: section number of extra 5 meta index. MOBI_NOTSET if index is not available. */
        uint32_t *non_text_index; /**< 80: first record number (starting with 0) that's not the book's text */
        uint32_t *full_name_offset; /**< 84: offset in record 0 (not from start of file) of the full name of the book */
        uint32_t *full_name_length; /**< 88: length of the full name */
        uint32_t *locale; /**< 92: first byte is main language: 09 = English, next byte is dialect, 08 = British, 04 = US */
        uint32_t *dict_input_lang; /**< 96: input language for a dictionary */
        uint32_t *dict_output_lang; /**< 100: output language for a dictionary */
        uint32_t *min_version; /**< 104: minimum mobipocket version support needed to read this file. */
        uint32_t *image_index; /**< 108: first record number (starting with 0) that contains an image (sequential) */
        uint32_t *huff_rec_index; /**< 112: first huffman compression record */
        uint32_t *huff_rec_count; /**< 116: huffman compression records count */
        uint32_t *datp_rec_index; /**< 120: section number of DATP record */
        uint32_t *datp_rec_count; /**< 124: DATP records count */
        uint32_t *exth_flags; /**< 128: bitfield. if bit 6 (0x40) is set, then there's an EXTH record */
        /* 32 unknown bytes 0? */
        /* unknown2 */
        /* unknown3 */
        /* unknown4 */
        /* unknown5 */
        uint32_t *unknown6; /**< 164: use MOBI_NOTSET */
        uint32_t *drm_offset; /**< 168: offset to DRM key info in DRMed files. MOBI_NOTSET if no DRM */
        uint32_t *drm_count; /**< 172: number of entries in DRM info */
        uint32_t *drm_size; /**< 176: number of bytes in DRM info */
        uint32_t *drm_flags; /**< 180: some flags concerning DRM info */
        /* 8 unknown bytes 0? */
        /* unknown7 */
        /* unknown8 */
        uint16_t *first_text_index; /**< 192: section number of first text record */
        uint16_t *last_text_index; /**< 194: */
        uint32_t *fdst_index; /**< 192 (KF8) section number of FDST record */
        //uint32_t *unknown9; /**< 196: */
        uint32_t *fdst_section_count; /**< 196 (KF8) */
        uint32_t *fcis_index; /**< 200: section number of FCIS record */
        uint32_t *fcis_count; /**< 204: FCIS records count */
        uint32_t *flis_index; /**< 208: section number of FLIS record */
        uint32_t *flis_count; /**< 212: FLIS records count */
        uint32_t *unknown10; /**< 216: */
        uint32_t *unknown11; /**< 220: */
        uint32_t *srcs_index; /**< 224: section number of SRCS record */
        uint32_t *srcs_count; /**< 228: SRCS records count */
        uint32_t *unknown12; /**< 232: */
        uint32_t *unknown13; /**< 236: */
        /* uint16_t fill 0 */
        uint16_t *extra_flags; /**< 242: extra flags */
        uint32_t *ncx_index; /**< 244: section number of NCX record  */
        uint32_t *unknown14; /**< 248: */
        uint32_t *fragment_index; /**< 248 (KF8) section number of fragments record */
        uint32_t *unknown15; /**< 252: */
        uint32_t *skeleton_index; /**< 252 (KF8) section number of SKEL record */
        uint32_t *datp_index; /**< 256: section number of DATP record */
        uint32_t *unknown16; /**< 260: */
        uint32_t *guide_index; /**< 260 (KF8) section number of guide record */
        uint32_t *unknown17; /**< 264: */
        uint32_t *unknown18; /**< 268: */
        uint32_t *unknown19; /**< 272: */
        uint32_t *unknown20; /**< 276: */
    } MOBIMobiHeader;

    /**
     @brief Main structure holding all metadata and unparsed records data
     
     In case of hybrid KF7/KF8 file there are two Records 0.
     In such case MOBIData is a circular linked list of two independent records, one structure per each Record 0 header.
     Records data (MOBIPdbRecord structure) is not duplicated in such case - each struct holds same pointers to all records data.
     */
    typedef struct MOBIData {
        bool use_kf8; /**< Flag: if set to true (default), KF8 part of hybrid file is parsed, if false - KF7 part will be parsed */
        uint32_t kf8_boundary_offset; /**< Set to KF8 boundary rec number if present, otherwise: MOBI_NOTSET */
        MOBIPdbHeader *ph; /**< Palmdoc database header structure or NULL if not loaded */
        MOBIRecord0Header *rh; /**< Record0 header structure or NULL if not loaded */
        MOBIMobiHeader *mh; /**< MOBI header structure or NULL if not loaded */
        MOBIExthHeader *eh; /**< Linked list of EXTH records or NULL if not loaded */
        MOBIPdbRecord *rec; /**< Linked list of palmdoc database records or NULL if not loaded */
        struct MOBIData *next; /**< Pointer to the other part of hybrid file or NULL if not a hybrid file */
    } MOBIData;
    
    /** @} */ // end of raw_structs group

    /**
     @defgroup parsed_structs Exported structures for the parsed records metadata and data
     @{
     */
    
    /**
     @brief Parsed FDST record
     
     FDST record contains offsets of main sections in RAWML - raw text data.
     The sections are usually html part, css parts, svg part.
     */
    typedef struct {
        size_t fdst_section_count; /**< Number of main sections */
        uint32_t *fdst_section_starts; /**< Array of section start offsets */
        uint32_t *fdst_section_ends; /**< Array of section end offsets */
    } MOBIFdst;

    /**
     @brief Maximum value of tag values in index entry (MOBIIndexTag) 
     FIXME: is 2 enough?
     */
#define MOBI_INDX_MAXTAGVALUES 2
    
    /**
     @brief Parsed tag for an index entry
     */
    typedef struct {
        size_t tagid; /**< Tag id */
        size_t tagvalues_count; /**< Number of tag values */
        uint32_t tagvalues[MOBI_INDX_MAXTAGVALUES]; /**< Array of tag values */
    } MOBIIndexTag;

    /**
     @brief Parsed INDX index entry
     */
    typedef struct {
        char *label; /**< Entry string, zero terminated */
        size_t tags_count; /**< Number of tags */
        MOBIIndexTag *tags; /**< Array of tags */
    } MOBIIndexEntry;

    /**
     @brief Parsed INDX record
     */
    typedef struct {
        size_t type; /**< Index type: 0 - normal, 2 - inflection */
        size_t entries_count; /**< Index entries count */
        size_t encoding; /**< Index encoding */
        size_t total_entries_count; /**< Total index entries count */
        size_t ordt_offset; /**< ORDT offset */
        size_t ligt_offset; /**< LIGT offset */
        size_t ordt_entries_count; /**< ORDT index entries count */
        size_t cncx_records_count; /**< Number of compiled NCX records */
        MOBIPdbRecord *cncx_record; /**< Link to CNCX record */
        MOBIIndexEntry *entries; /**< Index entries array */
    } MOBIIndx;
    
    /**
     @brief Reconstructed source file.
     
     All file parts are organized in a linked list.
     */
    typedef struct MOBIPart {
        size_t uid; /**< Unique id */
        MOBIFiletype type; /**< File type */
        size_t size; /**< File size */
        unsigned char *data; /**< File data */
        struct MOBIPart *next; /**< Pointer to next part or NULL */
    } MOBIPart;
    
    /**
     @brief Main structure containing reconstructed source parts and indices
    */
    typedef struct {
        size_t version; /**< Version of Mobipocket document */
        MOBIFdst *fdst; /**< Parsed FDST record or NULL if not present */
        MOBIIndx *skel; /**< Parsed skeleton index or NULL if not present */
        MOBIIndx *frag; /**< Parsed fragments index or NULL if not present */
        MOBIIndx *guide; /**< Parsed guide index or NULL if not present */
        MOBIIndx *ncx; /**< Parsed NCX index or NULL if not present */
        MOBIIndx *orth; /**< Parsed orth index or NULL if not present */
        MOBIPart *flow; /**< Linked list of reconstructed main flow parts or NULL if not present */
        MOBIPart *markup; /**< Linked list of reconstructed markup files or NULL if not present */
        MOBIPart *resources; /**< Linked list of reconstructed resources files or NULL if not present */
    } MOBIRawml;

    /** @} */ // end of parsed_structs group
    
    /** 
     @defgroup mobi_export Functions exported by the library
     @{
     */
    MOBI_EXPORT const char * mobi_version(void);
    MOBI_EXPORT MOBI_RET mobi_load_file(MOBIData *m, FILE *file);
    MOBI_EXPORT MOBI_RET mobi_load_filename(MOBIData *m, const char *path);
    
    MOBI_EXPORT MOBIData * mobi_init();
    MOBI_EXPORT void mobi_free(MOBIData *m);
    
    MOBI_EXPORT MOBI_RET mobi_parse_kf7(MOBIData *m);
    MOBI_EXPORT MOBI_RET mobi_parse_kf8(MOBIData *m);
    
    MOBI_EXPORT MOBI_RET mobi_parse_huffdic(const MOBIData *m, MOBIHuffCdic *cdic);
    MOBI_EXPORT MOBI_RET mobi_parse_fdst(const MOBIData *m, MOBIRawml *rawml);
    MOBI_EXPORT MOBI_RET mobi_parse_index(const MOBIData *m, MOBIIndx *indx, const size_t indx_record_number);
    MOBI_EXPORT MOBI_RET mobi_parse_rawml(MOBIRawml *rawml, const MOBIData *m);
    MOBI_EXPORT MOBI_RET mobi_get_rawml(const MOBIData *m, char *text, size_t *len);
    MOBI_EXPORT MOBI_RET mobi_dump_rawml(const MOBIData *m, FILE *file);
    MOBI_EXPORT MOBI_RET mobi_decode_font_resource(unsigned char **decoded_font, size_t *decoded_size, MOBIPart *part);
    MOBI_EXPORT MOBI_RET mobi_decode_audio_resource(unsigned char **decoded_resource, size_t *decoded_size, MOBIPart *part);
    MOBI_EXPORT MOBI_RET mobi_decode_video_resource(unsigned char **decoded_resource, size_t *decoded_size, MOBIPart *part);
    
    MOBI_EXPORT MOBIPdbRecord * mobi_get_record_by_uid(const MOBIData *m, const size_t uid);
    MOBI_EXPORT MOBIPdbRecord * mobi_get_record_by_seqnumber(const MOBIData *m, const size_t uid);
    MOBI_EXPORT MOBI_RET mobi_get_fullname(const MOBIData *m, char *fullname, const size_t len);
    MOBI_EXPORT size_t mobi_get_text_maxsize(const MOBIData *m);
    MOBI_EXPORT size_t mobi_get_kf8offset(const MOBIData *m);
    MOBI_EXPORT size_t mobi_get_kf8boundary_seqnumber(const MOBIData *m);
    MOBI_EXPORT size_t mobi_get_record_extrasize(const MOBIPdbRecord *record, const uint16_t flags);
    MOBI_EXPORT size_t mobi_get_fileversion(const MOBIData *m);
    MOBI_EXPORT size_t mobi_get_fdst_record_number(const MOBIData *m);
    MOBI_EXPORT MOBIExthMeta mobi_get_exthtagmeta_by_tag(const MOBIExthTag tag);
    MOBI_EXPORT MOBIFileMeta mobi_get_filemeta_by_type(const MOBIFiletype type);
    MOBI_EXPORT uint32_t mobi_decode_exthvalue(const unsigned char *data, const size_t size);
    MOBI_EXPORT char * mobi_decode_exthstring(const MOBIData *m, const unsigned char *data, const size_t size);
    MOBI_EXPORT struct tm * mobi_pdbtime_to_time(const long pdb_time);
    MOBI_EXPORT const char * mobi_get_locale_string(const uint32_t locale);
    MOBI_EXPORT size_t mobi_get_locale_number(const char *locale_string);
    
    MOBI_EXPORT bool mobi_exists_mobiheader(const MOBIData *m);
    MOBI_EXPORT bool mobi_exists_fdst(const MOBIData *m);
    MOBI_EXPORT bool mobi_exists_skel_indx(const MOBIData *m);
    MOBI_EXPORT bool mobi_exists_frag_indx(const MOBIData *m);
    MOBI_EXPORT bool mobi_exists_guide_indx(const MOBIData *m);
    MOBI_EXPORT bool mobi_exists_ncx(const MOBIData *m);
    MOBI_EXPORT bool mobi_exists_orth(const MOBIData *m);
    MOBI_EXPORT bool mobi_is_hybrid(const MOBIData *m);
    MOBI_EXPORT bool mobi_is_encrypted(const MOBIData *m);
    MOBI_EXPORT bool mobi_is_mobipocket(const MOBIData *m);
    
    MOBI_EXPORT MOBIRawml * mobi_init_rawml(const MOBIData *m);
    MOBI_EXPORT void mobi_free_rawml(MOBIRawml *rawml);
    
    /** @} */ // end of mobi_export group
    
#ifdef __cplusplus
}
#endif

#endif
