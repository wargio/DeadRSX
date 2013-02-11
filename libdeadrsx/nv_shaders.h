#ifndef __NV_SHADERS_H__
#define __NV_SHADERS_H__

#include <rsx/commands.h>
#include <rsx/rsx.h>

typedef struct {
	uint32_t size;
	uint32_t in_reg;
	uint32_t out_reg;
	uint32_t data[];
} realityVertexProgram_old;

typedef struct {
	uint32_t offset;
	uint32_t size;
	uint32_t num_regs;
	uint32_t data[];
} realityFragmentProgram_old; 

static realityVertexProgram_old nv40_vp = {
	.in_reg  = 0x00000309,
	.out_reg = 0x0000c001,
	.size = (3*4),
	.data = {
		/* MOV result.position, vertex.position */
		0x40041c6c, 0x0040000d, 0x8106c083, 0x6041ff80,
		/* MOV result.texcoord[0], vertex.texcoord[0] */
		0x401f9c6c, 0x0040080d, 0x8106c083, 0x6041ff9c,
		/* MOV result.texcoord[1], vertex.texcoord[1] */
		0x401f9c6c, 0x0040090d, 0x8106c083, 0x6041ffa1,
	}
};

/*******************************************************************************
 * NV30/NV40/G70 fragment shaders
 */

static realityFragmentProgram_old nv30_fp = {
.num_regs = 2,
.size = (2*4),
.data = {
/* TEX R0, fragment.texcoord[0], texture[0], 2D */
0x17009e00, 0x1c9dc801, 0x0001c800, 0x3fe1c800,
/* MOV R0, R0 */
0x01401e81, 0x1c9dc800, 0x0001c800, 0x0001c800,
}
};


void realityInstallFragmentProgram_old(gcmContextData *context, realityFragmentProgram_old *prog, uint32_t *addr) {
	// We don't actually need context, but if we leave it out people will forget.
	int i;
	for( i = 0; i < prog->size; ++i ) {
		addr[i] = (((prog->data[i] >> 16 ) & 0xffff) << 0) | (((prog->data[i] >> 0 ) & 0xffff) << 16);
	}
	assert(rsxAddressToOffset(addr, &prog->offset) == 0);
}

#endif
