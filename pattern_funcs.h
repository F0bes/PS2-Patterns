#pragma once

#include <tamtypes.h>
#include <packet.h>
#include <libpad.h>

#define MAX_PATTERNS 5

typedef void (*pattern_func_ptr_t)(qword_t*, struct padButtonStatus*);

extern pattern_func_ptr_t g_pattern_funcs[MAX_PATTERNS];

extern pattern_func_ptr_t g_pattern_func; // Current pattern function

void init_patterns();
void patterns_increment();
void patterns_decrement();
char* patterns_string();
