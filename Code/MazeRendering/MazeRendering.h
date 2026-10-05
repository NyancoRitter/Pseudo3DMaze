#pragma once
#include "MazeData.h"
#include "Parts/PinholeCamera.h"
#include <concepts>
#include "RayCast.h"

namespace PSEUDO_3D_MAZE
{
	/// <summary>
	/// Argument type for the line_drawing_callback_function called from the rendering function RenderMazeEdge().
	///		<remarks>
	///		Wall-like face is rendered as collection of vertical lines.
	///		This type is info for drawing 1 vertical line.
	///		* Indicating a (1-pixel thickness) line (x,y_min)-(x,y_max) should be drawn.
	///		* Some other infos provided.
	///		</remarks>
	/// </summary>
	struct DrawEdgeParam
	{
		/// <summary>x-position[pixel] of the line should be drawn</summary>
		int x;
		/// <summary>y-range[pixel] of the line should be drawn (always y_min less_eq y_max）</summary>
		int y_min, y_max;

		/// <summary>Edge Attribute of the render target Wall-like face</summary>
		EdgeAttr_t EdgeAttr;
		/// <summary>Depth (from Camera pos)</summary>
		double Depth;
		/// <summary>Square distance(from Camera Pos; in 2D world)</summary>
		double SqDist;
		/// <summary>
		/// (Horizontal dirrectional) Position of the line in the render target Wall-like face. 
		///		Leftmost=0.0 ~ 1.0=Rightmost
		///		<remarks>info for process likes texture-mapping</remarks>
		/// </summary>
		double LRPosRateOnEdge;
	};

	/// <summary>
	/// Maze (Edge only, no other: floor and ceil) rendering via pinhole-camera-model
	///		<remarks>Maybe, called as "Ray Casting" ?</remarks>
	/// </summary>
	/// <param name="Maze">Maze Edge info provider</param>
	/// <param name="Camera">Pinhole-Camera-Model</param>
	/// <param name="CameraPos">
	/// Camera Position (X,Y, height).
	/// * height should be in range [0.0, 1.0]. (Otherwise, may cause no good rendering result.)
	///		Floor=0.0 ~ 1.0=Ceil
	/// </param>
	/// <param name="CameraYaw_rad">Camera direction[rad] (see "Direction.h" for definition)</param>
	/// <param name="Near">Near-Plane distance from Camera</param>
	/// <param name="Far">Far-Plane distance from Camera</param>
	/// <param name="DrawVLineFunc">
	/// Callback function.
	/// A method for drawing a vertical line as part of an Edge(Wall-like face).
	/// This function is called at most once for each x-coordinate[pixel].
	/// </param>
	template< class MazeData_t, class DrawVerticalLineFunc >
		requires std::derived_from< MazeData_t, MazeData<MazeData_t> > &&
		         std::invocable< DrawVerticalLineFunc, const DrawEdgeParam & >
	void RenderMazeEdge(
		const MazeData_t &Maze,
		const PinholeCamera &Camera,
		Vec3d CameraPos,
		double CameraYaw_rad,
		double Near,
		double Far,
		DrawVerticalLineFunc DrawVLineFunc
	)
	{
		constexpr double WallHeight = 1.0;	//Floor-to-ceiling height

		const Vec2d FrontDir{ std::cos(CameraYaw_rad), std::sin(CameraYaw_rad) };	//Camera front directional Unit vector
		const Vec2d RightDir{ -FrontDir[1], FrontDir[0] }; //Camera rhs directional Unit vector
		const Vec2d CamPos = { CameraPos[0], CameraPos[1] };	//Camera pos (2D)
		const double CamHeight = CameraPos[2];

		for( int Pix_x=0; Pix_x<Camera.ImgW(); ++Pix_x )
		{
			const Vec2d Ray1 = FrontDir  +  Camera.Px_2_Cx( Pix_x ) * RightDir;	//intercection between Ray and Depth=1 plane
			const Vec2d RayNear = CamPos + Ray1*Near;	//intercection betewwn Ray and Near-Plane
			const Vec2d RayFar = CamPos + Ray1*Far;	//intercection betewwn Ray and Far-Plane

			if( std::optional<Impl::RayCastResult> RayCastResult_ = Impl::RayCast( RayNear, RayFar, Maze );	RayCastResult_ )
			{
				const Vec2d dPos = RayCastResult_->Pos - CamPos;
				DrawEdgeParam Params;
				Params.x = Pix_x;
				Params.EdgeAttr = RayCastResult_->EdgeAttr;
				Params.LRPosRateOnEdge = RayCastResult_->LRPosRateOnEdge;
				Params.Depth = dot( dPos, FrontDir );
				Params.SqDist = dPos.SqL2Norm();
				Params.y_min = (int)std::round( Camera.CyCz_2_Py( -(WallHeight - CamHeight), Params.Depth ) );
				Params.y_max = (int)std::round( Camera.CyCz_2_Py( CamHeight, Params.Depth ) );
				DrawVLineFunc( Params );
			}
		}
	}

}

//[EOF]