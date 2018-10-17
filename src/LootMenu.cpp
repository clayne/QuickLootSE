#include "skse64/GameReferences.h"
#include "skse64/GameRTTI.h"
#include "skse64/GameExtraData.h"
#include "skse64/GameObjects.h"
#include "skse64/GameEvents.h"
#include "skse64/NiNodes.h"
#include "skse64/NiControllers.h"
#include "skse64/NiObjects.h"
#include "skse64/Hooks_UI.h"

#include "LootMenu.h"
#include "Hooks.h"
#include "GFx.h"
#include "Settings.h"
#include "RE_ExtraLock.h"
#include "RE_PerkEntryVisitor.h"
#include "RE_BGSPerkEntry.h"
#include "RE_TESObjectLIGH.h"
#include "RE_GFxMovieDef.h"
#include "RE_GfxMovieView.h"
#include "RE_MenuManager.h"
#include "RE_InputManager.h"
#include "RE_PlayerCharacter.h"
#include "RE_IAnimationGraphManagerHolder.h"
#include "RE_TESObjectREFR.h"
#include "RE_Actor.h"
#include "RE_BGSEntryPointEntry.h"
#include "TESContainerExt.h"
#include "InventoryEntryDataExt.h"
#include "RE_ExtraDroppedItemList.h"
#include "RE_TESBoundObject.h"
#include "skse64/GameRTTI.h"
#include "RE_NiControllerManager.h"
#include "RE_NiControllerSequence.h"
#include "RE_UIMessage.h"
#include "RE_BSUIScaleformData.h"
#include "RE_ButtonEvent.h"

//#include <unordered_map>
//#include <algorithm>

typedef void BaseExtraList_SetInventoryChanges_t(BaseExtraList* o, ExtraContainerChanges::Data* changes);
RelocAddr<BaseExtraList_SetInventoryChanges_t*> BaseExtraList_SetInventoryChanges(0x0010F7B0);  // 1_5_50

//typedef bool GetAnimationVariableBool_t(IAnimationGraphManagerHolder * anim, const BSFixedString &variableName, bool &out);
//RelocAddr<GetAnimationVariableBool_t*> GetAnimationVariableBool(0x0060F710);

typedef bool Unk1Func_t(Actor * actor);
RelocAddr<Unk1Func_t*> Unk1Func(0x005EC270);  // 1_5_50

typedef bool Unk2Func_t(ActorProcessManager * actorprocess);
RelocAddr<Unk2Func_t*> Unk2Func(0x0067A380);  // 1_5_50

typedef bool IsInKillmove_t(Actor * actor);
RelocAddr<IsInKillmove_t*> IsInKillmove(0x00628430);  // 1_5_50

typedef bool IsActivationBlocked_t(BaseExtraList *, UInt32 flag);
RelocAddr<IsActivationBlocked_t*> IsActivationBlocked(0x001260D0);  // 1_5_50

typedef RE::ExtraLock* GetLock_t(TESObjectREFR * source);
RelocAddr<GetLock_t*> GetLock(0x002A76B0);  // 1_5_50

typedef TESForm* GetExtraOwner_t(BaseExtraList *);
RelocAddr<GetExtraOwner_t*> GetExtraOwner(0x001133D0);  // 1_5_50

typedef void RegisterMenuEventHandler_t(MenuControls *, MenuHandler::MenuEventHandler *);
RelocAddr<RegisterMenuEventHandler_t*> RegisterMenuEventHandler(0x008A8110);  // 1_5_50

typedef void RemoveMenuEventHandler_t(MenuControls *, MenuHandler::MenuEventHandler *);
RelocAddr<RemoveMenuEventHandler_t*> RemoveMenuEventHandler(0x008A81E0);  // 1_5_50

TESObjectREFR * containerRef = nullptr;

LootMenu * LootMenu::ms_pSingleton = nullptr;
SimpleLock LootMenu::ms_lock;

class HasActivateChoiceVisitor : public RE::PerkEntryVisitor
{
public:
	HasActivateChoiceVisitor(Actor *a_actor, TESObjectREFR *a_target)
	{
		m_actor = a_actor;
		m_target = a_target;
		m_result = false;
	}

	virtual UInt32 Visit(RE::BGSPerkEntry *perkEntry) override
	{
		if (perkEntry->CanProcess(2, &m_actor))
		{
			BGSEntryPointPerkEntry *entryPoint = (BGSEntryPointPerkEntry *)perkEntry;
			if (entryPoint->data)
			{
				BGSEntryPointFunctionDataActivateChoice *activateChoice = (BGSEntryPointFunctionDataActivateChoice*)entryPoint->data;
				BGSPerk *perk = entryPoint->perk;

				if (perk && activateChoice)
				{
					if ((activateChoice->flags & 1) == 0)
					{
						enum {
							VampireFeed = 0x0CF02C,
							Cannibalism = 0x0EE5C3
						};

						switch (perk->formID)
						{
						case VampireFeed:
						case Cannibalism:
							break;
						default:
							m_result = true;
						}
					}
					if ((activateChoice->flags & 2) != 0)
					{
						m_result = true;
					}
				}
			}
		}

		return 1;
	}

	bool GetResult() const
	{
		return m_result;
	}

protected:
	Actor			* m_actor;
	TESObjectREFR	* m_target;
	bool			m_result;
};

class CanPickpocketEquippedItemVisitor : public RE::PerkEntryVisitor
{
public:
	CanPickpocketEquippedItemVisitor(Actor *a_actor, TESObjectREFR *a_target, TESForm *a_item)
	{
		m_actor = a_actor;
		m_target = a_target;
		m_item = a_item;
		m_result = false;
	}

	virtual UInt32 Visit(RE::BGSPerkEntry *perkEntry) override
	{
		if (perkEntry->CanProcess(3, &m_actor))
		{
			BGSEntryPointPerkEntry *entryPoint = (BGSEntryPointPerkEntry *)perkEntry;
			if (entryPoint->perk)
			{
				BGSPerk *perk = entryPoint->perk;
				if (perk)
				{
					m_result = true;
					_MESSAGE("perk formID %x", perk->formID);
				}	
			}
		}

		return 1;
	}

	bool GetResult() const
	{
		return m_result;
	}

protected:
	Actor			* m_actor;
	TESObjectREFR	* m_target;
	TESForm			* m_item;
	bool			m_result;
};

class LootMenuUIDelegate : public UIDelegate_v1
{
public:
	typedef void (LootMenu::*FnCallback)(std::vector<GFxValue> &args);

	const char *	m_target;
	FnCallback		m_callback;

	LootMenuUIDelegate(const char *target, FnCallback callback)
		: m_target(target), m_callback(callback)
	{
	}

