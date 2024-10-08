#include "raylib.h"
#include "Math.h"

// Effectively GL_LINE_LOOP
void DrawLineLoop(Vector2* points, int pointCount, Color color)
{
    float thickness = 10.0f;
    for (int i = 0; i < pointCount; i++)
    {
        // Verify the values of curr and next in the debugger to help understand!
        int curr = i;
        int next = (i + 1) % pointCount;
        Vector2 A = points[curr];
        Vector2 B = points[next];
        DrawLineEx(A, B, thickness, color); 
    }
}

void DrawLineLoopAuto(Vector2* points, int pointCount, int createAmount)
{
    float thickness = 7.0f;
    Vector2 currentLayer[4];
    for (int i = 0; i < pointCount; i++) 
    {
        currentLayer[i] = points[i];
    }

    for (int k = 0; k < createAmount; k++)
    {
        Vector2 nextLayer[4];

        for (int i = 0; i < pointCount; i++)
        {
            int curr = i;
            int next = (i + 1) % pointCount;
            Vector2 A = currentLayer[curr];
            Vector2 B = currentLayer[next];

            DrawLineEx(A, B, thickness, RGB);

            nextLayer[curr] = (currentLayer[curr] + currentLayer[next]) * 0.5f;
        }
        for (int i = 0; i < pointCount; i++) 
        {
            currentLayer[i] = nextLayer[i];
        }
    }
} 

int main()
{
    InitWindow(800, 800, "Game");
    SetTargetFPS(60);
    float w = GetScreenWidth();
    float h = GetScreenHeight();
    float tc = 10.0f;

    Vector2 curr[4]
    {
        { 0.0f, 0.0f }, // top-left
        { w, 0.0f },    // top-right
        { w, h },       // bot-right
        { 0.0f, h }     // bot-left
    };

    Vector2 next[4]
    {
        (curr[0] + curr[1]) * 0.5f,
        (curr[1] + curr[2]) * 0.5f,
        (curr[2] + curr[3]) * 0.5f,
        (curr[3] + curr[0]) * 0.5f
    };

    Vector2 next1[4]
    {
        (next[0] + next[1]) * 0.5f,
        (next[1] + next[2]) * 0.5f,
        (next[2] + next[3]) * 0.5f,
        (next[3] + next[0]) * 0.5f
    };

    // **Consider prototyping the automatic (loop-based) vertex generation here before going to OpenGL!**
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("", 10, 10, 20, GRAY);
        //DrawLineLoop(curr, 4, VIOLET);
        //DrawLineLoop(next, 4, LIME);
        DrawLineLoopAuto(curr, 4, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
