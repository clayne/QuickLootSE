#pragma once

#include "skse64/NiControllers.h"  // NiTimeController


namespace RE
{
	class NiControllerManager : public ::NiTimeController
	{
	public:
		enum { kRTTI = 0x01B90490 };
	};
}