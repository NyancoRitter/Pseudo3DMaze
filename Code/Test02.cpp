#include "framework.h"
#include <windowsx.h>

#include "Test.h"
#include "MazeRendering/MazeRendering.h"
#include "WireframeObj/WFORenderer.h"

#include <memory>

using namespace PSEUDO_3D_MAZE;
using namespace Wireframe;

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
	public:
		TheMaze()
		{
			//Create Test Object here
			WFObjDef Def;
			Def.Vtxs = {
				{ 0.3, 0.3, 0.3 },	//0
				{ 0.7, 0.3, 0.3 },
				{ 0.7, 0.7, 0.3 },
				{ 0.3, 0.7, 0.3 },
				{ 0.5, 0.5, 0.9 },	//4
				{ 0.5, 0.5, 0.0 }	//5
			};
			Def.Faces = {
				{ 4, 0, 1 }, { 4, 1, 2 }, { 4, 2, 3 }, { 4, 3, 0 },
				{ 5, 1, 0 }, { 5, 2, 1 }, { 5, 3, 2 }, { 5, 0, 3 }
			};

			m_upWFObj = std::make_unique<WireframeObj>( std::move(Def) );
		};

	public:
		void RegisterObjsToWFORenderer( WFORenderer &Renderer ) const
		{
			if( m_upWFObj )
			{//Put 2 objs
				Renderer.TryAdd( {1,1}, *m_upWFObj, {1.0, 0.75, 0.0} );
				Renderer.TryAdd( {3,1}, *m_upWFObj, {0.0, 0.75, 1.0} );
			}
		}

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

	private:
		std::unique_ptr<WireframeObj> m_upWFObj;
	};

	//Depth of Near-Plane and Far-Plane
	constexpr double Near = 0.1;
	constexpr double Far = 6.0;

	//1D_Depth_Buffer :
	//Collecting depth value for each x[pixel] during Maze Edge Rendering.
	//Then, this info is used in object rendering.
	std::array<double, IMG_W> DepthBuff;

	//Draw a vertical line on DC with GDI
	void DrawVLine( HDC hDC, const DrawEdgeParam &P )
	{
		HPEN OldPen = SelectPen( hDC, GetStockPen(DC_PEN) );

		int c = (int)std::round( 255*(1.0 - P.Depth/Far) );
		SetDCPenColor( hDC, RGB( c, 0, 255-c ) );
		MoveToEx( hDC, P.x, P.y_min, NULL );
		LineTo( hDC, P.x, P.y_max );

		SelectPen( hDC, OldPen );

		DepthBuff[ P.x ] = P.Depth;
	}

	//Draw a line seg (a wire of Wireframe object)
	void DrawWire( HDC hDC, const DrawWireParam &P )
	{
		int L = (int)std::round( P.V[0][0] );
		int R = (int)std::round( P.V[1][0] );

		Vec2i PixPos[2] = {
			{ L, (int)std::round(P.V[0][1]) },
			{ R, (int)std::round(P.V[1][1]) }
		};

		if( L==R )
		{
			if( P.V[0][2] >= DepthBuff[L] )return;
		}
		else if( P.V[0][2]>=DepthBuff[L]  ||  P.V[1][2]>=DepthBuff[R] )
		{//Extract a partial range of line seg which depth is less than Depth Buffer Value.
			auto d = ( P.V[1] - P.V[0] ) / double(R-L);
			if( P.V[0][2]>=DepthBuff[L] )
			{
				double depth = P.V[0][2];
				do
				{
					++L;
					depth += d[2];
					if( depth < DepthBuff[L] )
					{
						PixPos[0][0] = L;
						PixPos[0][1] = (int)std::round( P.V[0][1] + (L - P.V[0][0])*d[1] );
						break;
					}
				}
				while( L < R );
				if( L>=R )return;
			}
			if( P.V[1][2]>=DepthBuff[R] )
			{
				double depth = P.V[1][2];
				do
				{
					--R;
					depth -= d[2];
					if( depth < DepthBuff[R] )
					{
						PixPos[1][0] = R;
						PixPos[1][1] = (int)std::round( P.V[0][1] + (R - P.V[0][0])*d[1] );
						break;
					}
				}
				while( L < R );
				if( L>=R )return;
			}
		}

		//
		HPEN OldPen = SelectPen( hDC, GetStockPen(DC_PEN) );

		const double ColRate = 1.0 - std::sqrt(P.ObjCenterSqDist) / Far;
		const auto CalcCol = [ColRate]( double RawCol ){	return (int)std::round(RawCol * ColRate * 255.0);	};
		SetDCPenColor( hDC, RGB( CalcCol(P.ObjColor[0]), CalcCol(P.ObjColor[1]), CalcCol(P.ObjColor[2]) ) );

		MoveToEx( hDC, PixPos[0][0], PixPos[0][1], NULL );
		LineTo( hDC, PixPos[1][0], PixPos[1][1] );

		SelectPen( hDC, OldPen );
	}
}

void Test02_Render( Vec2d CameraPos, double CameraYaw_rad, HDC hDC )
{
	static TheMaze Maze;
	static PinholeCamera Camera{ IMG_W, IMG_H, std::numbers::pi*0.5 };

	{//clear image
		RECT entire{ .left=0, .top=0, .right=IMG_W, .bottom=IMG_H };
		::FillRect( hDC, &entire, GetStockBrush(BLACK_BRUSH) );
	}

	const Vec3d CamPos3D{ CameraPos[0], CameraPos[1], 0.75 };

	//clear 1D_Depth_Buffer before RenderMazeEdge()
	DepthBuff.fill( Far );
	//Render the Maze Edge (1D_Depth_Buffer is updated)
	RenderMazeEdge(
		Maze, Camera, CamPos3D, CameraYaw_rad, Near, Far,
		[hDC]( const DrawEdgeParam &P ){	DrawVLine(hDC,P);	}
	);

	//Render Wireframe objects using the 1D_Depth_Buffer
	WFORenderer Renderer{ Camera, CamPos3D, CameraYaw_rad, Near, Far };
	Maze.RegisterObjsToWFORenderer( Renderer );
	Renderer.Render(
		[hDC]( const DrawWireParam &P ){	DrawWire(hDC,P);	}
	);
}
