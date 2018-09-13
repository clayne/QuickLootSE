#include "skse64/PluginAPI.h"
#include "skse64/GameMenus.h"
#include "skse64/GameRTTI.h"
#include "skse64/gamethreads.h"
#include "skse64/PapyrusEvents.h"
#include "skse64_common/BranchTrampoline.h"
#include "skse64_common/skse_version.h"

#include <shlobj.h>
#include "LootMenu.h"
#include "Hooks.h"
#include "Settings.h"
#include "Console.h"

IDebugLog				gLog;
PluginHandle			g_pluginHandle = kPluginHandle_Invalid;

SKSEMessagingInterface			* g_messaging = nullptr;
SKSETaskInterface				* g_task = nullptr;

void * g_moduleHandle = nullptr;

bool isOpen = false;
bool isDisabled = false;

class DelayedUpdater : public TaskDelegate
{
public:
	virtual void Run() override
	{
		LootMenu *lootMenu = LootMenu::GetSingleton();
		if (lootMenu)
		{
			lootMenu->Update();
		}
	}
	virtual void Dispose() override
	{
	}

	static void Register()
	{
		static DelayedUpdater singleton;
		g_task->AddTask(&singleton);
	}
};

class OnCrosshairRefChanged : public BSTEventSink<SKSECrosshairRefEvent>
{
public:
	virtual EventResult ReceiveEvent(SKSECrosshairRefEvent *evn, EventDispatcher<SKSECrosshairRefEvent> * dispatcher)
	{
		if (!isDisabled)
		{	
			BSFixedString s("LootMenu");
			if (evn->crosshairRef && LootMenu::CanOpen(evn->crosshairRef))
			{
				if (isOpen && evn->crosshairRef != containerRef)
				{
					if (containerRef->formType != kFormType_Character)
						LootMenu::GetSingleton()->PlayAnimationClose();

					containerRef = nullptr;
					CALL_MEMBER_FN(UIManager::GetSingleton(), AddMessage)(&s, UIMessage::kMessage_Close, NULL);
					CALL_MEMBER_FN(UIManager::GetSingleton(), AddMessage)(&s, UIMessage::kMessage_Open, NULL);
					isOpen = true;
					containerRef = evn->crosshairRef;
				}
				else
				{
					CALL_MEMBER_FN(UIManager::GetSingleton(), AddMessage)(&s, UIMessage::kMessage_Open, NULL);
					isOpen = true;
					containerRef = evn->crosshairRef;
				}
			}
			else
			{
				if (isOpen)
				{
					if (containerRef->formType != kFormType_Character)
						LootMenu::GetSingleton()->PlayAnimationClose();
						
					containerRef = nullptr;
					isOpen = false;
					CALL_MEMBER_FN(UIManager::GetSingleton(), AddMessage)(&s, UIMessage::kMessage_Close, NULL);
				}
			}
		}

		return kEvent_Continue;
	}
};

class LootMenuEventHandler : public BSTEventSink<MenuOpenCloseEvent>
{
public:
	virtual EventResult ReceiveEvent(MenuOpenCloseEvent *evn, EventDispatcher<MenuOpenCloseEvent> * dispatcher)
	{
		SimpleLocker lock(&LootMenu::ms_lock);
		if (isOpen && !isDisabled)
		{
			UIStringHolder *holder = UIStringHolder::GetSingleton();
			BSFixedString s("LootMenu");
			MenuManager *mm = MenuManager::GetSingleton();
			if (!mm)
				return kEvent_Continue;

			GFxMovieView * view = mm->GetMovieView(&s);
			if (!view)
				return kEvent_Continue;

			LootMenu * loot = LootMenu::GetSingleton();

			if (evn->opening)
			{
				IMenu *menu = mm->GetMenu(&evn->menuName);
				if (menu)
				{
					if ((menu->flags & 0x4000) != 0 && (evn->menuName != holder->tweenMenu))
					{
						if (loot)
							loot->Close();
					}
					else if ((menu->flags & 0x1) != 0)
					{
						view->SetVisible(false);
					}
				}
			}
			else if (!evn->opening)
			{
				if (mm->GetNumPauseGame() == 0 && view->GetVisible() == false)
				{
					view->SetVisible(true);

					if (loot && loot->m_bUpdateRequest)
					{
						_MESSAGE("UPDATING CONTAINER");
						_MESSAGE("    %p %s", containerRef->formID, CALL_MEMBER_FN(containerRef, GetReferenceName)());

						DelayedUpdater::Register();
					}
				}
				if (evn->menuName == holder->containerMenu)
				{
					if (loot)
						loot->m_bOpenAnim = false;
				}
			}
		}

		return kEvent_Continue;
	}
};

