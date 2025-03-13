#include <raylib.h>
#include <raymath.h>
#include <renoise.h>
#include <string.h>
#include <math.h>

int main(void) {
    Renoise_World* world = renoise_world_generate(8, 0.2);

    #define SCALE 8
    #define VIEW_DISTANCE 3
    #define FOV 90.0

    const int window_size = world->size * RENOISE_CHUNK_SIZE * SCALE;
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(window_size, window_size, "Renoise Example: Object Impermanence");
    SetTargetFPS(60);

    Renoise_Vector player_world_pos = {
        RENOISE_CHUNK_SIZE * (VIEW_DISTANCE + 1),
        RENOISE_CHUNK_SIZE * (VIEW_DISTANCE + 1),
    };
    Vector2 player_pos = (Vector2) { player_world_pos.x * SCALE, player_world_pos.y * SCALE };
    double player_angle = -45.0;
    bool seen_chunks[world->size][world->size];
    memset(seen_chunks, 0, sizeof(seen_chunks));

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(BLACK);
            // Draw the noise
            for (int64_t wy = 0; wy < world->size; ++wy) {
                for (int64_t wx = 0; wx < world->size; ++wx) {
                    int64_t windex = wx + wy*world->size;
                    Renoise_Chunk* chunk = world->chunks[windex];
                    double off_x = wx * RENOISE_CHUNK_SIZE * SCALE;
                    double off_y = wy * RENOISE_CHUNK_SIZE * SCALE;
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
                    if (seen_chunks[wy][wx]) {
                        DrawRectangle(
                            off_x,
                            off_y,
                            RENOISE_CHUNK_SIZE * SCALE,
                            RENOISE_CHUNK_SIZE * SCALE,
                            (Color) { 0, 228, 48, 63 }
                        );
                    }
                }
            }


            // Update the "player"
            double player_angle_rad = M_PI * player_angle / 180.0;
            double fov_rad = M_PI * FOV / 180.0;
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                player_angle_rad = atan2(GetMouseY() - player_pos.y, GetMouseX() - player_pos.x);
                player_angle = player_angle_rad / M_PI * 180.0;
            }

            // Calculate frustum bounding box (for chunk collision)
            Renoise_Vector frustum_points[3];
            frustum_points[0] = player_world_pos;
            frustum_points[1] = (Renoise_Vector) {
                player_world_pos.x + cos(player_angle_rad - fov_rad/2.0) * VIEW_DISTANCE * RENOISE_CHUNK_SIZE,
                player_world_pos.y + sin(player_angle_rad - fov_rad/2.0) * VIEW_DISTANCE * RENOISE_CHUNK_SIZE,
            };
            frustum_points[2] = (Renoise_Vector) {
                player_world_pos.x + cos(player_angle_rad + fov_rad/2.0) * VIEW_DISTANCE * RENOISE_CHUNK_SIZE,
                player_world_pos.y + sin(player_angle_rad + fov_rad/2.0) * VIEW_DISTANCE * RENOISE_CHUNK_SIZE,
            };
            Renoise_Vector frustum_point_min = {  INFINITY,  INFINITY };
            Renoise_Vector frustum_point_max = { -INFINITY, -INFINITY };
            for (int8_t i = 0; i < 3; ++i) {
                if (frustum_points[i].x < frustum_point_min.x)
                    frustum_point_min.x = frustum_points[i].x;
                if (frustum_points[i].y < frustum_point_min.y)
                    frustum_point_min.y = frustum_points[i].y;
                if (frustum_points[i].x > frustum_point_max.x)
                    frustum_point_max.x = frustum_points[i].x;
                if (frustum_points[i].y > frustum_point_max.y)
                    frustum_point_max.y = frustum_points[i].y;
            }

            // Mark chunks as "seen"
            for (int64_t chunk_index = 0; chunk_index < world->size*world->size; ++chunk_index) {
                Renoise_Chunk* chunk = world->chunks[chunk_index];
                // Use buffer so you can't see chunks changing (fully regenerating a chunk affects surrounding chunks)
                // static double buffer = RENOISE_CHUNK_SIZE/2;
                static double buffer = 0;
                if (
                    (chunk->x+1) * RENOISE_CHUNK_SIZE + buffer > frustum_point_min.x && (chunk->y+1) * RENOISE_CHUNK_SIZE + buffer > frustum_point_min.y &&
                    (chunk->x)   * RENOISE_CHUNK_SIZE - buffer < frustum_point_max.x && (chunk->y)   * RENOISE_CHUNK_SIZE - buffer < frustum_point_max.y
                ) {
                    seen_chunks[chunk->y][chunk->x] = true;
                } else if (seen_chunks[chunk->y][chunk->x]) {
                    seen_chunks[chunk->y][chunk->x] = false;
                    renoise_world_regenerate_full_chunk(world, chunk->x, chunk->y);
                }
            }


            // Draw the "player"
            DrawCircle(window_size/2, window_size/2, SCALE*2, ORANGE);
            DrawLineEx(
                player_pos,
                (Vector2) { frustum_points[1].x * SCALE, frustum_points[1].y * SCALE },
                SCALE, ORANGE
            );
            DrawLineEx(
                player_pos,
                (Vector2) { frustum_points[2].x * SCALE, frustum_points[2].y * SCALE },
                SCALE, ORANGE
            );
            DrawLineEx(
                (Vector2) { frustum_points[1].x * SCALE, frustum_points[1].y * SCALE },
                (Vector2) { frustum_points[2].x * SCALE, frustum_points[2].y * SCALE },
                SCALE, ORANGE
            );

            // Draw frustum bounding box
            DrawRectangleLinesEx(
                (Rectangle) {
                    frustum_point_min.x * SCALE,
                    frustum_point_min.y * SCALE,
                    (frustum_point_max.x - frustum_point_min.x) * SCALE,
                    (frustum_point_max.y - frustum_point_min.y) * SCALE,
                },
                SCALE/2.0, RED
            );

            DrawFPS(10, 10);
        EndDrawing();
    }
}
