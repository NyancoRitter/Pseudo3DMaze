#include "framework.h"
#include <windowsx.h>

#include "Test03.h"
#include "MazeRendering/MazeRendering.h"
#include "WireframeObj/WFORenderer.h"

#include <string>
#include <functional>
#include <span>

using namespace PSEUDO_3D_MAZE;
using namespace Wireframe;

/// <summary>Maze Map Definition</summary>
namespace
{
	//---
	//Edges
	//  # : Wall
	//  X : Door
	//  < : One-way. Is wall when seen from west side.
	//  > : One-way. Is wall when seen from east side.
	//  ^ : One-way. Is wall when seen from north side.
	//  v : One-way. Is wall when seen from south side.
	//  else : there is nothing.
	//Square
	//	* : there is object.
	//---
	//Note :
	//  Edges have no thickness.
	//	So, depending on the data, the rendering result may become unnatural (the 0-thickness will be exposed).
	const std::string MapDefStrs[] =
	{
		//0 1 2 3 4 5
		"#############",
		"# | < | | | #", //0
		"#-+-#####-###",
		"# |*| | > X #", //1
		"#-#X#^###-#X#",
		"# # X # X*|*#", //2
		"#-###v#-#-+-#",
		"# | | X # |*#", //3
		"#############"
	};
	
	constexpr EdgeAttr_t NONE = 0;
	constexpr EdgeAttr_t WALL = 1;
	constexpr EdgeAttr_t DOOR = 2;

	inline EdgeAttr_t EastAttr( char c )
	{
		if( c=='#' || c=='>' )return WALL;
		return ( c=='X' ? DOOR : NONE );
	}

	inline EdgeAttr_t WestAttr( char c )
	{
		if( c=='#' || c=='<' )return (unsigned char)WALL;
		return ( c=='X' ? DOOR : NONE );
	}

	inline EdgeAttr_t NorthAttr( char c )
	{
		if( c=='#' || c=='^' )return (unsigned char)WALL;
		return ( c=='X' ? DOOR : NONE );
	}

	inline EdgeAttr_t SouthAttr( char c )
	{
		if( c=='#' || c=='v' )return (unsigned char)WALL;
		return ( c=='X' ? DOOR : NONE );
	}

	//---
	class TheMaze : public MazeData<TheMaze>
	{
		friend class MazeData<TheMaze>;
	public:
		//constructed from the above format data
		TheMaze( std::span<const std::string> MapDefStrLines )
		{
			//---
			//interpret the argument
			if( MapDefStrLines.size() < 3 ){	throw std::invalid_argument( "insufficient arg size" );	}
			m_YSize = MapDefStrLines.size() / 2;
			if( MapDefStrLines.size() != (2*m_YSize + 1) ){	throw std::invalid_argument( "invalid arg size" );	}

			m_XSize = MapDefStrLines[0].size() / 2;
			for( size_t i=0; i<MapDefStrLines.size(); ++i )
			{
				if( MapDefStrLines[i].size() != (2*m_XSize + 1) )
				{	throw std::invalid_argument( "Size of arg[" + std::to_string(i) + "] is invalid" );	}
			}

			m_MapEdges.resize( m_YSize*m_XSize*4 );
			for( size_t y=0; y<m_YSize; ++y )
			{
				const size_t iy = 1 + 2*y;
				for( size_t x=0; x<m_XSize; ++x )
				{
					const size_t ix = 1 + 2*x;
					const size_t BaseIndex = ( y*m_XSize + x )*4;
					m_MapEdges[ BaseIndex + (size_t)Direction::EAST ] = EastAttr( MapDefStrLines[iy][ix+1] );
					m_MapEdges[ BaseIndex + (size_t)Direction::SOUTH ] = SouthAttr( MapDefStrLines[iy+1][ix] );
					m_MapEdges[ BaseIndex + (size_t)Direction::WEST ] = WestAttr( MapDefStrLines[iy][ix-1] );
					m_MapEdges[ BaseIndex + (size_t)Direction::NORTH ] = NorthAttr( MapDefStrLines[iy-1][ix] );

					if( MapDefStrLines[iy][ix] == '*' )
					{	m_WFObjPositions.push_back( {x,y} );	}
				}
			}

			//---
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
			if( m_upWFObj  &&  !m_WFObjPositions.empty() )
			{
				for( const auto &Pos : m_WFObjPositions )
				{	Renderer.TryAdd( Pos, *m_upWFObj, {255, 255, 255} );	}
			}
		}

	protected:
		EdgeAttr_t EdgeAttr_( int x, int y, Direction Dir ) const
		{
			if( x<0 || y<0 || x>=m_XSize || y>=m_YSize ){	return WALL;	}	//if out_of_range, returns WALL
			return m_MapEdges[ ( y*m_XSize + x )*4 + (size_t)Dir ];
		}

