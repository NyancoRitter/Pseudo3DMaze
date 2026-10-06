#pragma once
#include "WireframeObj.h"
#include "../Parts/PinholeCamera.h"
#include <algorithm>
#include <map>

class PinholeCamera;

namespace Wireframe
{
	/// <summary>
	/// Argument type for the line_drawing_callback_function called from Renderer::Render().
	///		<remarks>
	///		Rendering a Wireframe Object is rendering a set of wires(: line segments).
	///		This type is info for drawing 1 wire.
	///		* Indicating endpoints of line seqment should be drawn.
	///		* Some other infos provided.
	///		</remarks>
	/// </summary>
	struct DrawWireParam
	{
		/// <summary>
		/// Pixel position and depth of 2 endpoints of the line segment; as :
		///		{ x[pixel], y[pixel], depth from Camera }
		/// Sorted by x. ( V[0][0] less eq V[1][0] )
		/// </summary>
		Vec3d V[2];

		/// <summary>Squared distance between Camera and Object center in 2D(XY) coordinate (not 3D).</summary>
		double ObjCenterSqDist;
		/// <summary>Color value passed to Renderer::TryAdd()</summary>
		Vec3d ObjColor;
	};

	/// <summary>
	/// Wireframe Object Readering
	/// 
	/// Usage:
	/// 1. Construct for the specific setup. (CameraModel, Camera Position and rotation, etc)
	/// 2. Using TryAdd(), register all Wireframe Objects may be rendered in the current situation.
	/// 3. Finally, call Render().
	/// </summary>
	class WFORenderer
	{
	public:
		/// <summary>ctor</summary>
		/// <param name="Camera">Pinhole-Camera-Model</param>
		/// <param name="CameraPos">Camera Position (X,Y, height).</param>
		/// <param name="CameraYaw_rad">Camera direction[rad] (see "Direction.h" for definition)</param>
		/// <param name="Near">Near-Plane distance from Camera</param>
		/// <param name="Far">Far-Plane distance from Camera</param>
		WFORenderer(
			const PinholeCamera &Camera,
			const Vec3d &CameraPos,
			double CameraYaw_rad,
			double Near,
			double Far
		);

	public:
		/// <summary>
		/// Register information about a single object present in the current scene.
		/// (Should be called multiple times to register objects one by one.)
		/// </summary>
		/// <param name="iSquare">Index of the square where the object is located</param>
		/// <param name="WFObj">Wireframe Object</param>
		/// <param name="Color">Drawing color info</param>
		/// <returns>
		/// Indicates whether or not the information was actually stored internally.
		/// However, the user does not need to check this return value.
		/// </returns>
		bool TryAdd( Vec2i iSquare, const WireframeObj &WFObj, Vec3d Color );

		/// <summary>
		/// Render the registered objects.
		/// </summary>
		/// <param name="DrawFunc">
		/// Callback function.
		/// A method for drawing a line seg as part of Wireframe Object.
		/// </param>
		template< std::invocable< const DrawWireParam & > DrawWireFunc >
		void Render( DrawWireFunc DrawFunc ) const
		{
			if( m_RenderTgts.empty() )return;

			//Coordinate Trans Mat from Object-Definition-Coord to Camera-Coord
			const Vec3d CTM[3] =
			{
				{ m_RightDir, 0.0 },
				{ 0.0, 0.0, -1.0 },
				{ m_FrontDir, 0.0 }
			};
			//Coordinate Trans via Mat above
			const auto ToCamCoord = [&CTM]( const Vec3d &v )->Vec3d{	return { dot(CTM[0],v), dot(CTM[1],v), dot(CTM[2],v) };	};

			//Camera 3D Pos
			const Vec3d CamPos3D{	m_CamPos, m_CamHeight };

			//Sort to render objects starting from the farthest one
			std::sort(
				m_RenderTgts.begin(), m_RenderTgts.end(),
				[](auto &lhs, auto &rhs)->bool{	return (lhs.SqDist > rhs.SqDist);	}
			);

			for( auto &Tgt : m_RenderTgts )
			{
				const auto &WFObj = *Tgt.pWFObj;
				const Vec3d Offset = Vec3d( Tgt.iSquare[0], Tgt.iSquare[1], 0.0 ) - CamPos3D;

				//Calculate Vertex pos in Camera-Coord
				std::vector< Vec3d > Vtxs;
				{
					Vtxs.reserve( WFObj.Vtxs.size() );
					for( const auto &v : WFObj.Vtxs )
					{	Vtxs.push_back( ToCamCoord( v + Offset ) );	}
				}

				//Redner Wires
				for( const auto &wire : WFObj.Wires )
				{
					Vec3d V[2] = { Vtxs[wire.VtxIdxs.first], Vtxs[wire.VtxIdxs.second] };

					//trim with depth range
					if( !TrimLineSeg( V[0],V[1], 2, m_Near, m_Far ) )continue;

					//Backface_Culling
					if( !wire.NormalIdxs.empty() )
					{
						Vec3 Ray = V[0];
						if(
							std::none_of(
								wire.NormalIdxs.begin(), wire.NormalIdxs.end(),
								[&]( Idx_t iNormal ){	return ( dot( ToCamCoord(WFObj.Normals[iNormal]), Ray ) <= 0.0 ); }
							)
						)
						{	continue;	}
					}

					//Convert the component[0] and [1] to pixel position.
					//The result is { (pix_x, pix_y), depth }
					for( auto &v : V )
					{	v = Vec3d{ m_rCamera.Cam_2_Pix( v ), v[2] };	}
					//trim with pixel range :
					//With y range, then with x range.
					//The result becomes as V[0][0] <= V[1][0] (due to spec of TrimLineSeg() ).
					if( !TrimLineSeg( V[0], V[1], 1, 0, m_rCamera.ImgH()-1 ) )continue;
					if( !TrimLineSeg( V[0], V[1], 0, 0, m_rCamera.ImgW()-1 ) )continue;

					//
					DrawFunc( DrawWireParam{ {V[0],V[1]}, Tgt.SqDist, Tgt.Color } );
				}
			}
		}

	private:
		struct ObjInfo
		{
			//SqDist between Camera and Object center
			double SqDist;
			//Values passed to TryAdd()
			Vec2i iSquare;
			const WireframeObj *pWFObj;
			Vec3d Color;
		};

		//Extract the portion of line segment P-Q where the n-th component value is within the range [Min,Max].
		// * Returns false if both endpoints are outside the range.
		//   (In this case, P and Q are not changed)
		// * Otherwise,
		//		* Returns true
		//		* P and Q is modified to the extract result.
		//		* P becomes the side n-th component is smaller. (: Q become the side whare it is larger)
		static bool TrimLineSeg( Vec3d &P, Vec3d &Q, int nth, double Min, double Max );

	private:
		const PinholeCamera &m_rCamera;
		Vec2d m_CamPos;
		Vec2d m_FrontDir;
		Vec2d m_RightDir;
		double m_CamHeight;
		double m_Near;
		double m_Far;
		mutable std::vector< ObjInfo > m_RenderTgts;
	};

}