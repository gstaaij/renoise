#include "renoise.h"
#include <math.h>

Renoise_Gradient_Point renoise_gradient_point_generate() {
    double angle = ((double) random() / 2147483647.0) * 2*M_PI;
    return(Renoise_Gradient_Point) {
        .x = cos(angle),
        .y = sin(angle),
    };
}

// **************** **************** ****************
// ^    ^    ^    ^     ^    ^    ^     ^    ^    ^
// freq = 1/5 = 0.2
// count_part = RENOISE_CHUNK_SIZE * freq = 16*0.2 = 3.2
// count_extra = count_part % 1
// gradient_offset = (chunk_pos * count_extra) % 1
// grad_point_count = ceil(count_part - gradient_offset)

Renoise_Chunk renoise_chunk_generate(int64_t chunk_x, int64_t chunk_y, double frequency) {
    Renoise_Chunk chunk = {0};
    chunk.frequency = frequency;
    chunk.x = chunk_x;
    chunk.y = chunk_y;
    double grad_point_size = RENOISE_CHUNK_SIZE * frequency;
    double grad_point_size_decimal = fmod(grad_point_size, 1.0);
    chunk.grad_offset_x = fmod(chunk.x * grad_point_size_decimal, 1.0);
    chunk.grad_offset_y = fmod(chunk.y * grad_point_size_decimal, 1.0);
    chunk.grad_point_count_x = ceil(grad_point_size - chunk.grad_offset_x);
    chunk.grad_point_count_y = ceil(grad_point_size - chunk.grad_offset_y);

    uint64_t grad_point_count = chunk.grad_point_count_x * chunk.grad_point_count_y;
    chunk.grad_points = malloc(grad_point_count * sizeof(*chunk.grad_points));
    for (uint64_t i = 0; i < grad_point_count; ++i) {
        chunk.grad_points[i] = renoise_gradient_point_generate();
    }

    return chunk;
}

Renoise_World renoise_world_create(uint64_t world_size, double frequency) {
    Renoise_World world = {0};
    world.frequency = frequency;
    world.world_size = world_size;
    world.chunks = malloc(world_size*world_size * sizeof(*world.chunks));
    for (uint64_t i = 0; i < world_size*world_size; ++i) {
        uint64_t x = i % world_size;
        uint64_t y = i / world_size;
        world.chunks[i] = renoise_chunk_generate(x, y, frequency);
    }
    return world;
}

Renoise_Gradient_Point renoise_chunk_coord_to_gradient_coord(Renoise_Chunk* chunk, uint8_t chunk_x, uint8_t chunk_y) {
    return (Renoise_Gradient_Point) {
        .x = chunk_x * chunk->frequency - chunk->grad_offset_x,
        .y = chunk_y * chunk->frequency - chunk->grad_offset_y,
    };
}

double perlin_function(double t) {
    t = fabs(t);
    if (t >= 1.0) return 0.0;
    return 1 - (3 - 2*t) * t*t;
}

double perlin_falloff(double x, double y, Renoise_Gradient_Point gradient) {
    return perlin_function(x) * perlin_function(y) * (x * gradient.x + y * gradient.y);
}

void renoise_world_generate(Renoise_World* world) {
    for (uint64_t world_y = 1; world_y < world->world_size - 1; ++world_y) {
        for (uint64_t world_x = 1; world_x < world->world_size - 1; ++world_x) {
            uint64_t index = world_x + world_y * world->world_size;
            Renoise_Chunk* chunk = &world->chunks[index];

            // Generate points for this chunk
            for (uint8_t chunk_x = 0; chunk_x < RENOISE_CHUNK_SIZE; ++chunk_x) {
                for (uint8_t chunk_y = 0; chunk_y < RENOISE_CHUNK_SIZE; ++chunk_y) {
                    Renoise_Gradient_Point grad_coord = renoise_chunk_coord_to_gradient_coord(chunk, chunk_x, chunk_y);
                    int grad_cell_x = floor(grad_coord.x);
                    int grad_cell_y = floor(grad_coord.y);

                    chunk->points[chunk_y][chunk_x] = 0.0;
                    for (int grid_x = grad_cell_x; grid_x <= grad_cell_x + 1; ++grid_x) {
                        for (int grid_y = grad_cell_y; grid_y <= grad_cell_y + 1; ++grid_y) {
                            if (grid_x < 0) {
                                continue; // TODO: use chunk to the left
                            }
                            if (grid_x >= RENOISE_CHUNK_SIZE) {
                                continue; // TODO: use chunk to the right
                            }
                            if (grid_y < 0) {
                                continue; // TODO: use chunk above
                            }
                            if (grid_y >= RENOISE_CHUNK_SIZE) {
                                continue; // TODO: use chunk below
                            }

                            Renoise_Gradient_Point grad_point = chunk->grad_points[grid_x + grid_y * chunk->grad_point_count_x];
                            chunk->points[chunk_y][chunk_x] += perlin_falloff(grad_coord.x - grid_x, grad_coord.y - grid_y, grad_point);
                        }
                    }
                }
            }
        }
    }
}
