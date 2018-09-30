#pragma once

#include "skse64/GameMenus.h"

namespace TES
{
	// 1C8
	class MenuManager
	{
		typedef tHashSet<MenuTableItem, BSFixedString> MenuTable;

		// 030-040
		struct Unknown3
		{
			UInt64		unk00;		// 000 - New in SE. Not init'd?

			UInt64		frequency;	// 008 

			UInt64		unk_010;	// 010 (= 0)
			UInt32		unk_018;	// 018 (= 0)
			UInt32		unk_01C;	// 01C (= 0)
			UInt32		unk_020;	// 020 (= 0)
			UInt32		unk_024;	// 024 (= 0)
			float		unk_028;	// 028 (= frequency related)

			UInt32		unk_02C;	// 02C
			UInt32		unk_030;	// 030

			UInt32		unk_034;	// 034 (= 0)
			UInt16		unk_038;	// 038
			UInt8		unk_03A;	// 03A (= 0)
			UInt8		pad[5];		// 03B
		};
		STATIC_ASSERT(sizeof(Unknown3) == 0x40);

	public:
		UInt64					unk_000;	// 000

		EventDispatcher<MenuOpenCloseEvent>		menuOpenCloseEventDispatcher;	// 008
		EventDispatcher<MenuModeChangeEvent>	menuModeChangeEventDispatcher;	// 060
		EventDispatcher<void*>					unk_064;						// 0B8 - New in 1.6.87.0 - Kinect related?

		UnkArray			menuStack;					// 110
		MenuTable			menuTable;					// 128   (Entries ptr at 150)
		SimpleLock			menuTableLock;				// 158
		UInt32				numPauseGame;				// 160 (= 0) += 1 if (imenu->flags & 0x0001)
		UInt32				numItemMenu;				// 164 (= 0) += 1 if (imenu->flags & 0x2000)
		UInt32				numPreventGameLoad;			// 168 (= 0) += 1 if (imenu->flags & 0x0080)
		UInt32				numDoNotPreventSaveGame;	// 16C (= 0) += 1 if (imenu->flags & 0x0800)
		UInt32				numStopCrosshairUpdate;		// 170 (= 0) += 1 if (imenu->flags & 0x4000)
		UInt32				numFlag8000;				// 174 (= 0) += 1 if (imenu->flags & 0x8000)
		UInt32				numFlag20000;				// 178 (= 0)  = 1 if (imenu->flags & 0x20000)
		UInt8				numModal;					// 17C (= 0)  = 1 if (imenu->flags & 0x10)
		UInt8				pad_17D[3];					// 17D
		Unknown3			unk_180;					// 180
		bool				showMenus;					// 1C0 (= 0)
		bool				unk_1C1;					// 1C1 (= 0)
		char				pad[6];						// 1C2

	public:
		typedef IMenu*	(*CreatorFunc)(void);

	private:
		MEMBER_FN_PREFIX(MenuManager);
		DEFINE_MEMBER_FN(IsMenuOpen, bool, 0x00EBDE90, BSFixedString * menuName);
		DEFINE_MEMBER_FN(Register_internal, void, 0x00EBF700, const char * name, CreatorFunc creator);

	public:

		static MenuManager * GetSingleton(void)
		{
			// 502FDB8FEA80C3705F9E228F79D4EA7A399CC7FD+32
			static RelocPtr<MenuManager *> g_menuManager(0x01EE5B20);
			return *g_menuManager;
		}

		EventDispatcher<MenuOpenCloseEvent> * MenuOpenCloseEventDispatcher()
		{
			return &menuOpenCloseEventDispatcher;
		}

		bool				IsMenuOpen(BSFixedString * menuName);
		IMenu *				GetMenu(BSFixedString * menuName);
		GFxMovieView *		GetMovieView(BSFixedString * menuName);
		void				ShowMenus(bool show) { showMenus = show; }
		bool				IsShowingMenus() const { return showMenus; }

		typedef IMenu* (*CreatorFunc)(void);

		void Register(const char* name, CreatorFunc creator)
		{
			CALL_MEMBER_FN(this, Register_internal)(name, creator);
		}
	};
	STATIC_ASSERT(sizeof(MenuManager) == 0x1C8);
}