#pragma once

#include "skse64/GameInput.h"

namespace TES
{
	// 30
	class ButtonEvent : public IDEvent, public InputEvent
	{
	public:
		virtual					~ButtonEvent();
		virtual bool			IsIDEvent();
		virtual BSFixedString *	GetControlID();

		// 18 -> controlID from IDEvent
		UInt32			keyMask;	// 20 (00000038 when ALT is pressed, 0000001D when STRG is pressed)
		UInt32			pad24;		// 24
		float			pressure;   // 28 (isPressed)
		float			timer;		// 2C (hold duration)

		inline bool IsPressed() const
		{
			return pressure > 0;
		}

		inline bool IsDown() const
		{
			return (pressure > 0) && (timer == 0.0f);
		}

		inline bool IsUp() const
		{
			return (pressure == 0) && (timer > 0.0f);
		}
	};
}