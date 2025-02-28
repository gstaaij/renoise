#include <raylib.h>
#include <renoise.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main() {
    
    Renoise_Chunk chunk = renoise_chunk_generate(3, 6, 0.2);
    uint64_t grad_point_count = chunk.grad_point_count_x * chunk.grad_point_count_y;

    InitWindow(800, 600, "Hello, Raylib!");

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(BLACK);
            for (uint64_t i = 0; i < grad_point_count; ++i) {
                uint64_t x = i % chunk.grad_point_count_x;
                uint64_t y = i / chunk.grad_point_count_y;
                #define SCALE 200
                int xpos = x * SCALE + SCALE/2;
                int ypos = y * SCALE + SCALE/2;
                DrawRectangle(xpos - SCALE/20, ypos - SCALE/20, SCALE/10, SCALE/10, WHITE);
                DrawLineEx(
                    (Vector2) { xpos, ypos },
                    (Vector2) {
                        xpos + chunk.grad_points[i].x * SCALE/2,
                        ypos + chunk.grad_points[i].y * SCALE/2,
                    },
                    SCALE/20.0,
                    RED
                );
            }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}