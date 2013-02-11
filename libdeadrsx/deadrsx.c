//    DeadRSX Hardware Renderd Graphics Library
//    Author DarkhackerPS3  
//   -Thanks to Matt_P from EFNET #psl1ght for explaining stuff to me :P

//    DeadRSX is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//    DeadRSX is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with DeadRSX.  If not, see <http://www.gnu.org/licenses/>.

#include <assert.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>

#include <sysutil/video.h>
#include <rsx/gcm_sys.h>
#include <rsx/rsx.h>
#include <rsx/commands.h>
#include <nv40v1.h>

#include <sysmodule/sysmodule.h>
#include <pngdec/pngdec.h>

#include "deadrsx.h"
#include "texture.h"
// #include "nv_shaders.h" does not need be included in deadrsx.c

u32 *buffer[2];
u32 offset[2];
u32 *depth_buffer;
u32 depth_offset;
int pitch;
int depth_pitch;

gcmContextData *context; 
videoResolution res;
gcmSurface sf;

void deadrsx_init() {
	void *host_addr = memalign(1024*1024, 1024*1024);
	assert(host_addr != NULL);
	context = rsxInit(0x10000, 1024*1024, host_addr); 
	assert(context != NULL);
	videoState state;
	assert(videoGetState(0, 0, &state) == 0);
	assert(state.state == 0);
	assert(videoGetResolution(state.displayMode.resolution, &res) == 0);
	pitch = 4 * res.width;
	depth_pitch = 2 * res.width;
	videoConfiguration vconfig;
	memset(&vconfig, 0, sizeof(videoConfiguration));
	vconfig.resolution = state.displayMode.resolution;
	vconfig.format = VIDEO_BUFFER_FORMAT_XRGB;
	vconfig.pitch = pitch;
	assert(videoConfigure(0, &vconfig, NULL, 0) == 0);
	assert(videoGetState(0, 0, &state) == 0); 
	s32 buffer_size = pitch * res.height; 
	s32 depth_buffer_size = depth_pitch * res.height;
	printf("buffers will be 0x%x bytes\n", buffer_size);
	gcmSetFlipMode(GCM_FLIP_VSYNC); 
	buffer[0] = rsxMemalign(16, buffer_size);
	buffer[1] = rsxMemalign(16, buffer_size);
	assert(buffer[0] != NULL && buffer[1] != NULL);
	depth_buffer = rsxMemalign(16, depth_buffer_size * 2);
	assert(rsxAddressToOffset(buffer[0], &offset[0]) == 0);
	assert(rsxAddressToOffset(buffer[1], &offset[1]) == 0);
	assert(gcmSetDisplayBuffer(0, offset[0], pitch, res.width, res.height) == 0);
	assert(gcmSetDisplayBuffer(1, offset[1], pitch, res.width, res.height) == 0);
	assert(rsxAddressToOffset(depth_buffer, &depth_offset) == 0);
	gcmResetFlipStatus();
	flip(1);
        sysModuleLoad(SYSMODULE_PNGDEC);
}

void deadrsx_scale() { // code used from hermes Tiny3d to rescale screen
        if(res.width < 1280) {
//   	   rsxViewportTranslate(context, 38.0 , 16.0, 0.0, 0.0);
//         rsxViewportScale(context, (float) (res.width - 72) / 848.0,(res.height == 480) ? (512.0) / 576.0 : 548.0 / 512.0, Z_SCALE, 1.0);
	   const f32 scale[4] = {(float) (res.width - 72) / 848.0, (res.height == 480) ? (512.0) / 576.0 : 548.0 / 512.0, Z_SCALE, 1.0};
	   const f32 offset[4] = {38.0 , 16.0, 0.0, 0.0};
           rsxSetViewport(context, 0, 0, res.width, res.height, 0.0f, 1.0f, scale, offset);
        } else if(res.width == 1280) {
//         rsxViewportTranslate(context, 54.0, 24.0, 0.0, 0.0);
//         rsxViewportScale(context, 848.0 / 611.0 , 674.0 / 512.0, Z_SCALE, 1.0);
	   const f32 scale[4] = {848.0 / 611.0 , 674.0 / 512.0, Z_SCALE, 1.0};
	   const f32 offset[4] = {54.0, 24.0, 0.0, 0.0};
           rsxSetViewport(context, 0, 0, res.width, res.height, 0.0f, 1.0f, scale, offset);
        }else{
//         rsxViewportTranslate(context, 63.0, 40.0, 0.0, 0.0);
//         rsxViewportScale(context, 848.0 / 400.0 , 952.0 / 512.0, Z_SCALE, 1.0);
	   const f32 scale[4] = {848.0 / 400.0 , 952.0 / 512.0, Z_SCALE, 1.0};
	   const f32 offset[4] = {63.0, 40.0, 0.0, 0.0};
	   rsxSetViewport(context, 0, 0, res.width, res.height, 0.0f, 1.0f, scale, offset);
        }
}

void waitFlip() {
	while(gcmGetFlipStatus() != 0) 
		usleep(200);
	gcmResetFlipStatus();
}

void flip(s32 buffer) {
    assert(gcmSetFlip(context, buffer) == 0);
    rsxFlushBuffer(context);
    gcmSetWaitFlip(context);
}

