#include <raylib.h>
#include <renoise.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main() {
    
    Renoise_World world = renoise_world_create(12, 0.2);
    renoise_world_generate(&world);

    printf("\n");
    for (uint64_t i = 0; i < 6; ++i) {
        Renoise_Chunk* chunk = &world.chunks[i];
        printf("chunks[%"PRIu64"] = { .grad_point_count_x = %"PRIu64", .grad_point_count_y = %"PRIu64", .x = %"PRIi64", .y = %"PRIi64", .grad_offset_x = %.16lf, .grad_offset_y = %lf }\n", i, chunk->grad_point_count_x, chunk->grad_point_count_y, chunk->x, chunk->y, chunk->grad_offset_x, chunk->grad_offset_y);
    }

    #define SCALE 5
    const int window_size = world.world_size * RENOISE_CHUNK_SIZE * SCALE + 1/world.frequency * SCALE * 2;
    InitWindow(window_size, window_size, "Renoise Example: Object Impermanence");
    SetTargetFPS(60);

    // Visual control booleans
    bool background = false;
    bool text = false;
    bool extra = false;
    bool grad_vectors = true;

    bool was_dragging = false;
    int64_t mouse_chunk_x_start = 0;
    int64_t mouse_chunk_y_start = 0;

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
            if (IsKeyPressed(KEY_V) || IsKeyPressed(KEY_G)) {
                grad_vectors = !grad_vectors;
            }
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
                        double xpos = off_x + (cx + chunk->grad_offset_x) / world.frequency * SCALE;
                        double ypos = off_y + (cy + chunk->grad_offset_y) / world.frequency * SCALE;
                        if (grad_vectors) DrawRectangle(xpos - SCALE/2, ypos - SCALE/2, SCALE, SCALE, WHITE);
                        if (grad_vectors) DrawLineEx(
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

            int64_t mouse_chunk_x = (GetMouseX() - 1/world.frequency * SCALE) / RENOISE_CHUNK_SIZE / SCALE;
            int64_t mouse_chunk_y = (GetMouseY() - 1/world.frequency * SCALE) / RENOISE_CHUNK_SIZE / SCALE;
            if (mouse_chunk_x < 0 || mouse_chunk_x >= (int64_t) world.world_size
             || mouse_chunk_y < 0 || mouse_chunk_y >= (int64_t) world.world_size) goto end_select;
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                renoise_world_regenerate_rect(
                    &world,
                    mouse_chunk_x_start,
                    mouse_chunk_y_start,
                    mouse_chunk_x - mouse_chunk_x_start + 1,
                    mouse_chunk_y - mouse_chunk_y_start + 1
                );
            } else if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                DrawRectangleLinesEx(
                    (Rectangle) {
                        mouse_chunk_x * RENOISE_CHUNK_SIZE * SCALE + 1/world.frequency * SCALE,
                        mouse_chunk_y * RENOISE_CHUNK_SIZE * SCALE + 1/world.frequency * SCALE,
                        RENOISE_CHUNK_SIZE * SCALE,
                        RENOISE_CHUNK_SIZE * SCALE,
                    },
                    SCALE/2.0,
                    RED
                );
                DrawText(
                    TextFormat("(%"PRIi64", %"PRIi64")", mouse_chunk_x, mouse_chunk_y),
                    mouse_chunk_x * RENOISE_CHUNK_SIZE * SCALE + 1/world.frequency * SCALE,
                    mouse_chunk_y * RENOISE_CHUNK_SIZE * SCALE + 1/world.frequency * SCALE,
                    24,
                    YELLOW
                );
            } else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                mouse_chunk_x_start = mouse_chunk_x;
                mouse_chunk_y_start = mouse_chunk_y;
            }
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                DrawRectangleLinesEx(
                    (Rectangle) {
                        mouse_chunk_x_start * RENOISE_CHUNK_SIZE * SCALE + 1/world.frequency * SCALE,
                        mouse_chunk_y_start * RENOISE_CHUNK_SIZE * SCALE + 1/world.frequency * SCALE,
                        (mouse_chunk_x - mouse_chunk_x_start + 1) * RENOISE_CHUNK_SIZE * SCALE,
                        (mouse_chunk_y - mouse_chunk_y_start + 1) * RENOISE_CHUNK_SIZE * SCALE,
                    },
                    SCALE/2.0,
                    RED
                );
                DrawText(
                    TextFormat("(%"PRIi64", %"PRIi64", %"PRIi64", %"PRIi64")", mouse_chunk_x_start, mouse_chunk_y_start, mouse_chunk_x - mouse_chunk_x_start + 1, mouse_chunk_y - mouse_chunk_y_start + 1),
                    mouse_chunk_x_start * RENOISE_CHUNK_SIZE * SCALE + 1/world.frequency * SCALE,
                    mouse_chunk_y_start * RENOISE_CHUNK_SIZE * SCALE + 1/world.frequency * SCALE,
                    24,
                    YELLOW
                );
            }
            end_select:
            DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}