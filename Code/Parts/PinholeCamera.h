#pragma once

#include <limits>
#include <cmath>
#include <stdexcept>
#include "Vec.h"


/// <summary>
/// Simple Pinhole-Camera Model
///		<remarks>
///		* Camera Coordinate System :
///			* X+ : rhs dir for Camera
///			* Y+ : downword dir for Camera
///			* Z+ : front dir for Camera
///		* Pixel Coordinate :
///		  integer position is center of pixel.
///		  For example, (0.0, 0.0) is center of the top-left pixel.
///		</remarks>
/// </summary>
class PinholeCamera
{
public:
	/// <summary>
	/// ctor.
	/// image size[pixel] and FOV[rad] should be specified here.
	///		<remarks>image_center is simply decided as center of image.</remarks>
	/// </summary>
	/// <param name="ImgW">画像サイズ[pixel]</param>
	/// <param name="ImgH">画像サイズ[pixel]</param>
	/// <param name="FOV_h">左右方向画角[rad]</param>
	PinholeCamera( int ImgW, int ImgH, double FOV_h )
		: m_ImgW(ImgW), m_ImgH(ImgH)
	{
		if( ImgW<=0 || ImgH<=0 )
		{	throw std::invalid_argument( "Invalid Img Size" );	}

		if( FOV_h<=0.0 || FOV_h>=180.0 )
		{	throw std::invalid_argument( "Invalid FOV_h" );	}

		m_ICx = (ImgW - 1) * 0.5;
		m_ICy = (ImgH - 1) * 0.5;
		m_F = m_ICx / std::tan( FOV_h*0.5 );
	}

public:
	/// <summary>image size[pixel] (: the value specified via ctor)</summary>
	/// <returns>x directional size[pixel]</returns>
	int ImgW() const {	return m_ImgW;	}

	/// <summary>image size[pixel] (: the value specified via ctor)</summary>
	/// <returns>y directional size[pixel]</returns>
	int ImgH() const {	return m_ImgH;	}

	/// <summary>
	/// Calculate the position[pixel] where the positon C(in Camera Coodinate system) projected.
	/// </summary>
	/// <param name="C">
	/// Position in Camera Coordinate System.
	/// Note : C[2] should be greater than 0. (Otherwise, it may cause unexpected behavior.)
	/// </param>
	/// <returns>Projected potision (x,y) [pixel]</returns>
	Vec2d Cam_2_Pix( const Vec3d &C ) const
	{
		const double rate = m_F / C[2];
		return { m_ICx + C[0]*rate, m_ICy + C[1]*rate };
	}

	/// <summary>
	/// X only version of Cam_2_Pix().
	/// Calculate the x-position[pixel] where the position (CX, *, CZ) in Camera Coodinate system projected.
	/// </summary>
	/// <param name="CX">X-Position in Camera Coordinate System CX </param>
	/// <param name="CZ">Z-Position in Camera Coordinate System CZ．(Should be greater than 0, or unexpected behavior)</param>
	/// <returns>Projected x-potision [pixel]</returns>
	double CxCz_2_Px( double CX, double CZ ) const {	return m_ICx + m_F*CX/CZ;	}

	/// <summary>
	/// Y only version of Cam_2_Pix().
	/// Calculate the y-position[pixel] where the position (*, CY, CZ) in Camera Coodinate system projected.
	/// </summary>
	/// <param name="CY">Y-Position in Camera Coordinate System CX </param>
	/// <param name="CZ">Z-Position in Camera Coordinate System CZ．(Should be greater than 0, or unexpected behavior)</param>
	/// <returns>Projected y-potision [pixel]</returns>
	double CyCz_2_Py( double CY, double CZ ) const {	return m_ICy + m_F*CY/CZ;	}

	/// <summary>
	/// Calculate CX of the position (CX, *, 1) in Camera Coordinate System,
	/// which pojected to the image x-position.
	/// </summary>
	/// <param name="Px">x-position [pixel]</param>
	/// <returns>CX on the CZ=1 plane</returns>
	double Px_2_Cx( double Px ) const {	return ( Px - m_ICx ) / m_F;	}

	/// <summary>
	/// Calculate CY of the position (*, CY, 1) in Camera Coordinate System,
	/// which pojected to the image y-position.
	/// </summary>
	/// <param name="Py">y-position [pixel]</param>
	/// <returns>CY on the CZ=1 plane</returns>
	double Py_2_Cy( double Py ) const {	return ( Py - m_ICy ) / m_F;	}

private:
	int m_ImgW, m_ImgH;	//image size [pixel] (just for image_size_getter.)
	double m_ICx, m_ICy;	//image center [pixel]
	double m_F;
};

