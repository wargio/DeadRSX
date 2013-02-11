#pragma once
#include <stdint.h>

typedef struct {
	uint32_t offset;
	uint32_t format;
	uint32_t wrap;
	uint32_t enable;
	uint32_t swizzle;
	uint32_t filter;
	uint16_t width;
	uint16_t height;
	uint32_t borderColor;
	uint32_t stride;
} realityTexture;

void load_tex(uint32_t unit, uint32_t offset, uint32_t width, uint32_t height, uint32_t stride, uint32_t fmt, int smooth );

typedef struct {
	uint32_t *data;
	uint32_t width;
	uint32_t height;
} Image;

Image loadPng(const uint8_t *png, u32 size);
