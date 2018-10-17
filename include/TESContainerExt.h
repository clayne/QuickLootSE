#pragma once


#include "skse64/GameFormComponents.h"  // TESContainer


class TESContainerExt
{
public:
	explicit TESContainerExt(TESContainer* a_container) : container(a_container) {}

	bool GetContainerItemAt(UInt32 idx, TESContainer::Entry *& entry) const
	{
		return (idx < container->numEntries) ? (entry = container->entries[idx], true) : false;
	}

	TESContainer* container;
};