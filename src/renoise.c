#include "renoise.h"
#include <string.h>
#include <math.h>

Renoise_Vector renoise_gradient_point_generate() {
    double angle = ((double) random() / 2147483647.0) * 2*M_PI;
    return (Renoise_Vector) {
        .x = cos(angle),
        .y = sin(angle),
    };
}

// **************** **************** **************** **************** **************** ****************
// ^    ^    ^    ^     ^    ^    ^     ^    ^    ^     ^    ^    ^     ^    ^    ^     ^    ^    ^    ^
// freq = 1/5 = 0.2
// count_part = RENOISE_CHUNK_SIZE * freq = 16*0.2 = 3.2
// count_extra = count_part % 1
// gradient_offset = (chunk_pos * count_extra) % 1
// grad_point_count = ceil(count_part - gradient_offset)

Renoise_Chunk* renoise_chunk_generate(int64_t chunk_x, int64_t chunk_y, double frequency) {
    // TODO: make lower frequencies work
    assert(RENOISE_CHUNK_SIZE * frequency >= 1.0 && "ERROR: Frequency too low!");

    Renoise_Chunk* chunk = malloc(sizeof(Renoise_Chunk));
    memset(chunk, 0, sizeof(Renoise_Chunk));
    chunk->frequency = frequency;
    chunk->x = chunk_x;
    chunk->y = chunk_y;
    double grad_point_size = RENOISE_CHUNK_SIZE * frequency;
    double grad_point_size_decimal = fmod(grad_point_size, 1.0);

    chunk->grad_offset_x = chunk->x * (1.0 - grad_point_size_decimal);
    chunk->grad_offset_y = chunk->y * (1.0 - grad_point_size_decimal);
    // The rounding is to prevent floating point errors from messing with the fmod
    chunk->grad_offset_x = round(chunk->grad_offset_x / chunk->frequency) * chunk->frequency;
    chunk->grad_offset_y = round(chunk->grad_offset_y / chunk->frequency) * chunk->frequency;
    chunk->grad_offset_x = fmod(chunk->grad_offset_x, 1.0);
    chunk->grad_offset_y = fmod(chunk->grad_offset_y, 1.0);

    chunk->grad_point_count_x = ceil(grad_point_size - chunk->grad_offset_x);
    chunk->grad_point_count_y = ceil(grad_point_size - chunk->grad_offset_y);

    int64_t grad_point_count = chunk->grad_point_count_x * chunk->grad_point_count_y;
    chunk->grad_points = malloc(grad_point_count * sizeof(*chunk->grad_points));
    assert(chunk->grad_points != NULL && "ERROR: Out of memory; buy more RAM.");
    for (int64_t i = 0; i < grad_point_count; ++i) {
        chunk->grad_points[i] = renoise_gradient_point_generate();
    }

    return chunk;
}

void renoise_chunk_free(Renoise_Chunk* chunk) {
    free(chunk->grad_points);
    free(chunk);
}

Renoise_Vector renoise_chunk_coord_to_gradient_coord(Renoise_Chunk* chunk, uint8_t chunk_x, uint8_t chunk_y) {
    return (Renoise_Vector) {
        .x = chunk_x * chunk->frequency - chunk->grad_offset_x,
        .y = chunk_y * chunk->frequency - chunk->grad_offset_y,
    };
}

Renoise_World* renoise_world_generate(int64_t world_size, double frequency) {
    Renoise_World* world = malloc(sizeof(Renoise_World));
    memset(world, 0, sizeof(Renoise_World));
    world->frequency = frequency;
    world->size = world_size;
    world->chunks = malloc(world->size*world->size * sizeof(*world->chunks));
    assert(world->chunks != NULL && "ERROR: Out of memory; buy more RAM.");
    for (int64_t i = 0; i < world->size*world->size; ++i) {
        int64_t x = i % world->size;
        int64_t y = i / world->size;
        world->chunks[i] = renoise_chunk_generate(x, y, frequency);
    }

    for (int64_t world_y = 1; world_y < world->size - 1; ++world_y) {
        for (int64_t world_x = 1; world_x < world->size - 1; ++world_x) {
            // Generate points for this chunk
            renoise_world_generate_chunk_points(world, world_x, world_y);
        }
    }

    return world;
}

void renoise_world_free(Renoise_World* world) {
    for (int64_t i = 0; i < world->size*world->size; ++i) {
        renoise_chunk_free(world->chunks[i]);
    }
    free(world->chunks);
    free(world);
}

double perlin_function(double t) {
    t = fabs(t);
    if (t >= 1.0) return 0.0;
    return 1 - (3 - 2*t) * t*t;
}

double perlin_falloff(double x, double y, Renoise_Vector gradient) {
    return perlin_function(x) * perlin_function(y) * (x * gradient.x + y * gradient.y);
}