	void Run() override
	{
		LootMenu * loot = LootMenu::GetSingleton();
		if (loot)
		{
			GFxMovieView *view = loot->view;

			char target[64];
			strcpy_s(target, 64, "_root.Menu_mc");
			strcat_s(target, m_target);

			std::vector<GFxValue> args;
			(loot->*m_callback)(args);

			if (args.empty())
				view->Invoke(target, nullptr, nullptr, 0);
			else
				view->Invoke(target, nullptr, &args[0], args.size());
		}
	}

	void Dispose() override
	{
		if (this)
			Heap_Free(this);
	}

	static void Queue(const char *target, FnCallback callback)
	{
		if (g_task)
		{
			LootMenuUIDelegate* delg = (LootMenuUIDelegate*)Heap_Allocate(sizeof(LootMenuUIDelegate));
			new (delg)LootMenuUIDelegate(target, callback);
			g_task->AddUITask(delg);
		}
	}
};

static bool IsLocked(TESObjectREFR * source)
{
	RE::ExtraLock* lock = GetLock(source);
	if (!lock)
		return false;

	return (lock->unk08 & 1) != 0;
}

bool IsValidItem(TESForm *item)
{
	if (!item)
		return false;

	if (item->formType == kFormType_LeveledItem)
		return false;

	if (item->formType == kFormType_Light)
	{
		TESObjectLIGH *light = DYNAMIC_CAST(item, TESForm, TESObjectLIGH);
		if (light && !reinterpret_cast<RE::TESObjectLIGH*>(light)->CanBeCarried())
			return false;
	}
	else
	{
		if (!item->IsPlayable())
			return false;
	}

	TESFullName *fullName = DYNAMIC_CAST(item, TESForm, TESFullName);
	if (!fullName)
		return false;

	const char *name = fullName->name.data;
	if (!name || !name[0])
		return false;

	return true;
}

IMenu* LootMenuCreator::Create(void)
{
	void* p = ScaleformHeap_Allocate(sizeof(LootMenu));
	if (p)
	{
		IMenu* menu = new (p)LootMenu("LootMenu");
		_MESSAGE("LootMenu %p, allocate %p", menu, p);
		return menu;
	}
	else
	{
		return nullptr;
	}
}

LootMenu::LootMenu(const char* swfPath)
{
	if (CALL_MEMBER_FN(GFxLoader::GetSingleton(), LoadMovie)(this, &view, swfPath, 1, 0.0))
	{
		IMenu::flags = 0x02 | 0x800 | 0x10000;
		IMenu::unk0C = 2;

		m_bNowTaking = false;
		m_bUpdateRequest = false;
		m_bOpenAnim = false;
		m_selectedIndex = -1;
		m_owner = nullptr;
		m_bTakeAll = false;
	}
}

LootMenu::~LootMenu()
{
	Clear();
}

void LootMenu::Setup()
{
	if (view)
	{
		RE::GFxMovieDef* def = reinterpret_cast<RE::GFxMovieView*>(view)->GetMovieDef();

		double x = Settings::iPositionX;
		double y = Settings::iPositionY;
		double scale = Settings::iScale;
		double opacity = Settings::iOpacity;

		x = (0 <= x && x <= 100) ? (x * def->GetWidth() * 0.01) : -1;
		y = (0 <= y && y <= 100) ? (y * def->GetHeight() * 0.01) : -1;
		if (scale >= 0)
		{
			if (scale < 25)
				scale = 25;
			else if (scale > 400)
				scale = 400;
		}
		if (opacity >= 0)
		{
			if (opacity > 100)
				opacity = 100;
		}

		GFxValue args[4];
		args[0].SetNumber(x);
		args[1].SetNumber(y);
		args[2].SetNumber(scale);
		args[3].SetNumber(opacity);
		view->Invoke("_root.Menu_mc.Setup", nullptr, args, 4);
	}
}

bool LootMenu::CanOpen(TESObjectREFR *a_ref)
{
	SimpleLocker lock(&ms_lock);

	if (!a_ref || !a_ref->baseForm)
		return false;

	RE::MenuManager *mm = reinterpret_cast<RE::MenuManager*>(MenuManager::GetSingleton());
	if (mm && mm->numPauseGame + mm->numStopCrosshairUpdate > 0)
		return false;

	RE::InputManager *mapping = reinterpret_cast<RE::InputManager*>(InputManager::GetSingleton());
	if (!mapping || !mapping->IsMovementControlsEnabled())
		return false;

	// player is grabbing / in favor state / in killmove
	if (reinterpret_cast<RE::PlayerCharacter*>((*g_thePlayer))->isGrabbing || CALL_MEMBER_FN(reinterpret_cast<RE::PlayerCharacter*>((*g_thePlayer)), GetActorInFavorState)() || IsInKillmove((*g_thePlayer)))
		return false;
	
	bool bAnimationDriven = false;
	static BSFixedString strAnimationDriven("bAnimationDriven");
	if (reinterpret_cast<RE::IAnimationGraphManagerHolder*>(&((*g_thePlayer)->animGraphHolder))->GetAnimationVariableBool(strAnimationDriven, bAnimationDriven) && bAnimationDriven)
		return false;

#if 0
	if ((*g_thePlayer)->GetVampireFeed() || (*g_thePlayer)->GetCanibal())
		return false;
#endif

	// in combat
	if (Settings::bDisableInCombat && (*g_thePlayer)->IsInCombat())
		return false;
	
	// theft
	if (Settings::bDisableTheft && CALL_MEMBER_FN(a_ref, IsOffLimits)())
		return false;

	if (IsActivationBlocked(&a_ref->extraData, 1))
		return false;
	
	TESObjectREFR *containerref = nullptr;
	TESForm *baseForm = a_ref->baseForm;

	if (baseForm->formType == kFormType_Activator)
	{
		UInt32 refHandle = 0;
		if (CALL_MEMBER_FN(a_ref->extraData, GetAshPileRefHandle_Impl)(refHandle) && refHandle != (*g_invalidRefHandle))
		{
			TESObjectREFR * refPtr = nullptr;
			if (LookupREFRByHandle(&refHandle, &refPtr))
				containerref = refPtr;
		}
	}
	else  if (baseForm->formType == kFormType_Container)
	{
		if (!IsLocked(a_ref))
			containerref = a_ref;
	}
	else if (baseForm->formType == kFormType_NPC)
	{
		if (a_ref->IsDead(true))
			containerref = a_ref;

		// pickpocket
		if (!Settings::bDisablePickpocketing && !a_ref->IsDead(true) && (!reinterpret_cast<RE::TESObjectREFR*>(a_ref)->IsChild()) && ((*g_thePlayer)->actorState.flags04 & ActorState::kState_Sneaking) != 0)
		{
			Actor * actorRef = (Actor*)a_ref;

			if ((actorRef->flags1 & Actor::kFlags_IsPlayerTeammate) == 0 && !reinterpret_cast<RE::Actor*>(actorRef)->IsInFaction((TESFaction*)LookupFormByID(0x05C84E)))
			{
				if (Unk1Func(actorRef))
					return false;

				if (actorRef->processManager && Unk2Func(actorRef->processManager))
					return false;

				if (actorRef->IsInCombat())
					return false;

				containerref = a_ref;
			}
		}
	}

	if (!containerref)
		return false;

	UInt32 numItems = CALL_MEMBER_FN(containerref, GetNumItems)(false, false);

	// empty container
	if (numItems == 0)
		return false;

	// do not open it which has any activate choice or replaced default.
	if (reinterpret_cast<RE::Actor*>((*g_thePlayer))->CanProcessEntryPointPerkEntry(RE::BGSEntryPointPerkEntry::kEntryPoint_Activate))
	{
		HasActivateChoiceVisitor visitor((*g_thePlayer), a_ref);
		reinterpret_cast<RE::Actor*>((*g_thePlayer))->VisitEntryPointPerkEntries(RE::BGSEntryPointPerkEntry::kEntryPoint_Activate, visitor);
		if (visitor.GetResult())
			return false;
	}

	return true;
}

