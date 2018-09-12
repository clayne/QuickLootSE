#pragma once

struct Settings
{
	static bool	bDisableInCombat;
	static bool	bDisableTheft;
	static bool	bDisablePickpocketing;
	static bool bSelfTranslation;
	static int	iOpacity;
	static int	iScale;
	static int	iPositionX;
	static int	iPositionY;

	static TCHAR sPickPocket[256];
	static TCHAR sSteal[256];
	static TCHAR sTake[256];
	static TCHAR sSearch[256];
	static TCHAR sTakeAll[256];

	static bool SetBool(const char *name, bool val);
	static bool SetInt(const char *name, int val);
	static bool Set(const char *name, int val);

	static void Load();
};
