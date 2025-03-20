// renoise: a library for generating and regenerating terrain noise
// Copyright (C) 2025  gstaaij
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#define RENOISE_VERSION "0.2.0"

#define RENOISE_CHUNK_SIZE 16
static_assert(RENOISE_CHUNK_SIZE >= 0 && RENOISE_CHUNK_SIZE < 256, "RENOISE_CHUNK_SIZE should fit in an unsigned 8-bit integer");

typedef struct {
    double x;
    double y;
} Renoise_Vector;

typedef struct {
    int64_t x;
    int64_t y;
    double frequency;

    Renoise_Vector* grad_points;
    int64_t grad_point_count_x;
    int64_t grad_point_count_y;
    double grad_offset_x;
    double grad_offset_y;
    double points[RENOISE_CHUNK_SIZE][RENOISE_CHUNK_SIZE];
} Renoise_Chunk;

typedef struct {
    Renoise_Chunk** chunks;
    int64_t size;
    double frequency;
} Renoise_World;

Renoise_Vector renoise_gradient_point_generate();
Renoise_Chunk* renoise_chunk_generate(int64_t chunk_x, int64_t chunk_y, double frequency);
void renoise_chunk_free(Renoise_Chunk* chunk);
Renoise_Vector renoise_chunk_coord_to_gradient_coord(Renoise_Chunk* chunk, uint8_t chunk_x, uint8_t chunk_y);
Renoise_World* renoise_world_generate(int64_t world_size, double frequency);
void renoise_world_free(Renoise_World* world);
void renoise_world_generate_chunk_points(Renoise_World* world, int64_t chunk_x, int64_t chunk_y);
void renoise_world_regenerate_rect(Renoise_World* world, int64_t chunk_x, int64_t chunk_y, int64_t width, int64_t height);
void renoise_world_regenerate_full_chunk(Renoise_World* world, int64_t chunk_x, int64_t chunk_y);
