#include <raylib.h>
#include <renoise.h>

int main() {
    
    InitWindow(800, 600, "Hello, Raylib!");

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RED);
        DrawText(rn_test(), 10, 10, 24, WHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}