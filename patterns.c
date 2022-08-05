#include <draw.h>
#include <gs_gp.h>
#include <gs_psm.h>
#include <packet.h>
#include "fontx_improved.h"
#include <graph.h>
#include <dma.h>
#include <libpad.h>
#include <loadfile.h>
#include <stdio.h>
#include <kernel.h>
#include <sifrpc.h>
#include <sio.h>

#include "vmode.h"
#include "pattern_funcs.h"
#include "delay.h"

// CRUSTY PAD IMPLEMENTATION
static int waitPadReady()
{
	int state;
	int lastState;
	char stateString[16];

	state = padGetState(0, 0);
	lastState = -1;
	while ((state != PAD_STATE_STABLE) && (state != PAD_STATE_FINDCTP1))
	{
		if (state != lastState)
		{
			padStateInt2String(state, stateString);
		}
		lastState = state;
		state = padGetState(0, 0);
	}
	return 0;
}

static char padBuf[256] __attribute__((aligned(64)));
struct padButtonStatus pbs;
void loadpad()
{
	SifInitRpc(0);
	int ret;

	ret = SifLoadModule("rom0:SIO2MAN", 0, NULL);
	if (ret < 0)
	{
		printf("sifLoadModule sio failed: %d\n", ret);
		SleepThread();
	}

	ret = SifLoadModule("rom0:PADMAN", 0, NULL);
	if (ret < 0)
	{
		printf("sifLoadModule pad failed: %d\n", ret);
		SleepThread();
	}
	padInit(0);
	padPortOpen(0, 0, padBuf);
	waitPadReady();

	padSetMainMode(0, 0, PAD_MMODE_DUALSHOCK, PAD_MMODE_LOCK);
	waitPadReady();

	ret = padGetState(0, 0);
	while ((ret != PAD_STATE_STABLE) && (ret != PAD_STATE_FINDCTP1))
	{
		if (ret == PAD_STATE_DISCONN)
		{
			printf("Can't find pad");
		}
		ret = padGetState(0, 0);
	}
	return;
}

framebuffer_t fb;
void update_vmode()
{
	if (fb.address)
		graph_vram_free(fb.address);
	fb.address = graph_vram_allocate(g_VMODE.width, g_VMODE.height, GS_PSM_24, GRAPH_ALIGN_PAGE);
	fb.width = g_VMODE.width;
	fb.height = g_VMODE.height;
	fb.psm = GS_PSM_24;
	fb.mask = 0;

	zbuffer_t zb;
	zb.address = 0;
	zb.enable = 0;
	zb.mask = 0;
	zb.method = ZTEST_METHOD_ALLPASS;
	zb.zsm = GS_PSMZ_16;

	graph_set_mode(g_VMODE.interlace, g_VMODE.graph_mode, 0, 0);
	graph_set_screen(0, 0, g_VMODE.width, g_VMODE.height);
	graph_set_framebuffer_filtered(fb.address, fb.width, fb.height, 0, 0);
	graph_enable_output();

	qword_t data[20] __attribute__((aligned(64)));
	if (data == NULL)
	{
		printf("aligned_alloc failed\n");
		return;
	}
	qword_t* q = data;

	q = draw_setup_environment(q, 0, &fb, &zb);
	q = draw_primitive_xyoffset(q, 0, 0, 0);
	q = draw_disable_tests(q, 0, &zb);
	FlushCache(0);
	dma_channel_send_normal(DMA_CHANNEL_GIF, data, q - data, 0, 0);
	graph_wait_vsync();

	return;
}

fontx_t krom_u;

