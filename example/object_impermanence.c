#include <raylib.h>
#include <renoise.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main() {
    
    Renoise_Chunk chunk = renoise_chunk_generate(3, 6, 0.2);
    uint64_t grad_point_count = chunk.grad_point_count_x * chunk.grad_point_count_y;
    for (uint64_t i = 0; i < grad_point_count; ++i) {
        uint64_t x = i % chunk.grad_point_count_x;
        uint64_t y = i / chunk.grad_point_count_y;
        printf("%"PRIu64", %"PRIu64":  {%lf; %lf}\n", x, y, chunk.grad_points[i].x, chunk.grad_points[i].y);
    }

    return 0;

    InitWindow(800, 600, "Hello, Raylib!");

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RED);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}