#include <raylib.h>
#include <renoise.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main() {
    
    Renoise_World world = renoise_world_create(6, 0.1);
    renoise_world_generate(&world);

    InitWindow(1920, 1080, "Hello, Raylib!");

    bool background = false;
    bool text = false;
    bool extra = false;

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(BLACK);
            if (IsKeyPressed(KEY_B)) {
                background = !background;
            }
            if (IsKeyPressed(KEY_T)) {
                text = !text;
            }
            if (IsKeyPressed(KEY_E)) {
                extra = !extra;
            }
            #define SCALE 10
            double y = 0;
            for (uint64_t wy = 0; wy < world.world_size; ++wy) {
                double x = 0;
                Renoise_Chunk* chunk = NULL;
                for (uint64_t wx = 0; wx < world.world_size; ++wx) {
                    uint64_t windex = wx + wy*world.world_size;
                    chunk = &world.chunks[windex];
                    double off_x = wx * RENOISE_CHUNK_SIZE * SCALE + 1/world.frequency * SCALE;
                    double off_y = wy * RENOISE_CHUNK_SIZE * SCALE + 1/world.frequency * SCALE;
                    for (uint8_t chunk_x = 0; chunk_x < RENOISE_CHUNK_SIZE; ++chunk_x) {
                        for (uint8_t chunk_y = 0; chunk_y < RENOISE_CHUNK_SIZE; ++chunk_y) {
                            uint8_t gray_val = (chunk->points[chunk_y][chunk_x] + 1.0) / 2.0 * 255;
                            DrawRectangle(
                                off_x + chunk_x * SCALE,
                                off_y + chunk_y * SCALE,
                                SCALE,
                                SCALE,
                                (Color) { gray_val, gray_val, gray_val, 255 }
                            );
                        }
                    }
                    if (background) DrawRectangle(
                        off_x,
                        off_y,
                        RENOISE_CHUNK_SIZE * SCALE,
                        RENOISE_CHUNK_SIZE * SCALE,
                        (Color) { 0, ((double) wx / (double) world.world_size) * 255, ((double) wy / (double) world.world_size) * 255, 127 }
                    );
                    for (uint64_t ci = 0; ci < chunk->grad_point_count_x*chunk->grad_point_count_y; ++ci) {
                        uint64_t cx = ci % chunk->grad_point_count_x;
                        uint64_t cy = ci / chunk->grad_point_count_x;
                        // double xpos = (double) (x + cx) / world.frequency * SCALE + (chunk->grad_offset_x * SCALE) + 1/world.frequency * SCALE;
                        // double ypos = (double) (y + cy) / world.frequency * SCALE + (chunk->grad_offset_y * SCALE) + 1/world.frequency * SCALE;
                        double xpos = off_x + (cx + chunk->grad_offset_x) / world.frequency * SCALE;
                        double ypos = off_y + (cy + chunk->grad_offset_y) / world.frequency * SCALE;
                        DrawRectangle(xpos - SCALE/2, ypos - SCALE/2, SCALE, SCALE, WHITE);
                        DrawLineEx(
                            (Vector2) { xpos, ypos },
                            (Vector2) {
                                xpos + chunk->grad_points[ci].x * 1/world.frequency * SCALE/2.0,
                                ypos + chunk->grad_points[ci].y * 1/world.frequency * SCALE/2.0,
                            },
                            SCALE/3.0,
                            RED
                        );
                        if (extra) DrawText(
                            TextFormat("%"PRIu64": { %"PRIu64", %"PRIu64" }; { %.02lf, %.02lf }", ci, cx, cy, xpos, ypos),
                            off_x + 10,
                            off_y + ci * SCALE + (text ? 24 : 0),
                            SCALE, YELLOW
                        );
                    }
                    if (text) DrawText(
                        TextFormat("(%"PRIu64", %"PRIu64")", chunk->grad_point_count_x, chunk->grad_point_count_y),
                        off_x,
                        off_y,
                        24, YELLOW
                    );

                    x += chunk->grad_point_count_x;
                }
                y += chunk->grad_point_count_y;
            }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}