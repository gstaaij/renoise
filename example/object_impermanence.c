#include <raylib.h>
#include <raymath.h>
#include <renoise.h>
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

    double player_world_x = RENOISE_CHUNK_SIZE * (VIEW_DISTANCE + 1);
    double player_world_y = player_world_x;
    Vector2 player_pos = (Vector2) { player_world_x * SCALE, player_world_y * SCALE };
    double player_angle = -45.0;

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
                }
            }


            // Update the "player"
            double player_angle_rad = M_PI * player_angle / 180.0;
            double fov_rad = M_PI * FOV / 180.0;
            double view_dist_pixels = VIEW_DISTANCE * RENOISE_CHUNK_SIZE * SCALE;
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                player_angle_rad = atan2(GetMouseY() - player_pos.y, GetMouseX() - player_pos.x);
                player_angle = player_angle_rad / M_PI * 180.0;
            }

            // Calculate frustum bounding box (for chunk collision)
            // This code is terrible
            double points_x[3];
            double points_y[3];
            points_x[0] = player_world_x;
            points_y[0] = player_world_y;
            points_x[1] = player_world_x + cos(player_angle_rad - fov_rad/2.0) * VIEW_DISTANCE * RENOISE_CHUNK_SIZE;
            points_y[1] = player_world_y + sin(player_angle_rad - fov_rad/2.0) * VIEW_DISTANCE * RENOISE_CHUNK_SIZE;
            points_x[2] = player_world_x + cos(player_angle_rad + fov_rad/2.0) * VIEW_DISTANCE * RENOISE_CHUNK_SIZE;
            points_y[2] = player_world_y + sin(player_angle_rad + fov_rad/2.0) * VIEW_DISTANCE * RENOISE_CHUNK_SIZE;
            double point_min_x =  INFINITY;
            double point_min_y =  INFINITY;
            double point_max_x = -INFINITY;
            double point_max_y = -INFINITY;
            for (int8_t i = 0; i < 3; ++i) {
                if (points_x[i] < point_min_x)
                    point_min_x = points_x[i];
                if (points_y[i] < point_min_y)
                    point_min_y = points_y[i];
                if (points_x[i] > point_max_x)
                    point_max_x = points_x[i];
                if (points_y[i] > point_max_y)
                    point_max_y = points_y[i];
            }

            // Draw the "player"
            DrawCircle(window_size/2, window_size/2, SCALE*2, ORANGE);
            DrawLineEx(
                player_pos,
                Vector2Add(player_pos, (Vector2) { cos(player_angle_rad - fov_rad/2.0) * view_dist_pixels, sin(player_angle_rad - fov_rad/2.0) * view_dist_pixels }),
                SCALE, ORANGE
            );
            DrawLineEx(
                player_pos,
                Vector2Add(player_pos, (Vector2) { cos(player_angle_rad + fov_rad/2.0) * view_dist_pixels, sin(player_angle_rad + fov_rad/2.0) * view_dist_pixels }),
                SCALE, ORANGE
            );
            DrawLineEx(
                Vector2Add(player_pos, (Vector2) { cos(player_angle_rad - fov_rad/2.0) * view_dist_pixels, sin(player_angle_rad - fov_rad/2.0) * view_dist_pixels }),
                Vector2Add(player_pos, (Vector2) { cos(player_angle_rad + fov_rad/2.0) * view_dist_pixels, sin(player_angle_rad + fov_rad/2.0) * view_dist_pixels }),
                SCALE, ORANGE
            );

            // Draw frustum bounding box
            DrawRectangleLinesEx(
                (Rectangle) {
                    point_min_x * SCALE,
                    point_min_y * SCALE,
                    (point_max_x - point_min_x) * SCALE,
                    (point_max_y - point_min_y) * SCALE,
                },
                SCALE/2.0, RED
            );

            DrawFPS(10, 10);
        EndDrawing();
    }
}