void renoise_world_generate_chunk_points(Renoise_World* world, int64_t world_x, int64_t world_y) {
    Renoise_Chunk* chunk = world->chunks[world_x + world_y * world->size];

    for (uint8_t chunk_x = 0; chunk_x < RENOISE_CHUNK_SIZE; ++chunk_x) {
        for (uint8_t chunk_y = 0; chunk_y < RENOISE_CHUNK_SIZE; ++chunk_y) {
            Renoise_Vector grad_coord = renoise_chunk_coord_to_gradient_coord(chunk, chunk_x, chunk_y);
            int64_t grad_cell_x = floor(grad_coord.x);
            int64_t grad_cell_y = floor(grad_coord.y);

            chunk->points[chunk_y][chunk_x] = 0.0;
            for (int64_t grid_x = grad_cell_x; grid_x <= grad_cell_x + 1; ++grid_x) {
                for (int64_t grid_y = grad_cell_y; grid_y <= grad_cell_y + 1; ++grid_y) {
                    Renoise_Chunk* query_chunk = chunk;
                    int64_t query_x = grid_x;
                    int64_t query_y = grid_y;
                    int64_t current_world_x = world_x;
                    int64_t current_world_y = world_y;
                    while (query_x < 0) {
                        // Use chunk to the left
                        current_world_x -= 1;
                        assert(current_world_x >= 0);
                        query_chunk = world->chunks[current_world_x + current_world_y * world->size];
                        query_x = query_chunk->grad_point_count_x - 1;
                    }
                    while (query_x >= query_chunk->grad_point_count_x) {
                        // Use chunk to the right
                        current_world_x += 1;
                        assert(current_world_x < world->size);
                        query_chunk = world->chunks[current_world_x + current_world_y * world->size];
                        query_x = 0;
                    }
                    while (query_y < 0) {
                        // Use chunk above
                        current_world_y -= 1;
                        assert(current_world_y >= 0);
                        query_chunk = world->chunks[current_world_x + current_world_y * world->size];
                        query_y = query_chunk->grad_point_count_y - 1;
                    }
                    while (query_y >= query_chunk->grad_point_count_y) {
                        // Use chunk below
                        current_world_y += 1;
                        assert(current_world_y < world->size);
                        query_chunk = world->chunks[current_world_x + current_world_y * world->size];
                        query_y = 0;
                    }
                    assert(query_x >= 0);
                    assert(query_x < query_chunk->grad_point_count_x);
                    assert(query_y >= 0);
                    assert(query_y < query_chunk->grad_point_count_y);

                    Renoise_Vector grad_point = query_chunk->grad_points[query_x + query_y * query_chunk->grad_point_count_x];
                    chunk->points[chunk_y][chunk_x] += perlin_falloff(grad_coord.x - grid_x, grad_coord.y - grid_y, grad_point);
                }
            }
        }
    }
}

void renoise_world_regenerate_rect(Renoise_World* world, int64_t chunk_x, int64_t chunk_y, int64_t width, int64_t height) {
    for (int64_t world_y = chunk_y; world_y < chunk_y + height; ++world_y) {
        for (int64_t world_x = chunk_x; world_x < chunk_x + width; ++world_x) {
            int64_t index = world_x + world_y * world->size;
            Renoise_Chunk* chunk = world->chunks[index];
            for (int64_t grad_y = 0; grad_y < chunk->grad_point_count_y; ++grad_y) {
                for (int64_t grad_x = 0; grad_x < chunk->grad_point_count_x; ++grad_x) {
                    // Skip outer gradient vectors
                    if (world_x == chunk_x && grad_x == 0) continue;
                    if (world_x == chunk_x + width - 1 && grad_x == chunk->grad_point_count_x - 1) continue;
                    if (world_y == chunk_y && grad_y == 0) continue;
                    if (world_y == chunk_y + height - 1 && grad_y == chunk->grad_point_count_y - 1) continue;

                    chunk->grad_points[grad_x + grad_y * chunk->grad_point_count_x] = renoise_gradient_point_generate();
                }
            }
        }
    }

    // Regenerate chunk points
    for (int64_t world_y = chunk_y; world_y < chunk_y + height; ++world_y) {
        if (world_y < 1 || world_y >= world->size - 1) continue;
        for (int64_t world_x = chunk_x; world_x < chunk_x + width; ++world_x) {
            if (world_x < 1 || world_x >= world->size - 1) continue;
            renoise_world_generate_chunk_points(world, world_x, world_y);
        }
    }
}

void renoise_world_regenerate_full_chunk(Renoise_World* world, int64_t chunk_x, int64_t chunk_y) {
    Renoise_Chunk* chunk = world->chunks[chunk_x + chunk_y * world->size];
    for (int64_t grad_y = 0; grad_y < chunk->grad_point_count_y; ++grad_y) {
        for (int64_t grad_x = 0; grad_x < chunk->grad_point_count_x; ++grad_x) {
            chunk->grad_points[grad_x + grad_y * chunk->grad_point_count_x] = renoise_gradient_point_generate();
        }
    }

    // Regenerate chunk points
    for (int64_t world_y = chunk_y - 1; world_y <= chunk_y + 1; ++world_y) {
        if (world_y < 1 || world_y >= world->size - 1) continue;
        for (int64_t world_x = chunk_x - 1; world_x <= chunk_x + 1; ++world_x) {
            if (world_x < 1 || world_x >= world->size - 1) continue;
            renoise_world_generate_chunk_points(world, world_x, world_y);
        }
    }
}