void clear_fb()
{
	qword_t data[5] __attribute__((aligned(64)));

	qword_t* q = data;
	PACK_GIFTAG(q, GIF_SET_TAG(1, 1, GIF_PRE_ENABLE, GIF_SET_PRIM(GIF_PRIM_SPRITE, 0, 0, 0, 0, 0, 0, 0, 0), GIF_FLG_PACKED, 3),
		GIF_REG_RGBAQ | (GIF_REG_XYZ2 << 4) | (GIF_REG_XYZ2 << 8));
	q++;
	// EVEN
	// RGBAQ
	q->dw[0] = (u64)((0x00) | ((u64)0x00 << 32));
	q->dw[1] = (u64)((0x00) | ((u64)0xFF << 32));
	q++;
	// XYZ2
	q->dw[0] = (u64)((((0 << 4)) | (((u64)(0 << 4)) << 32)));
	q->dw[1] = (u64)(0);
	q++;
	// XYZ2
	q->dw[0] = (u64)(((((g_VMODE.width + 1) << 4)) | (((u64)((g_VMODE.height + 1) << 4)) << 32)));
	q->dw[1] = (u64)(0);
	q++;

	q = draw_finish(q);
	FlushCache(0);
	dma_channel_send_normal(DMA_CHANNEL_GIF, data, q - data, 0, 0);
	draw_wait_finish();
	return;
}

void render()
{
	packet_t* packet = packet_init(4000, PACKET_NORMAL);
	vertex_t v0;
	v0.x = g_VMODE.width / 2;
	v0.y = g_VMODE.height / 2;
	v0.z = 0;

	color_t c0;
	c0.r = 0xFF;
	c0.g = 0x00;
	c0.b = 0x00;
	c0.a = 0xFF;

	u32 enable_pattern = 1;
	u32 mode_text_frame_cnt = 480;
	while (1)
	{
		v0.x = g_VMODE.width / 2;
		v0.y = (g_VMODE.height / 2);

		clear_fb(packet->data);
		if (enable_pattern)
			g_pattern_func(packet->data, &pbs);

		qword_t* q = packet->data;

		if (mode_text_frame_cnt)
		{
			q = fontx_print_ascii(q, 0, patterns_string(), CENTER_ALIGN, &v0, &c0, &krom_u);
			v0.y += 20;
			q = fontx_print_ascii(q, 0, vmode_string(), CENTER_ALIGN, &v0, &c0, &krom_u);
			mode_text_frame_cnt--;
			FlushCache(0);
			dma_channel_send_normal(DMA_CHANNEL_GIF, packet->data, q - packet->data, 0, 0);
		}

		padRead(0, 0, &pbs);

		if (!(pbs.btns & PAD_R1))
		{
			patterns_increment();
			mode_text_frame_cnt = 240;
			input_delay();
		}
		if (!(pbs.btns & PAD_L1))
		{
			patterns_decrement();
			mode_text_frame_cnt = 240;
			input_delay();
		}
		if (!(pbs.btns & PAD_R2))
		{
			vmode_increment();
			update_vmode();
			mode_text_frame_cnt = 240;
			input_delay();
			input_delay();
		}
		if (!(pbs.btns & PAD_L2))
		{
			vmode_decrement();
			update_vmode();
			mode_text_frame_cnt = 240;
			input_delay();
			input_delay();
		}
		if (!(pbs.btns & PAD_START))
		{
			enable_pattern = !enable_pattern;
			input_delay();
		}
		if (!(pbs.btns & PAD_TRIANGLE))
		{
			mode_text_frame_cnt = 120;
		}
		graph_wait_vsync();
	}
}

int main(void)
{
	printf("Patterns, developed by Fobes (github.com/f0bes)\n Enjoy :^)\n");
	sio_puts("Patterns, developed by Fobes (github.com/f0bes)\n Enjoy :^)\n");

	printf("L2/R2 - change vmode\n");
	printf("L1/R1 - change pattern\n");
	printf("START - toggle pattern\n");
	printf("TRIANGLE - show current mode / vmode\n");
	sio_puts("DPAD - Change line distance\n");
	sio_puts("L2/R2 - change vmode\n");
	sio_puts("L1/R1 - change pattern\n");
	sio_puts("START - toggle pattern\n");
	sio_puts("TRIANGLE - show current mode / vmode\n");
	sio_puts("DPAD - Change line distance\n");

	dma_channel_initialize(DMA_CHANNEL_GIF, NULL, 0);
	dma_channel_fast_waits(DMA_CHANNEL_GIF);

	fb.address = 0;
	init_vmode();
	update_vmode();
	loadpad();
	fontx_load("rom0:KROM", &krom_u, SINGLE_BYTE, 2, 1, 1);
	init_patterns();
	render();
}
