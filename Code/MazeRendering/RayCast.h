#pragma once
#include "MazeData.h"
#include <concepts>
#include <optional>

/// <summary>Internal Implementation of the function RenderMazeEdge()</summary>
namespace PSEUDO_3D_MAZE::Impl
{
	/// <summary>Result type of RayCast()</summary>
	struct RayCastResult
	{
		Vec2d Pos;	//intersection of Ray and Edge
		EdgeAttr_t EdgeAttr;	//Attribute of the Edge
		double LRPosRateOnEdge;	//(Same as DrawEdgeParam::LRPosRateOnEdge)
	};

	/// <summary>
	/// 2D Ray Casting :
	/// Find the first hit edge which attribute achieves MazeData_t::IsWallLikeFace()==true.
	/// </summary>
	/// <param name="RayStart">Start position of Ray</param>
	/// <param name="RayEnd">End position of Ray</param>
	/// <param name="Maze">Maze Data</param>
	/// <returns>returns info of hit pos, or nullopt if not found.</returns>
	template< class MazeData_t >
		requires std::derived_from< MazeData_t, MazeData<MazeData_t> >
	std::optional< RayCastResult > RayCast(
		Vec2d RayStart, Vec2d RayEnd,
		const MazeData_t &Maze
	)
	{//TODO : Improve inefficient search processes if possible...
		const Vec2d S2E = RayEnd - RayStart;
		std::optional< RayCastResult > Result;
			
		{//Check Vertical Edge
			//X-coordinate of Vertical Edge is always integer
			const int MinX = (int)ceil( (std::min)( RayStart[0], RayEnd[0] ) );
			const int MaxX = (int)floor( (std::max)( RayStart[0], RayEnd[0] ) );
			if( MinX<=MaxX )
			{
				const double dY_per_X = S2E[1]/S2E[0];
				if( RayStart[0] < RayEnd[0] )
				{
					for( int X=MinX;	X<=MaxX;	++X )
					{
						//(X,Y) is intersection of Ray and Edge
						const double Y = RayStart[1] + ( X - RayStart[0] ) * dY_per_X;
						const EdgeAttr_t EdgeAttr = Maze.EdgeAttr( X, (int)std::floor(Y), Direction::WEST );
						if( MazeData_t::IsWallLikeFace( EdgeAttr ) )
						{
							Result = RayCastResult{ {(double)X,Y}, EdgeAttr, (Y - std::floor(Y)) };
							break;
						}
					}
				}
				else
				{
					for( int X=MaxX;	X>=MinX;	--X )
					{
						const double Y = RayStart[1] + ( X - RayStart[0] ) * dY_per_X;
						const EdgeAttr_t EdgeAttr = Maze.EdgeAttr( X-1, (int)std::floor(Y), Direction::EAST );
						if( MazeData_t::IsWallLikeFace( EdgeAttr ) )
						{
							Result = RayCastResult{ {(double)X,Y}, EdgeAttr, (std::ceil(Y) - Y)  };
							break;
						}
					}
				}
			}
		}
		const double Abs_dX = ( Result  ?  std::abs(Result->Pos[0] - RayStart[0])  :  std::numeric_limits<double>::max() );
		{//Check Horizontal Edge
			//Y-coordinate of Horizontal Edge is always integer
			const int MinY = (int)ceil( (std::min)( RayStart[1], RayEnd[1] ) );
			const int MaxY = (int)floor( (std::max)( RayStart[1], RayEnd[1] ) );
			if( MinY <= MaxY )
			{
				const double dX_per_Y = S2E[0]/S2E[1];
				if( RayStart[1] < RayEnd[1] )
				{
					for( int Y=MinY;	Y<=MaxY;	++Y )
					{
						const double dX = ( Y - RayStart[1] ) * dX_per_Y;
						if( std::abs(dX) > Abs_dX )break;
						const double X = RayStart[0] + dX;
						const EdgeAttr_t EdgeAttr = Maze.EdgeAttr( (int)std::floor(X), Y, Direction::NORTH );
						if( MazeData_t::IsWallLikeFace( EdgeAttr ) )
						{
							Result = RayCastResult{ {X,(double)Y}, EdgeAttr, (std::ceil(X) - X) };
							break;
						}
					}
				}
				else
				{
					for( int Y=MaxY;	Y>=MinY;	--Y )
					{
						const double dX = ( Y - RayStart[1] ) * dX_per_Y;
						if( std::abs(dX) > Abs_dX )break;
						const double X = RayStart[0] + dX;
						const EdgeAttr_t EdgeAttr = Maze.EdgeAttr( (int)std::floor(X), Y-1, Direction::SOUTH );
						if( MazeData_t::IsWallLikeFace( EdgeAttr ) )
						{
							Result = RayCastResult{ {X,(double)Y}, EdgeAttr, (X - std::floor(X)) };
							break;
						}
					}
				}
			}
		}
		return Result;
	}
}
