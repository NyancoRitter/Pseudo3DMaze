#pragma once
#include <memory>

namespace Test03
{
	//image size to be rendered
	inline constexpr int IMG_W = 640;
	inline constexpr int IMG_H = 480;

	/// <summary>
	/// Operation types
	/// 	<remarks>
	/// 	Impletation Memo :
	/// 	  Field Values should be integer sequence started with 0.
	/// 	  (There are impls depends on this spec)
	/// 	</remarks>
	/// </summary>
	enum class Key
	{
		GoForward = 0,
		GoBackward,
		GoLeft,
		GoRight,
		TurnLeft,
		TurnRight
	};
	constexpr size_t nUsedKeys = 6;

	/// <summary>
	/// Input info.
	/// Should be handled properly and passed to GameLike::Update() .
	/// </summary>
	class Controller
	{
	public:
		/// <summary>
		/// This method should be called once after each GameLike::Update().
		/// </summary>
		void ToNextTimeStep()
		{
			for( auto &h : KeyHistory ){	h = (h&0b1) | (h<<1);	}
		}

		/// <summary>
		/// This method should be called when key state changed.
		/// </summary>
		/// <param name="key"></param>
		/// <param name="Pressed">
		/// current key state.
		/// true=pressed, false=released.
		/// </param>
		void OnKeyStateChanged( Key key, bool Pressed )
		{
			if( Pressed )
			{	KeyHistory[(size_t)key] |= 0b1; }
			else
			{	KeyHistory[(size_t)key] &= 0b1111'1110;	}
		}

		bool Pressed( Key key ) const {	return KeyHistory[(size_t)key] & 0b1;	}
	private:
		//(Assuming all keys are not pressed at init.) 
		uint8_t KeyHistory[nUsedKeys] = {0};
	};

	/// <summary>
	/// "Game-like" implementation :
	/// * Move one by one square
	///		* movement can be obstructed by wall
	/// * Turn 90 deg at a time
	/// </summary>
	class GameLike
	{
	public:
		/// <summary>ctor</summary>
		/// <exception cref="std::invalid_argument">throw exception if initialization failed</exception>
		GameLike();

		~GameLike();
	public:
		/// <summary>
		/// Update (: move and turn).
		/// (assumed to be called from within Main_Loop)
		/// </summary>
		/// <param name="Ctrller">input info</param>
		/// <returns>
		/// Indicating whether redraw via Render() needed.
		/// </returns>
		bool Update( const Controller &Ctrller );

		/// <summary>Rendering</summary>
		/// <param name="hDC">Top-left IMG_W*IMG_H rectanglar region will be drawn</param>
		void Render( HDC hDC ) const;

	private:
		GameLike( const GameLike & ) = delete;
		GameLike &operator=( const GameLike & ) = delete;
	private:
		class Impl;
		std::unique_ptr<Impl> m_upImpl;
	};
}
