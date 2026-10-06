#include "WFORenderer.h"
#include <algorithm>

namespace Wireframe
{
	WFORenderer::WFORenderer(
		const PinholeCamera &Camera,
		const Vec3d &CameraPos,
		double CameraYaw_rad,
		double Near,
		double Far
	)
		: m_rCamera(Camera)
		, m_CamPos{ CameraPos[0], CameraPos[1] }
		, m_FrontDir{ std::cos(CameraYaw_rad), std::sin(CameraYaw_rad) }
		, m_RightDir{ -m_FrontDir[1], m_FrontDir[0] }
		, m_CamHeight( CameraPos[2] )
		, m_Near(Near)
		, m_Far(Far)
	{}

	bool WFORenderer::TryAdd( Vec2i iSquare, const WireframeObj &WFObj, Vec3d Color )
	{
		//dCenterPos = from CameraPos to Obj Center
		const Vec2d dCenterPos = Vec2d(iSquare[0]+0.5, iSquare[1]+0.5) - m_CamPos;
		const double SqDist = dCenterPos.SqL2Norm();

		//---
		//MEMO:
		//With this very rough pruning,
		//even objects partially visible may be rejected.
		//---
		
		//(1) If too far, reject.
		// * Here, checking distance (not depth) to limit more.
		if( SqDist > m_Far*m_Far )return false;

		//CC = {Cam RightDir Component, Cam FrontDir Component} of Obj Center
		const Vec2d CC{ dot( m_RightDir, dCenterPos ), dot( m_FrontDir, dCenterPos ) };
		//(2) If too near or not at front side, reject.
		if( CC[1] < m_Near )return false;
		//(3) Obj_Pos vs (horizontal)FOV :
		//If a circle centered at CC is completely outside FOV, reject.
		//	* About the circle radius :
		//	  Considering the case where Obj completely fills 1x1 square space,
		//	  this radius must be at least `sqrt(2.0)/2` (= half of the diagonal).
		//	  But here, less radius value is used under the assumption that actual objects are smaller enough.
		//	  (So, this value may be needed to modify if objs should be partially rendered are rejected in practice.)
		constexpr double CheckRadius = 0.5;
		const auto OutsideDir = Normalize(
			CC[0]>=0.0 ?
			Vec2d( 1.0, -m_rCamera.Px_2_Cx( m_rCamera.ImgW()-1 ) ) :
			Vec2d( -1.0, m_rCamera.Px_2_Cx( 0 ) )
		);
		if( dot( CC, OutsideDir ) > CheckRadius )
		{	return false;	}

		//
		m_RenderTgts.emplace_back( SqDist, iSquare, &WFObj, Color );
		return true;
	}

	bool WFORenderer::TrimLineSeg(Vec3d &P, Vec3d &Q, int nth, double Min, double Max)
	{
		if( (P[nth]<Min && Q[nth]<Min) || (P[nth]>Max && Q[nth]>Max) ){	return false;	}

		if( P[nth]>Q[nth] )
		{	std::swap( P,Q );	}

		if( P[nth]<Min )
		{
			double rate = double( Q[nth] - Min ) / ( Q[nth] - P[nth] );
			P = Q + ( P - Q )*rate;
		}
		if( Q[nth]>Max )
		{
			double rate = double( Max - P[nth] ) / ( Q[nth] -  P[nth] );
			Q = P + ( Q - P )*rate;
		}
		return true;
	}

}