	private:
		size_t m_XSize = 0;
		size_t m_YSize = 0;
		std::vector< EdgeAttr_t > m_MapEdges;
		std::vector< Vec2i > m_WFObjPositions;
		std::unique_ptr<WireframeObj> m_upWFObj;
	};
}

namespace Test03
{
	//Depth of Near-Plane and Far-Plane
	constexpr double Near = 0.1;
	constexpr double Far = 6.0;

	//Worker func used in DrawVLine()
	static inline COLORREF WallColor( double Rate )
	{	return RGB( (int)(255*Rate), (int)(30*Rate), (int)(10*Rate) );	}

	//Worker func used in DrawVLine()
	static inline COLORREF DoorColor( double Rate )
	{	return RGB( (int)(150*Rate), (int)(100*Rate), (int)(50*Rate) );	}

	using DepthBuff_t = std::array<double, IMG_W>;

	//Draw a vertical line on DC with GDI
	//and update DepthBuff.
	void DrawVLine( HDC hDC, const DrawEdgeParam &P, DepthBuff_t &DepthBuff )
	{
		HPEN OldPen = SelectPen( hDC, GetStockPen(DC_PEN) );

		const double ColorRate = (1.0 - P.Depth/Far);
		const COLORREF WallCol = WallColor( ColorRate * ( std::abs(P.LRPosRateOnEdge-0.5)>=0.49 ? 0.8 : 1.0 ) );

		if( P.EdgeAttr==DOOR  &&  std::abs(P.LRPosRateOnEdge-0.5)<=0.3 )
		{//draw a brown rect (intending door texture!)
			const int DoorTop_y = (int)std::round( 0.7*P.y_min + 0.3*P.y_max );
			
			SetDCPenColor( hDC, WallCol );
			MoveToEx( hDC, P.x, P.y_min, NULL );
			LineTo( hDC, P.x, DoorTop_y );

			SetDCPenColor( hDC, DoorColor(ColorRate) );
			LineTo( hDC, P.x, P.y_max );
		}
		else
		{
			SetDCPenColor( hDC, WallCol );
			MoveToEx( hDC, P.x, P.y_min, NULL );
			LineTo( hDC, P.x, P.y_max );
		}

		SelectPen( hDC, OldPen );

		DepthBuff[ P.x ] = P.Depth;
	}

	//Draw a line seg (a wire of Wireframe object)
	void DrawWire( HDC hDC, const DrawWireParam &P, const DepthBuff_t &DepthBuff )
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
		const auto CalcCol = [ColRate]( double RawCol ){	return (int)std::round(RawCol * ColRate);	};
		SetDCPenColor( hDC, RGB( CalcCol(P.ObjColor[0]), CalcCol(P.ObjColor[1]), CalcCol(P.ObjColor[2]) ) );

		MoveToEx( hDC, PixPos[0][0], PixPos[0][1], NULL );
		LineTo( hDC, PixPos[1][0], PixPos[1][1] );

