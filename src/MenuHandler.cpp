#include "MenuHandler.h"

namespace MenuHandler
{
	MenuEventHandler::MenuEventHandler() : unk0C(0), unk0D(0){}

	MenuEventHandler::~MenuEventHandler(){}

	bool MenuEventHandler::ProcessKinect(KinectEvent * evn)
	{
		return false;
	}

	bool MenuEventHandler::ProcessThumbstick(ThumbstickEvent *evn)
	{
		return false;
	}

	bool MenuEventHandler::ProcessMouseMove(MouseMoveEvent *evn)
	{
		return false;
	}

	bool MenuEventHandler::ProcessButton(ButtonEvent *evn)
	{
		return false;
	}
}