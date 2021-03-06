//---------------------------------------------------------------------------
#include <windows.h>
#include <signal.h>
#include "luafar.h"

#ifdef _MSC_VER
#define LUAPLUG WINAPI
#else
#define LUAPLUG WINAPI __declspec(dllexport)
#endif

#ifdef FUNC_OPENLIBS
extern int FUNC_OPENLIBS(lua_State*);
#else
#define FUNC_OPENLIBS NULL
#endif

#ifndef ENV_PREFIX
# ifdef _WIN64
#  define ENV_PREFIX L"LUAFAR64"
# else
#  define ENV_PREFIX L"LUAFAR"
# endif
#endif

lua_State* LS;
CRITICAL_SECTION FindFileSection; // http://forum.farmanager.com/viewtopic.php?f=9&p=107075#p107075

intptr_t WINAPI DlgProc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void *Param2)
{
	return LF_DlgProc(LS, hDlg, Msg, Param1, Param2);
}

static void InitLuaState2(lua_State *L, TPluginData* PluginData);  /* forward declaration */
static void laction(int i);  /* forward declaration */
struct PluginStartupInfo Info;
struct FarStandardFunctions FSF;
GUID PluginId;
TPluginData PluginData = { &Info, &FSF, &PluginId, DlgProc, 0, NULL, NULL, NULL, laction, SIG_DFL };
wchar_t PluginName[512], PluginDir[512];
int Init1_Done = 0, Init2_Done = 0; // Ensure initializations are done only once

static void lstop(lua_State *L, lua_Debug *ar)
{
	(void)ar;  /* unused arg. */
	lua_sethook(L, NULL, 0, 0);
	luaL_error(L, "interrupted!");
}

