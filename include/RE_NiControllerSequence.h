#pragma once

#include "skse64/NiObjects.h"  // NiObject


namespace RE
{
	class NiControllerSequence : public ::NiObject
	{
	public:
		enum { kRTTI = 0x01B904A8 };


		virtual bool	Unk_21(UInt32 arg1, UInt32 arg2);


		const BSFixedString& GetName() const
		{
			return m_name;
		}


		BSFixedString	m_name;	// 08
	};
}