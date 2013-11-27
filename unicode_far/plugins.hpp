#pragma once

/*
plugins.hpp

������ � ��������� (������ �������, ���-��� ������ � flplugin.cpp)
*/
/*
Copyright � 1996 Eugene Roshal
Copyright � 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "bitflags.hpp"
#include "plclass.hpp"
#include "PluginA.hpp"
#include "configdb.hpp"
#include "mix.hpp"

class SaveScreen;
class FileEditor;
class Viewer;
class Frame;
class Panel;

enum
{
	PLUGIN_FARGETFILE,
	PLUGIN_FARGETFILES,
	PLUGIN_FARPUTFILES,
	PLUGIN_FARDELETEFILES,
	PLUGIN_FARMAKEDIRECTORY,
	PLUGIN_FAROTHER
};

// ����� ��� ���� Plugin.WorkFlags
enum PLUGINITEMWORKFLAGS
{
	PIWF_CACHED        = 0x00000001, // ����������
	PIWF_PRELOADED     = 0x00000002, //
	PIWF_DONTLOADAGAIN = 0x00000004, // �� ��������� ������ �����, �������� �
	//   ���������� �������� ��������� ������ ����
	PIWF_DATALOADED    = 0x00000008, // LoadData ������� �����������
};

// ����� ��� ���� Plugin.FuncFlags - ���������� �������
enum PLUGINITEMCALLFUNCFLAGS
{
	PICFF_LOADED               = 0x00000001, // DLL �������� ;-)
	PICFF_OPENPANEL            = 0x00000004, //
	PICFF_ANALYSE              = 0x00000008, //
	PICFF_CLOSEPANEL           = 0x00000010, //
	PICFF_GETGLOBALINFO        = 0x00000002, //
	PICFF_SETSTARTUPINFO       = 0x00000004, //
	PICFF_OPENPLUGIN           = 0x00000008, //
	PICFF_OPENFILEPLUGIN       = 0x00000010, //
	PICFF_CLOSEPLUGIN          = 0x00000020, //
	PICFF_GETPLUGININFO        = 0x00000040, //
	PICFF_GETOPENPANELINFO     = 0x00000080, //
	PICFF_GETFINDDATA          = 0x00000100, //
	PICFF_FREEFINDDATA         = 0x00000200, //
	PICFF_GETVIRTUALFINDDATA   = 0x00000400, //
	PICFF_FREEVIRTUALFINDDATA  = 0x00000800, //
	PICFF_SETDIRECTORY         = 0x00001000, //
	PICFF_GETFILES             = 0x00002000, //
	PICFF_PUTFILES             = 0x00004000, //
	PICFF_DELETEFILES          = 0x00008000, //
	PICFF_MAKEDIRECTORY        = 0x00010000, //
	PICFF_PROCESSHOSTFILE      = 0x00020000, //
	PICFF_SETFINDLIST          = 0x00040000, //
	PICFF_CONFIGURE            = 0x00080000, //
	PICFF_EXITFAR              = 0x00100000, //
	PICFF_PROCESSPANELINPUT    = 0x00200000, //
	PICFF_PROCESSPANELEVENT    = 0x00400000, //
	PICFF_PROCESSEDITOREVENT   = 0x00800000, //
	PICFF_COMPARE              = 0x01000000, //
	PICFF_PROCESSEDITORINPUT   = 0x02000000, //
	PICFF_MINFARVERSION        = 0x04000000, //
	PICFF_PROCESSVIEWEREVENT   = 0x08000000, //
	PICFF_PROCESSDIALOGEVENT   = 0x10000000, //
	PICFF_PROCESSSYNCHROEVENT  = 0x20000000, //
	PICFF_PROCESSCONSOLEINPUT  = 0x40000000, //
	PICFF_CLOSEANALYSE         = 0x80000000, //
};

// ����� ��� ���� PluginManager.Flags
enum PLUGINSETFLAGS
{
	PSIF_ENTERTOOPENPLUGIN        = 0x00000001, // ��������� � ������ OpenPlugin
	PSIF_PLUGINSLOADDED           = 0x80000000, // ������� ���������
};

ENUM(OPENFILEPLUGINTYPE)
{
	OFP_NORMAL,
	OFP_ALTERNATIVE,
	OFP_SEARCH,
	OFP_CREATE,
	OFP_EXTRACT,
	OFP_COMMANDS,
};

// ��������� ������ ������������ plugin.call � �.�.
typedef unsigned int CALLPLUGINFLAGS;
static const CALLPLUGINFLAGS
	CPT_MENU        = 0x00000001L,
	CPT_CONFIGURE   = 0x00000002L,
	CPT_CMDLINE     = 0x00000004L,
	CPT_INTERNAL    = 0x00000008L,
	CPT_MASK        = 0x0000000FL,
	CPT_CHECKONLY   = 0x10000000L;

struct CallPluginInfo
{
	CALLPLUGINFLAGS CallFlags;
	int OpenFrom;
	union
	{
		GUID *ItemGuid;
		const wchar_t *Command;
	};
	// ������������ � ������� CallPluginItem ��� ���������� ����
	Plugin *pPlugin;
	GUID FoundGuid;
};

struct PluginHandle
{
	HANDLE hPlugin;
	class Plugin *pPlugin;
};

class Dialog;

class PluginManager
{
public:
	PluginManager();
	~PluginManager();

	// API functions
	HANDLE Open(Plugin *pPlugin,int OpenFrom,const GUID& Guid,intptr_t Item);
	HANDLE OpenFilePlugin(const string* Name, int OpMode, OPENFILEPLUGINTYPE Type);
	HANDLE OpenFindListPlugin(const PluginPanelItem *PanelItem,size_t ItemsNumber);
	void ClosePanel(HANDLE hPlugin);
	void GetOpenPanelInfo(HANDLE hPlugin, OpenPanelInfo *Info);
	int GetFindData(HANDLE hPlugin,PluginPanelItem **pPanelItem,size_t *pItemsNumber,int OpMode);
	void FreeFindData(HANDLE hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber,bool FreeUserData);
	int GetVirtualFindData(HANDLE hPlugin,PluginPanelItem **pPanelItem,size_t *pItemsNumber,const string& Path);
	void FreeVirtualFindData(HANDLE hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber);
	int SetDirectory(HANDLE hPlugin,const string& Dir,int OpMode,struct UserDataItem *UserData=nullptr);
	int GetFile(HANDLE hPlugin,PluginPanelItem *PanelItem,const string& DestPath,string &strResultName,int OpMode);
	int GetFiles(HANDLE hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber,bool Move,const wchar_t **DestPath,int OpMode);
	int PutFiles(HANDLE hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber,bool Move,int OpMode);
	int DeleteFiles(HANDLE hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber,int OpMode);
	int MakeDirectory(HANDLE hPlugin,const wchar_t **Name,int OpMode);
	int ProcessHostFile(HANDLE hPlugin,PluginPanelItem *PanelItem,size_t ItemsNumber,int OpMode);
	int ProcessKey(HANDLE hPlugin,const INPUT_RECORD *Rec,bool Pred);
	int ProcessEvent(HANDLE hPlugin,int Event,void *Param);
	int Compare(HANDLE hPlugin,const PluginPanelItem *Item1,const PluginPanelItem *Item2,unsigned int Mode);
	int ProcessEditorInput(INPUT_RECORD *Rec);
	int ProcessEditorEvent(int Event,void *Param,int EditorID);
	int ProcessSubscribedEditorEvent(int Event,void *Param,int EditorID, const std::list<GUID> &PluginIds);
	int ProcessViewerEvent(int Event,void *Param,int ViewerID);
	int ProcessDialogEvent(int Event,FarDialogEvent *Param);
	int ProcessConsoleInput(ProcessConsoleInputInfo *Info);
	string GetCustomData(const string& Name) const;

	int UnloadPlugin(Plugin *pPlugin, int From);
	HANDLE LoadPluginExternal(const string& lpwszModuleName, bool LoadToMem);
	int UnloadPluginExternal(HANDLE hPlugin);
	bool IsPluginUnloaded(Plugin* pPlugin);
	void LoadModels();
	void LoadPlugins();
	std::list<Plugin*>::const_iterator begin() const { return SortedPlugins.cbegin(); }
	std::list<Plugin*>::const_iterator end() const { return SortedPlugins.cend(); }
	std::list<Plugin*>::const_iterator cbegin() const { return begin(); }
	std::list<Plugin*>::const_iterator cend() const { return end(); }
#if defined(_MSC_VER) && _MSC_VER < 1700
	// buggy implementation of begin()/end() in VC10, name "iterator" is hardcoded.
	typedef std::list<Plugin*>::const_iterator iterator;
#endif
	typedef Plugin* value_type;
	Plugin *GetPlugin(const string& ModuleName);
	#if 1
	//Maximus: ��� ������ �����
	bool IsPluginValid(Plugin *pPlugin);
	#endif
	size_t GetPluginsCount() const { return SortedPlugins.size(); }
#ifndef NO_WRAPPER
	size_t OemPluginsPresent() const { return OemPluginsCount > 0; }
#endif // NO_WRAPPER
	bool IsPluginsLoaded() const { return Flags.Check(PSIF_PLUGINSLOADDED); }
	bool CheckFlags(DWORD NewFlags) const { return Flags.Check(NewFlags); }
	void Configure(int StartPos=0);
	void ConfigureCurrent(Plugin *pPlugin,const GUID& Guid);
	int CommandsMenu(int ModalType,int StartPos,const wchar_t *HistoryName=nullptr);
	bool GetDiskMenuItem(Plugin *pPlugin,size_t PluginItem,bool &ItemPresent, wchar_t& PluginHotkey, string &strPluginText, GUID &Guid);
	int UseFarCommand(HANDLE hPlugin,int CommandType);
	void ReloadLanguage();
	void DiscardCache();
	int ProcessCommandLine(const string& Command,Panel *Target=nullptr);
	bool SetHotKeyDialog(Plugin *pPlugin, const GUID& Guid, PluginsHotkeysConfig::HotKeyTypeEnum HotKeyType, const string& DlgPluginTitle);
	void ShowPluginInfo(Plugin *pPlugin, const GUID& Guid);
	size_t GetPluginInformation(Plugin *pPlugin, FarGetPluginInformation *pInfo, size_t BufferSize);
	// $ .09.2000 SVS - ������� CallPlugin - ����� ������ �� ID � ��������� OpenFrom = OPEN_*
	int CallPlugin(const GUID& SysID,int OpenFrom, void *Data, void **Ret=nullptr);
	int CallPluginItem(const GUID& Guid, CallPluginInfo *Data);
	Plugin *FindPlugin(const GUID& SysID) const;
	static const GUID& GetGUID(HANDLE hPlugin);
	void RefreshPluginsList();
	void UndoRemove(Plugin* plugin);
	FileEditor* GetCurEditor() const { return m_CurEditor; }
	void SetCurEditor(FileEditor* Editor) { m_CurEditor = Editor; }
	Viewer* GetCurViewer() const { return m_CurViewer; }
	void SetCurViewer(Viewer* Viewer) { m_CurViewer = Viewer; }

private:
	void LoadIfCacheAbsent();
	void ReadUserBackgound(SaveScreen *SaveScr);
	void GetHotKeyPluginKey(Plugin *pPlugin, string &strPluginKey);
	void GetPluginHotKey(Plugin *pPlugin,const GUID& Guid, PluginsHotkeysConfig::HotKeyTypeEnum HotKeyType,string &strHotKey);
	Plugin* LoadPlugin(const string& lpwszModuleName, const api::FAR_FIND_DATA &FindData, bool LoadToMem);
	bool AddPlugin(Plugin *pPlugin);
	bool RemovePlugin(Plugin *pPlugin);
	bool UpdateId(Plugin *pPlugin, const GUID& Id);
	void LoadPluginsFromCache();

	#if 1
	//Maximus: ����������� ���� ��������
	void GetPluginVersion(LPCTSTR ModuleName,string &strModuleVer);
	#endif

	std::vector<std::unique_ptr<GenericPluginModel>> PluginModels;
	std::unordered_map<GUID, std::unique_ptr<Plugin>, uuid_hash, uuid_equal> Plugins;
	std::list<Plugin*> SortedPlugins;
	std::list<Plugin*> UnloadedPlugins;
	BitFlags Flags;
#ifndef NO_WRAPPER
	size_t OemPluginsCount;
#endif // NO_WRAPPER
	FileEditor* m_CurEditor;
	Viewer* m_CurViewer;

	friend class Plugin;
};
