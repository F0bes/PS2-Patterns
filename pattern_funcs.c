#include "pattern_funcs.h"
#include "libpad.h"
#include "vmode.h"
#include "fontx_improved.h"
#include "delay.h"

#include <stdlib.h>
#include <kernel.h>

// GS stuff
#include <graph.h>
#include <dma.h>
#include <packet.h>
#include <gs_psm.h>
#include <gs_gp.h>
#include <draw.h>

pattern_func_ptr_t g_pattern_funcs[MAX_PATTERNS];
pattern_func_ptr_t g_pattern_func; // Current pattern function
s32 g_iPattern; // Current pattern index

extern fontx_t krom_u;

// array of 5 strings that will be written to in init_patterns()
char g_strPatterns[MAX_PATTERNS][128];

void func_vlines(qword_t*, struct padButtonStatus*);
void func_hlines(qword_t*, struct padButtonStatus*);
void func_vhlines(qword_t*, struct padButtonStatus*);
void func_spectrum(qword_t*, struct padButtonStatus*);
void func_borders(qword_t* q, struct padButtonStatus* pbs);

void init_patterns()
{
	g_pattern_funcs[0] = func_vlines;
	strcpy(g_strPatterns[0], "VERT LINE");
	g_pattern_funcs[1] = func_hlines;
	strcpy(g_strPatterns[1], "HORZ LINE");
	g_pattern_funcs[2] = func_vhlines;
	strcpy(g_strPatterns[2], "VERT & HORZ LINE");
	g_pattern_funcs[3] = func_spectrum;
	strcpy(g_strPatterns[3], "SPECTRUM");
	g_pattern_funcs[4] = func_borders;
	strcpy(g_strPatterns[4], "BORDERS");

	g_pattern_func = g_pattern_funcs[0];
	g_iPattern = 0;
}

void patterns_increment()
{
	g_iPattern++;
	if (g_iPattern >= MAX_PATTERNS)
		g_iPattern = 0;
	g_pattern_func = g_pattern_funcs[g_iPattern];
}

void patterns_decrement()
{
	g_iPattern--;
	if (g_iPattern < 0)
		g_iPattern = MAX_PATTERNS - 1;
	g_pattern_func = g_pattern_funcs[g_iPattern];
}

char* patterns_string()
{
	return g_strPatterns[g_iPattern];
}

static u32 line_flip;
static u32 vlines_off = 1;
void func_vlines(qword_t* q, struct padButtonStatus* pbs)
{
	qword_t* start_q = q;
	PACK_GIFTAG(q, GIF_SET_TAG(g_VMODE.width / 2, 1, GIF_PRE_ENABLE, GIF_SET_PRIM(GIF_PRIM_SPRITE, 0, 0, 0, 0, 0, 0, 0, 0), GIF_FLG_PACKED, 3),
		GIF_REG_RGBAQ | (GIF_REG_XYZ2 << 4) | (GIF_REG_XYZ2 << 8));
	q++;
	u32 cur_x = line_flip ? vlines_off : 0;
	for (int i = 0; i < g_VMODE.width; i += 2)
	{
		// RGBAQ
		q->dw[0] = (u64)((0xFF) | ((u64)0xFF << 32));
		q->dw[1] = (u64)((0xFF) | ((u64)0xFF << 32));
		q++;
		if (cur_x <= g_VMODE.width)
		{
			// XYZ2
			q->dw[0] = (u64)((((cur_x << 4)) | (((u64)(0 << 4)) << 32)));
			q->dw[1] = (u64)(0);
			q++;
			// XYZ2
			q->dw[0] = (u64)((((cur_x + vlines_off << 4)) | (((u64)(g_VMODE.height << 4)) << 32)));
			q->dw[1] = (u64)(0);
		}
		else
		{
			// XYZ2
			q->dw[0] = (u64)(0);
			q->dw[1] = (u64)(0);
			q++;
			// XYZ2
			q->dw[0] = (u64)(0);
			q->dw[1] = (u64)(0);
		}
		q++;

		cur_x += (vlines_off * 2);
	}

	if (!(pbs->btns & PAD_LEFT))
	{
		if (vlines_off > 1)
			vlines_off--;
		input_delay();
	}
	if (!(pbs->btns & PAD_RIGHT))
	{
		vlines_off++;
		input_delay();
	}
	if(!(pbs->btns & PAD_CROSS))
	{
		line_flip = !line_flip;
		input_delay();
	}

	q = draw_finish(q);
	FlushCache(0);
	dma_channel_send_normal(DMA_CHANNEL_GIF, start_q, q - start_q, 0, 0);
	draw_wait_finish();
}

