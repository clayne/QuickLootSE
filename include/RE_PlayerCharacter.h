#pragma once

#include "common/ITypes.h"  // UInt8, UInt32
#include "skse64/GameEvents.h"  // BSTEventSink, BSTEventSource
#include "skse64/GameReferences.h"  // Character, Actor


namespace RE
{
	class PlayerCharacter : public ::Character
	{
	public:
		virtual ~PlayerCharacter();

		// parents
		BSTEventSink <void*>	menuOpenCloseEvent;			// 2B0 .?AV?$BSTEventSink@VMenuOpenCloseEvent@@@@
		BSTEventSink <void*>	menuModeChangeEvent;		// 2B8 .?AV?$BSTEventSink@VMenuModeChangeEvent@@@@
		BSTEventSink <void*>	userEventEnabledEvent;		// 2C0 .?AV?$BSTEventSink@VUserEventEnabledEvent@@@@
		BSTEventSink <void*>	trackedStatsEvent;			// 2C8  BSTEventSink@UTESTrackedStatsEvent@@@@@

		BSTEventSource <void*>	actorCellEventSource;		// 2D0 .?AV?$BSTEventSource@UBGSActorCellEvent@@@@  
															// tArray<void*>: 4 PlayerRegionState, BGSPlayerMusicChanger, CellAcousticSpaceListener, PlayerParentCellListener
		BSTEventSource <void*>	actorDeathEventSource;		// 328 .?AV?$BSTEventSource@UBGSActorDeathEvent@@@@ 
															// tArray<void*>: 1 BGSPlayerMusicChanger
		BSTEventSource <void*>	positionPlayerEventSource;	// 380 .?AV?$BSTEventSource@UPositionPlayerEvent@@@@
															// tArray<void*>: 9 MovementAvoidBoxEventAdapter, GarbaseCollector, Main, MenuTopicManager, TES (85E27728),
															// PathManagerPositionPlayerAdapter, CharacterCollisionMessagePlayerAdapter, PlayerSleepWaitMovementControllerAdapter, SkyrimVM

		UInt8	pad3D8[0xAF8 - 0x3D8];	// 3D8
		UInt32	padAF8;					// AF8
		UInt32	isGrabbing;				// AFC - 0 (is not grabbing), 1 (is grabbing), 2 (in act of grabbing?)
		UInt8	padB00[0xBE0 - 0xB00];	// B00

		MEMBER_FN_PREFIX(PlayerCharacter);
		DEFINE_MEMBER_FN(GetActorInFavorState, Actor*, 0x006B3860);
	};
	STATIC_ASSERT(offsetof(PlayerCharacter, isGrabbing) == 0xAFC);
	STATIC_ASSERT(sizeof(PlayerCharacter) == 0xBE0);
}