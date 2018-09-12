#include "skse64/GameMenus.h"

namespace MenuHandler
{
	class MenuEventHandler
	{
	public:
		MenuEventHandler();
		virtual ~MenuEventHandler();

		virtual bool	CanProcess(InputEvent *evn) = 0;
		virtual bool	ProcessKinect(KinectEvent *evn);
		virtual bool	ProcessThumbstick(ThumbstickEvent *evn);
		virtual bool	ProcessMouseMove(MouseMoveEvent *evn);
		virtual bool	ProcessButton(ButtonEvent *evn);

		UInt32	unk08;		// 08
		UInt8	unk0C;		// 0C - This appears to be 1 when a menu is open
		UInt8	unk0D;
		UInt8	pad0E[2];
	};
}
STATIC_ASSERT(sizeof(MenuHandler::MenuEventHandler) == 0x10);