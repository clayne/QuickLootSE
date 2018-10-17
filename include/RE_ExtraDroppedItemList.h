#pragma once

#include "common/ITypes.h"  // UInt32
#include "skse64/GameBSExtraData.h"  // BSExtraData
#include "skse64/GameTypes.h"  // tList


namespace RE
{
	class ExtraDroppedItemList : public ::BSExtraData
	{
	public:
		enum { kExtraTypeID = kExtraData_DroppedItemList };

		virtual ~ExtraDroppedItemList();

		tList<UInt32>	handles;	// 08
	private:
	};
}