void LootMenu::Close()
{
	SimpleLocker lock(&ms_lock);

	if (!containerRef)
		return;

	if (containerRef->formType != kFormType_Character)
		PlayAnimationClose();

	Clear();

	if (view)
		reinterpret_cast<RE::GFxMovieView*>(view)->SetVisible(false);
}

void LootMenu::Update()
{
	SimpleLocker lock(&ms_lock);

	//_MESSAGE("Start");
	//double PCFreq = 0.0;
	//__int64 CounterStart = 0;

	//LARGE_INTEGER li;
	//if (!QueryPerformanceFrequency(&li))
	//	_MESSAGE("QueryPerformanceFrequency failed!");

	//PCFreq = double(li.QuadPart) / 1000.0;

	//QueryPerformanceCounter(&li);
	//CounterStart = li.QuadPart;

	if (!containerRef)
		return;

	items.clear();

	//UInt32 numItems = CALL_MEMBER_FN(containerRef, GetNumItems)(false, false);
	//_MESSAGE("Name: %s, NumItems: %d", CALL_MEMBER_FN(containerRef, GetReferenceName)(), numItems);
	bool isPickPocketing = !Settings::bDisablePickpocketing && containerRef->baseForm->formType == kFormType_NPC && !containerRef->IsDead(true) && ((*g_thePlayer)->actorState.flags04 & ActorState::kState_Sneaking) != 0;

	m_owner = nullptr;
	//if (isPickPocketing)
	//{
	//	m_owner = containerRef->baseForm;
	//}

	if (containerRef->formType != kFormType_Character)
	{
		m_owner = CALL_MEMBER_FN(containerRef, GetOwner_Impl)();
	}

	TESContainer *container = DYNAMIC_CAST(containerRef->baseForm, TESForm, TESContainer);
	if (!container)
		return;

	std::map<TESForm *, SInt32> itemMap;
	//std::unordered_map<TESForm *, SInt32> itemMapU;

	TESContainer::Entry *entry = nullptr;
	UInt32 index = 0;
	TESContainerExt containerExt(container);
	while (containerExt.GetContainerItemAt(index++, entry))
	{
		if (!entry)
			continue;

		if (!IsValidItem(entry->form))
			continue;

		UInt32 count = container->CountItem(entry->form);
		itemMap[entry->form] = count;
		//itemMapU[entry->form] = count;

		//const char * name = "";
		//TESFullName * fullName = DYNAMIC_CAST(entry->form, TESForm, TESFullName);
		//if (fullName)
		//	name = fullName->name.data;

		//_MESSAGE("Item name %s, count %d", name, count);
	}

	ExtraContainerChanges *exChanges = static_cast<ExtraContainerChanges*>(containerRef->extraData.GetByType(kExtraData_ContainerChanges));
	ExtraContainerChanges::Data * changes = (exChanges) ? exChanges->data : nullptr;
	if (!changes)
	{
		void * p = (ExtraContainerChanges::Data*)Heap_Allocate(sizeof(ExtraContainerChanges::Data));
	
		changes = CALL_MEMBER_FN((ExtraContainerChanges::Data*)p, ctor)(containerRef);
		BaseExtraList_SetInventoryChanges(&(containerRef->extraData), changes);
		CALL_MEMBER_FN(changes, InitContainer)();
	}
	if (changes && changes->objList)
	{
		for (EntryDataList::Iterator iterator = changes->objList->Begin(); !iterator.End(); ++iterator)
		{
			InventoryEntryData *pEntry = iterator.Get();
	
			if (!pEntry)
				continue;

			TESForm *item = pEntry->type;
			if (!item)
				continue;

			if (!IsValidItem(item))
				continue;
	
			SInt32 totalCount = pEntry->countDelta; 
			SInt32 baseCount = 0;

			auto it = itemMap.find(item);
			if (it != itemMap.end())
				baseCount = it->second;
			if (baseCount < 0)
				continue;

			if (item->formID != 0xF)
				totalCount += baseCount;
			
			itemMap[item] = -1;		// mark as processed.
	
			if (totalCount <= 0)
				continue;

			InventoryEntryData *wornEntry = nullptr;
			InventoryEntryData *defaultEntry = nullptr;
			if (item->formID != 0xF && pEntry->extendDataList)
			{
				for (ExtendDataList::Iterator it = pEntry->extendDataList->Begin(); !it.End(); ++it)
				{
					BaseExtraList *extraList = it.Get();

					if (!extraList)
						continue;

					//const type_info& id = typeid(*extraList->m_data);
					//_MESSAGE("%s", id.name());

					if (isPickPocketing)
					{
						if (extraList->HasType(kExtraData_Worn) || extraList->HasType(kExtraData_WornLeft))
						{
							_MESSAGE("Has worn item formID %x", pEntry->type->formID);

							if (reinterpret_cast<RE::Actor*>((*g_thePlayer))->CanProcessEntryPointPerkEntry(RE::BGSEntryPointPerkEntry::kEntryPoint_Can_Pickpocket_Equipped_Item))
							{
								CanPickpocketEquippedItemVisitor visitor((*g_thePlayer), containerRef, pEntry->type);
								reinterpret_cast<RE::Actor*>((*g_thePlayer))->VisitEntryPointPerkEntries(RE::BGSEntryPointPerkEntry::kEntryPoint_Can_Pickpocket_Equipped_Item, visitor);
								if (!visitor.GetResult())
								{
									wornEntry = pEntry;
									continue;
								}
							}
							else
							{
								wornEntry = pEntry;
								continue;
							}
						}
					}

					int count = CALL_MEMBER_FN(extraList, GetItemCount_Impl)();

					if (count <= 0)
					{
						totalCount += count;
						continue;
					}
					if (totalCount <= 0)
						continue;

					totalCount -= count;

					InventoryEntryData *pNewEntry = nullptr;
					if (extraList->HasType(kExtraData_TextDisplayData) || extraList->HasType(kExtraData_Health))
					{
						//_MESSAGE("Has TextDisplayData");
						pNewEntry = InventoryEntryData::Create(item, count);
						InventoryEntryDataExt inventoryEntryDataExt(pNewEntry);
						inventoryEntryDataExt.AddEntryList(extraList);
					}
					else if (extraList->HasType(kExtraData_Ownership))
					{
						//_MESSAGE("Has Ownership");
						pNewEntry = InventoryEntryData::Create(item, count);
						InventoryEntryDataExt inventoryEntryDataExt(pNewEntry);
						inventoryEntryDataExt.AddEntryList(extraList);
					}
					else
					{
						if (!defaultEntry)
							defaultEntry = InventoryEntryData::Create(item, 0);

						InventoryEntryDataExt inventoryEntryDataExt(defaultEntry);
						inventoryEntryDataExt.AddEntryList(extraList);
						defaultEntry->countDelta += count;
					}

					if (pNewEntry)
					{
						items.push_back(ItemData(pNewEntry, m_owner));
					}
				}
			}

			if (totalCount > 0)	// rest
			{
				if (!defaultEntry)
					defaultEntry = InventoryEntryData::Create(item, 0);

				defaultEntry->countDelta += totalCount;
			}

			if (defaultEntry)
			{
				if (wornEntry)
					continue;

				items.push_back(ItemData(defaultEntry, m_owner));
			}
		}
	}

	for (auto &node : itemMap)
	{
		if (node.second <= 0)
			continue;

		if (!IsValidItem(node.first))
			continue;

		InventoryEntryData * entry = nullptr;
		entry = InventoryEntryData::Create(node.first, node.second);
		items.push_back(ItemData(entry, m_owner));
	}


	RE::ExtraDroppedItemList *exDroppedItemList = static_cast<RE::ExtraDroppedItemList*>(containerRef->extraData.GetByType(kExtraData_DroppedItemList));
	if (exDroppedItemList)
	{
		
		for (tList<UInt32>::Iterator it = exDroppedItemList->handles.Begin(); !it.End(); ++it)
		{
			UInt32 handle = reinterpret_cast<UInt32>(it.Get()); // this is stupid, but whatever

			if (handle == (*g_invalidRefHandle))
				continue;

			TESObjectREFR * refPtr = nullptr;
			LookupREFRByHandle(&handle, &refPtr);
			if (!refPtr)
				continue;
		
			if (!IsValidItem(refPtr->baseForm))
				continue;

			InventoryEntryData * entry = nullptr;
			entry = InventoryEntryData::Create(refPtr->baseForm, 1);
			InventoryEntryDataExt inventoryEntryDataExt(entry);
			inventoryEntryDataExt.AddEntryList(&refPtr->extraData);
			items.push_back(ItemData(entry, m_owner));
		}
	}

	//_MESSAGE("item size %d", items.size());

	if (!items.empty())
	{
		Sort();
	}

	InvokeScaleform_Open();

	m_bUpdateRequest = false;

	//QueryPerformanceCounter(&li);
	//double GetCounter = (double)(li.QuadPart - CounterStart) / PCFreq;

	//_MESSAGE("Finish %f", GetCounter);
}

