/*

   DeadRSX SAMPLE || Background
   Author DarkhackerPS3
*/

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <io/pad.h>
#include <sysmodule/sysmodule.h>
#include <sysutil/sysutil.h>

#include <deadrsx/deadrsx.h>
#include <deadrsx/nv_shaders.h>

#define NV30_3D_BLEND_FUNC_DST_ALPHA_ZERO_U8	0x00

int i;
int currentBuffer = 0;

int app_state = 1; // used to switch screens
int background_color = COLOR_WHITE;

padInfo padinfo;
padData paddata;

u32 *tx_mem;

void drawFrame(int buffer, long frame) {

//	realityViewportTranslate(context, 0.0, 0.0, 0.0, 0.0);
//	realityViewportScale(context, 1.0, 1.0, 1.0, 0.0); 

        deadrsx_scale(); // scales screen to 847x511 - deadrsx library

	rsxZControl(context, 0, 1, 1); // disable viewport culling
	rsxSetBlendFunc(context,
				 NV30_3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA >>16,
				 NV30_3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA >>16,
				 (u16)NV30_3D_BLEND_FUNC_DST_RGB_ONE_MINUS_SRC_ALPHA,
				 (u16)NV30_3D_BLEND_FUNC_DST_ALPHA_ZERO);
	rsxSetBlendEquation(context, (u16)NV40_3D_BLEND_EQUATION_RGB_FUNC_ADD ,
				      NV40_3D_BLEND_EQUATION_ALPHA_FUNC_ADD>>16);
	rsxSetBlendEnable(context, 1);
	rsxSetViewportClip(context, 0, res.width, res.height);
	setupRenderTarget(buffer);

	deadrsx_background(background_color);

	rsxSetClearDepthValue(context, 0xffff);
	rsxSetClearDepthValue(context, GCM_CLEAR_M);
//	const void *ucode = rsxVertexProgramGetUCode((rsxVertexProgram*)&nv40_vp);
	
//	realityLoadVertexProgram_old(context, &nv40_vp);
//	realityLoadFragmentProgram_old(context, &nv30_fp);
	rsxLoadVertexProgram(context, (rsxVertexProgram*)&nv40_vp, rsxVertexProgramGetUCode((rsxVertexProgram*)&nv40_vp));
	rsxLoadFragmentProgramLocation(context, (rsxFragmentProgram*)&nv30_fp, nv30_fp.offset,GCM_LOCATION_RSX);
	
        switch(app_state) {

		case 1:
			background_color = COLOR_WHITE;
			break;

		case 2: // screen while in-game xmb open
			background_color = COLOR_BLACK;
			break;

        }
}

static void eventHandle(u64 status, u64 param, void * userdata) {
    (void)param;
    (void)userdata;
	if(status == SYSUTIL_EXIT_GAME){
		exit(0);
	}else if(status == SYSUTIL_MENU_OPEN){
		app_state = 2;
	}else if(status == SYSUTIL_MENU_CLOSE) {
       	 app_state = 1;
	}else{
		printf("Unhandled event: %08llX\n", (unsigned long long int)status);
	}
}
void appCleanup(){
	sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT0);
}
void loading() {
// where all are loading going be done :D

}
void ps3_pad() {

  ioPadGetInfo(&padinfo);

  switch(app_state) {
  case 1: // are only screen open at the moment
	for(i=0; i<MAX_PADS; i++){
	  if(padinfo.status[i]){
	  ioPadGetData(i, &paddata);
	    if(paddata.BTN_CROSS || paddata.BTN_START){
		exit(0);
	    }
	  }		
	}
  break;
  }

}
s32 main(s32 argc, const char* argv[])
{

	atexit(appCleanup);
	deadrsx_init();
	ioPadInit(7);
	sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT0, eventHandle, NULL);

	u32 *frag_mem = rsxMemalign(256, 256);
	printf("frag_mem = 0x%08lx\n", (u64) frag_mem);
	realityInstallFragmentProgram_old(context, &nv30_fp, frag_mem);

        loading(); // where all the loading done xD

	long frame = 0; 

	while(1){
                ps3_pad(); // where all are controls are
		waitFlip(); // Wait for the last flip to finish, so we can draw to the old buffer
		drawFrame(currentBuffer, frame++); // Draw into the unused buffer
		flip(currentBuffer); // Flip buffer onto screen
		currentBuffer = !currentBuffer;
		sysUtilCheckCallback();

	}
	
	return 0;
}

