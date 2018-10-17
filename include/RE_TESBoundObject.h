#pragma once

#include "skse64/GameObjects.h"  // TESObject
#include "skse64/GameReferences.h"  // TESObjectREFR


namespace RE
{
	class TESBoundObject : public TESObject
	{
	public:
		virtual void	Unk_47(void);
		virtual void	Unk_48(void);
		virtual void	Unk_49(void);
		virtual void	Unk_4A(void);
		virtual void	Unk_4B(void);
		virtual void	Unk_4C(void);
		virtual void	OnRemovedFrom(::TESObjectREFR* ref);
	};
}