static u32 hlines_off = 1;
void func_hlines(qword_t* q, struct padButtonStatus* pbs)
{
	qword_t* start_q = q;
	PACK_GIFTAG(q, GIF_SET_TAG(g_VMODE.height / 2, 1, GIF_PRE_ENABLE, GIF_SET_PRIM(GIF_PRIM_SPRITE, 0, 0, 0, 0, 0, 0, 0, 0), GIF_FLG_PACKED, 3),
		GIF_REG_RGBAQ | (GIF_REG_XYZ2 << 4) | (GIF_REG_XYZ2 << 8));
	q++;
	u32 cur_y = line_flip ? hlines_off : 0;
	for (int i = 0; i < g_VMODE.height; i += 2)
	{
		// RGBAQ
		q->dw[0] = (u64)((0xFF) | ((u64)0xFF << 32));
		q->dw[1] = (u64)((0xFF) | ((u64)0xFF << 32));
		q++;
		// Prevent going so far off the screen that the lines loop back
		if (cur_y <= g_VMODE.height)
		{
			// XYZ2
			q->dw[0] = (u64)((((0 << 4)) | (((u64)(cur_y << 4)) << 32)));
			q->dw[1] = (u64)(0);
			q++;
			// XYZ2
			q->dw[0] = (u64)((((g_VMODE.width << 4)) | (((u64)(cur_y + hlines_off << 4)) << 32)));
			q->dw[1] = (u64)(0);
		}
		else
		{
			// XYZ2
			q->dw[0] = (u64)(0);
			q->dw[1] = (u64)(0);
			q++;
			// XYZ2
			q->dw[0] = (u64)(0);
			q->dw[1] = (u64)(0);
		}
		q++;
		cur_y += (hlines_off * 2);
	}

	if (!(pbs->btns & PAD_DOWN))
	{
		if (hlines_off > 1)
			hlines_off--;
		input_delay();
	}
	if (!(pbs->btns & PAD_UP))
	{
		hlines_off++;
		input_delay();
	}
	if(!(pbs->btns & PAD_CROSS))
	{
		line_flip = !line_flip;
		input_delay();
	}
	q = draw_finish(q);
	FlushCache(0);
	dma_channel_send_normal(DMA_CHANNEL_GIF, start_q, q - start_q, 0, 0);
	draw_wait_finish();
}

void func_vhlines(qword_t* q, struct padButtonStatus* pbs)
{
	func_vlines(q, pbs);
	func_hlines(q, pbs);
}

