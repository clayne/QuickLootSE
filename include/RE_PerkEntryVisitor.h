#pragma once

#include "skse64/GameObjects.h"


namespace RE
{
	class PerkEntryVisitor
	{
	public:
		virtual UInt32 Visit(BGSPerkEntry* perkEntry) = 0;
	};
}