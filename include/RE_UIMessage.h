#pragma once


namespace RE
{
	class UIMessage
	{
	public:
		enum Type
		{
			kMessage_Refresh = 0,				// 0 used after ShowAllMapMarkers
			kMessage_Open,						// 1
			kMessage_PreviouslyKnownAsClose,	// 2
			kMessage_Close,						// 3
			kMessage_Unk04,
			kMessage_Unk05,
			kMessage_Scaleform,					// 6 BSUIScaleformData
			kMessage_Message					// 7 BSUIMessageData
		};
	};
}