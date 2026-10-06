#include "WireframeObj.h"
#include <map>
#include <algorithm>

namespace Wireframe
{
	WireframeObj::WireframeObj( WFObjDef Def )
		: Vtxs( std::move( Def.Vtxs ) )
	{
		const size_t nVtx = Vtxs.size();
		this->Normals.reserve( Def.Faces.size() );

		std::map< std::pair<Idx_t,Idx_t>, std::vector<Idx_t> > WireMap;

		for( const auto &F : Def.Faces )
		{
			//invalid face definition is just ignored
			if( F.size() < 3 )continue;
			if( std::any_of( F.begin(), F.end(), [nVtx](Idx_t idx){	return (idx>=nVtx);	} ) )continue;

			{//Face Normal
				Vec3d N = cross( ( Vtxs[F[1]] - Vtxs[F[0]] ), ( Vtxs[F[2]] - Vtxs[F[0]] ) );
				this->Normals.push_back( Normalize(N) );
			}
			const Idx_t iN = (Idx_t)( this->Normals.size() - 1 );
			for( size_t i=0; i<F.size(); ++i )
			{
				auto iv_a = F[i];
				auto iv_b = F[ (i+1)==F.size() ? 0 : i+1 ];
				if( iv_a > iv_b ){	std::swap( iv_a, iv_b );	}
				WireMap[ std::make_pair(iv_a,iv_b) ].push_back( iN );
			}
		}

		this->Wires.reserve( WireMap.size() );
		for( auto &w : WireMap )
		{
			this->Wires.emplace_back( w.first, std::move(w.second) );
		}
	}
}
