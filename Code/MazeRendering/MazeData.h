#pragma once
#include <cstdint>
#include "Direction.h"

namespace PSEUDO_3D_MAZE
{
	//----------
	//MazeData<T> provides info necessary to render "Edges" for the rendering function RenderMazeEdge().
	//	"Edge" :
	//		Border between neighboring squares(Maze is square grid).
	//		(Since I don't know what to call this part, the word "Edge" used here.)
	//----------
	//2D Coordinate system :
	//( See also "Direction.h" )
	// * Origin is northwest corner of the square(index=(0,0))
	// * Axis orientation
	//		* X+ : East
	//		* Y+ : South
	// * Square size is 1.0x1.0
	//   So, center of the square(index=(x,y)) is (x+0.5, y+0.5)
	//----------

	/// <summary>type for attribute of Edge</summary>
	using EdgeAttr_t = uint8_t;

	/// <summary>
	/// Maze Edge info provider
	/// ( Used as argument of rendering function RenderMazeEdge() )
	/// </summary>
	/// <typeparam name="Deriv_t">Derived type(CRTP)</typeparam>
	template< class Deriv_t >
	class MazeData
	{
	public:
		/// <summary>
		/// Attribute of the specified side Edge of the squrare(index=(x,y)) ,
		/// **when looking the Edge from outside of the square** (not from inside).
		///		<remarks>
		///		For example, 
		///		EdgeAttr(x,y,EAST) and EdgeAttr(x+1,y,WEST) can be different value.
		///		</remarks>
		/// </summary>
		/// <param name="x">index of the tgt square</param>
		/// <param name="y">index of the tgt square</param>
		/// <param name="Dir">which side it is viewed from</param>
		/// <returns>
		/// Attribute.
		/// Even if the index(x,y) is out of range, returns some proper value (for example, value indicating "wall" ).
		/// </returns>
		EdgeAttr_t EdgeAttr( int x, int y, Direction Dir ) const
		{	return static_cast<const Deriv_t*>(this)->EdgeAttr_(x,y,Dir);	}

		/// <summary>
		/// Check if the Attribute value of Edge is "Wall" (or something equivalent).
		/// That is, returning true means "should be rendered" .
		/// </summary>
		/// <param name="Attr">Attribute value</param>
		/// <returns>true, if wall-like, (for renderer)</returns>
		static bool IsWallLikeFace( EdgeAttr_t Attr )
		{	return Deriv_t::IsWallLikeFace_(Attr);	}

	protected:	//(CRTP) overridable
		//Implementation of EdgeAttr()
		EdgeAttr_t EdgeAttr_( int x, int y, Direction Dir ) const {	return EdgeAttr_t(0);	}

		//Implementation of IsWallLikeFace()
		static bool IsWallLikeFace_( EdgeAttr_t Attr ){	return (Attr!=0);	}
	};
}
