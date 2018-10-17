#include "skse64_common/BranchTrampoline.h"
#include "skse64_common/SafeWrite.h"
#include "skse64/GameInput.h"
#include "skse64/GameData.h"
#include "xbyak/xbyak.h"
#include "Hooks.h"
#include "LootMenu.h"
#include "RE_GFxMovieView.h"
#include "RE_ButtonEvent.h"
#include "RE_PlayerControls.h"
#include "RE_ReadyWeaponHandler.h"


struct SprintHandler;
struct RunHandler;
struct TogglePOVHandler;


RelocPtr<uintptr_t> ThirdPersonVtbl(0x01649388);  // 1_5_50
typedef bool _TPCanProcess(PlayerInputHandler * handler, InputEvent * evn);
RelocAddr<_TPCanProcess*> TPCanProcess(0x0084FD50);  // 1_5_50

RelocPtr<uintptr_t> FirstPersonVtbl(0x016C3F10);  // 1_5_50
typedef bool _FPCanProcess(PlayerInputHandler * handler, InputEvent * evn);
RelocAddr<_FPCanProcess*> FPCanProcess(0x00847F50);  // 1_5_50

RelocPtr<uintptr_t> StartActivationAddr(0x00708FCF);  // 1_5_50
typedef void _StartActivationFunc(PlayerCharacter * player);
RelocAddr<_StartActivationFunc*> StartActivationFunc(0x006AA180);  // 1_5_50

RelocPtr<uintptr_t> ReadyWeaponHandlerAddrVtbl(0x016892B8);  // 1_5_50
typedef void ReadyWeaponHandler_t(RE::ReadyWeaponHandler * handler, ButtonEvent* evn, RE::PlayerControls::Data24 * data);
RelocAddr<ReadyWeaponHandler_t*> ReadyWeaponHandler_(0x00709500);  // 1_5_50

RelocPtr<uintptr_t> TESOBjectCONTvtbl(0x01573990);  // 1_5_50
typedef bool _GetCrosshairTextCONT(TESObjectCONT * cont, TESObjectREFR *ref, BSString * dst);
RelocAddr<_GetCrosshairTextCONT*> GetCrosshairTextCONT(0x0022BB80);  // 1_5_50

RelocPtr<uintptr_t> TESOBjectACTIvtbl(0x01571750);  // 1_5_50
typedef bool _GetCrosshairTextACTI(TESObjectACTI * acti, TESObjectREFR *ref, BSString * dst);
RelocAddr<_GetCrosshairTextACTI*> GetCrosshairTextACTI(0x00224910);  // 1_5_50

RelocPtr<uintptr_t> TESNPCvtbl(0x015B9D30);  // 1_5_50
typedef bool _GetCrosshairTextNPC(TESNPC * npc, TESObjectREFR *ref, BSString * dst);
RelocAddr<_GetCrosshairTextNPC*> GetCrosshairTextNPC(0x00361830);  // 1_5_50

RelocPtr<uintptr_t> FavoriteHandlervtbl(0x016D2518);  // 1_5_50
typedef bool _FavoriteCanProcess(MenuHandler::MenuEventHandler * handler, InputEvent * evn);
RelocAddr<_FavoriteCanProcess*> FavoriteCanProcess(0x008AAE80);  // 1_5_50

RelocPtr<uintptr_t> PlayerRunHandlervtbl(0x01689388);  // 1_5_50
typedef bool _PlayerRunCanProcess(RunHandler * handler, InputEvent * evn);
RelocAddr<_PlayerRunCanProcess*> PlayerRunCanProcess(0x0070A9A0);  // 1_5_50

RelocPtr<uintptr_t> PlayerSprintHandlervtbl(0x01689238);  // 1_5_50
typedef bool _PlayerSprintCanProcess(SprintHandler * handler, InputEvent * evn);
RelocAddr<_PlayerSprintCanProcess*> PlayerSprintCanProcess(0x0070AA80);  // 1_5_50

RelocPtr<uintptr_t> TogglePOVHandlervtbl(0x01689478);  // 1_5_50
typedef bool _TogglePOVProcessButton(TogglePOVHandler * handler, ButtonEvent* evn, RE::PlayerControls::Data24* unk024);
RelocAddr<_TogglePOVProcessButton*> TogglePOVProcessButton(0x007097F0);  // 1_5_50

RelocPtr<uintptr_t> retnAddrEx(0x006B08D6);  // 1_5_50

RelocAddr<DebugNotification_t> DebugNotification(0x008DA5C0);  // 1_5_50

bool isKeyboardKeyDown = false;
bool isGamepadKeyDown = false;

