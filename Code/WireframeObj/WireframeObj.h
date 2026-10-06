#pragma once
#include <vector>
#include <variant>
#include <map>
#include "../Parts/Vec.h"

namespace Wireframe
{
	//----------
	//3D Coordinate system (is left-handed system as) :
	// * Orientations of X and Y axes are (same as "MazeData.h") :
	//		* X+ : East
	//		* Y+ : South
	// * Valid Z axis is "height" : range is [(floor)0.0 - 1.0(ceil)]
	//----------
	//Premise :
	//  Object should be defined within a 1x1x1 cube space.
	//  i.e. X,Y,Z of all vertex should be [0.0 , 1.0].
	//----------

	//type for index value
	using Idx_t = uint8_t;

	/// <summary>
	/// Data type used in WireframeObj.
	/// Definition of 1 edge.
	/// </summary>
	struct Wire
	{
		//Pair of vertex indexes (specifyng endpoints).
		std::pair<Idx_t,Idx_t> VtxIdxs;
		//A Set of indexex of the normal_vectors of the faces sharing this edge.
		//This is Info for Backface_Culling.
		//If empty, Culling is disabled for this edge.
		std::vector<Idx_t> NormalIdxs;
	};

	/// <summary>
	/// Helper type to create WireframeObj, can be passed to WireframeObj::ctor().
	/// </summary>
	struct WFObjDef
	{
		//Pos of Vertices
		std::vector< Vec3d > Vtxs;

		//Faces．
		//Each element is a polygon face defined via a sequence of vertex(indexes).
		// * So at least 3 indexes needed.
		// * The normal of the face is calculated with the first 3 vertexes based on
		//  	CrossProd( 1st-->2nd, 1st-->3rd );
		//		* (As described above, a single normal is calculated.
		//		  That is, premise is that all vertices lie on the same plane.)
		std::vector< std::vector<Idx_t> > Faces;
	};

	/// <summary>
	/// Wireframe object data for rendering process.
	/// * Holding normal_vector infos for Backface_Culling.
	/// </summary>
	struct WireframeObj
	{
		//Pos of Vertices
		std::vector< Vec3d > Vtxs;
		//unit_normal_vectors of the faces
		std::vector< Vec3d > Normals;
		//edges
		std::vector< Wire > Wires;

		//---
		WireframeObj(){}
		//Construct from WFObjDef
		WireframeObj( WFObjDef Def );
	};

}