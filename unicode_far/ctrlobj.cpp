/*
ctrlobj.cpp

���������� ���������� ���������, ������� ��������� ���������� � ����
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

#include "headers.hpp"
#pragma hdrstop

#include "ctrlobj.hpp"
#include "manager.hpp"
#include "cmdline.hpp"
#include "hilight.hpp"
#include "history.hpp"
#include "treelist.hpp"
#include "filefilter.hpp"
#include "filepanels.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "fileowner.hpp"
#include "dirmix.hpp"
#include "console.hpp"
#include "shortcuts.hpp"
#include "poscache.hpp"
#include "plugins.hpp"
#include "desktop.hpp"
#include "keybar.hpp"

ControlObject::ControlObject():
	CmdLine(nullptr),
	TopMenuBar(nullptr),
	FPanels(nullptr)
{
	_OT(SysLog(L"[%p] ControlObject::ControlObject()", this));

	SetColor(COL_COMMANDLINEUSERSCREEN);
	GotoXY(0, ScrY - 3);
	ShowCopyright();
	GotoXY(0, ScrY - 2);
	MoveCursor(0, ScrY - 1);

	Desktop = desktop::create();
	Global->WindowManager->InsertWindow(Desktop);
	Desktop->FillFromBuffer();

	HiFiles = new HighlightFiles;
	Plugins = new PluginManager;

	CmdHistory=new History(HISTORYTYPE_CMD, string(), Global->Opt->SaveHistory);
	CmdHistory->SetAddMode(true, 2, false); // case insensitive
	FolderHistory=new History(HISTORYTYPE_FOLDER, string(), Global->Opt->SaveFoldersHistory);
	ViewHistory=new History(HISTORYTYPE_VIEW, string(), Global->Opt->SaveViewHistory);
	FolderHistory->SetAddMode(true,2,true);
	ViewHistory->SetAddMode(true,Global->Opt->FlagPosixSemantics?1:2,true);

	FileFilter::InitFilter();
}


void ControlObject::Init(int DirCount)
{
	FPanels = FilePanels::create();
	CmdLine=new CommandLine();
	Desktop->FillFromBuffer();
	this->TopMenuBar=&(FPanels->TopMenuBar);
	FPanels->Init(DirCount);
	FPanels->SetScreenPosition();

	if (Global->Opt->ShowMenuBar)
		this->TopMenuBar->Show();

//  FPanels->Redraw();
	CmdLine->Show();

	if (Global->Opt->ShowKeyBar)
		FPanels->GetKeybar().Show();

	// LoadPlugins() before panel updates
	//
	Global->WindowManager->InsertWindow(FPanels); // before PluginCommit()
	{
		string strOldTitle;
		Console().GetTitle(strOldTitle);
		Global->WindowManager->PluginCommit();
		Plugins->LoadPlugins();
		Console().SetTitle(strOldTitle);
	}

	Cp()->LeftPanel->Update(0);
	Cp()->RightPanel->Update(0);

	Cp()->LeftPanel->GoToFile(Global->Opt->LeftPanel.CurFile);
	Cp()->RightPanel->GoToFile(Global->Opt->RightPanel.CurFile);

	FarChDir(Cp()->ActivePanel()->GetCurDir());
	Cp()->ActivePanel()->SetFocus();

	Macro.LoadMacros(true);
	Cp()->LeftPanel->SetCustomSortMode(Global->Opt->LeftPanel.SortMode, true);
	Cp()->RightPanel->SetCustomSortMode(Global->Opt->RightPanel.SortMode, true);
	Global->WindowManager->SwitchToPanels();  // otherwise panels are empty
	/*
		FarChDir(StartCurDir);
	*/
//  _SVS(SysLog(L"ActivePanel->GetCurDir='%s'",StartCurDir));
//  _SVS(char PPP[NM];Cp()->GetAnotherPanel(Cp()->ActivePanel)->GetCurDir(PPP);SysLog(L"AnotherPanel->GetCurDir='%s'",PPP));
}

void ControlObject::CreateDummyFilePanels()
{
	FPanels = FilePanels::create(false);
}

ControlObject::~ControlObject()
{
	if (Global->CriticalInternalError)
		return;

	_OT(SysLog(L"[%p] ControlObject::~ControlObject()", this));

	if (Cp() && Cp()->ActivePanel())
	{
		if (Global->Opt->AutoSaveSetup)
			Global->Opt->Save(false);

		if (Cp()->ActivePanel()->GetMode() != PLUGIN_PANEL)
		{
			FolderHistory->AddToHistory(Cp()->ActivePanel()->GetCurDir());
		}
	}

	Global->WindowManager->CloseAll();
	FPanels=nullptr;
	FileFilter::CloseFilter();
	History::CompactHistory();
	FilePositionCache::CompactHistory();

	delete CmdLine;
	delete ViewHistory;
	delete FolderHistory;
	delete CmdHistory;
	delete Plugins;
	delete HiFiles;
}


void ControlObject::ShowCopyright(DWORD Flags)
{
	if (Flags&1)
	{
		string strOut(Global->Version());
		strOut.append(EOL_STR).append(Global->Copyright()).append(EOL_STR);
		Console().Write(strOut);
		Console().Commit();
	}
	else
	{
		COORD Size, CursorPosition;
		Console().GetSize(Size);
		Console().GetCursorPosition(CursorPosition);
		int FreeSpace=Size.Y-CursorPosition.Y-1;

		if (FreeSpace<5)
			ScrollScreen(5-FreeSpace);

		GotoXY(0,ScrY-4);
		Text(Global->Version());
		GotoXY(0,ScrY-3);
		Text(Global->Copyright());
	}
}

FilePanels* ControlObject::Cp()
{
	return FPanels.get();
}
