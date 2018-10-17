#pragma once

#include "skse64/GameForms.h"  // TESForm


namespace RE
{
	class InventoryEntryData
	{
	public:
		MEMBER_FN_PREFIX(InventoryEntryData);
		DEFINE_MEMBER_FN(IsOwnedBy2, bool, 0x001D7780, TESForm* actor, TESForm* itemOwner, bool unk1);
		DEFINE_MEMBER_FN(GetOwner, TESForm*, 0x001D6810)
		DEFINE_MEMBER_FN(IsQuestItem, bool, 0x001D6D90)
	};
}