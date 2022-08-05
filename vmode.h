#pragma once

#include <graph.h>

#define MAX_VMODES 5

typedef struct
{
	int width;
	int height;
	int iHeight;
	int interlace;
	int graph_mode;
} t_VMODE;

extern t_VMODE g_VMODES[MAX_VMODES];

extern t_VMODE g_VMODE; // Current VMODE

void init_vmode();
void vmode_increment();
void vmode_decrement();
char* vmode_string();
