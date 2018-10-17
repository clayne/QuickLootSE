#pragma once

#include "skse64/GameMenus.h"  // IUIMessageData

#include "GFx.h"  // GFxEvent


namespace RE
{
	class BSUIScaleformData : public ::IUIMessageData
	{
	public:
		virtual ~BSUIScaleformData() {}		// 00897320

		//void		** _vtbl;		// 00 - 0110DF70
		GFxEvent	* event;		// 08
	};
}