#pragma once
#include "Parts/Vec.h"

//image size to be rendered
inline constexpr int IMG_W = 640;
inline constexpr int IMG_H = 480;

/// <summary>
/// Test
/// </summary>
/// <param name="CameraPos"></param>
/// <param name="CameraYaw_rad"></param>
/// <param name="hDC">canvas</param>
void Test01_Render( Vec2d CameraPos, double CameraYaw_rad, HDC hDC );