void func_spectrum(qword_t* q, struct padButtonStatus* pbs)
{
	qword_t* start_q = q;

	const u32 width_scale = g_VMODE.width / 32;
	u32 cur_y = (g_VMODE.width / 16) + 20;
	u32 y_increment = g_VMODE.width / 16;
	// RED, GREEN
	PACK_GIFTAG(q, GIF_SET_TAG(64, 1, GIF_PRE_ENABLE, GIF_SET_PRIM(GIF_PRIM_SPRITE, 0, 0, 0, 0, 0, 0, 0, 0), GIF_FLG_PACKED, 3),
		GIF_REG_RGBAQ | (GIF_REG_XYZ2 << 4) | (GIF_REG_XYZ2 << 8));
	q++;
	for (int r = 0; r < 32; r++)
	{
		// RGBAQ
		q->dw[0] = (u64)(((r + 1) * 8) | ((u64)0x00 << 32));
		q->dw[1] = (u64)((0x00) | ((u64)0xFF << 32));
		q++;
		// XYZ2
		q->dw[0] = (u64)((((r * width_scale) + 20) << 4) | (((u64)(cur_y << 4)) << 32));
		q->dw[1] = (u64)(0);
		q++;
		// XYZ2
		q->dw[0] = (u64)(((((r * width_scale) + width_scale) + 20) << 4) | (((u64)((cur_y + y_increment) << 4)) << 32));
		q->dw[1] = (u64)(0);
		q++;
	}
	cur_y += y_increment + 5;
	for (int g = 0; g < 32; g++)
	{
		// RGBAQ
		q->dw[0] = (u64)((0x00) | ((u64)((g + 1) * 8) << 32));
		q->dw[1] = (u64)((0x00) | ((u64)0xFF << 32));
		q++;
		// XYZ2
		q->dw[0] = (u64)((((g * width_scale) + 20) << 4) | ((u64)((cur_y << 4)) << 32));
		q->dw[1] = (u64)(0);
		q++;
		// XYZ2
		q->dw[0] = (u64)(((((g * width_scale) + width_scale) + 20) << 4) | (((u64)((cur_y + y_increment) << 4)) << 32));
		q->dw[1] = (u64)(0);
		q++;
	}

	cur_y += y_increment + 5;

	// BLUE, WHITE
	PACK_GIFTAG(q, GIF_SET_TAG(64, 1, GIF_PRE_ENABLE, GIF_SET_PRIM(GIF_PRIM_SPRITE, 0, 0, 0, 0, 0, 0, 0, 0), GIF_FLG_PACKED, 3),
		GIF_REG_RGBAQ | (GIF_REG_XYZ2 << 4) | (GIF_REG_XYZ2 << 8));
	q++;
	for (int b = 0; b < 32; b++)
	{
		// RGBAQ
		q->dw[0] = (u64)((0) | ((u64)0x00 << 32));
		q->dw[1] = (u64)(((b + 1) * 8) | ((u64)0xFF << 32));
		q++;
		// XYZ2
		q->dw[0] = (u64)((((b * width_scale) + 20) << 4) | (((u64)(cur_y << 4)) << 32));
		q->dw[1] = (u64)(0);
		q++;
		// XYZ2
		q->dw[0] = (u64)(((((b * width_scale) + width_scale) + 20) << 4) | (((u64)((cur_y + y_increment) << 4)) << 32));
		q->dw[1] = (u64)(0);
		q++;
	}
	cur_y += y_increment + 5;
	for (int w = 0; w < 32; w++)
	{
		// RGBAQ
		q->dw[0] = (u64)(((w + 1) * 8) | ((u64)((w + 1) * 8) << 32));
		q->dw[1] = (u64)(((w + 1) * 8) | ((u64)(0xFF) << 32));
		q++;
		// XYZ2
		q->dw[0] = (u64)((((w * width_scale) + 20) << 4) | ((u64)((cur_y << 4)) << 32));
		q->dw[1] = (u64)(0);
		q++;
		// XYZ2
		q->dw[0] = (u64)(((((w * width_scale) + width_scale) + 20) << 4) | (((u64)((cur_y + y_increment) << 4)) << 32));
		q->dw[1] = (u64)(0);
		q++;
	}

	color_t colour;
	colour.r = 0xFF;
	colour.g = 0xFF;
	colour.b = 0x00;
	colour.a = 0xFF;
	// DRAW HEADERS
	for (int i = 0; i < 16; i++)
	{
		vertex_t v0;
		v0.x = width_scale + 10 + ((i * width_scale) * 2);
		v0.y = y_increment - 5;
		v0.z = 0;

		char our_num[2];
		sprintf(our_num, "%X", i);
		q = fontx_print_ascii(q, 0, our_num, CENTER_ALIGN, &v0, &colour, &krom_u);
	}
	// DRAW ROW TEXT

	vertex_t v0;
	v0.x = 0;
	v0.y = (g_VMODE.width / 16) + 20 + ((g_VMODE.width / 16) / 2);
	v0.z = 0;

	{ // RED
		colour.r = 0xFF;
		colour.g = 0x00;
		colour.b = 0x00;
		q = fontx_print_ascii(q, 0, "R", LEFT_ALIGN, &v0, &colour, &krom_u);
	}
	v0.y += y_increment;
	{ // GREEN
		colour.r = 0x00;
		colour.g = 0xFF;
		colour.b = 0x00;
		q = fontx_print_ascii(q, 0, "G", LEFT_ALIGN, &v0, &colour, &krom_u);
	}
	v0.y += y_increment;
	{ // BLUE
		colour.r = 0x00;
		colour.g = 0x00;
		colour.b = 0xFF;
		q = fontx_print_ascii(q, 0, "B", LEFT_ALIGN, &v0, &colour, &krom_u);
	}
	v0.y += y_increment;
	{ // WHITE

		colour.r = 0xFF;
		colour.g = 0xFF;
		colour.b = 0xFF;
		q = fontx_print_ascii(q, 0, "W", LEFT_ALIGN, &v0, &colour, &krom_u);
	}

	q = draw_finish(q);
	FlushCache(0);
	dma_channel_send_normal(DMA_CHANNEL_GIF, start_q, q - start_q, 0, 0);
	draw_wait_finish();
}

u32 border_offsets[4] = {0, 0, 0, 0};
u32 sel_border = 0;

