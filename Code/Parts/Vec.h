#pragma once

#include <array>
#include <cmath>
#include <limits>

template< class VAL_T, size_t DIM >
	requires std::integral<VAL_T> || std::floating_point<VAL_T>
struct Vec : public std::array<VAL_T, DIM>
{
	//ctor
	Vec() = default;

	//ctor
	template< class ...Args >
	requires ( sizeof...(Args) == DIM )
	constexpr Vec( Args... args )
		: std::array<VAL_T, DIM>{ static_cast<VAL_T>(args)... }
	{}

	//ctor
	constexpr Vec( Vec<VAL_T, DIM-1> s, VAL_T LastComponent ) requires (DIM>0)
	{
		for( size_t i=0; i<DIM-1; ++i ){	(*this)[i] = s[i];	}
		(*this)[DIM-1] = LastComponent;
	}

	//unary OP
	constexpr Vec &operator *=( VAL_T s ){	for( size_t i=0; i<DIM; ++i ){	(*this)[i] *= s;	}	return *this;	}
	constexpr Vec &operator /=( VAL_T s ){	for( size_t i=0; i<DIM; ++i ){	(*this)[i] /= s;	}	return *this;	}
	constexpr Vec &operator +=( const Vec &rhs ){	for( size_t i=0; i<DIM; ++i ){	(*this)[i] += rhs[i];	}	return *this;	}
	constexpr Vec &operator -=( const Vec &rhs ){	for( size_t i=0; i<DIM; ++i ){	(*this)[i] -= rhs[i];	}	return *this;	}

	constexpr Vec operator -() const
	{
		Vec Ret;
		for( size_t i=0; i<DIM; ++i ){	Ret[i] = -(*this)[i];	}
		return Ret;
	}

	//Square of L2-Nrom
	constexpr  VAL_T SqL2Norm() const;
	//L2-Norm
	constexpr  VAL_T L2Norm() const {	return std::sqrt( SqL2Norm() );	}
};

//alias
template< class VAL_T > using Vec2 = Vec<VAL_T,2>;
using Vec2i = Vec2<int>;
using Vec2d = Vec2<double>;

template< class VAL_T > using Vec3 = Vec<VAL_T,3>;
using Vec3d = Vec<double,3>;

//binary OP
template< class VAL_T, size_t DIM >
constexpr  Vec<VAL_T,DIM> operator*( const Vec<VAL_T,DIM> &lhs, VAL_T s ){	return Vec<VAL_T,DIM>{lhs} *= s;	}

template< class VAL_T, size_t DIM >
constexpr  Vec<VAL_T,DIM> operator*( VAL_T s, const Vec<VAL_T,DIM> &rhs ){	return Vec<VAL_T,DIM>{rhs} *= s;	}

template< class VAL_T, size_t DIM >
constexpr  Vec<VAL_T,DIM> operator/( const Vec<VAL_T,DIM> &lhs, VAL_T s ){	return Vec<VAL_T,DIM>{lhs} /= s;	}

template< class VAL_T, size_t DIM >
constexpr  Vec<VAL_T,DIM> operator+( const Vec<VAL_T,DIM> &lhs, const Vec<VAL_T,DIM> &rhs ){	return Vec<VAL_T,DIM>{lhs} += rhs;	}

template< class VAL_T, size_t DIM >
constexpr  Vec<VAL_T,DIM> operator-( const Vec<VAL_T,DIM> &lhs, const Vec<VAL_T,DIM> &rhs ){	return Vec<VAL_T,DIM>{lhs} -= rhs;	}

//dot product
template< class VAL_T, size_t DIM >
constexpr VAL_T dot( const Vec<VAL_T,DIM> &lhs, const Vec<VAL_T,DIM> &rhs )
{
	VAL_T result = VAL_T(0);
	for( size_t i=0; i<DIM; ++i ){	result += lhs[i]*rhs[i];	}
	return result;
}

//cross product
template<class VAL_T>
constexpr Vec3<VAL_T> cross( const Vec3<VAL_T> &lhs, const Vec3<VAL_T> &rhs )
{	return Vec3<VAL_T>( lhs[1]*rhs[2] - lhs[2]*rhs[1], lhs[2]*rhs[0] - lhs[0]*rhs[2], lhs[0]*rhs[1] - lhs[1]*rhs[0] );	}

//corss product for Vec2 :
//Assuming both arguments as 3D-vector(x,y,0), and returns the z-value of the cross-product.
template<class VAL_T>
constexpr VAL_T cross( const Vec2<VAL_T> &lhs, const Vec2<VAL_T> &rhs ){	return ( lhs[0]*rhs[1] - lhs[1]*rhs[0] ); }

//SqL2Norm()
template< class VAL_T, size_t DIM >
	requires std::integral<VAL_T> || std::floating_point<VAL_T>
constexpr VAL_T Vec<VAL_T,DIM>::SqL2Norm() const {	return dot(*this,*this);	}

//Get Normalized Vector, or copy of V if V is Zero-Vector
template< class VAL_T, size_t DIM >
constexpr Vec<VAL_T,DIM> Normalize( const Vec<VAL_T,DIM> &V )
{
	if( VAL_T L=V.L2Norm();	L < std::numeric_limits<VAL_T>::epsilon() )
	{	return V;	}
	else
	{	return V / L;	}
}