bool ThirdPersonCanProcess(PlayerInputHandler * handler, InputEvent* evn)
{
	bool result = TPCanProcess(handler, evn);
	BSFixedString s("LootMenu");
	GFxMovieView * view = NULL;
	MenuManager * mm = MenuManager::GetSingleton();
	if (mm)
		view = mm->GetMovieView(&s);

	if (result && view && reinterpret_cast<RE::GFxMovieView*>(view)->GetVisible())
	{
		result = (evn->GetControlID() == &InputStringHolder::GetSingleton()->togglePOV);
	}

	return result;
}

bool FirstPersonCanProcess(PlayerInputHandler * handler, InputEvent* evn)
{
	bool result = FPCanProcess(handler, evn);
	BSFixedString s("LootMenu");
	GFxMovieView * view = NULL;
	MenuManager * mm = MenuManager::GetSingleton();
	if (mm)
		view = mm->GetMovieView(&s);

	if (result && view && reinterpret_cast<RE::GFxMovieView*>(view)->GetVisible())
	{
		result = (evn->GetControlID() == &InputStringHolder::GetSingleton()->togglePOV);
	}

	return result;
}

void StartActivation(PlayerCharacter * player)
{
	BSFixedString s("LootMenu");
	GFxMovieView * view = NULL;
	MenuManager * mm = MenuManager::GetSingleton();
	if (mm)
		view = mm->GetMovieView(&s);
		
	if (view && reinterpret_cast<RE::GFxMovieView*>(view)->GetVisible())
	{
		LootMenu * loot = LootMenu::GetSingleton();
		if (loot)
			loot->TakeItem();
	}
	else
	{
		StartActivationFunc(player);
	}
}

void ReadyWeaponProcessButton_Hook(RE::ReadyWeaponHandler * handler, ButtonEvent* evn, RE::PlayerControls::Data24 * data)
{
	BSFixedString s("LootMenu");
	GFxMovieView * view = NULL;
	MenuManager * mm = MenuManager::GetSingleton();
	if (mm)
		view = mm->GetMovieView(&s);

	static bool bProcessLongTap = false;

	if (!reinterpret_cast<RE::ButtonEvent*>(evn)->IsDown())
	{
		if (bProcessLongTap && evn->timer > 2.0f)
		{
			bProcessLongTap = false;

			if (!view || !reinterpret_cast<RE::GFxMovieView*>(view)->GetVisible())
			{
				bool disabled = !isDisabled;
				isDisabled = disabled;

				DebugNotification((disabled ? "QuickLoot has been disabled." : "QuickLoot has been enabled."), false, true);
			}
		}
		return;
	}

	bProcessLongTap = true;

	if (view && reinterpret_cast<RE::GFxMovieView*>(view)->GetVisible())
	{
		StartActivationFunc((*g_thePlayer));
	}
	else
	{
		ReadyWeaponHandler_(handler, evn, data);
	}
}


bool GetCrosshairText_HookCONT(TESObjectCONT * cont, TESObjectREFR *ref, BSString * dst)
{
	UInt64 retnAddr = *(UInt64*)(&cont - 1);

	if (retnAddr == retnAddrEx.GetUIntPtr() && LootMenu::CanOpen(ref) && !isDisabled)
		return false;

	return GetCrosshairTextCONT(cont, ref, dst);
}

bool GetCrosshairText_HookACTI(TESObjectACTI * acti, TESObjectREFR *ref, BSString * dst)
{
	UInt64 retnAddr = *(UInt64*)(&acti - 1);

	if (retnAddr == retnAddrEx.GetUIntPtr() && LootMenu::CanOpen(ref) && !isDisabled)
		return false;

	return GetCrosshairTextACTI(acti, ref, dst);
}

bool GetCrosshairText_HookNPC(TESNPC * npc, TESObjectREFR *ref, BSString * dst)
{
	UInt64 retnAddr = *(UInt64*)(&npc - 1);

	if (retnAddr == retnAddrEx.GetUIntPtr() && LootMenu::CanOpen(ref) && !isDisabled)
		return false;

	return GetCrosshairTextNPC(npc, ref, dst);
}

bool FavoriteCanProcess_Hook(MenuHandler::MenuEventHandler * handler, InputEvent *evn)
{
	bool result = FavoriteCanProcess(handler, evn);
	BSFixedString s("LootMenu");
	GFxMovieView * view = NULL;
	MenuManager * mm = MenuManager::GetSingleton();
	if (mm)
		view = mm->GetMovieView(&s);

	if (result && view && reinterpret_cast<RE::GFxMovieView*>(view)->GetVisible())
	{
		if (evn->deviceType == kDeviceType_Gamepad && evn->eventType == InputEvent::kEventType_Button)
		{
			ButtonEvent *button = static_cast<ButtonEvent *>(evn);
			/*LootMenu * loot = LootMenu::GetSingleton();

			if (button->IsDown())
			{
				if (button->keyMask == 1)
				{
					loot->SetIndex(-1);
				}
				else if (button->keyMask == 2)
				{
					loot->SetIndex(1);
				}
			}*/
			result = (button->keyMask != 0x01 && button->keyMask != 0x02);
		}
	}

	return result;
}

