#pragma once

#include "skse64/GameExtraData.h"  // InventoryEntryData


class InventoryEntryDataExt
{
public:
	InventoryEntryDataExt(InventoryEntryData* a_inventoryEntryData) : inventoryEntryData(a_inventoryEntryData) {}


	void AddEntryList(BaseExtraList *extra)
	{
		if (!extra)
			return;

		if (!inventoryEntryData->extendDataList)
			inventoryEntryData->extendDataList->Create();
		if (inventoryEntryData->extendDataList)
			inventoryEntryData->extendDataList->Insert(extra);
	}


	InventoryEntryData* inventoryEntryData;
};