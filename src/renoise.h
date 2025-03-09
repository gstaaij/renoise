#pragma once
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef RENOISE_CHUNK_SIZE
#define RENOISE_CHUNK_SIZE 16
#endif // RENOISE_CHUNK_SIZE
static_assert(RENOISE_CHUNK_SIZE >= 0 && RENOISE_CHUNK_SIZE < 256, "RENOISE_CHUNK_SIZE should fit in an unsigned 8-bit integer");

typedef struct {
    double x;
    double y;
} Renoise_Gradient_Point;

typedef struct {
    Renoise_Gradient_Point* grad_points;
    uint64_t grad_point_count_x;
    uint64_t grad_point_count_y;
    int64_t x;
    int64_t y;
    double grad_offset_x;
    double grad_offset_y;
    double frequency;
    double points[RENOISE_CHUNK_SIZE][RENOISE_CHUNK_SIZE];
} Renoise_Chunk;

typedef struct {
    Renoise_Chunk* chunks;
    uint64_t world_size;
    double frequency;
} Renoise_World;

Renoise_Gradient_Point renoise_gradient_point_generate();
Renoise_Chunk renoise_chunk_generate(int64_t chunk_x, int64_t chunk_y, double frequency);
Renoise_World renoise_world_create(uint64_t world_size, double frequency);
void renoise_world_generate(Renoise_World* world);