bool PlayerRunCanProcess_Hook(RunHandler * handler, InputEvent* evn)
{
	bool result = PlayerRunCanProcess(handler, evn);
	BSFixedString s("LootMenu");
	GFxMovieView * view = NULL;
	MenuManager * mm = MenuManager::GetSingleton();
	if (mm)
		view = mm->GetMovieView(&s);

	if (result && view && reinterpret_cast<RE::GFxMovieView*>(view)->GetVisible())
	{
		if (evn->deviceType == kDeviceType_Keyboard && evn->eventType == InputEvent::kEventType_Button)
		{
			ButtonEvent *button = static_cast<ButtonEvent *>(evn);
			InputStringHolder *holder = InputStringHolder::GetSingleton();

			if (button->keyMask == InputManager::GetSingleton()->GetMappedKey(holder->run, kDeviceType_Keyboard, InputManager::kContext_Gameplay))
			{
				isKeyboardKeyDown = true;
				return result;
			}
		}
	}

	isKeyboardKeyDown = false;

	return result;
}

bool PlayerSprintCanProcess_Hook(SprintHandler * handler, InputEvent* evn)
{
	bool result = PlayerSprintCanProcess(handler, evn);
	BSFixedString s("LootMenu");
	GFxMovieView * view = NULL;
	MenuManager * mm = MenuManager::GetSingleton();
	if (mm)
		view = mm->GetMovieView(&s);

	if (result && view && reinterpret_cast<RE::GFxMovieView*>(view)->GetVisible())
	{
		if (evn->deviceType == kDeviceType_Gamepad && evn->eventType == InputEvent::kEventType_Button)
		{
			ButtonEvent *button = static_cast<ButtonEvent *>(evn);
			InputStringHolder *holder = InputStringHolder::GetSingleton();

			if (button->keyMask == InputManager::GetSingleton()->GetMappedKey(holder->sprint, kDeviceType_Gamepad, InputManager::kContext_Gameplay))
			{
				isGamepadKeyDown = true;
				return result;
			}
		}
	}

	isGamepadKeyDown = false;
	
	return result;
}

void TogglePOVProcessButton_Hook(TogglePOVHandler * handler, ButtonEvent* evn, RE::PlayerControls::Data24* unk024)
{
	BSFixedString s("LootMenu");
	GFxMovieView * view = NULL;
	MenuManager * mm = MenuManager::GetSingleton();
	if (mm)
		view = mm->GetMovieView(&s);

	if (view && reinterpret_cast<RE::GFxMovieView*>(view)->GetVisible())
	{
		LootMenu * loot = LootMenu::GetSingleton();
		if (loot)
			loot->TakeAllItems();
	}
	else
	{
		TogglePOVProcessButton(handler, evn, unk024);
	}
}

namespace Hooks
{
	void Install()
	{
		SafeWrite64(ThirdPersonVtbl.GetUIntPtr() + 0x1 * 8, GetFnAddr(&ThirdPersonCanProcess));
		SafeWrite64(FirstPersonVtbl.GetUIntPtr() + 0x1 * 8, GetFnAddr(&FirstPersonCanProcess));

		SafeWrite64(ReadyWeaponHandlerAddrVtbl.GetUIntPtr() + 0x04 * 8, GetFnAddr(&ReadyWeaponProcessButton_Hook));

		SafeWrite64(TESOBjectCONTvtbl.GetUIntPtr() + 0x4C * 8, GetFnAddr(&GetCrosshairText_HookCONT));
		SafeWrite64(TESOBjectACTIvtbl.GetUIntPtr() + 0x4C * 8, GetFnAddr(&GetCrosshairText_HookACTI));
		SafeWrite64(TESNPCvtbl.GetUIntPtr() + 0x4C * 8, GetFnAddr(&GetCrosshairText_HookNPC));

		SafeWrite64(FavoriteHandlervtbl.GetUIntPtr() + 0x01 * 8, GetFnAddr(&FavoriteCanProcess_Hook));
		SafeWrite64(PlayerRunHandlervtbl.GetUIntPtr() + 0x01 * 8, GetFnAddr(&PlayerRunCanProcess_Hook));
		SafeWrite64(PlayerSprintHandlervtbl.GetUIntPtr() + 0x01 * 8, GetFnAddr(&PlayerSprintCanProcess_Hook));
		SafeWrite64(TogglePOVHandlervtbl.GetUIntPtr() + 0x04 * 8, GetFnAddr(&TogglePOVProcessButton_Hook));

		g_branchTrampoline.Write5Call(StartActivationAddr.GetUIntPtr(), GetFnAddr(StartActivation));
	}
}