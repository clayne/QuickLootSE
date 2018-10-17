#pragma once

#include "common/ITypes.h"  // UInt32, SInt32
#include "skse64/GameTypes.h"  // BSFixedString


namespace RE
{
	class IAnimationGraphManagerHolder
	{
	public:
		virtual ~IAnimationGraphManagerHolder();

		virtual void	Unk_01(void);
		virtual void	Unk_02(void);
		virtual void	Unk_03(void);
		virtual void	Unk_04(void);
		virtual void	Unk_05(void);
		virtual void	Unk_06(void);
		virtual void	Unk_07(void);
		virtual void	Unk_08(void);
		virtual void	Unk_09(void);
		virtual void	Unk_10(void);
		virtual void	Unk_11(void);
		virtual void	Unk_12(void);
		virtual void	Unk_13(void);
		virtual void	Unk_14(void);
		virtual void	Unk_15(void);
		virtual void	Unk_16(void);
		virtual void	Unk_17(void);
		virtual bool	GetAnimationVariableBool(const BSFixedString& variableName, bool& out);
	};
}