		SelectPen( hDC, OldPen );
	}

	/// <summary>
	/// Helper for animation, used in GameLike::Impl.
	/// Execute the specified function only specified times.
	/// </summary>
	class Repeat
	{
	public:
		Repeat( unsigned int nTimes, std::function<void(void)> f )
			: m_nRest(nTimes), m_f( std::move(f) )
		{}

		bool operator()()
		{
			if( !AlreadyCompleted() )
			{
				m_f();
				--m_nRest;
			}
			return AlreadyCompleted();
		}

		bool AlreadyCompleted() const {	return m_nRest==0;	}
	private:
		unsigned int m_nRest;
		std::function<void(void)> m_f;
	};

	/// <summary>GameLike::Impl</summary>
	class GameLike::Impl
	{
	public:
		Impl() : m_Maze(MapDefStrs) {	ResetCamera();	}

	public:
		bool Update( const Controller &Ctrller )
		{
			if( m_TaskInProgress )
			{//during animation
				if( m_TaskInProgress() )
				{//animation completed
					m_TaskInProgress = nullptr;
					ResetCamera();
				}
				return true;
			}

			if( Ctrller.Pressed( Key::GoForward ) ){	return StartMove( m_Dir );	}	
			if( Ctrller.Pressed( Key::GoBackward ) ){	return StartMove( OppositeDirOf(m_Dir) );	}
			if( Ctrller.Pressed( Key::GoLeft ) ){	return StartMove( LeftDirOf(m_Dir) );	}
			if( Ctrller.Pressed( Key::GoRight ) ){	return StartMove( RightDirOf(m_Dir) );	}
			if( Ctrller.Pressed( Key::TurnLeft ) ){	return StartTurn( false );	}
			if( Ctrller.Pressed( Key::TurnRight ) ){	return StartTurn( true );	}

			return false;
		}

		void Render( HDC hDC ) const
		{
			{//clear image
				RECT entire{ .left=0, .top=0, .right=IMG_W, .bottom=IMG_H };
				::FillRect( hDC, &entire, GetStockBrush(BLACK_BRUSH) );
			}

			//clear 1D_Depth_Buffer before RenderMazeEdge()
			DepthBuff_t DepthBuff;
			DepthBuff.fill( Far );

			//Here, at rendering,
			//viewpoint position offsetted backward a bit is used. (rather than raw current camera position)
			const Vec3d OffsettedCamPos = m_CameraPos - 0.3* Vec3d{ std::cos(m_CameraYaw_rad), std::sin(m_CameraYaw_rad), 0.0 };

			//Render the Maze Edge (1D_Depth_Buffer is updated)
			RenderMazeEdge(
				m_Maze, m_Camera, OffsettedCamPos, m_CameraYaw_rad, Near, Far,
				[hDC, &DepthBuff]( const DrawEdgeParam &P ){	DrawVLine(hDC,P,DepthBuff);	}
			);
			//Render Wireframe objects using the 1D_Depth_Buffer
			WFORenderer Renderer{ m_Camera, OffsettedCamPos, m_CameraYaw_rad, Near, Far };
			m_Maze.RegisterObjsToWFORenderer( Renderer );
			Renderer.Render(
				[hDC, &DepthBuff]( const DrawWireParam &P ){	DrawWire(hDC,P,DepthBuff);	}
			);
		}

	private:
		/// <summary>Reset m_CameraPos/m_CameraYaw_rad to the value corresponding to m_Pos/m_Dir.</summary>
		void ResetCamera()
		{
			constexpr double CamHeight = 0.5;
			m_CameraPos = Vec3d( m_Pos[0]+0.5, m_Pos[1]+0.5, CamHeight );
			m_CameraYaw_rad = AngleOf( m_Dir );
		}

		/// <summary>Move to neighbor square if possible.</summary>
		/// <param name="Dir">Move direction</param>
		/// <returns>always false (for convenience)</returns>
		bool StartMove( Direction Dir )
		{
			constexpr Vec2i dPos[4] = { {1,0}, {0,1}, {-1,0}, {0,-1} };
			constexpr unsigned int nFrames = 16;

			const auto To = m_Pos + dPos[ (size_t)Dir ];
			//check if Wall prevents the move
			if( m_Maze.EdgeAttr( To[0], To[1], OppositeDirOf(Dir) ) == WALL )return false;
			//Update
			m_Pos = To;

			//Start animation
			const double dir_rad = AngleOf(Dir);
			const Vec3d UnitDirVec = { std::cos(dir_rad), std::sin(dir_rad), 0.0 };
			m_TaskInProgress = Repeat(
				nFrames,
				[this, MovePerFrame=UnitDirVec/(double)nFrames](){	m_CameraPos += MovePerFrame;	}
			);
			return false;
		}

		/// <summary>Turn</summary>
		/// <param name="ToRight">
		/// Turning direction.
		///		true = to right dir, false = to left dir
		/// </param>
		/// <returns>always false (for convenience)</returns>
		bool StartTurn( bool ToRight )
		{
			constexpr unsigned int nFrames = 16;

			//Update
			m_Dir = ( ToRight ? RightDirOf(m_Dir) : LeftDirOf(m_Dir) );

			//Start animation
			const double TurnAngle = 0.5*( ToRight ? std::numbers::pi : -std::numbers::pi );
			m_TaskInProgress = Repeat(
				nFrames,
				[this, RotPerFrame=TurnAngle/nFrames](){	m_CameraYaw_rad += RotPerFrame;	}
			);
			return false;
		}

	private:
		TheMaze m_Maze;	//Maze Data
		PinholeCamera m_Camera{ IMG_W, IMG_H, std::numbers::pi*0.5 };	//Camera Model

		//Position and Pose of "Player"
		Vec2i m_Pos{ 0,0 };
		Direction m_Dir = Direction::EAST;

		//Position and Pose for rendering
		Vec3d m_CameraPos{ 0.5, 0.5, 0.5 };
		double m_CameraYaw_rad = 0;

		//Move/Turn Animation
		std::function<bool()> m_TaskInProgress;
	};

	//---
	GameLike::GameLike() : m_upImpl( std::make_unique<GameLike::Impl>() ) {}
	GameLike::~GameLike(){}
	bool GameLike::Update( const Controller &Ctrller ){	return m_upImpl->Update(Ctrller);	}
	void GameLike::Render( HDC hDC ) const {	m_upImpl->Render(hDC);	}
}

