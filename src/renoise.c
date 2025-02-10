#include "renoise.h"
#include <math.h>

Renoise_Gradient_Point renoise_gradient_point_generate() {
    double angle = ((double) random() / 2147483647.0) * M_2_PI;
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
    double grad_point_size = RENOISE_CHUNK_SIZE * frequency;
    double grad_point_size_decimal = fmod(grad_point_size, 1.0);
    chunk.grad_offset_x = fmod(chunk_x * grad_point_size_decimal, 1.0);
    chunk.grad_offset_y = fmod(chunk_y * grad_point_size_decimal, 1.0);
    chunk.grad_point_count_x = ceil(grad_point_size - chunk.grad_offset_x);
    chunk.grad_point_count_y = ceil(grad_point_size - chunk.grad_offset_y);

    uint64_t grad_point_count = chunk.grad_point_count_x * chunk.grad_point_count_y;
    chunk.grad_points = malloc(grad_point_count * sizeof(*chunk.grad_points));
    for (uint64_t i = 0; i < grad_point_count; ++i) {
        chunk.grad_points[i] = renoise_gradient_point_generate();
    }

    // TODO: generate points (maybe in other function?)
    return chunk;
}
