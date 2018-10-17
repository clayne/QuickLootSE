#pragma once

#include "common/ITypes.h"  // UInt32
#include "skse64/GameBSExtraData.h"  // BSExtraData


namespace RE
{
	class ExtraLock : public BSExtraData
	{
	public:
		enum { kExtraTypeID = kExtraData_Lock};

		explicit ExtraLock(UInt32 arg1) : unk08(arg1) {}
		virtual ~ExtraLock();

		UInt32	unk08;
	};
}