void LootMenu::Sort()
{
	qsort(&items[0], items.size(), sizeof(ItemData), [](const void *pA, const void *pB) -> int {
		const ItemData &a = *(const ItemData *)pA;
		const ItemData &b = *(const ItemData *)pB;
	
		if (a.pEntry == b.pEntry)
			return 0;
		return (a < b) ? -1 : 1;
	});
}

void LootMenu::SetIndex(SInt32 index)
{
	SimpleLocker lock(&ms_lock);

	if (containerRef)
	{
		const int tail = items.size() - 1;
		m_selectedIndex += index;
		if (m_selectedIndex > tail)
			m_selectedIndex = tail;
		else if (m_selectedIndex < 0)
			m_selectedIndex = 0;

		InvokeScaleform_SetIndex();
	}

}

void LootMenu::TakeItem()
{
	SimpleLocker lock(&ms_lock);

	if (!containerRef)
		return;
	
	if (m_selectedIndex < 0 || items.size() <= m_selectedIndex)
		return;

	m_bNowTaking = true;
	m_bTakeAll = false;

	auto it = items.begin() + m_selectedIndex;
	InventoryEntryData *item = nullptr;
	item = it->pEntry;

	bool bTakeSingle = false;
	bool isPickpocket = false;
	bool pickSuccess = true;

	if (!item)
		return;

	BaseExtraList *extraData = nullptr;
	if (item->extendDataList)
	{
		for (ExtendDataList::Iterator it = item->extendDataList->Begin(); !it.End(); ++it)
		{
			extraData = it.Get();
			if (extraData)
				break;
		}
	}

	if (extraData && extraData->HasType(kExtraData_ItemDropper))
	{
		// picks up the weapons that dropped on the ground. 

		TESObjectREFR* refItem = (TESObjectREFR*)((UInt64)extraData - 0x70);
		reinterpret_cast<RE::TESObjectREFR*>((*g_thePlayer))->PickUpItem(refItem, 1, false, true);
	}
	else
	{
		UInt32 lootMode = RE::TESObjectREFR::kRemoveType_Take;			// take
		UInt32 numItems = item->countDelta;

		InputEventDispatcher * dispatcher = InputEventDispatcher::GetSingleton();
		if (dispatcher)
		{
			if (dispatcher->IsGamepadEnabled())
			{
				if (numItems > 1 && isGamepadKeyDown)
				{
					bTakeSingle = true;
					numItems = 1;
				}
			}
			else
			{
				if (numItems > 1 && isKeyboardKeyDown)
				{
					bTakeSingle = true;
					numItems = 1;
				}
			}
		}

		if (containerRef->baseForm->formType == kFormType_NPC)
		{
			if (containerRef->IsDead(false))		// dead body
			{
				CALL_MEMBER_FN((*g_thePlayer), PlayPickupEvent)(item->type, m_owner, containerRef, 6); // 6 = dead body
			}
			else
			{
				if (!Settings::bDisablePickpocketing)
				{
					Actor * actorRef = (Actor*)containerRef;
					pickSuccess = CALL_MEMBER_FN((*g_thePlayer), TryToPickpocket)(actorRef, item, numItems, true);
					CALL_MEMBER_FN((*g_thePlayer), PlayPickupEvent)(item->type, m_owner, containerRef, 3); // 3 = thief
					lootMode = RE::TESObjectREFR::kRemoveType_Steal;
					isPickpocket = true;
				}
			}
		}
		else									// container
		{
			CALL_MEMBER_FN((*g_thePlayer), PlayPickupEvent)(item->type, m_owner, containerRef, 5); // 5 = container

			if (CALL_MEMBER_FN(containerRef, IsOffLimits)())
				lootMode = RE::TESObjectREFR::kRemoveType_Steal;		// steal

			if (!m_bOpenAnim)
			{
				SendChestLootedEvent();	
			}
		}

		if (!pickSuccess)
		{
			CALL_MEMBER_FN((*g_thePlayer), OnCrosshairRefChanged)();
			return;
		}

		UInt32 handle = 0;
		reinterpret_cast<RE::TESObjectREFR*>(containerRef)->RemoveItem(&handle, item->type, numItems, lootMode, extraData, (*g_thePlayer), 0, 0);

		// remove arrow projectile 3D.
		static_cast<RE::TESBoundObject*>(item->type)->OnRemovedFrom(containerRef);

		if (containerRef->formType == kFormType_Character)
		{
			Actor *actor = static_cast<Actor*>(containerRef);
			if (actor->processManager)
			{
				// dispel worn item enchants
				typedef void FnDispelWornItemEnchants_t(Actor*);
				RelocAddr<FnDispelWornItemEnchants_t*> FnDispelWornItemEnchants(0x00557110);  // 1_5_50

				FnDispelWornItemEnchants(actor);
				CALL_MEMBER_FN(actor->processManager, UpdateEquipment)(actor);
			}
			if (isPickpocket)
				SendItemsPickpocketedEvent(numItems);
		}
		else
		{
			if (CALL_MEMBER_FN(containerRef, IsOffLimits)())
			{
				UInt32 totalValue = CALL_MEMBER_FN(item, GetValue)() * numItems;
				CALL_MEMBER_FN((*g_thePlayer), SendStealAlarm)(containerRef, nullptr, 0, totalValue, m_owner, true);
			}

			PlayAnimationOpen();
		}

		// plays item pickup sound
		reinterpret_cast<RE::Actor*>((*g_thePlayer))->PlaySounds(item->type, true, false); // actor->UnkA3(TESForm *, bool isPickup, bool unk)
	}

	m_bNowTaking = false;

	if (bTakeSingle)
	{
		Update();
	}
	else
	{
		items.erase(it);
		InvokeScaleform_Open();
	}
}

