#pragma once

/*
global.hpp

�������� ���������� ����������
�������� ���������.
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

class global: NonCopyable
{
public:
	global();
	~global();

	HANDLE MainThreadHandle() const {return m_MainThreadHandle;}
	inline bool IsMainThread() const {return GetCurrentThreadId() == m_MainThreadId;}
	uint64_t FarUpTime() const;
	static bool IsPtr(const void* Address);
	static bool IsUserAdmin();
	static const wchar_t* Version();
	static const wchar_t* Copyright();

	static void CatchError();
	static DWORD CaughtError() {return m_LastError;}
	static NTSTATUS CaughtStatus() {return m_LastStatus;}

	// BUGBUG

	clock_t StartIdleTime;
	int WaitInMainLoop;
	int WaitInFastFind;
	string g_strFarModuleName;
	string g_strFarINI;
	string g_strFarPath;
	string strGlobalSearchString;
	string strInitTitle;
	bool GlobalSearchCase;
	bool GlobalSearchWholeWords; // �������� "Whole words" ��� ������
	bool GlobalSearchHex; // �������� "Search for hex" ��� ������
	bool GlobalSearchReverse;
	int ScreenSaverActive;
	int CloseFAR, CloseFARMenu, AllowCancelExit;
	int DisablePluginsOutput;
	BOOL IsProcessAssignMacroKey;
	BOOL IsRedrawWindowInProcess;
	size_t PluginPanelsCount;
	BOOL ProcessException;
	BOOL ProcessShowClock;
	const wchar_t *HelpFileMask;
	bool OnlyEditorViewerUsed; // -e or -v
#if defined(SYSLOG)
	BOOL StartSysLog;
#endif
#ifdef DIRECT_RT
	bool DirectRT;
#endif
	class SaveScreen *GlobalSaveScrPtr;
	int CriticalInternalError;
	int KeepUserScreen;
	int Macro_DskShowPosType; // ��� ����� ������ �������� ���� ������ ������ (0 - ������� �� ��������, 1 - ����� (AltF1), 2 - ������ (AltF2))
	DWORD ErrorMode;
#ifndef NO_WRAPPER
	string strRegUser;
#endif // NO_WRAPPER

	// BUGBUG end

private:
	DWORD m_MainThreadId;
	LARGE_INTEGER m_FarUpTime;
	HANDLE m_MainThreadHandle;

	static thread_local DWORD m_LastError;
	static thread_local NTSTATUS m_LastStatus;

public:
	class ScreenBuf* ScrBuf;
	class FormatScreen FS;
	class Manager* WindowManager;
	class Options *Opt;
	class Language *Lang;
	class elevation *Elevation;
	class Database* Db;
	class ControlObject* CtrlObject;
};

#define MSG(ID) Global->Lang->GetMsg(ID)

extern global* Global;