void func_borders(qword_t* q, struct padButtonStatus* pbs)
{
	qword_t* start_q = q;
	PACK_GIFTAG(q, GIF_SET_TAG(1, 1, GIF_PRE_ENABLE, GIF_SET_PRIM(GIF_PRIM_LINE_STRIP, 0, 0, 0, 0, 0, 0, 0, 0), GIF_FLG_PACKED, 6),
		GIF_REG_RGBAQ | (GIF_REG_XYZ2 << 4) | (GIF_REG_XYZ2 << 8) | (GIF_REG_XYZ2 << 12) | (GIF_REG_XYZ2 << 16) | (GIF_REG_XYZ2 << 20));
	q++;
	// RGBAQ
	q->dw[0] = (u64)((0xFF) | ((u64)0xFF << 32));
	q->dw[1] = (u64)((0xFF) | ((u64)0xFF << 32));
	q++;
	// XYZ2
	q->dw[0] = (u64)((border_offsets[3] << 4) | ((u64)(border_offsets[0] << 4) << 32));
	q->dw[1] = (u64)(0);
	q++;
	// XYZ2
	q->dw[0] = (u64)((((g_VMODE.width - 1) - border_offsets[1]) << 4) | ((u64)(border_offsets[0] << 4) << 32));
	q->dw[1] = (u64)(0);
	q++;
	// XYZ2
	q->dw[0] = (u64)((((g_VMODE.width - 1) - border_offsets[1]) << 4) | ((u64)(((g_VMODE.height - 1) - border_offsets[2]) << 4) << 32));
	q->dw[1] = (u64)(0);
	q++;
	// XYZ2
	q->dw[0] = (u64)((border_offsets[3] << 4) | ((u64)(((g_VMODE.height - 1) - border_offsets[2]) << 4) << 32));
	q->dw[1] = (u64)(0);
	q++;
	// XYZ2
	q->dw[0] = (u64)((border_offsets[3] << 4) | ((u64)(border_offsets[0] << 4) << 32));
	q->dw[1] = (u64)(0);
	q++;

	if (!(pbs->btns & PAD_DOWN))
	{
		if (border_offsets[sel_border] > 0)
			border_offsets[sel_border]--;
		input_delay();
	}
	if (!(pbs->btns & PAD_UP))
	{
		border_offsets[sel_border]++;
		input_delay();
	}
	if (!(pbs->btns & PAD_LEFT))
	{

		if (sel_border == 0)
			sel_border = 4;

		sel_border--;
		input_delay();
	}
	if (!(pbs->btns & PAD_RIGHT))
	{
		if (sel_border == 3)
			sel_border = -1;

		sel_border++;
		input_delay();
	}

	char b1OffsetStr[6];
	char b2OffsetStr[6];
	char b3OffsetStr[6];
	char b4OffsetStr[6];
	sprintf(b1OffsetStr, "%d", border_offsets[0]);
	sprintf(b2OffsetStr, "%d", border_offsets[1]);
	sprintf(b3OffsetStr, "%d", border_offsets[2]);
	sprintf(b4OffsetStr, "%d", border_offsets[3]);
	vertex_t v0;
	v0.z = 0;

	color_t colour_normal;
	colour_normal.r = 0xFF;
	colour_normal.g = 0xFF;
	colour_normal.b = 0xFF;
	colour_normal.a = 0xFF;

	color_t colour_selected;
	colour_selected.r = 0xFF;
	colour_selected.g = 0x00;
	colour_selected.b = 0x00;
	colour_selected.a = 0xFF;

	// First border
	v0.x = g_VMODE.width / 2;
	v0.y = 20 + border_offsets[0];

	q = fontx_print_ascii(q, 0, b1OffsetStr, CENTER_ALIGN, &v0,
		sel_border == 0 ? &colour_selected : &colour_normal, &krom_u);

	// Second border
	v0.x = (g_VMODE.width - 20) - border_offsets[1];
	v0.y = g_VMODE.height / 2;

	q = fontx_print_ascii(q, 0, b2OffsetStr, RIGHT_ALIGN, &v0,
		sel_border == 1 ? &colour_selected : &colour_normal, &krom_u);

	// Third border
	v0.x = g_VMODE.width / 2;
	v0.y = (g_VMODE.height - 20) - border_offsets[2];

	q = fontx_print_ascii(q, 0, b3OffsetStr, CENTER_ALIGN, &v0,
		sel_border == 2 ? &colour_selected : &colour_normal, &krom_u);

	// Fourth border
	v0.x = 20 + border_offsets[3];
	v0.y = g_VMODE.height / 2;

	q = fontx_print_ascii(q, 0, b4OffsetStr, CENTER_ALIGN, &v0,
		sel_border == 3 ? &colour_selected : &colour_normal, &krom_u);

	q = draw_finish(q);
	FlushCache(0);
	dma_channel_send_normal(DMA_CHANNEL_GIF, start_q, q - start_q, 0, 0);
	draw_wait_finish();
}
