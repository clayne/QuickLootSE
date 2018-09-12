#pragma once

#include "skse64/PluginAPI.h"
#include "skse64/GameMenus.h"
#include "skse64/ScaleformLoader.h"
#include <vector>
#include "ItemData.h"
#include "MenuHandler.h"

extern TESObjectREFR		* containerRef;
extern bool isDisabled;
extern SKSETaskInterface	* g_task;
extern bool IsValidItem(TESForm * item);

class LootMenuCreator
{
public:
	static IMenu* Create(void);

private:
	LootMenuCreator();
};

class LootMenu : public IMenu,
	public MenuHandler::MenuEventHandler
{
public:
	static LootMenu *			ms_pSingleton;
	static SimpleLock			ms_lock;

	TESForm *				m_owner;
	SInt32					m_selectedIndex;
	std::vector<ItemData>	items;
	bool					m_bNowTaking;
	bool					m_bOpenAnim;
	bool					m_bUpdateRequest;
	bool					m_bTakeAll;

	LootMenu(const char* swfPath);
	virtual ~LootMenu();

	virtual UInt32 ProcessMessage(UIMessage * message) override;
	virtual bool CanProcess(InputEvent *evn) override;
	virtual bool ProcessButton(ButtonEvent *evn) override;

	void Setup();
	static bool CanOpen(TESObjectREFR * ref);
	void Close();
	void Update();
	void Sort();
	void SetIndex(SInt32 index);
	void TakeItem();
	void TakeAllItems();
	void PlaySounds(TESForm *item);
	void PlayAnimation(const char *fromName, const char *toName);
	void PlayAnimationOpen();
	void PlayAnimationClose();
	void SendChestLootedEvent();
	void SendItemsPickpocketedEvent(UInt32 numItems);

	void InvokeScaleform_Open();
	void InvokeScaleform_SetContainer();
	void InvokeScaleform_SetIndex();
	void SetScaleformArgs_OpenContainer(std::vector<GFxValue> &args);
	void SetScaleformArgs_SetContainer(std::vector<GFxValue> &args);
	void SetScaleformArgs_SetIndex(std::vector<GFxValue> &args);

	void OnMenuOpen();
	void OnMenuClose();
	void Clear();

	static void Init(void);
	static LootMenu * GetSingleton()	{ return ms_pSingleton; }
};