class ContainerChangedEventHandler : public BSTEventSink<TESContainerChangedEvent>
{
public:
	virtual EventResult ReceiveEvent(TESContainerChangedEvent *evn, EventDispatcher<TESContainerChangedEvent> * dispatcher)
	{
		SimpleLocker lock(&LootMenu::ms_lock);
		if (containerRef && !isDisabled)
		{
			LootMenu * loot = LootMenu::GetSingleton();
			if (!loot || loot->m_bNowTaking)
				return kEvent_Continue;
			
			UInt32 containerFormID = containerRef->formID;
			if (evn->fromFormId == containerFormID || evn->toFormId == containerFormID)
			{
				if (loot->m_bUpdateRequest == false)
				{
					_MESSAGE("CONTAINER HAS BEEN UPDATED WITHOUT QUICK LOOTING");
					_MESSAGE("    %p %s\n", containerFormID, CALL_MEMBER_FN(containerRef, GetReferenceName)());
				}

				MenuManager *mm = MenuManager::GetSingleton();
				if (!mm)
					return kEvent_Continue;

				if (mm->GetNumPauseGame() > 0)
				{
					loot->m_bUpdateRequest = true;
				}
				else
				{
					DelayedUpdater::Register();
				}
			}
		}

		return kEvent_Continue;
	}
};

OnCrosshairRefChanged			g_onCrosshairRefChanged;
LootMenuEventHandler			g_lootMenuEventHandler;
ContainerChangedEventHandler	g_containerChangedEventHandler;

void MessageHandler(SKSEMessagingInterface::Message * msg)
{
	switch (msg->type)
	{
	case SKSEMessagingInterface::kMessage_InputLoaded:
	{
		LootMenu::Init();

		auto crosshairrefdispatch = (EventDispatcher<SKSECrosshairRefEvent>*)g_messaging->GetEventDispatcher(SKSEMessagingInterface::kDispatcher_CrosshairEvent);
		crosshairrefdispatch->AddEventSink(&g_onCrosshairRefChanged);

		MenuManager *mm = MenuManager::GetSingleton();
		mm->MenuOpenCloseEventDispatcher()->AddEventSink(&g_lootMenuEventHandler);

		GetEventDispatcherList()->unk370.AddEventSink(&g_containerChangedEventHandler);
	}
		break;
	}
}


extern "C"
{

	bool SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info)
	{

		gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim Special Edition\\SKSE\\QuickLootSE.log");

		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "Quick Loot SE plugin";
		info->version = 3;

		g_pluginHandle = skse->GetPluginHandle();

		if (skse->isEditor)
		{
			_MESSAGE("loaded in editor, marking as incompatible");
			return false;
		}

		if (skse->runtimeVersion != RUNTIME_VERSION_1_5_50)
		{
			_MESSAGE("This plugin is not compatible with this versin of game.");
			return false;
		}

		if (!g_branchTrampoline.Create(1024 * 64))
		{
			_ERROR("couldn't create branch trampoline. this is fatal. skipping remainder of init process.");
			return false;
		}

		if (!g_localTrampoline.Create(1024 * 64, g_moduleHandle))
		{
			_ERROR("couldn't create codegen buffer. this is fatal. skipping remainder of init process.");
			return false;
		}

		return true;
	}

	bool SKSEPlugin_Load(const SKSEInterface * skse)
	{
		_MESSAGE("Load");
		Settings::Load();
		Hooks::Install();
		ConsoleCommand::Register();

		g_messaging = (SKSEMessagingInterface *)skse->QueryInterface(kInterface_Messaging);
		g_task = (SKSETaskInterface *)skse->QueryInterface(kInterface_Task);

		g_messaging->RegisterListener(g_pluginHandle, "SKSE", MessageHandler);

		return true;
	}

}