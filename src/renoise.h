#pragma once
#include <stdlib.h>

#ifndef RENOISE_CHUNK_SIZE
#define RENOISE_CHUNK_SIZE 16
#endif // RENOISE_CHUNK_SIZE

typedef struct {
    double x;
    double y;
} Renoise_Gradient_Point;

typedef struct {
    Renoise_Gradient_Point* items;
    size_t capacity;
    size_t count;
} Renoise_Chunk;

Renoise_Gradient_Point renoise_gradient_point_generate();
