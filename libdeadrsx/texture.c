#include <assert.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

#include <pngdec/pngdec.h>

#include <rsx/rsx.h>
#include <rsx/commands.h>
#include <nv40v1.h>

#include "texture.h"

extern gcmContextData *context;

void load_tex(uint32_t unit, uint32_t offset, uint32_t width, uint32_t height, uint32_t stride, uint32_t fmt, int smooth )
{
  realityTexture tex;
  tex.swizzle =
    NV30_3D_TEX_SWIZZLE_S0_X_S1 | NV30_3D_TEX_SWIZZLE_S0_Y_S1 |
    NV30_3D_TEX_SWIZZLE_S0_Z_S1 | NV30_3D_TEX_SWIZZLE_S0_W_S1 |
    NV30_3D_TEX_SWIZZLE_S1_X_X | NV30_3D_TEX_SWIZZLE_S1_Y_Y |
    NV30_3D_TEX_SWIZZLE_S1_Z_Z | NV30_3D_TEX_SWIZZLE_S1_W_W ;

  tex.offset = offset;

  tex.format =  fmt |
		NV40_3D_TEX_FORMAT_LINEAR  | 
		NV30_3D_TEX_FORMAT_DIMS_2D |
		NV30_3D_TEX_FORMAT_DMA0 |
		NV30_3D_TEX_FORMAT_NO_BORDER | (0x8000) |
		(1 << NV40_3D_TEX_FORMAT_MIPMAP_COUNT__SHIFT);

  tex.wrap =  NV30_3D_TEX_WRAP_S_REPEAT |
	      NV30_3D_TEX_WRAP_T_REPEAT |
	      NV30_3D_TEX_WRAP_R_REPEAT;

  tex.enable = NV40_3D_TEX_ENABLE_ENABLE;

  if(smooth)
    tex.filter = NV30_3D_TEX_FILTER_MIN_LINEAR |
	         NV30_3D_TEX_FILTER_MAG_LINEAR | 0x3fd6;
  else
    tex.filter = NV30_3D_TEX_FILTER_MIN_NEAREST |
	         NV30_3D_TEX_FILTER_MAG_NEAREST | 0x3fd6;

  tex.width = width;
  tex.height = height;
  tex.stride = stride;

  rsxLoadTexture(context, unit, (gcmTexture *)(&tex));
}

// Load a png from ram 
// I can't be bothered handling errors correctly, lets just abort
Image loadPng(const uint8_t *png, u32 size) {
	pngData p;
	Image image;
	pngLoadFromBuffer((void*)png, size,&p);
	image.data = (uint32_t*)p.bmp_out;
	image.width = p.width;
	image.height = p.height;
	return image;
}


