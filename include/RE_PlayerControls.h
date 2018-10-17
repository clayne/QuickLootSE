#pragma once

#include "skse64/GameInput.h"


namespace RE
{
	class PlayerControls
	{
	public:
		virtual			~PlayerControls();
		virtual UInt32	Unk_01();


		struct Data24
		{
			float	movementX;	// 00
			float	movementY;	// 04
			float	unk08;		// 08
			float	unk0C;		// 0C
			float	unk10;		// 10
			float	unk14;		// 14
			float	unk18;		// 18
			float	unk1C;		// 1C
		};


		UInt8	pad00[0x24];	// 00
		Data24	unk24;			// 24
	};
	STATIC_ASSERT(offsetof(PlayerControls, unk24) == 0x24);
}