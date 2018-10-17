#pragma once

#include "common/ITypes.h"  // UInt8, SInt32


namespace RE
{
	class InputManager
	{
	public:
		enum ControlState
		{
			kControlState_Looking = 0x002,
			kControlState_Flying = 0x040,
			kControlState_Sneaking = 0x080,
			kControlState_Menu = 0x100,
			kControlState_Movement = 0x401
		};


		UInt8	pad000[0x118];			// 000
		SInt32	controlState;			// 118 - init'd to 0xFFFFFFFF
		UInt8	pad11C[0x128 - 0x11C];	// 11C


		inline bool IsMovementControlsEnabled() const { return (controlState & kControlState_Movement) == kControlState_Movement; }
	};
	STATIC_ASSERT(offsetof(InputManager, controlState) == 0x118);
	STATIC_ASSERT(sizeof(InputManager) == 0x128);
}