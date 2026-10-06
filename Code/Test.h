#pragma once
#include "Parts/Vec.h"

//image size to be rendered
inline constexpr int IMG_W = 640;
inline constexpr int IMG_H = 480;

/// <summary>
/// Test-01 : Simple Maze Wall Rendering.
/// Test only the function `RenderMazeEdge()`
/// </summary>
/// <param name="CameraPos"></param>
/// <param name="CameraYaw_rad"></param>
/// <param name="hDC">canvas</param>
void Test01_Render( Vec2d CameraPos, double CameraYaw_rad, HDC hDC );

/// <summary>
/// Test-02 : Maze Wall and Wireframe Object.
/// Test the Combination of `RenderMazeEdge()` and `WFORenderer::Render()`
/// </summary>
/// <param name="CameraPos"></param>
/// <param name="CameraYaw_rad"></param>
/// <param name="hDC">canvas</param>
void Test02_Render( Vec2d CameraPos, double CameraYaw_rad, HDC hDC );
