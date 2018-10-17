#pragma once

#include "common/ITypes.h"  // UInt8, UInt32


namespace RE
{
	class MenuManager
	{
	public:
		UInt8	pad000[0x160];			// 000
		UInt32	numPauseGame;			// 160 (= 0) += 1 if (imenu->flags & 0x0001)
		UInt8	pad164[0x170 - 0x164];	// 164
		UInt32	numStopCrosshairUpdate;	// 170 (= 0) += 1 if (imenu->flags & 0x4000)
		UInt8	pad174[0x1C8 - 0x174];	// 174
	};
	STATIC_ASSERT(offsetof(MenuManager, numPauseGame) == 0x160);
	STATIC_ASSERT(offsetof(MenuManager, numStopCrosshairUpdate) == 0x170);
	STATIC_ASSERT(sizeof(MenuManager) == 0x1C8);
}