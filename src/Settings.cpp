#include "Settings.h"
#include "skse64/GameStreams.h"
#include <Shlwapi.h>
#include <string>

#pragma comment(lib, "shlwapi.lib")

bool	Settings::bDisableInCombat = false;
bool	Settings::bDisableTheft = false;
bool	Settings::bDisablePickpocketing = false;
bool	Settings::bSelfTranslation = false;
int		Settings::iOpacity = 25;
int		Settings::iScale = 100;
int		Settings::iPositionX = -1;
int		Settings::iPositionY = -1;

TCHAR	Settings::sPickPocket[256];
TCHAR 	Settings::sSteal[256];
TCHAR 	Settings::sTake[256];
TCHAR 	Settings::sSearch[256];
TCHAR	Settings::sTakeAll[256];

std::wstring thisPath = L"";

bool Settings::SetBool(const char *name, bool val)
{
	if (_stricmp(name, "bDisableInCombat") == 0)
	{
		bDisableInCombat = val;
		WritePrivateProfileString(L"Settings", L"bDisableInCombat", val ? L"1" : L"0", thisPath.c_str());
	}
	else if (_stricmp(name, "bDisableTheft") == 0)
	{
		bDisableTheft = val;
		WritePrivateProfileString(L"Settings", L"bDisableTheft", val ? L"1" : L"0", thisPath.c_str());
	}
	else if (_stricmp(name, "bDisablePickpocketing") == 0)
	{
		bDisablePickpocketing = val;
		WritePrivateProfileString(L"Settings", L"bDisablePickpocketing", val ? L"1" : L"0", thisPath.c_str());
	}
	else if (_stricmp(name, "bSelfTranslation") == 0)
	{
		bSelfTranslation = val;
		WritePrivateProfileString(L"Settings", L"bSelfTranslation", val ? L"1" : L"0", thisPath.c_str());
	}
	else
	{
		return false;
	}

	_MESSAGE("  %s = %d", name, val);

	return true;
}


bool Settings::SetInt(const char *name, int val)
{
	if (_stricmp(name, "iOpacity") == 0)
	{
		iOpacity = val;
		WritePrivateProfileString(L"Settings", L"iOpacity", std::to_wstring(val).c_str(), thisPath.c_str());
	}
	else if (_stricmp(name, "iScale") == 0)
	{
		iScale = val;
		WritePrivateProfileString(L"Settings", L"iScale", std::to_wstring(val).c_str(), thisPath.c_str());
	}
	else if (_stricmp(name, "iPositionX") == 0)
	{
		iPositionX = val;
		WritePrivateProfileString(L"Settings", L"iPositionX", std::to_wstring(val).c_str(), thisPath.c_str());
	}
	else if (_stricmp(name, "iPositionY") == 0)
	{
		iPositionY = val;
		WritePrivateProfileString(L"Settings", L"iPositionY", std::to_wstring(val).c_str(), thisPath.c_str());
	}
	else
	{
		return false;
	}

	_MESSAGE("  %s = %d", name, val);

	return true;
}


bool Settings::Set(const char *name, int val)
{
	if (!name || !name[0])
		return false;

	if (name[0] == 'b')
		return SetBool(name, (val != 0));
	if (name[0] == 'i')
		return SetInt(name, val);

	return false;
}


void Settings::Load()
{
	TCHAR path[MAX_PATH];
	HMODULE hm;
	if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		L"QuickLootSE.dll", &hm))
	{
		GetModuleFileName(hm, path, MAX_PATH);
		PathRemoveFileSpec(path);
		thisPath = std::wstring(path);
		if (!thisPath.empty() && thisPath.at(thisPath.length() - 1) != '\\'){
			thisPath += L"\\QuickLootSE.ini";
		}
	}

	Settings::bDisableInCombat = GetPrivateProfileInt(L"Settings", L"bDisableInCombat", 0, thisPath.c_str());
	Settings::bDisablePickpocketing = GetPrivateProfileInt(L"Settings", L"bDisablePickpocketing", 0, thisPath.c_str());
	Settings::bDisableTheft = GetPrivateProfileInt(L"Settings", L"bDisableTheft", 0, thisPath.c_str());
	Settings::bSelfTranslation = GetPrivateProfileInt(L"Settings", L"bSelfTranslation", 0, thisPath.c_str());
	Settings::iOpacity = GetPrivateProfileInt(L"Settings", L"iOpacity", 25, thisPath.c_str());
	Settings::iPositionX = GetPrivateProfileInt(L"Settings", L"iPositionX", -1, thisPath.c_str());
	Settings::iPositionY = GetPrivateProfileInt(L"Settings", L"iPositionY", -1, thisPath.c_str());
	Settings::iScale = GetPrivateProfileInt(L"Settings", L"iScale", 100, thisPath.c_str());
	
	GetPrivateProfileString(L"Translation", L"sPickPocket", L"Pickpocket", Settings::sPickPocket, 255, thisPath.c_str());
	GetPrivateProfileString(L"Translation", L"sSteal", L"Steal", Settings::sSteal, 255, thisPath.c_str());
	GetPrivateProfileString(L"Translation", L"sTake", L"Take", Settings::sTake, 255, thisPath.c_str());
	GetPrivateProfileString(L"Translation", L"sSearch", L"Search", Settings::sSearch, 255, thisPath.c_str());
	GetPrivateProfileString(L"Translation", L"sTakeAll", L"TakeAll", Settings::sTakeAll, 255, thisPath.c_str());
}