static void laction(int i)
{
	signal(i, PluginData.old_action);
	lua_sethook(LS, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}
//---------------------------------------------------------------------------

BOOL WINAPI DllMain(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
	(void) lpReserved;

	if(DLL_PROCESS_ATTACH == dwReason && hDll)
	{
		InitializeCriticalSection(&FindFileSection);
		if((LS = luaL_newstate()) != NULL)
		{
			GetModuleFileNameW((HINSTANCE)hDll, PluginName, sizeof(PluginName)/sizeof(PluginName[0]));
			wcscpy(PluginDir, PluginName);
			wcsrchr(PluginDir, L'\\')[1] = 0;
		}
	}
	else if(DLL_PROCESS_DETACH == dwReason)
	{
		if(LS)
		{
			lua_close(LS);
			LS = NULL;
		}
		DeleteCriticalSection(&FindFileSection);
	}

	return TRUE;
}

// This function must have __cdecl calling convention, it is not `LUAPLUG'.
__declspec(dllexport) int luaopen_luaplug(lua_State *L)
{
	TPluginData *pd = (TPluginData*)lua_newuserdata(L, sizeof(TPluginData));
	memcpy(pd, &PluginData, sizeof(TPluginData));
	LF_InitLuaState1(L, FUNC_OPENLIBS);
	InitLuaState2(L, pd);
	luaL_ref(L, LUA_REGISTRYINDEX);
	return 0;
}

static void InitLuaState2(lua_State *L, TPluginData* PluginData)
{
	LF_InitLuaState2(L, PluginData);
	LF_ProcessEnvVars(L, ENV_PREFIX, PluginDir);
	lua_pushcfunction(L, luaopen_luaplug);
	lua_setglobal(L, "_luaplug");
}

//---------------------------------------------------------------------------

__declspec(dllexport) lua_State* GetLuaState()
{
	return LS;
}
//---------------------------------------------------------------------------

void LUAPLUG GetGlobalInfoW(struct GlobalInfo *globalInfo)
{
	if(LS)
	{
		if(!Init1_Done)
		{
			LF_InitLuaState1(LS, FUNC_OPENLIBS);
			Init1_Done = 1;
		}

		if(LF_GetGlobalInfo(LS, globalInfo, PluginDir))
			PluginId = globalInfo->Guid;
		else
		{
			lua_close(LS);
			LS = NULL;
		}
	}
}
//---------------------------------------------------------------------------

void LUAPLUG SetStartupInfoW(const struct PluginStartupInfo *aInfo)
{
	if(LS && !Init2_Done)
	{
		Info = *aInfo;
		FSF = *aInfo->FSF;
		Info.FSF = &FSF;
		InitLuaState2(LS, &PluginData);

		if(LF_RunDefaultScript(LS))
			Init2_Done = 1;
		else
		{
			lua_close(LS);
			LS = NULL;
		}
	}
}
//---------------------------------------------------------------------------

void LUAPLUG GetPluginInfoW(struct PluginInfo *Info)
{
	if(LS) LF_GetPluginInfo(LS, Info);
}
//---------------------------------------------------------------------------

intptr_t LUAPLUG ProcessSynchroEventW(const struct ProcessSynchroEventInfo *Info)
{
	if(LS && Init2_Done) return LF_ProcessSynchroEvent(LS, Info);

	return 0;
}
//---------------------------------------------------------------------------

// This is exported in order not to crash when run from under Far 2.0.xxxx
// Minimal Far version = 3.0.0
int LUAPLUG GetMinFarVersionW()
{
	return (3<<8) | 0 | (0<<16);
}
//---------------------------------------------------------------------------

#ifdef EXPORT_OPEN
static int RecreateLuaState()
{
	lua_getglobal(LS, "RecreateLuaState");
	if (lua_toboolean(LS,-1))
	{
		struct ExitInfo Info = { sizeof(struct ExitInfo) };
		lua_State *newLS = LS;
		LS = NULL;
		LF_ExitFAR(newLS, &Info);
		lua_close(newLS);
		if ((newLS = luaL_newstate()) != NULL)
		{
			int OK = 0;
			struct GlobalInfo GInfo;
			LF_InitLuaState1(newLS, FUNC_OPENLIBS);
			if (LF_GetGlobalInfo(newLS, &GInfo, PluginDir))
			{
				InitLuaState2(newLS, &PluginData);
				lua_pushboolean(newLS,1);
				lua_setglobal(newLS, "IsLuaStateRecreated");
				OK = LF_RunDefaultScript(newLS);
			}
			if (OK) LS = newLS;
			else lua_close(newLS);
		}
		return 1;
	}
	lua_pop(LS,1);
	return 0;
}

HANDLE LUAPLUG OpenW(const struct OpenInfo *Info)
{
	if(LS && Init2_Done)
	{
		static int Depth = 0; // prevents crashes (this function can be called recursively)
		HANDLE h;
		++Depth;
#ifdef EXPORT_PROCESSDIALOGEVENT
		EnterCriticalSection(&FindFileSection);
		h = LF_Open(LS, Info);
		LeaveCriticalSection(&FindFileSection);
#else
		h = LF_Open(LS, Info);
#endif
		return --Depth==0 && RecreateLuaState() ? NULL : LS ? h : NULL;
	}
	return NULL;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_GETFINDDATA
intptr_t LUAPLUG GetFindDataW(struct GetFindDataInfo *Info)
{
	if(LS && Init2_Done) return LF_GetFindData(LS, Info);

	return FALSE;
}

void LUAPLUG FreeFindDataW(const struct FreeFindDataInfo *Info)
{
	if(LS && Init2_Done) LF_FreeFindData(LS, Info);
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_CLOSEPANEL
void LUAPLUG ClosePanelW(const struct ClosePanelInfo *Info)
{
	if(LS && Init2_Done) LF_ClosePanel(LS, Info);
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_GETFILES
intptr_t LUAPLUG GetFilesW(struct GetFilesInfo *Info)
{
	if(LS && Init2_Done)
		return LF_GetFiles(LS, Info);

	return 0;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_GETOPENPANELINFO
void LUAPLUG GetOpenPanelInfoW(struct OpenPanelInfo *Info)
{
	if(LS && Init2_Done) LF_GetOpenPanelInfo(LS, Info);
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_EXITFAR
void LUAPLUG ExitFARW(const struct ExitInfo *Info)
{
	if(LS && Init2_Done)
	{
		lua_State* oldState = LS;
		LS = NULL;
		LF_ExitFAR(oldState, Info);
		lua_close(oldState);
	}
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_COMPARE
intptr_t LUAPLUG CompareW(const struct CompareInfo *Info)
{
	if(LS && Init2_Done) return LF_Compare(LS, Info);

	return 0;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_CONFIGURE
intptr_t LUAPLUG ConfigureW(const struct ConfigureInfo *Info)
{
	if(LS && Init2_Done) return LF_Configure(LS, Info);

	return FALSE;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_DELETEFILES
intptr_t LUAPLUG DeleteFilesW(const struct DeleteFilesInfo *Info)
{
	if(LS && Init2_Done) return LF_DeleteFiles(LS, Info);

	return FALSE;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_MAKEDIRECTORY
intptr_t LUAPLUG MakeDirectoryW(struct MakeDirectoryInfo *Info)
{
	if(LS && Init2_Done) return LF_MakeDirectory(LS, Info);

	return 0;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PROCESSPANELEVENT
intptr_t LUAPLUG ProcessPanelEventW(const struct ProcessPanelEventInfo *Info)
{
	if(LS && Init2_Done) return LF_ProcessPanelEvent(LS, Info);

	return FALSE;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PROCESSHOSTFILE
intptr_t LUAPLUG ProcessHostFileW(const struct ProcessHostFileInfo *Info)
{
	if(LS && Init2_Done) return LF_ProcessHostFile(LS, Info);

	return FALSE;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PROCESSPANELINPUT
intptr_t LUAPLUG ProcessPanelInputW(const struct ProcessPanelInputInfo *Info)
{
	if(LS && Init2_Done) return LF_ProcessPanelInput(LS, Info);

	return FALSE;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PUTFILES
intptr_t LUAPLUG PutFilesW(const struct PutFilesInfo *Info)
{
	if(LS && Init2_Done) return LF_PutFiles(LS, Info);

	return 0;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_SETDIRECTORY
intptr_t LUAPLUG SetDirectoryW(const struct SetDirectoryInfo *Info)
{
	if(LS && Init2_Done) return LF_SetDirectory(LS, Info);

	return FALSE;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_SETFINDLIST
intptr_t LUAPLUG SetFindListW(const struct SetFindListInfo *Info)
{
	if(LS && Init2_Done) return LF_SetFindList(LS, Info);

	return FALSE;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PROCESSEDITORINPUT
intptr_t LUAPLUG ProcessEditorInputW(const struct ProcessEditorInputInfo *Info)
{
	if(LS && Init2_Done) return LF_ProcessEditorInput(LS, Info);

	return 0;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PROCESSEDITOREVENT
intptr_t LUAPLUG ProcessEditorEventW(const struct ProcessEditorEventInfo *Info)
{
	if(LS && Init2_Done) return LF_ProcessEditorEvent(LS, Info);

	return 0;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PROCESSVIEWEREVENT
intptr_t LUAPLUG ProcessViewerEventW(const struct ProcessViewerEventInfo *Info)
{
	if(LS && Init2_Done) return LF_ProcessViewerEvent(LS, Info);

	return 0;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PROCESSDIALOGEVENT
intptr_t LUAPLUG ProcessDialogEventW(const struct ProcessDialogEventInfo *Info)
{
	if(LS && Init2_Done)
	{
#ifdef EXPORT_OPEN
		intptr_t r;
		EnterCriticalSection(&FindFileSection);
		r = LF_ProcessDialogEvent(LS, Info);
		LeaveCriticalSection(&FindFileSection);
		return r;
#else
		return LF_ProcessDialogEvent(LS, Info);
#endif
	}
	return 0;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_GETCUSTOMDATA
intptr_t LUAPLUG GetCustomDataW(const wchar_t *FilePath, wchar_t **CustomData)
{
	if(LS && Init2_Done) return LF_GetCustomData(LS, FilePath, CustomData);

	return 0;
}

void LUAPLUG FreeCustomDataW(wchar_t *CustomData)
{
	if(LS && Init2_Done) LF_FreeCustomData(LS, CustomData);
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_ANALYSE
HANDLE LUAPLUG AnalyseW(const struct AnalyseInfo *Info)
{
	if(LS && Init2_Done) return LF_Analyse(LS, Info);

	return 0;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_CLOSEANALYSE
void LUAPLUG CloseAnalyseW(const struct CloseAnalyseInfo *Info)
{
	if(LS && Init2_Done) LF_CloseAnalyse(LS, Info);
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PROCESSCONSOLEINPUT
intptr_t LUAPLUG ProcessConsoleInputW(struct ProcessConsoleInputInfo *Info)
{
	if(LS && Init2_Done) return LF_ProcessConsoleInput(LS, Info);

	return 0;
}
#endif
//---------------------------------------------------------------------------
