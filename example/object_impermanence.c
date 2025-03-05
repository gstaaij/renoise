#include <raylib.h>
#include <renoise.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main() {
    
    Renoise_World world = renoise_world_create(5, 0.2);

    InitWindow(1920, 1080, "Hello, Raylib!");

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(BLACK);
            #define SCALE 40
            uint64_t y = 0;
            for (uint64_t wy = 0; wy < world.world_size; ++wy) {
                uint64_t x = 0;
                Renoise_Chunk* chunk = NULL;
                for (uint64_t wx = 0; wx < world.world_size; ++wx) {
                    uint64_t windex = wx + wy*world.world_size;
                    chunk = &world.chunks[windex];
                    DrawRectangle(
                        wx * RENOISE_CHUNK_SIZE * SCALE + SCALE/2,
                        wy * RENOISE_CHUNK_SIZE * SCALE + SCALE/2,
                        RENOISE_CHUNK_SIZE * SCALE + SCALE/2,
                        RENOISE_CHUNK_SIZE * SCALE + SCALE/2,
                        (Color) { ((double) wx / (double) world.world_size) * 255, 255, ((double) wy / (double) world.world_size) * 255, 127 }
                    );
                    DrawText(
                        TextFormat("(%"PRIu64", %"PRIu64")", chunk->grad_point_count_x, chunk->grad_point_count_y),
                        wx * RENOISE_CHUNK_SIZE * SCALE + SCALE/2,
                        wy * RENOISE_CHUNK_SIZE * SCALE + SCALE/2,
                        24, WHITE
                    );
                    for (uint64_t ci = 0; ci < chunk->grad_point_count_x*chunk->grad_point_count_y; ++ci) {
                        uint64_t cx = ci % chunk->grad_point_count_x;
                        uint64_t cy = ci / chunk->grad_point_count_y;
                        int xpos = (x + cx) / chunk->frequency * SCALE + (chunk->grad_offset_x * SCALE) + SCALE/2;
                        int ypos = (y + cy) / chunk->frequency * SCALE + (chunk->grad_offset_y * SCALE) + SCALE/2;
                        DrawRectangle(xpos - SCALE/20, ypos - SCALE/20, SCALE/10, SCALE/10, WHITE);
                        DrawLineEx(
                            (Vector2) { xpos, ypos },
                            (Vector2) {
                                xpos + chunk->grad_points[ci].x * SCALE/2,
                                ypos + chunk->grad_points[ci].y * SCALE/2,
                            },
                            SCALE/20.0,
                            RED
                        );
                    }
                    x += chunk->grad_point_count_x + chunk->grad_offset_x;
                }
                y += chunk->grad_point_count_y + chunk->grad_offset_y;
            }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}