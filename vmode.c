#include "vmode.h"

#include <stddef.h> // NULL
#include <tamtypes.h>

t_VMODE g_VMODES[MAX_VMODES];

s32 g_iVMODE; // Current VMODE index
t_VMODE g_VMODE; // Current VMODE

char g_strVMODES[MAX_VMODES][256];
static char g_strVMODE[128]; // VMODE buffer used by vmode_string()

void init_vmode()
{
	// Progressive NTSC
	g_VMODES[0].width = 640;
	g_VMODES[0].height = 224;
	g_VMODES[0].interlace = GRAPH_MODE_NONINTERLACED;
	g_VMODES[0].graph_mode = GRAPH_MODE_NTSC;
	strcpy(g_strVMODES[0], "NTSC");

	// Interlaced NTSC
	g_VMODES[1].width = 640;
	g_VMODES[1].height = 448;
	g_VMODES[1].interlace = GRAPH_MODE_INTERLACED;
	g_VMODES[1].graph_mode = GRAPH_MODE_NTSC;
	strcpy(g_strVMODES[1], "NTSC");

	// Progressive PAL
	g_VMODES[2].width = 640;
	g_VMODES[2].height = 256;
	g_VMODES[2].interlace = GRAPH_MODE_NONINTERLACED;
	g_VMODES[2].graph_mode = GRAPH_MODE_PAL;
	strcpy(g_strVMODES[2], "PAL");

	// Interlaced PAL
	g_VMODES[3].width = 640;
	g_VMODES[3].height = 512;
	g_VMODES[3].interlace = GRAPH_MODE_INTERLACED;
	g_VMODES[3].graph_mode = GRAPH_MODE_PAL;
	strcpy(g_strVMODES[3], "PAL");

	// DTV 408p
	g_VMODES[4].width = 720;
	g_VMODES[4].height = 480;
	g_VMODES[4].interlace = GRAPH_MODE_NONINTERLACED;
	g_VMODES[4].graph_mode = GRAPH_MODE_HDTV_480P;
	strcpy(g_strVMODES[4], "DTV 408p");

	// DTV 1080i
	//g_VMODES[5].width = 640;
	//g_VMODES[5].height = 540;
	//g_VMODES[5].interlace = GRAPH_MODE_INTERLACED;
	//g_VMODES[5].graph_mode = GRAPH_MODE_HDTV_1080I;
	//strcpy(g_strVMODES[5], "DTV 1080i");

	g_VMODE = g_VMODES[1];
}

void vmode_increment()
{
	g_iVMODE++;
	if (g_iVMODE >= MAX_VMODES)
		g_iVMODE = 0;
	g_VMODE = g_VMODES[g_iVMODE];
}

void vmode_decrement()
{
	g_iVMODE--;
	if (g_iVMODE == -1)
		g_iVMODE = MAX_VMODES - 1;

	g_VMODE = g_VMODES[g_iVMODE];
}

char* vmode_string()
{
	sprintf(g_strVMODE, "%s: %dx%d %s", g_strVMODES[g_iVMODE], g_VMODE.width, g_VMODE.height, g_VMODE.interlace == GRAPH_MODE_INTERLACED ? "interlaced" : "noninterlaced");
	return (char*)g_strVMODE;
}
