#pragma once

#include "skse64/ScaleformTypes.h"  // GRefCountBase

#include "RE_GFxMovieDef.h"  // RE::GFxMovieDef


namespace RE
{
	class GFxMovieView : public ::GRefCountBase
	{
	public:
		GFxMovieView();
		virtual ~GFxMovieView();

		virtual GFxMovieDef*	GetMovieDef();
		virtual void			Unk_02(void);
		virtual void			Unk_03(void);
		virtual void			Unk_04(void);
		virtual void			Unk_05(void);
		virtual void			Unk_06(void);
		virtual void			Unk_07(void);
		virtual void			SetVisible(bool visible);
		virtual bool			GetVisible();
	};
}