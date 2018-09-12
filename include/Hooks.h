#pragma once

extern bool isGamepadKeyDown;
extern bool isKeyboardKeyDown;

typedef void (*DebugNotification_t)(const char *, bool, bool);
extern RelocAddr<DebugNotification_t> DebugNotification;

namespace Hooks
{
	void Install();
}
