#include "RE_ObScript.h"
#include "skse64_common\Relocation.h"

namespace RE
{
	// 698845F80EE915EE803E515A37C1269ED6A2FE43+47
	RelocPtr <ObScriptCommand>	g_firstObScriptCommand(0x01DDE910);
	// 698845F80EE915EE803E515A37C1269ED6A2FE43+1E
	RelocPtr <ObScriptCommand>	g_firstConsoleCommand(0x01DECF60);
}