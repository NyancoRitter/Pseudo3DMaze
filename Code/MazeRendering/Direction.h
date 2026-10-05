#pragma once
#include <numbers>
namespace PSEUDO_3D_MAZE
{
	/// <summary>4 directions</summary>
	enum class Direction
	{
		/// <summary>East : Unit_Dir_Vec = (1,0),  angle = 0</summary>
		EAST = 0,
		/// <summary>South : Unit_Dir_Vec = (0,1),  angle = PI/2</summary>
		SOUTH = 1,
		/// <summary>West : Unit_Dir_Vec = (-1,0),  angle = PI</summary>
		WEST = 2,
		/// <summary>North : Unit_Dir_Vec = (0,-1),  angle = PI * 3/2</summary>
		NORTH = 3
	};

	inline constexpr Direction RightDirOf( Direction Dir ){	return (Direction)( ((int)Dir + 1)%4 );	}
	inline constexpr Direction LeftDirOf( Direction Dir ){	return (Direction)( ((int)Dir + 3)%4 );	}
	inline constexpr Direction OppositeDirOf( Direction Dir ){	return (Direction)( ((int)Dir + 2)%4 );	}

	/// <summary>Angle value of Dir</summary>
	/// <returns>angle[rad]</returns>
	inline constexpr double AngleOf( Direction Dir ){	return std::numbers::pi * 0.5 * (int)Dir;	}
}
