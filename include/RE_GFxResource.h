#pragma once

#include <atomic>


namespace RE
{
	class GFxResource
	{
		GFxResource();
		virtual ~GFxResource();

		virtual void	Unk_00();
		virtual void	Unk_01();
		virtual void	Unk_02();
	};
}