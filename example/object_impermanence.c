#include <raylib.h>
#include <renoise.h>
#include <stdio.h>

int main() {
    
    Renoise_Gradient_Point gp = renoise_gradient_point_generate();
    printf("{%lf; %lf}", gp.x, gp.y);

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