#pragma once

#include "common/ITypes.h"  // UInt32
#include "skse64/GameInput.h"  // InputEvent, ThumbstickEvent, MouseMoveEvent, ButtonEvent
#include "RE_PlayerControls.h"  // PlayerControls


namespace RE
{
	class PlayerInputHandler
	{
	public:
		PlayerInputHandler();
		virtual ~PlayerInputHandler();

		virtual	bool	CanProcess(InputEvent* evn);
		virtual	void	ProcessThumbstick(ThumbstickEvent* evn, RE::PlayerControls::Data24* arg2);
		virtual	void	ProcessMouseMove(MouseMoveEvent* evn, RE::PlayerControls::Data24* arg2);
		virtual	void	ProcessButton(ButtonEvent* evn, RE::PlayerControls::Data24* arg2);


		inline bool IsEnabled() const { return enabled != 0; }
		inline void Enable(bool bEnable = true) { enabled = (bEnable) ? 1 : 0; }


		UInt32	enabled;	// 08
		UInt32	pad0C;		// 0C
	};
}