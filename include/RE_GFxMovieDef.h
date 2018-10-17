#pragma once

#include "common/ITypes.h"  // UInt32
#include "skse64/ScaleformState.h"  // GFxStateBag

#include "RE_GFxResource.h"  // RE::GFxResource
#include "RE_GFxStateBag.h"  // RE::GFxStateBag


namespace RE
{
	class GFxMovieDef : public RE::GFxResource, public RE::GFxStateBag
	{
	public:
		virtual UInt32	GetVersion() const = 0;
		virtual UInt32	GetLoadingFrame() const = 0;
		virtual float	GetWidth() const = 0;
		virtual float	GetHeight() const = 0;
	};
}