void LootMenu::TakeAllItems()
{
	SimpleLocker lock(&ms_lock);

	if (!containerRef)
		return;

	if (containerRef->baseForm->formType == kFormType_NPC)
	{
		if (!containerRef->IsDead(true) && ((*g_thePlayer)->actorState.flags04 & ActorState::kState_Sneaking) != 0)
		{
			DebugNotification("TakeAllItems feature cannot be used for pickpocketing.", false, true);
			return;
		}
	}
	else
	{
		if (CALL_MEMBER_FN(containerRef, IsOffLimits)())
		{
			DebugNotification("TakeAllItems feature cannot be used for stealing.", false, true);
			return;
		}
	}

	if (items.empty())
		return;

	//_MESSAGE("Start");
	//double PCFreq = 0.0;
	//__int64 CounterStart = 0;

	//LARGE_INTEGER li;
	//if (!QueryPerformanceFrequency(&li))
	//	_MESSAGE("QueryPerformanceFrequency failed!");

	//PCFreq = double(li.QuadPart) / 1000.0;

	//QueryPerformanceCounter(&li);
	//CounterStart = li.QuadPart;

	m_bTakeAll = true;

	while (m_selectedIndex < items.size())
	{
		m_bNowTaking = true;

		auto it = items.begin() + m_selectedIndex;
		//auto it = items.begin() + (items.size() - 1);

		InventoryEntryData *item = nullptr;
		item = it->pEntry;

		if (!item)
			return;

		BaseExtraList *extraData = nullptr;
		if (item->extendDataList)
		{
			for (ExtendDataList::Iterator it = item->extendDataList->Begin(); !it.End(); ++it)
			{
				extraData = it.Get();
				if (extraData)
					break;
			}
		}

		if (extraData && extraData->HasType(kExtraData_ItemDropper))
		{
			// picks up the weapons that dropped on the ground. 

			TESObjectREFR* refItem = (TESObjectREFR*)((UInt64)extraData - 0x70);
			reinterpret_cast<RE::TESObjectREFR*>((*g_thePlayer))->PickUpItem(refItem, 1, false, true);
		}
		else
		{
			UInt32 lootMode = RE::TESObjectREFR::kRemoveType_Take;			// take
			UInt32 numItems = item->countDelta;

			if (containerRef->baseForm->formType == kFormType_NPC)
			{
				if (containerRef->IsDead(false))		// dead body
				{
					CALL_MEMBER_FN((*g_thePlayer), PlayPickupEvent)(item->type, m_owner, containerRef, 6); // 6 = dead body
				}
			}
			else									// container
			{
				CALL_MEMBER_FN((*g_thePlayer), PlayPickupEvent)(item->type, m_owner, containerRef, 5); // 5 = container

				if (!m_bOpenAnim)
				{
					SendChestLootedEvent();
				}
			}

			UInt32 handle = 0;
			reinterpret_cast<RE::TESObjectREFR*>(containerRef)->RemoveItem(&handle, item->type, numItems, lootMode, extraData, (*g_thePlayer), 0, 0);

			// remove arrow projectile 3D.
			static_cast<RE::TESBoundObject*>(item->type)->OnRemovedFrom(containerRef);

			if (containerRef->formType == kFormType_Character)
			{
				Actor *actor = static_cast<Actor*>(containerRef);
				if (actor->processManager)
				{
					// dispel worn item enchants
					typedef void FnDispelWornItemEnchants_t(Actor*);
					RelocAddr<FnDispelWornItemEnchants_t*> FnDispelWornItemEnchants(0x00557110);  // 1_5_50

					FnDispelWornItemEnchants(actor);
					CALL_MEMBER_FN(actor->processManager, UpdateEquipment)(actor);
				}
			}
			else
			{
				PlayAnimationOpen();
			}

			// plays item pickup sound
			reinterpret_cast<RE::Actor*>((*g_thePlayer))->PlaySounds(item->type, true, false); // actor->UnkA3(TESForm *, bool isPickup, bool unk)
		}

		m_bNowTaking = false;

		items.erase(it);
		//items.pop_back();
		m_selectedIndex = 0;
	}

	InvokeScaleform_Open();
	m_bTakeAll = false;

	//QueryPerformanceCounter(&li);
	//double GetCounter = (double)(li.QuadPart - CounterStart) / PCFreq;

	//_MESSAGE("Finish %f", GetCounter);
}

