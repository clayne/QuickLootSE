#pragma once

#include "common/ITypes.h"  // UInt8, UInt32
#include "skse64/GameFormComponents.h"  // TESFullName, TESModelTextureSwap, TESIcon, BGSMessageIcon, TESWeightForm, TESValueForm, BGSDestructibleObjectForm, BGSEquipType
#include "skse64/GameObjects.h"  // TESBoundAnimObject


namespace RE
{
	class TESObjectLIGH : public ::TESBoundAnimObject
	{
	public:
		enum { kTypeID = kFormType_Light };

		// parents
		TESFullName					fullName;		// 030
		TESModelTextureSwap			texSwap;		// 040
		TESIcon						icon;			// 078
		BGSMessageIcon				messageIcon;	// 088
		TESWeightForm				weight;			// 0AC
		TESValueForm				value;			// 0B0
		BGSDestructibleObjectForm	destructible;	// 0C0
		BGSEquipType				equipType;		// 0D0

		// members

		enum Flag
		{
			kFlag_Dynamic = 1 << 0,
			kFlag_CanBeCarried = 1 << 1,
			kFlag_Flicker = 1 << 3,
			kFlag_Pulse = 1 << 7,
			kFlag_ShadowSpotlight = 1 << 10,
			kFlag_Hemisphere = 1 << 11,
			kFlag_ShadowOmniDirectional = 1 << 12,
			kFlag_PortalStrict = 1 << 13
		};

		struct DataE0
		{
			UInt8	pad00[0x0C];		// 10
			UInt32	flags;				// 0C
			UInt8	pad10[0x28 - 0x10];	// 10
		};
		STATIC_ASSERT(offsetof(DataE0, flags) == 0x0C);
		STATIC_ASSERT(sizeof(DataE0) == 0x28);

		DataE0	unk0E0;					// 0E0 - DATA
		UInt8	pad108[0x130 - 0x108];	// 108

		bool CanBeCarried() const
		{
			return (unk0E0.flags & kFlag_CanBeCarried) == kFlag_CanBeCarried;
		}
	};
	STATIC_ASSERT(offsetof(TESObjectLIGH, unk0E0) == 0x0E0);
	STATIC_ASSERT(sizeof(TESObjectLIGH) == 0x130);
}