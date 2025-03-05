#include <raylib.h>
#include <renoise.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main() {
    
    Renoise_World world = renoise_world_create(6, 0.2);

    Renoise_Chunk* chunk = &world.chunks[1];
    printf("\nchunks[1] = { .grad_point_count_x = %"PRIu64", .grad_point_count_y = %"PRIu64", .chunk_x = %"PRIi64", .chunk_y = %"PRIi64", .grad_offset_x = %lf, .grad_offset_y = %lf }\n", chunk->grad_point_count_x, chunk->grad_point_count_y, chunk->chunk_x, chunk->chunk_y, chunk->grad_offset_x, chunk->grad_offset_y);
    for (size_t i = 0; i < chunk->grad_point_count_x*chunk->grad_point_count_y; ++i) {
        printf("  chunks[1][%zu] = { %lf, %lf }\n", i, chunk->grad_points[i].x,chunk->grad_points[i].y);
    }
    // return 0;

    InitWindow(1920, 1080, "Hello, Raylib!");

    bool background = true;
    bool text = true;
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
                    if (background) DrawRectangle(
                        wx * RENOISE_CHUNK_SIZE * SCALE + 1/world.frequency * SCALE/2,
                        wy * RENOISE_CHUNK_SIZE * SCALE + 1/world.frequency * SCALE/2,
                        RENOISE_CHUNK_SIZE * SCALE,
                        RENOISE_CHUNK_SIZE * SCALE,
                        (Color) { 0, ((double) wx / (double) world.world_size) * 255, ((double) wy / (double) world.world_size) * 255, 255 }
                    );
                    for (uint64_t ci = 0; ci < chunk->grad_point_count_x*chunk->grad_point_count_y; ++ci) {
                        uint64_t cx = ci % chunk->grad_point_count_x;
                        uint64_t cy = ci / chunk->grad_point_count_x;
                        double xpos = (double) (x + cx) / world.frequency * SCALE + (chunk->grad_offset_x * SCALE) + 1/world.frequency * SCALE/2;
                        double ypos = (double) (y + cy) / world.frequency * SCALE + (chunk->grad_offset_y * SCALE) + 1/world.frequency * SCALE/2;
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
                            wx * RENOISE_CHUNK_SIZE * SCALE + 1/world.frequency * SCALE/2 + 10,
                            wy * RENOISE_CHUNK_SIZE * SCALE + 1/world.frequency * SCALE/2 + ci * SCALE + (text ? 24 : 0),
                            SCALE, YELLOW
                        );
                    }
                    if (text) DrawText(
                        TextFormat("(%"PRIu64", %"PRIu64")", chunk->grad_point_count_x, chunk->grad_point_count_y),
                        wx * RENOISE_CHUNK_SIZE * SCALE + 1/world.frequency * SCALE/2,
                        wy * RENOISE_CHUNK_SIZE * SCALE + 1/world.frequency * SCALE/2,
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