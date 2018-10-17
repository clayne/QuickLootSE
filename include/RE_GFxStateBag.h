#pragma once

#include "skse64/ScaleformState.h"


namespace RE
{
	class GFxStateBag
	{
	protected:
		virtual GFxStateBag*	GetStateBagImpl(void) const;

	public:
		virtual	~GFxStateBag();

		virtual void	SetState(UInt32 state, GFxState* pstate);
		virtual void*	GetStateAddRef(UInt32 state) const;
		virtual void	GetStatesAddRef(GFxState** pstateList, const UInt32 *pstates, UInt32 count) const;
	};
}