/*
$info$
tags: thunklibs|GLESv2
$end_info$
*/

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdio.h>
#include <cstring>

#include "common/Guest.h"

#include "thunks.inl"
#include "function_packs.inl"
#include "function_packs_public.inl"

typedef void voidFunc();

static struct { const char* name; voidFunc* fn; } symtab[] = {
	#include "tab_function_packs.inl"
	{ nullptr, nullptr }
};

LOAD_LIB(libGLESv2)
