#include "framework.h"
#include <windowsx.h>

#include "Test.h"
#include "MazeRendering/MazeRendering.h"

using namespace PSEUDO_3D_MAZE;

/// <summary>
/// A Simplest Implementation using PSEUDO_3D_MAZE code
/// </summary>
namespace
{
	//Maze Map.
	//Define "whether or not each square is filled."
	//	0=empty space,  1=filled
	const uint8_t Map[4][6] = {
		{ 0,0,1,1,0,0 },
		{ 0,0,0,0,0,1 },
		{ 0,1,0,1,0,0 },
		{ 0,0,0,1,0,0 }
	};

	//This class simply refers the Map above
	class TheMaze : public MazeData<TheMaze>
	{
		friend class MazeData<TheMaze>;
	protected:
		EdgeAttr_t EdgeAttr_( int x, int y, Direction Dir ) const
		{
			//If the square(x,y) is looked from outside is not empty space, answering "Wall" for all 4 Edges of the square.
			//	With this implementation,
			//	when camera is positioned inside filled square, and looking neghbor square which is not filled,
			//	the "Wall" at the Edge between the 2 squares will not be rendered.
			return SquareVal(x,y);
		}

		static uint8_t SquareVal( int x, int y )
		{
			if( x<0 || y<0 || x>=6 || y>=4 )return 1;	//if out_of_range, returns "filled"
			else return Map[y][x];
		}
	};

	//Depth of Near-Plane and Far-Plane
	constexpr double Near = 0.1;
	constexpr double Far = 6.0;

	//Draw a vertical line on DC with GDI
	void DrawVLine( HDC hDC, const DrawEdgeParam &P )
	{
		HPEN OldPen = SelectPen( hDC, GetStockPen(DC_PEN) );

		int c = (int)std::round( 255*(1.0 - P.Depth/Far) );
		SetDCPenColor( hDC, RGB( c, 0, 255-c ) );
		MoveToEx( hDC, P.x, P.y_min, NULL );
		LineTo( hDC, P.x, P.y_max );

		SelectPen( hDC, OldPen );
	}
}

void Test01_Render( Vec2d CameraPos, double CameraYaw_rad, HDC hDC )
{
	static TheMaze Maze;
	static PinholeCamera Camera{ IMG_W, IMG_H, std::numbers::pi*0.5 };

	{//clear image
		RECT entire{ .left=0, .top=0, .right=IMG_W, .bottom=IMG_H };
		::FillRect( hDC, &entire, GetStockBrush(BLACK_BRUSH) );
	}

	RenderMazeEdge(
		Maze, Camera, {CameraPos[0],CameraPos[1],0.5}, CameraYaw_rad, Near, Far,
		[hDC]( const DrawEdgeParam &P ){	DrawVLine(hDC,P);	}
	);
}