// unused, the legacy one
void LootMenu::PlaySounds(TESForm *item)
{
	typedef bool FnPlaySound_t(BGSSoundDescriptorForm *, UInt32 flag, const NiPoint3 *, NiNode *);
	RelocAddr<FnPlaySound_t*> fnPlaySound(0x004F65A0);  // 1_5_50

	typedef BGSSoundDescriptorForm* _GetBGSSoundDescriptorForm(const char * soundDesc);
	RelocAddr<_GetBGSSoundDescriptorForm*> GetBGSSoundDescriptorForm(0x004F6750);  // 1_5_50

	if (item)
	{
		switch (item->formType)
		{
		case kFormType_Ammo:
		{
			BGSPickupPutdownSounds * mainSounds = DYNAMIC_CAST(item, TESForm, BGSPickupPutdownSounds);
			if (mainSounds && mainSounds->pickUp)
				fnPlaySound(mainSounds->pickUp, 0, &containerRef->pos, (*g_thePlayer)->GetNiNode());
			else
			{
				BGSSoundDescriptorForm * sounds = GetBGSSoundDescriptorForm("ITMArrowsUpSD");
				if (sounds)
					fnPlaySound(sounds, 0, &containerRef->pos, (*g_thePlayer)->GetNiNode());
			}
		}
			break;
		case kFormType_Armor:
		{
			BGSPickupPutdownSounds * mainSounds = DYNAMIC_CAST(item, TESForm, BGSPickupPutdownSounds);
			if (mainSounds && mainSounds->pickUp)
				fnPlaySound(mainSounds->pickUp, 0, &containerRef->pos, (*g_thePlayer)->GetNiNode());
			else
			{
				BGSSoundDescriptorForm * sounds = GetBGSSoundDescriptorForm("ITMGenericArmorUpSD");
				if (sounds)
					fnPlaySound(sounds, 0, &containerRef->pos, (*g_thePlayer)->GetNiNode());
			}
		}
			break;
		case kFormType_Book:
		{
			BGSPickupPutdownSounds * mainSounds = DYNAMIC_CAST(item, TESForm, BGSPickupPutdownSounds);
			if (mainSounds && mainSounds->pickUp)
				fnPlaySound(mainSounds->pickUp, 0, &containerRef->pos, (*g_thePlayer)->GetNiNode());
			else
			{
				BGSSoundDescriptorForm * sounds = GetBGSSoundDescriptorForm("ITMGenericBookUpSD");
				if (sounds)
					fnPlaySound(sounds, 0, &containerRef->pos, (*g_thePlayer)->GetNiNode());
			}
		}
			break;
		case kFormType_Ingredient:
		{
			BGSPickupPutdownSounds * mainSounds = DYNAMIC_CAST(item, TESForm, BGSPickupPutdownSounds);
			if (mainSounds && mainSounds->pickUp)
				fnPlaySound(mainSounds->pickUp, 0, &containerRef->pos, (*g_thePlayer)->GetNiNode());
			else
			{
				BGSSoundDescriptorForm * sounds = GetBGSSoundDescriptorForm("ITMGenericIngredientUpSD");
				if (sounds)
					fnPlaySound(sounds, 0, &containerRef->pos, (*g_thePlayer)->GetNiNode());
			}
		}
			break;
		case kFormType_Weapon:
		{
			BGSPickupPutdownSounds * mainSounds = DYNAMIC_CAST(item, TESForm, BGSPickupPutdownSounds);
			if (mainSounds && mainSounds->pickUp)
				fnPlaySound(mainSounds->pickUp, 0, &containerRef->pos, (*g_thePlayer)->GetNiNode());
			else
			{
				BGSSoundDescriptorForm * sounds = GetBGSSoundDescriptorForm("ITMGenericWeaponUpSD");
				if (sounds)
					fnPlaySound(sounds, 0, &containerRef->pos, (*g_thePlayer)->GetNiNode());
			}
		}
			break;
		case kFormType_Key:
		{
			BGSPickupPutdownSounds * mainSounds = DYNAMIC_CAST(item, TESForm, BGSPickupPutdownSounds);
			if (mainSounds && mainSounds->pickUp)
				fnPlaySound(mainSounds->pickUp, 0, &containerRef->pos, (*g_thePlayer)->GetNiNode());
			else
			{
				BGSSoundDescriptorForm * sounds = GetBGSSoundDescriptorForm("ITMKeyUpSD");
				if (sounds)
					fnPlaySound(sounds, 0, &containerRef->pos, (*g_thePlayer)->GetNiNode());
			}
		}
			break;
		default:
		{
			BGSPickupPutdownSounds * mainSounds = DYNAMIC_CAST(item, TESForm, BGSPickupPutdownSounds);
			if (mainSounds && mainSounds->pickUp)
				fnPlaySound(mainSounds->pickUp, 0, &containerRef->pos, (*g_thePlayer)->GetNiNode());
			else
			{
				BGSSoundDescriptorForm * sounds = GetBGSSoundDescriptorForm("ITMGenericUpSD");
				if (sounds)
					fnPlaySound(sounds, 0, &containerRef->pos, (*g_thePlayer)->GetNiNode());
			}
		}
			break;
		}
	}
	else
	{
		BGSSoundDescriptorForm * sounds = GetBGSSoundDescriptorForm("ITMGenericUpSD");
		if (sounds)
			fnPlaySound(sounds, 0, &containerRef->pos, (*g_thePlayer)->GetNiNode());
	}
}

