// FontX but without the stupid fixed XY offset and the stupid unsigned char*

/**
 * @file
 * Font library.
 */

#ifndef __FONT_H__
#define __FONT_H__

#include <draw.h>
#include <tamtypes.h>

/** FontX2 types */
#define SINGLE_BYTE 0
#define DOUBLE_BYTE 1

typedef struct
{
	/** Name of font */
	char name[9];
	/** Enable/Disable bold effect */
	char bold;
	/** Margin width between characters */
	char w_margin;
	/** Margin height between lines */
	char h_margin;
	/** Size in byte of each row */
	int rowsize;
	/** Size in bytes of each character */
	int charsize;
	/** Offset from beginning of font to character */
	int offset;
	/** The font data */
	char* font;
} fontx_t;

/** FontStudio type fonts */
typedef struct
{
	char A;
	char B;
	char C;
	char ox;
	char oy;
	char width;
	char height;
	unsigned short u1;
	unsigned short v1;
	unsigned short u2;
	unsigned short v2;
} inidata_t;

typedef struct
{
	float scale;
	char height;
	unsigned short* charmap;
	int totalchars;
	int spacewidth;
	inidata_t* chardata;
} fsfont_t;

/** Alignments */
#define LEFT_ALIGN 0
#define CENTER_ALIGN 1
#define RIGHT_ALIGN 2

#ifdef __cplusplus
extern "C" {
#endif

// FontX2 type fonts

// README: dma heavy
// A 16x16 pt font with a character that consists of nothing but points
// would take roughly 2 kilobytes to draw (16x16)*8 bytes

/** Loads a FontX2 type font with set characteristics
 * Use "rom0:KROM" as the path to load the PS2's internal FontX2 font
 */
int fontx_load(const char* path, fontx_t* fontx, int type, int wmargin, int hmargin, int bold);

/** Frees memory if a font is loaded */
void fontx_unload(fontx_t* fontx);

/** Prints an ascii/JISX201 formatted string */
qword_t* fontx_print_ascii(qword_t* q, int context, const char* str, int alignment, vertex_t* v0, color_t* c0, fontx_t* fontx);

/** Prints a SJIS formatted string */
qword_t* fontx_print_sjis(qword_t* q, int context, const char* str, int alignment, vertex_t* v0, color_t* c0, fontx_t* ascii, fontx_t* kanji);

#ifdef __cplusplus
}
#endif

#endif /* __FONT_H__ */