void setupRenderTarget(u32 currentBuffer) {
	  sf.colorFormat = GCM_TF_COLOR_X8R8G8B8;
	  sf.colorTarget = GCM_TF_TARGET_0;
	  sf.colorLocation[0] = GCM_LOCATION_RSX;
	  sf.colorOffset[0] = offset[currentBuffer];
	  sf.colorPitch[0] = pitch;

	  sf.colorLocation[1] = GCM_LOCATION_RSX;
	  sf.colorLocation[2] = GCM_LOCATION_RSX;
	  sf.colorLocation[3] = GCM_LOCATION_RSX;
	  sf.colorOffset[1] = 0;
	  sf.colorOffset[2] = 0;
	  sf.colorOffset[3] = 0;
	  sf.colorPitch[1] = 64;
	  sf.colorPitch[2] = 64;
	  sf.colorPitch[3] = 64;

	  sf.depthFormat = GCM_TF_ZETA_Z16;
	  sf.depthLocation = GCM_LOCATION_RSX;
	  sf.depthOffset = depth_offset;
	  sf.depthPitch = depth_pitch;

	  sf.type = GCM_TF_TYPE_LINEAR;
	  sf.antiAlias 	= GCM_TF_CENTER_1;

	  sf.width = res.width;
	  sf.height = res.height;
	  sf.x = 0;
	  sf.y = 0;

	  rsxSetSurface (context, &sf);
}

void deadrsx_background(u32 trueColor) {
	rsxSetClearColor(context, trueColor);
}

void deadrsx_offset(u32 dooffset, int x, int y, int w, int h) {
	load_tex(0, dooffset, w, h, w*4,  NV40_3D_TEX_FORMAT_FORMAT_A8R8G8B8, 1);
	rsxDrawVertexBegin(context, GCM_TYPE_QUADS);
	{
		rsxDrawVertex2f(context, 0, 0.0, 0.0);
		rsxDrawVertex4f(context, 0, x, y, 0.0, 1.0);
		rsxDrawVertex2f(context, 0, 1.0, 0.0);
		rsxDrawVertex4f(context, 0, x + w, y, 0.0, 1.0);
		rsxDrawVertex2f(context, 0, 1.0, 1.0);
		rsxDrawVertex4f(context, 0, x + w, y + h, 0.0, 1.0); 
		rsxDrawVertex2f(context, 0, 0.0, 1.0);
		rsxDrawVertex4f(context, 0, x, y + h, 0.0, 1.0); 
	}
	rsxDrawVertexEnd(context);
}

void deadrsx_scaleoffset(u32 dooffset, int x, int y, int w, int h, int ow, int oh) {
	load_tex(0, dooffset, ow, oh, ow*4,  NV40_3D_TEX_FORMAT_FORMAT_A8R8G8B8, 1);
	rsxDrawVertexBegin(context, GCM_TYPE_QUADS);
	{
		rsxDrawVertex2f(context, 0, 0.0, 0.0);
		rsxDrawVertex4f(context, 0, x, y, 0.0, 1.0);
		rsxDrawVertex2f(context, 0, 1.0, 0.0);
		rsxDrawVertex4f(context, 0, x + w, y, 0.0, 1.0);
		rsxDrawVertex2f(context, 0, 1.0, 1.0);
		rsxDrawVertex4f(context, 0, x + w, y + h, 0.0, 1.0); 
		rsxDrawVertex2f(context, 0, 0.0, 1.0);
		rsxDrawVertex4f(context, 0, x, y + h, 0.0, 1.0); 
	}
	rsxDrawVertexEnd(context);
}

void deadrsx_sprite(u32 dooffset, float x, float y, float w, float h, int ow, int oh, int tilex, int tiley, int ax, int ay) {

        float wx = 1.0/ax;
        float wy = 1.0/ay;
	load_tex(0, dooffset, ow, oh, ow*4,  NV40_3D_TEX_FORMAT_FORMAT_A8R8G8B8, 1);
	rsxDrawVertexBegin(context, GCM_TYPE_QUADS);
	{
		rsxDrawVertex2f(context, 0, tilex * wx, tiley * wy);
		rsxDrawVertex4f(context, 0, x, y, 0.0, 1.0);
		rsxDrawVertex2f(context, 0, (tilex + 1) * wx, tiley * wy);
		rsxDrawVertex4f(context, 0, x + w, y, 1.0, 1.0);
		rsxDrawVertex2f(context, 0, (tilex + 1) * wx, (tiley + 1) * wy);
		rsxDrawVertex4f(context, 0, x + w, y + h, 1.0, 1.0); 
		rsxDrawVertex2f(context, 0, tilex * wx, (tiley + 1) * wy);
		rsxDrawVertex4f(context, 0, x, y + h, 0.0, 1.0);
	}
	rsxDrawVertexEnd(context);

}

void deadrsx_loadfile(char inputpath[], pngData inputImage, u32 *inputOffset) {
	pngLoadFromFile(inputpath, &inputImage);
	  if(inputImage.bmp_out) {
	     uint32_t * text= rsxMemalign(16, 4*inputImage.width*inputImage.height);
	     if(text)
	        memcpy(text, inputImage.bmp_out,inputImage.pitch*inputImage.height);
	     free(inputImage.bmp_out);
	     inputImage.bmp_out = text;
	  }
	assert(rsxAddressToOffset(inputImage.bmp_out, inputOffset) == 0);
}