void LootMenu::PlayAnimation(const char *fromName, const char *toName)
{
	NiNode *niNode = containerRef->GetNiNode();
	if (!niNode)
		return;
	NiTimeController *controller = niNode->m_controller;
	if (!controller)
		return;
	RE::NiControllerManager* manager = static_cast<RE::NiControllerManager*>(controller);
	if (!manager)
		return;
	RE::NiControllerSequence *fromSeq = CALL_MEMBER_FN(manager, GetSequenceByName)(fromName);
	RE::NiControllerSequence *toSeq = CALL_MEMBER_FN(manager, GetSequenceByName)(toName);
	if (!fromSeq || !toSeq)
		return;

	typedef void Fn_t(TESObjectREFR *, RE::NiControllerManager *, RE::NiControllerSequence *, RE::NiControllerSequence *, bool);
	RelocAddr<Fn_t*> fn(0x0018A020);  // 1_5_50

	fn(containerRef, manager, toSeq, fromSeq, false);
}

void LootMenu::PlayAnimationOpen()
{
	if (containerRef && !m_bOpenAnim)
	{
		PlayAnimation("Close", "Open");
		if (containerRef->formType != kFormType_Character)
			CALL_MEMBER_FN(containerRef, ActivateRefChildren)(*g_thePlayer);
	}
	m_bOpenAnim = true;
}

void LootMenu::PlayAnimationClose()
{
	if (this)
	{
		if (containerRef && m_bOpenAnim)
			PlayAnimation("Open", "Close");
		m_bOpenAnim = false;
	}
}

void LootMenu::SendChestLootedEvent()
{
	typedef void FnSendChestLootedEvent_t();
	RelocAddr<FnSendChestLootedEvent_t*> fnSendChestLootedEvent(0x008607B0);  // 1_5_50

	(*fnSendChestLootedEvent)();
}

void LootMenu::SendItemsPickpocketedEvent(UInt32 numItems)
{
	typedef void FnSendItemsPickpocketedEvent_t(UInt32);
	RelocAddr<FnSendItemsPickpocketedEvent_t*> fnSendItemsPickpocketedEvent(0x008607E0);  // 1_5_50

	(*fnSendItemsPickpocketedEvent)(numItems);
}

void LootMenu::InvokeScaleform_Open()
{
	if (items.empty())
	{
		view->Invoke("_root.Menu_mc.openContainer", nullptr, nullptr, 0);
		InvokeScaleform_SetContainer();

		CALL_MEMBER_FN((*g_thePlayer), OnCrosshairRefChanged)();

		return;
	}
	
	if (!m_bTakeAll)
	{
		if (m_selectedIndex >= items.size())
			m_selectedIndex = items.size() - 1;
		else if (m_selectedIndex < 0)
			m_selectedIndex = 0;
	}
	else
	{
		m_selectedIndex = 0;
	}

	LootMenuUIDelegate::Queue(".openContainer", &LootMenu::SetScaleformArgs_OpenContainer);
	InvokeScaleform_SetContainer();
}

void LootMenu::InvokeScaleform_SetContainer()
{
	LootMenuUIDelegate::Queue(".setContainer", &LootMenu::SetScaleformArgs_SetContainer);
}

void LootMenu::InvokeScaleform_SetIndex()
{
	LootMenuUIDelegate::Queue(".setSelectedIndex", &LootMenu::SetScaleformArgs_SetIndex);
}

void LootMenu::SetScaleformArgs_OpenContainer(std::vector<GFxValue> &args)
{
	SimpleLocker lock(&ms_lock);

	GFxValue argItems;
	view->CreateArray(&argItems);

	for (int i = 0; i < items.size(); ++i)
	{
		ItemData &itemData = items[i];
		GFxValue text;
		text.SetString(itemData.GetName());
		GFxValue count;
		count.SetNumber((double)itemData.GetCount());
		GFxValue value;
		value.SetNumber((double)itemData.GetValue());
		GFxValue weight;
		weight.SetNumber(itemData.GetWeight());
		GFxValue isStolen;
		if (containerRef->baseForm->formType == kFormType_NPC)
		{
			isStolen.SetBool((!containerRef->IsDead(true) && ((*g_thePlayer)->actorState.flags04 & ActorState::kState_Sneaking) != 0) ? true : itemData.IsStolen()); // kind of cheating, but whatever
		}
		else
			isStolen.SetBool(itemData.IsStolen());
		GFxValue iconLabel;
		iconLabel.SetString(itemData.GetIcon());
		GFxValue itemIndex;
		itemIndex.SetNumber((double)0);
		GFxValue itemChance;
		itemChance.SetNumber((double)itemData.GetPickpocketChance());

		GFxValue item;
		view->CreateObject(&item);
		item.SetMember("text", &text);
		item.SetMember("count", &count);
		item.SetMember("value", &value);
		item.SetMember("weight", &weight);
		item.SetMember("isStolen", &isStolen);
		item.SetMember("iconLabel", &iconLabel);
		item.SetMember("itemChance", &itemChance);

		TESForm *form = itemData.pEntry->type;
		if (form->formType == kFormType_Book)
		{
			TESObjectBOOK *book = static_cast<TESObjectBOOK*>(form);
			GFxValue isRead;
			isRead.SetBool((book->data.flags & TESObjectBOOK::Data::kType_Read) == TESObjectBOOK::Data::kType_Read);
			item.SetMember("isRead", &isRead);
		}

		if (form->IsArmor() || form->IsWeapon())
		{
			GFxValue isEnchanted;
			isEnchanted.SetBool(itemData.IsEnchanted());
			item.SetMember("isEnchanted", &isEnchanted);
		}

		argItems.PushBack(&item);
	}

	args.push_back(argItems);
}

void LootMenu::SetScaleformArgs_SetContainer(std::vector<GFxValue> &args)
{
	SimpleLocker lock(&ms_lock);

	GFxValue argRefID;
	argRefID.SetNumber((double)containerRef->formID);
	GFxValue argTitle;
	argTitle.SetString(CALL_MEMBER_FN(containerRef, GetReferenceName)());
	GFxValue argTake;
	GFxValue argSearch;
	GFxValue argTakeAll;

	// I'm an idiot, but whatever LOL
	if (!Settings::bSelfTranslation)
	{
		static char *sPickpocket = (*g_gameSettingCollection)->Get("sPickpocket")->data.s;
		static char *sSteal = (*g_gameSettingCollection)->Get("sSteal")->data.s;
		static char *sTake = (*g_gameSettingCollection)->Get("sTake")->data.s;
		static char *sSearch = (*g_gameSettingCollection)->Get("sSearch")->data.s;
		static char *sTakeAll = (*g_gameSettingCollection)->Get("sTakeAll")->data.s;

		if (containerRef->baseForm->formType == kFormType_NPC)
		{
			argTake.SetString((!containerRef->IsDead(true) && ((*g_thePlayer)->actorState.flags04 & ActorState::kState_Sneaking) != 0) ? sPickpocket : sTake);
		}
		else
			argTake.SetString((CALL_MEMBER_FN(containerRef, IsOffLimits)()) ? sSteal : sTake);

		argSearch.SetString(sSearch);
		argTakeAll.SetString(sTakeAll);
	}
	else
	{
		const wchar_t *sPickpocket = Settings::sPickPocket;
		const wchar_t *sSteal = Settings::sSteal;
		const wchar_t *sTake = Settings::sTake;
		const wchar_t *sSearch = Settings::sSearch;
		const wchar_t *sTakeAll = Settings::sTakeAll;

		if (containerRef->baseForm->formType == kFormType_NPC)
		{
			argTake.SetWideString((!containerRef->IsDead(true) && ((*g_thePlayer)->actorState.flags04 & ActorState::kState_Sneaking) != 0) ? sPickpocket : sTake);
		}
		else
			argTake.SetWideString((CALL_MEMBER_FN(containerRef, IsOffLimits)()) ? sSteal : sTake);

		argSearch.SetWideString(sSearch);
		argTakeAll.SetWideString(sTakeAll);
	}
	
	GFxValue argSelectedIndex;
	argSelectedIndex.SetNumber((double)m_selectedIndex);

	args.push_back(argRefID);				
	args.push_back(argTitle);				
	args.push_back(argTake);				
	args.push_back(argSearch);
	args.push_back(argTakeAll);
	args.push_back(argSelectedIndex);		
}


