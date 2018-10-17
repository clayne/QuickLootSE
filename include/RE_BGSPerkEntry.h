#pragma once

#include "common/ITypes.h"  // UInt8, UInt16, Uint32


namespace RE
{
	class BGSPerkEntry
	{
	public:
		virtual bool CanProcess(UInt32 numArgs, void* args);

		UInt8	rank;		// 08
		UInt8	priority;	// 09
		UInt16	type;		// 0A
		UInt32	pad0C;		// 0C
	};
}