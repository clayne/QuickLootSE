#pragma once

#include "common/ITypes.h"  // UInt32
#include "skse64/GameForms.h"  // TESFaction
#include "skse64/GameReferences.h"  // TESObjectREFR

#include "RE_PerkEntryVisitor.h"  // PerkEntryVisitor


namespace RE
{
	class Actor : public ::TESObjectREFR
	{
	public:
		enum { kTypeID = kFormType_Character };

		virtual ~Actor();
		virtual void	Unk_9C(void);
		virtual void	Unk_9D(void);
		virtual void	Unk_9E(void);
		virtual void	Unk_9F(void);
		virtual void	Unk_A0(void);
		virtual void	Unk_A1(void);
		virtual void	Unk_A2(void);
		virtual void	PlaySounds(TESForm* form, bool isPickup, bool unk);
		virtual void	Unk_A4(void);
		virtual void	Unk_A5(void);
		virtual void	Unk_A6(void);
		virtual void	Unk_A7(void);
		virtual void	Unk_A8(void);
		virtual void	Unk_A9(void);
		virtual void	Unk_AA(void);
		virtual void	Unk_AB(void);
		virtual void	Unk_AC(void);
		virtual void	Unk_AD(void);
		virtual void	Unk_AE(void);
		virtual void	Unk_AF(void);
		virtual void	Unk_B0(void);
		virtual void	Unk_B1(void);
		virtual void	Unk_B2(void);
		virtual void	Unk_B3(void);
		virtual void	Unk_B4(void);
		virtual void	Unk_B5(void);
		virtual void	Unk_B6(void);
		virtual void	Unk_B7(void);
		virtual void	Unk_B8(void);
		virtual void	Unk_B9(void);
		virtual void	Unk_BA(void);
		virtual void	Unk_BB(void);
		virtual void	Unk_BC(void);
		virtual void	Unk_BD(void);
		virtual void	Unk_BE(void);
		virtual void	Unk_BF(void);
		virtual void	Unk_C0(void);
		virtual void	Unk_C1(void);
		virtual void	Unk_C2(void);
		virtual void	Unk_C3(void);
		virtual void	Unk_C4(void);
		virtual void	Unk_C5(void);
		virtual void	Unk_C6(void);
		virtual void	Unk_C7(void);
		virtual void	Unk_C8(void);
		virtual void	Unk_C9(void);
		virtual void	Unk_CA(void);
		virtual void	Unk_CB(void);
		virtual void	Unk_CC(void);
		virtual void	Unk_CD(void);
		virtual void	Unk_CE(void);
		virtual void	Unk_CF(void);
		virtual void	Unk_D0(void);
		virtual void	Unk_D1(void);
		virtual void	Unk_D2(void);
		virtual void	Unk_D3(void);
		virtual void	Unk_D4(void);
		virtual void	Unk_D5(void);
		virtual void	Unk_D6(void);
		virtual void	Unk_D7(void);
		virtual void	Unk_D8(void);
		virtual void	Unk_D9(void);
		virtual void	Unk_DA(void);
		virtual void	Unk_DB(void);
		virtual void	Unk_DC(void);
		virtual void	Unk_DD(void);
		virtual void	Unk_DE(void);
		virtual void	Unk_DF(void);
		virtual void	Unk_E0(void);
		virtual void	Unk_E1(void);
		virtual void	Unk_E2(void);
		virtual void	Unk_E3(void);
		virtual void	Unk_E4(void);
		virtual void	Unk_E5(void);
		virtual void	Unk_E6(void);
		virtual void	Unk_E7(void);
		virtual void	Unk_E8(void);
		virtual void	Unk_E9(void);
		virtual void	Unk_EA(void);
		virtual void	Unk_EB(void);
		virtual void	Unk_EC(void);
		virtual void	Unk_ED(void);
		virtual void	Unk_EE(void);
		virtual void	Unk_EF(void);
		virtual void	Unk_F0(void);
		virtual void	Unk_F1(void);
		virtual void	Unk_F2(void);
		virtual void	Unk_F3(void);
		virtual void	Unk_F4(void);
		virtual void	Unk_F5(void);
		virtual void	Unk_F6(void);
		virtual void	Unk_F7(void);
		virtual void	Unk_F8(void);
		virtual bool	IsInFaction(TESFaction* faction);
		virtual void	Unk_FA(void);
		virtual void	Unk_FB(void);
		virtual void	Unk_FC(void);
		virtual void	Unk_FD(void);
		virtual void	Unk_FE(void);
		virtual bool	CanProcessEntryPointPerkEntry(UInt32 entryType);
		virtual void	VisitEntryPointPerkEntries(UInt32 entryType, PerkEntryVisitor& visitor);
	};
}