void LootMenu::SetScaleformArgs_SetIndex(std::vector<GFxValue> &args)
{
	SimpleLocker lock(&ms_lock);

	GFxValue idx;
	idx.SetNumber((double)m_selectedIndex);

	args.push_back(idx);
}

void LootMenu::OnMenuOpen()
{
	SimpleLocker lock(&ms_lock);

	if (containerRef)	// is another container already opened ?
	{
		if (containerRef->baseForm->formType == kFormType_Activator)
		{
			UInt32 refHandle = 0;
			if (CALL_MEMBER_FN(containerRef->extraData, GetAshPileRefHandle_Impl)(refHandle) && refHandle != (*g_invalidRefHandle))
			{
				TESObjectREFR * refPtr = nullptr;
				if (LookupREFRByHandle(&refHandle, &refPtr))
					containerRef = refPtr;
			}
		}
	}

	if (view)
	{
		ms_pSingleton = this;
		m_selectedIndex = 0;

		RegisterMenuEventHandler(MenuControls::GetSingleton(), this);

		Setup();
		Update();

		reinterpret_cast<RE::GFxMovieView*>(view)->SetVisible(true);
	}

}

void LootMenu::OnMenuClose()
{
	SimpleLocker lock(&ms_lock);

	if (view)
	{
		ms_pSingleton = nullptr;

		Clear();

		reinterpret_cast<RE::GFxMovieView*>(view)->SetVisible(false);

		RemoveMenuEventHandler(MenuControls::GetSingleton(), this);
	}
}

void LootMenu::Clear()
{
	SimpleLocker lock(&ms_lock);

	m_owner = nullptr;
	m_selectedIndex = -1;

	items.clear();

	m_bNowTaking = false;
	m_bOpenAnim = false;
	m_bUpdateRequest = false;
	m_bTakeAll = false;
}

void LootMenu::Init(void)
{
	MenuManager * mm = MenuManager::GetSingleton();
	if (!mm)
		return;

	mm->Register("LootMenu", LootMenuCreator::Create);
}

UInt32 LootMenu::ProcessMessage(UIMessage * message)
{
	UInt32 result = 2;

	if (view)
	{
		switch (message->message)
		{
		case UIMessage::kMessage_Open:
			OnMenuOpen();
			break;
		case UIMessage::kMessage_Close:
			OnMenuClose();
			break;
		case RE::UIMessage::kMessage_Scaleform:
			if (reinterpret_cast<RE::GFxMovieView*>(view)->GetVisible() && message->objData)
			{
				RE::BSUIScaleformData *scaleformData = (RE::BSUIScaleformData *)message->objData;

				GFxEvent *event = scaleformData->event;

				if (event->type == GFxEvent::MouseWheel)
				{
					GFxMouseEvent *mouse = (GFxMouseEvent *)event;
					if (mouse->scrollDelta > 0)
						SetIndex(-1);
					else if (mouse->scrollDelta < 0)
						SetIndex(1);
				}
				else if (event->type == GFxEvent::KeyDown)
				{
					GFxKeyEvent *key = (GFxKeyEvent *)event;
					if (key->keyCode == GFxKey::Up)
						SetIndex(-1);
					else if (key->keyCode == GFxKey::Down)
						SetIndex(1);
				}
				else if (event->type == GFxEvent::CharEvent)
				{
					GFxCharEvent *charEvent = (GFxCharEvent *)event;
				}
			}
		}
	}

	return result;
}

bool LootMenu::CanProcess(InputEvent *evn)
{
	if (!containerRef)
		return false;

	if (!view)
		return false;

	if (!reinterpret_cast<RE::GFxMovieView*>(view)->GetVisible())
		return false;

	if (evn->eventType != InputEvent::kEventType_Button)
		return false;

	ButtonEvent *button = static_cast<ButtonEvent *>(evn);
	if (evn->deviceType == kDeviceType_Gamepad)
	{
		return (button->keyMask == 0x1 || button->keyMask == 0x2);
	}
	else if (evn->deviceType == kDeviceType_Mouse)
	{
		return (button->keyMask == 0x9 || button->keyMask == 0x8);
	}
	else if (evn->deviceType == kDeviceType_Keyboard)
	{
		InputStringHolder *holder = InputStringHolder::GetSingleton();
		return (evn->GetControlID() == &holder->zoomIn || evn->GetControlID() == &holder->zoomOut);
	}

	return false;
}


bool LootMenu::ProcessButton(ButtonEvent *evn)
{
	if (!reinterpret_cast<RE::ButtonEvent*>(evn)->IsDown())
		return true;

	if (evn->deviceType == kDeviceType_Gamepad)
	{
		if (evn->keyMask == 0x1)
			SetIndex(-1);
		else if (evn->keyMask == 0x2)
			SetIndex(1);
	}
	else if (evn->deviceType == kDeviceType_Mouse)
	{
		if (evn->keyMask == 0x8)
			SetIndex(-1);
		else if (evn->keyMask == 0x9)
			SetIndex(1);
	}
	else if (evn->deviceType == kDeviceType_Keyboard)
	{
		InputStringHolder *holder = InputStringHolder::GetSingleton();
		if (evn->GetControlID() == &holder->zoomIn)
			SetIndex(-1);
		else if (evn->GetControlID() == &holder->zoomOut)
			SetIndex(1);
	}

	return true;
}