#pragma once

/*
fileedit.hpp

�������������� ����� - ���������� ��� editor.cpp
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

#include "window.hpp"
#include "editor.hpp"
#include "plugin.hpp"
#include "namelist.hpp"

// ���� �������� Editor::SaveFile()
enum
{
	SAVEFILE_ERROR   = 0,         // �������� ���������, �� ����������
	SAVEFILE_SUCCESS = 1,         // ���� ������� ���������, ���� ��������� ���� �� ����
	SAVEFILE_CANCEL  = 2          // ���������� ��������, �������� �� ���������
};

enum FFILEEDIT_FLAGS
{
	FFILEEDIT_NEW                   = 0x00010000,  // ���� ���� ����������! ����� ��� ��� ������ �������! ���� ������ � ��� ���.
	FFILEEDIT_REDRAWTITLE           = 0x00020000,  // ����� ��������� ���������?
	FFILEEDIT_FULLSCREEN            = 0x00040000,  // ������������� �����?
	FFILEEDIT_DISABLEHISTORY        = 0x00080000,  // ��������� ������ � �������?
	FFILEEDIT_ENABLEF6              = 0x00100000,  // ������������� �� ������ �����?
	FFILEEDIT_SAVETOSAVEAS          = 0x00200000,  // $ 17.08.2001 KM  ��������� ��� ������ �� AltF7.
	//   ��� �������������� ���������� ����� �� ������ ���
	//   ������� F2 ������� ����� ShiftF2.
	FFILEEDIT_SAVEWQUESTIONS        = 0x00400000,  // ��������� ��� ��������
	FFILEEDIT_LOCKED                = 0x00800000,  // �������������?
	FFILEEDIT_OPENFAILED            = 0x01000000,  // ���� ������� �� �������
	FFILEEDIT_DELETEONCLOSE         = 0x02000000,  // ������� � ����������� ���� ������ � ��������� (���� ��� ����)
	FFILEEDIT_DELETEONLYFILEONCLOSE = 0x04000000,  // ������� � ����������� ������ ����
	FFILEEDIT_DISABLESAVEPOS        = 0x08000000,  // �� ��������� ������� ��� �����
	FFILEEDIT_CANNEWFILE            = 0x10000000,  // ����������� ����� ����?
	FFILEEDIT_SERVICEREGION         = 0x20000000,  // ������������ ��������� �������
	FFILEEDIT_CODEPAGECHANGEDBYUSER = 0x40000000,
};

typedef std::shared_ptr<FileEditor> fileeditor_ptr;

class FileEditor: public window
{
public:
	static fileeditor_ptr create(const string&  Name, uintptr_t codepage, DWORD InitFlags, int StartLine = -1, int StartChar = -1, const string* PluginData = nullptr, EDITOR_FLAGS OpenModeExstFile = EF_OPENMODE_QUERY);
	static fileeditor_ptr create(const string&  Name, uintptr_t codepage, DWORD InitFlags, int StartLine, int StartChar, const string* Title, int X1, int Y1, int X2, int Y2, int DeleteOnClose = 0, window_ptr Update = nullptr, EDITOR_FLAGS OpenModeExstFile = EF_OPENMODE_QUERY);
	virtual ~FileEditor();

	virtual BOOL IsFileModified() const override { return m_editor->IsFileModified(); }
	virtual int GetTypeAndName(string &strType, string &strName) override;
	virtual __int64 VMProcess(int OpCode, void *vParam = nullptr, __int64 iParam = 0) override;
	virtual void Show() override;

	void ShowStatus();
	void SetLockEditor(BOOL LockMode);
	bool IsFullScreen() { return m_Flags.Check(FFILEEDIT_FULLSCREEN); }
	void SetNamesList(NamesList& Names);
	void SetEnableF6(bool AEnableF6) { m_Flags.Change(FFILEEDIT_ENABLEF6, AEnableF6); InitKeyBar(); }
	// ��������� ��� ������ �� AltF7. ��� �������������� ���������� ����� ��
	// ������ ��� ������� F2 ������� ����� ShiftF2.
	void SetSaveToSaveAs(bool ToSaveAs) { m_Flags.Change(FFILEEDIT_SAVETOSAVEAS, ToSaveAs); InitKeyBar(); }
	intptr_t EditorControl(int Command, intptr_t Param1, void *Param2);
	bool SetCodePage(uintptr_t codepage);  //BUGBUG
	BOOL IsFileChanged() const { return m_editor->IsFileChanged(); }
	void GetEditorOptions(Options::EditorOptions& EdOpt) const;
	void SetEditorOptions(const Options::EditorOptions& EdOpt);
	void CodepageChangedByUser() { m_Flags.Set(FFILEEDIT_CODEPAGECHANGEDBYUSER); }
	void SetPluginTitle(const string* PluginTitle);
	int GetId() const { return m_editor->EditorID; }
	FileEditor* GetById(int ID) { return GetId()==ID?this:nullptr; }

private:
	FileEditor();

	virtual void DisplayObject() override;
	virtual void InitKeyBar() override;
	virtual int ProcessKey(const Manager::Key& Key) override;
	virtual int ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	virtual void ShowConsoleTitle() override;
	virtual void OnChangeFocus(int focus) override;
	virtual void SetScreenPosition() override;
	virtual const wchar_t *GetTypeName() override { return L"[FileEdit]"; }
	virtual int GetType() const override { return windowtype_editor; }
	virtual void OnDestroy() override;
	virtual int GetCanLoseFocus(int DynamicMode = FALSE) const override;
	virtual bool CanFastHide() const override; // ��� ���� CtrlAltShift
	virtual void ResizeConsole() override;
	virtual string GetTitle() const override;

	/* Ret:
		0 - �� ������� ������
		1 - ������� ���� � �������
		2 - ������� ������ ����
	*/
	void SetDeleteOnClose(int NewMode);
	int ReProcessKey(int Key, int CalledFromControl = TRUE);
	bool AskOverwrite(const string& FileName);
	// ���������� ������� ����, �������� �� ���� ���������
	// ������������ ��� �������� ������� ���������� � ������� �� CtrlF10
	bool isTemporary() const;
	void Init(const string& Name, uintptr_t codepage, const string* Title, DWORD InitFlags, int StartLine, int StartChar, const string* PluginData, int DeleteOnClose, window_ptr Update, EDITOR_FLAGS OpenModeExstFile);
	int LoadFile(const string& Name, int &UserBreak);
	bool ReloadFile(uintptr_t codepage);
	//TextFormat, Codepage � AddSignature ������������ ������, ���� bSaveAs = true!
	int SaveFile(const string& Name, int Ask, bool bSaveAs, int TextFormat = 0, uintptr_t Codepage = CP_UNICODE, bool AddSignature = false);
	void SetTitle(const string* Title);
	BOOL SetFileName(const string& NewFileName);
	int ProcessEditorInput(const INPUT_RECORD& Rec);
	void ChangeEditKeyBar();
	DWORD EditorGetFileAttributes(const string& Name);
	void SetPluginData(const string* PluginData);
	const wchar_t *GetPluginData() { return strPluginData.data(); }
	bool LoadFromCache(EditorPosCache &pc);
	void SaveToCache();
	void ReadEvent(void);
	int  ProcessQuitKey(int FirstSave, BOOL NeedQuestion = TRUE, bool DeleteWindow = true);
	bool UpdateFileList();

	static uintptr_t GetDefaultCodePage();

	std::unique_ptr<Editor> m_editor;
	NamesList EditNamesList;
	bool F4KeyOnly;
	string strFileName;
	string strFullFileName;
	string strStartDir;
	string strTitle;
	string strPluginTitle;
	string strPluginData;
	api::FAR_FIND_DATA FileInfo;
	wchar_t AttrStr[4];            // 13.02.2001 IS - ���� �������� ����� ���������, ����� �� ��������� �� ����� ���
	DWORD m_FileAttributes;          // 12.02.2001 IS - ���� �������� �������� ����� ��� ��������, ���������� ���-������...
	BOOL  FileAttributesModified;  // 04.11.2003 SKV - ���� �� ��������������� ��������� ��� save
	DWORD SysErrorCode;
	bool m_bClosing;               // 28.04.2005 AY: true ����� �������� ������������ (�.�. � �����������)
	bool bEE_READ_Sent;
	bool bLoaded;
	bool m_bAddSignature;
	bool BadConversion;
	uintptr_t m_codepage; //BUGBUG
};

bool dlgOpenEditor(string &strFileName, uintptr_t &codepage);
bool dlgBadEditorCodepage(uintptr_t &codepage);
