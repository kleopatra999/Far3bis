/*
stddlg.cpp

���� ������ ����������� ��������
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

#include "stddlg.hpp"
#include "keys.hpp"
#include "dialog.hpp"
#include "ctrlobj.hpp"
#include "farexcpt.hpp"
#include "strmix.hpp"
#include "macro.hpp"
#include "keyboard.hpp"
#include "imports.hpp"
#include "message.hpp"
#include "lasterror.hpp"
#include "TaskBar.hpp"
#include "language.hpp"
#include "DlgGuid.hpp"
#include "datetime.hpp"
#include "interf.hpp"

int GetSearchReplaceString(
	bool IsReplaceMode,
	const wchar_t *Title,
	const wchar_t *SubTitle,
	string& SearchStr,
	string& ReplaceStr,
	const wchar_t *TextHistoryName,
	const wchar_t *ReplaceHistoryName,
	bool* pCase,
	bool* pWholeWords,
	bool* pReverse,
	bool* pRegexp,
	bool* pPreserveStyle,
	const wchar_t *HelpTopic,
	bool HideAll,
	const GUID* Id)
{
	int Result = 0;

	if (!TextHistoryName)
		TextHistoryName = L"SearchText";

	if (!ReplaceHistoryName)
		ReplaceHistoryName = L"ReplaceText";

	if (!Title)
		Title=MSG(IsReplaceMode?MEditReplaceTitle:MEditSearchTitle);

	if (!SubTitle)
		SubTitle=MSG(MEditSearchFor);


	bool Case=pCase?*pCase:false;
	bool WholeWords=pWholeWords?*pWholeWords:false;
	bool Reverse=pReverse?*pReverse:false;
	bool Regexp=pRegexp?*pRegexp:false;
	bool PreserveStyle=pPreserveStyle?*pPreserveStyle:false;

	#if 1
	//Maximus: ��������� "�����" ��������
	int BorderW = (72<(ScrX-1))?72:(ScrX-1);
	int ElemW = BorderW - 2; // 70
	int ElemX2 = (BorderW + 4) / 2; // 40
	#endif

	if (IsReplaceMode)
	{
		/*
		  0         1         2         3         4         5         6         7
		  0123456789012345678901234567890123456789012345678901234567890123456789012345
		00
		01   +----------------------------- Replace ------------------------------+
		02   | Search for                                                         |
		03   |                                                                   |
		04   | Replace with                                                       |
		05   |                                                                   |
		06   +--------------------------------------------------------------------+
		07   | [ ] Case sensitive                 [ ] Regular expressions         |
		08   | [ ] Whole words                    [ ] Preserve style              |
		09   | [ ] Reverse search                                                 |
		10   +--------------------------------------------------------------------+
		11   |                      [ Replace ]  [ Cancel ]                       |
		12   +--------------------------------------------------------------------+
		13
		*/
		FarDialogItem ReplaceDlgData[]=
		{
			#if 1
			//Maximus: ��������� "�����" ��������
			{DI_DOUBLEBOX,3,1,BorderW,12,0,nullptr,nullptr,0,Title},
			#else
			{DI_DOUBLEBOX,3,1,72,12,0,nullptr,nullptr,0,Title},
			#endif
			{DI_TEXT,5,2,0,2,0,nullptr,nullptr,0,SubTitle},
			#if 1
			//Maximus: ��������� "�����" ��������
			{DI_EDIT,5,3,ElemW,3,0,TextHistoryName,nullptr,DIF_FOCUS|DIF_USELASTHISTORY|(*TextHistoryName?DIF_HISTORY:0),SearchStr.data()},
			#else
			{DI_EDIT,5,3,70,3,0,TextHistoryName,nullptr,DIF_FOCUS|DIF_USELASTHISTORY|(*TextHistoryName?DIF_HISTORY:0),SearchStr.data()},
			#endif
			{DI_TEXT,5,4,0,4,0,nullptr,nullptr,0,MSG(MEditReplaceWith)},
			#if 1
			//Maximus: ��������� "�����" ��������
			{DI_EDIT,5,5,ElemW,5,0,ReplaceHistoryName,nullptr,(*ReplaceHistoryName?DIF_HISTORY:0)/*|DIF_USELASTHISTORY*/,ReplaceStr.data()},
			#else
			{DI_EDIT,5,5,70,5,0,ReplaceHistoryName,nullptr,(*ReplaceHistoryName?DIF_HISTORY:0)/*|DIF_USELASTHISTORY*/,ReplaceStr.data()},
			#endif
			{DI_TEXT,-1,6,0,6,0,nullptr,nullptr,DIF_SEPARATOR,L""},
			{DI_CHECKBOX,5,7,0,7,Case,nullptr,nullptr,0,MSG(MEditSearchCase)},
			{DI_CHECKBOX,5,8,0,8,WholeWords,nullptr,nullptr,0,MSG(MEditSearchWholeWords)},
			{DI_CHECKBOX,5,9,0,9,Reverse,nullptr,nullptr,0,MSG(MEditSearchReverse)},
			#if 1
			//Maximus: ��������� "�����" ��������
			{DI_CHECKBOX,ElemX2,7,0,7,Regexp,nullptr,nullptr,0,MSG(MEditSearchRegexp)},
			#else
			{DI_CHECKBOX,40,7,0,7,Regexp,nullptr,nullptr,0,MSG(MEditSearchRegexp)},
			#endif
			{DI_CHECKBOX,40,8,0,8,PreserveStyle,nullptr,nullptr,0,MSG(MEditSearchPreserveStyle)},
			{DI_TEXT,-1,10,0,10,0,nullptr,nullptr,DIF_SEPARATOR,L""},
			{DI_BUTTON,0,11,0,11,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MEditReplaceReplace)},
			{DI_BUTTON,0,11,0,11,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MEditSearchCancel)},
		};
		auto ReplaceDlg = MakeDialogItemsEx(ReplaceDlgData);

		if (!pCase)
			ReplaceDlg[6].Flags |= DIF_DISABLE; // DIF_HIDDEN ??
		if (!pWholeWords)
			ReplaceDlg[7].Flags |= DIF_DISABLE; // DIF_HIDDEN ??
		if (!pReverse)
			ReplaceDlg[8].Flags |= DIF_DISABLE; // DIF_HIDDEN ??
		if (!pRegexp)
			ReplaceDlg[9].Flags |= DIF_DISABLE; // DIF_HIDDEN ??
		if (!pPreserveStyle)
			ReplaceDlg[10].Flags |= DIF_DISABLE; // DIF_HIDDEN ??

		auto Dlg = Dialog::create(ReplaceDlg);
		#if 1
		//Maximus: ��������� "�����" ��������
		Dlg->SetPosition(-1,-1,BorderW+4,14);
		#else
		Dlg->SetPosition(-1,-1,76,14);
		#endif

		if (HelpTopic && *HelpTopic)
			Dlg->SetHelp(HelpTopic);

		if(Id) Dlg->SetId(*Id);

		Dlg->Process();

		if(Dlg->GetExitCode() == 12)
		{
			Result = 1;
			SearchStr = ReplaceDlg[2].strData;
			ReplaceStr = ReplaceDlg[4].strData;
			Case=ReplaceDlg[6].Selected == BSTATE_CHECKED;
			WholeWords=ReplaceDlg[7].Selected == BSTATE_CHECKED;
			Reverse=ReplaceDlg[8].Selected == BSTATE_CHECKED;
			Regexp=ReplaceDlg[9].Selected == BSTATE_CHECKED;
			PreserveStyle=ReplaceDlg[10].Selected == BSTATE_CHECKED;
		}
	}
	else
	{
		/*
		  0         1         2         3         4         5         6         7
		  0123456789012345678901234567890123456789012345678901234567890123456789012345
		00
		01   +------------------------------ Search ------------------------------+
		02   | Search for                                                         |
		03   |                                                                   |
		04   +--------------------------------------------------------------------+
		05   | [ ] Case sensitive                 [ ] Regular expressions         |
		06   | [ ] Whole words                    [ ] Reverse search              |
		07   +--------------------------------------------------------------------+
		08   |                   { Search } [ All ] [ Cancel ]                    |
		09   +--------------------------------------------------------------------+
		*/
		FarDialogItem SearchDlgData[]=
		{
			#if 1
			//Maximus: ��������� "�����" ��������
			{DI_DOUBLEBOX,3,1,BorderW,9,0,nullptr,nullptr,0,Title},
			#else
			{DI_DOUBLEBOX,3,1,72,9,0,nullptr,nullptr,0,Title},
			#endif
			{DI_TEXT,5,2,0,2,0,nullptr,nullptr,0,SubTitle},
			#if 1
			//Maximus: ��������� "�����" ��������
			{DI_EDIT,5,3,ElemW,3,0,TextHistoryName,nullptr,DIF_FOCUS|DIF_USELASTHISTORY|(*TextHistoryName?DIF_HISTORY:0),SearchStr.data()},
			#else
			{DI_EDIT,5,3,70,3,0,TextHistoryName,nullptr,DIF_FOCUS|DIF_USELASTHISTORY|(*TextHistoryName?DIF_HISTORY:0),SearchStr.data()},
			#endif
			{DI_TEXT,-1,4,0,4,0,nullptr,nullptr,DIF_SEPARATOR,L""},
			{DI_CHECKBOX,5,5,0,5,Case,nullptr,nullptr,0,MSG(MEditSearchCase)},
			{DI_CHECKBOX,5,6,0,6,WholeWords,nullptr,nullptr,0,MSG(MEditSearchWholeWords)},
			#if 1
			//Maximus: ��������� "�����" ��������
			{DI_CHECKBOX,ElemX2,5,0,5,Regexp,nullptr,nullptr,0,MSG(MEditSearchRegexp)},
			{DI_CHECKBOX,ElemX2,6,0,6,Reverse,nullptr,nullptr,0,MSG(MEditSearchReverse)},
			#else
			{DI_CHECKBOX,40,5,0,5,Regexp,nullptr,nullptr,0,MSG(MEditSearchRegexp)},
			{DI_CHECKBOX,40,6,0,6,Reverse,nullptr,nullptr,0,MSG(MEditSearchReverse)},
			#endif
			{DI_TEXT,-1,7,0,7,0,nullptr,nullptr,DIF_SEPARATOR,L""},
			{DI_BUTTON,0,8,0,8,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MEditSearchSearch)},
			{DI_BUTTON,0,8,0,8,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MEditSearchAll)},
			{DI_BUTTON,0,8,0,8,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MEditSearchCancel)},
		};
		auto SearchDlg = MakeDialogItemsEx(SearchDlgData);

		if (!pCase)
			SearchDlg[4].Flags |= DIF_DISABLE; // DIF_HIDDEN ??
		if (!pWholeWords)
			SearchDlg[5].Flags |= DIF_DISABLE; // DIF_HIDDEN ??
		if (!pRegexp)
			SearchDlg[6].Flags |= DIF_DISABLE; // DIF_HIDDEN ??
		if (!pReverse)
			SearchDlg[7].Flags |= DIF_DISABLE; // DIF_HIDDEN ??

		if (HideAll)
			SearchDlg[10].Flags |= DIF_HIDDEN;

		auto Dlg = Dialog::create(SearchDlg);
		#if 1
		//Maximus: ��������� "�����" ��������
		Dlg->SetPosition(-1,-1,BorderW+4,11);
		#else
		Dlg->SetPosition(-1,-1,76,11);
		#endif

		if (HelpTopic && *HelpTopic)
			Dlg->SetHelp(HelpTopic);

		if(Id) Dlg->SetId(*Id);

		Dlg->Process();
		int ExitCode = Dlg->GetExitCode();

		if (ExitCode == 9 || ExitCode == 10)
		{
			Result = ExitCode == 9? 1 : 2;
			SearchStr = SearchDlg[2].strData;
			ReplaceStr.clear();
			Case=SearchDlg[4].Selected == BSTATE_CHECKED;
			WholeWords=SearchDlg[5].Selected == BSTATE_CHECKED;
			Regexp=SearchDlg[6].Selected == BSTATE_CHECKED;
			Reverse=SearchDlg[7].Selected == BSTATE_CHECKED;
		}
	}

	if (pCase)
		*pCase=Case;
	if (pWholeWords)
		*pWholeWords=WholeWords;
	if (pReverse)
		*pReverse=Reverse;
	if (pRegexp)
		*pRegexp=Regexp;
	if (pPreserveStyle)
		*pPreserveStyle=PreserveStyle;

	return Result;
}

int GetString(
	const wchar_t *Title,
	const wchar_t *Prompt,
	const wchar_t *HistoryName,
	const wchar_t *SrcText,
	string &strDestText,
	const wchar_t *HelpTopic,
	DWORD Flags,
	int *CheckBoxValue,
	const wchar_t *CheckBoxText,
	Plugin* PluginNumber,
	const GUID* Id
)
{
	int Substract=5; // �������������� �������� :-)
	int ExitCode;
	bool addCheckBox=Flags&FIB_CHECKBOX && CheckBoxValue && CheckBoxText;
	int offset=addCheckBox?2:0;
	FarDialogItem StrDlgData[]=
	{
		{DI_DOUBLEBOX, 3, 1, 72, 4, 0, nullptr, nullptr, 0,                                L""},
		{DI_TEXT,      5, 2,  0, 2, 0, nullptr, nullptr, DIF_SHOWAMPERSAND,                L""},
		{DI_EDIT,      5, 3, 70, 3, 0, nullptr, nullptr, DIF_FOCUS|DIF_DEFAULTBUTTON,      L""},
		{DI_TEXT,     -1, 4,  0, 4, 0, nullptr, nullptr, DIF_SEPARATOR,                    L""},
		{DI_CHECKBOX,  5, 5,  0, 5, 0, nullptr, nullptr, 0,                                L""},
		{DI_TEXT,     -1, 6,  0, 6, 0, nullptr, nullptr, DIF_SEPARATOR,                    L""},
		{DI_BUTTON,    0, 7,  0, 7, 0, nullptr, nullptr, DIF_CENTERGROUP,                  L""},
		{DI_BUTTON,    0, 7,  0, 7, 0, nullptr, nullptr, DIF_CENTERGROUP,                  L""},
	};
	auto StrDlg = MakeDialogItemsEx(StrDlgData);

	if (addCheckBox)
	{
		Substract-=2;
		StrDlg[0].Y2+=2;
		StrDlg[4].Selected = *CheckBoxValue != 0;
		StrDlg[4].strData = CheckBoxText;
	}

	if (Flags&FIB_BUTTONS)
	{
		Substract-=3;
		StrDlg[0].Y2+=2;
		StrDlg[2].Flags&=~DIF_DEFAULTBUTTON;
		StrDlg[5+offset].Y1=StrDlg[4+offset].Y1=5+offset;
		StrDlg[4+offset].Type=StrDlg[5+offset].Type=DI_BUTTON;
		StrDlg[4+offset].Flags=StrDlg[5+offset].Flags=DIF_CENTERGROUP;
		StrDlg[4+offset].Flags|=DIF_DEFAULTBUTTON;
		StrDlg[4+offset].strData = MSG(MOk);
		StrDlg[5+offset].strData = MSG(MCancel);
	}

	if (Flags&FIB_EXPANDENV)
	{
		StrDlg[2].Flags|=DIF_EDITEXPAND;
	}

	if (Flags&FIB_EDITPATH)
	{
		StrDlg[2].Flags|=DIF_EDITPATH;
	}

	if (Flags&FIB_EDITPATHEXEC)
	{
		StrDlg[2].Flags|=DIF_EDITPATHEXEC;
	}

	if (HistoryName)
	{
		StrDlg[2].strHistory=HistoryName;
		StrDlg[2].Flags|=DIF_HISTORY|(Flags&FIB_NOUSELASTHISTORY?0:DIF_USELASTHISTORY);
	}

	if (Flags&FIB_PASSWORD)
		StrDlg[2].Type=DI_PSWEDIT;

	if (Title)
		StrDlg[0].strData = Title;

	if (Prompt)
	{
		StrDlg[1].strData = Prompt;
		TruncStrFromEnd(StrDlg[1].strData, 66);

		if (Flags&FIB_NOAMPERSAND)
			StrDlg[1].Flags&=~DIF_SHOWAMPERSAND;
	}

	if (SrcText)
		StrDlg[2].strData = SrcText;

	{
		auto Dlg = Dialog::create(make_range(StrDlg.data(), StrDlg.data() + StrDlg.size() - Substract));
		Dlg->SetPosition(-1,-1,76,offset+((Flags&FIB_BUTTONS)?8:6));
		if(Id) Dlg->SetId(*Id);

		if (HelpTopic)
			Dlg->SetHelp(HelpTopic);

		Dlg->SetPluginOwner(PluginNumber);

		Dlg->Process();

		ExitCode=Dlg->GetExitCode();

		if (ExitCode == -2 && Global->CtrlObject->Macro.IsExecuting() != MACROSTATE_NOMACRO)
			Global->CtrlObject->Macro.SendDropProcess();
	}

	if (ExitCode == 2 || ExitCode == 4 || (addCheckBox && ExitCode == 6))
	{
		if (!(Flags&FIB_ENABLEEMPTY) && StrDlg[2].strData.empty())
			return FALSE;

		strDestText = StrDlg[2].strData;

		if (addCheckBox)
			*CheckBoxValue=StrDlg[4].Selected;

		return TRUE;
	}

	return FALSE;
}

/*
  ����������� ������ ����� ������.
  ����� ��� ���������� ���������� ������ � ������.

  Name      - ���� ����� ������� ����� (max 256 ��������!!!)
  Password  - ���� ����� ������� ������ (max 256 ��������!!!)
  Title     - ��������� ������� (����� ���� nullptr)
  HelpTopic - ���� ������ (����� ���� nullptr)
  Flags     - ����� (GNP_*)
*/
int GetNameAndPassword(const string& Title, string &strUserName, string &strPassword,const wchar_t *HelpTopic,DWORD Flags)
{
	static string strLastName, strLastPassword;
	int ExitCode;
	/*
	  0         1         2         3         4         5         6         7
	  0123456789012345678901234567890123456789012345678901234567890123456789012345
	|0                                                                             |
	|1   +------------------------------- Title -------------------------------+   |
	|2   | User name                                                           |   |
	|3   | *******************************************************************|   |
	|4   | User password                                                       |   |
	|5   | ******************************************************************* |   |
	|6   +---------------------------------------------------------------------+   |
	|7   |                         [ Ok ]   [ Cancel ]                         |   |
	|8   +---------------------------------------------------------------------+   |
	|9                                                                             |
	*/
	FarDialogItem PassDlgData[]=
	{
		{DI_DOUBLEBOX,  3, 1,72, 8,0,nullptr,nullptr,0,NullToEmpty(Title.data())},
		{DI_TEXT,       5, 2, 0, 2,0,nullptr,nullptr,0,MSG(MNetUserName)},
		{DI_EDIT,       5, 3,70, 3,0,L"NetworkUser",nullptr,DIF_FOCUS|DIF_USELASTHISTORY|DIF_HISTORY,(Flags&GNP_USELAST)?strLastName.data():strUserName.data()},
		{DI_TEXT,       5, 4, 0, 4,0,nullptr,nullptr,0,MSG(MNetUserPassword)},
		{DI_PSWEDIT,    5, 5,70, 5,0,nullptr,nullptr,0,(Flags&GNP_USELAST)?strLastPassword.data():strPassword.data()},
		{DI_TEXT,      -1, 6, 0, 6,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,     0, 7, 0, 7,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MOk)},
		{DI_BUTTON,     0, 7, 0, 7,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCancel)},
	};
	auto PassDlg = MakeDialogItemsEx(PassDlgData);

	{
		auto Dlg = Dialog::create(PassDlg);
		Dlg->SetPosition(-1,-1,76,10);
		Dlg->SetId(GetNameAndPasswordId);

		if (HelpTopic)
			Dlg->SetHelp(HelpTopic);

		Dlg->Process();
		ExitCode=Dlg->GetExitCode();
	}

	if (ExitCode!=6)
		return FALSE;

	// ���������� ������.
	strUserName = PassDlg[2].strData;
	strLastName = strUserName;
	strPassword = PassDlg[4].strData;
	strLastPassword = strPassword;
	return TRUE;
}

IFileIsInUse* CreateIFileIsInUse(const string& File)
{
	IFileIsInUse *pfiu = nullptr;
	IRunningObjectTable *prot;
	if (SUCCEEDED(GetRunningObjectTable(0, &prot)))
	{
		IMoniker *pmkFile;
		if (SUCCEEDED(CreateFileMoniker(File.data(), &pmkFile)))
		{
			IEnumMoniker *penumMk;
			if (SUCCEEDED(prot->EnumRunning(&penumMk)))
			{
				HRESULT hr = E_FAIL;
				ULONG celt;
				IMoniker *pmk;
				while (FAILED(hr) && (penumMk->Next(1, &pmk, &celt) == S_OK))
				{
					DWORD dwType;
					if (SUCCEEDED(pmk->IsSystemMoniker(&dwType)) && dwType == MKSYS_FILEMONIKER)
					{
						IMoniker *pmkPrefix;
						if (SUCCEEDED(pmkFile->CommonPrefixWith(pmk, &pmkPrefix)))
						{
							if (pmkFile->IsEqual(pmkPrefix) == S_OK)
							{
								IUnknown *punk;
								if (prot->GetObject(pmk, &punk) == S_OK)
								{
									hr = punk->QueryInterface(
#ifdef __GNUC__
										IID_IFileIsInUse, IID_PPV_ARGS_Helper(&pfiu)
#else
										IID_PPV_ARGS(&pfiu)
#endif
										);
									punk->Release();
								}
							}
							pmkPrefix->Release();
						}
					}
					pmk->Release();
				}
				penumMk->Release();
			}
			pmkFile->Release();
		}
		prot->Release();
	}
	return pfiu;
}

int OperationFailed(const string& Object, LNGID Title, const string& Description, bool AllowSkip)
{
	std::list<string> Msg;
	IFileIsInUse *pfiu = nullptr;
	LNGID Reason = MObjectLockedReasonOpened;
	bool SwitchBtn = false, CloseBtn = false;
	DWORD Error = Global->CaughtError();
	if(Error == ERROR_ACCESS_DENIED ||
		Error == ERROR_SHARING_VIOLATION ||
		Error == ERROR_LOCK_VIOLATION ||
		Error == ERROR_DRIVE_LOCKED)
	{
		string FullName;
		ConvertNameToFull(Object, FullName);
		pfiu = CreateIFileIsInUse(FullName);
		if (pfiu)
		{
			FILE_USAGE_TYPE UsageType = FUT_GENERIC;
			pfiu->GetUsage(&UsageType);
			switch(UsageType)
			{
			case FUT_PLAYING:
				Reason = MObjectLockedReasonPlayed;
				break;
			case FUT_EDITING:
				Reason = MObjectLockedReasonEdited;
				break;
			case FUT_GENERIC:
				Reason = MObjectLockedReasonOpened;
				break;
			}
			DWORD Capabilities = 0;
			pfiu->GetCapabilities(&Capabilities);
			if(Capabilities&OF_CAP_CANSWITCHTO)
			{
				SwitchBtn = true;
			}
			if(Capabilities&OF_CAP_CANCLOSE)
			{
				CloseBtn = true;
			}
			LPWSTR AppName = nullptr;
			if(SUCCEEDED(pfiu->GetAppName(&AppName)))
			{
				Msg.emplace_back(AppName);
			}
		}
		else
		{
			DWORD dwSession;
			WCHAR szSessionKey[CCH_RM_SESSION_KEY+1] = {};
			if (Imports().RmStartSession(&dwSession, 0, szSessionKey) == ERROR_SUCCESS)
			{
				PCWSTR pszFile = FullName.data();
				if (Imports().RmRegisterResources(dwSession, 1, &pszFile, 0, nullptr, 0, nullptr) == ERROR_SUCCESS)
				{
					DWORD dwReason;
					DWORD RmGetListResult;
					UINT nProcInfoNeeded;
					UINT nProcInfo = 1;
					std::vector<RM_PROCESS_INFO> rgpi(nProcInfo);
					while((RmGetListResult=Imports().RmGetList(dwSession, &nProcInfoNeeded, &nProcInfo, rgpi.data(), &dwReason)) == ERROR_MORE_DATA)
					{
						nProcInfo = nProcInfoNeeded;
						rgpi.resize(nProcInfo);
					}
					if(RmGetListResult ==ERROR_SUCCESS)
					{
						for (size_t i = 0; i < nProcInfo; i++)
						{
							string tmp = rgpi[i].strAppName;
							if (*rgpi[i].strServiceShortName)
							{
								tmp.append(L" [").append(rgpi[i].strServiceShortName).append(L"]");
							}
							tmp += L" (PID: " + std::to_wstring(rgpi[i].Process.dwProcessId);
							HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, rgpi[i].Process.dwProcessId);
							if (hProcess)
							{
								FILETIME ftCreate, ftExit, ftKernel, ftUser;
								if (GetProcessTimes(hProcess, &ftCreate, &ftExit, &ftKernel, &ftUser) && rgpi[i].Process.ProcessStartTime == ftCreate)
								{
									string Name;
									if (api::GetModuleFileNameEx(hProcess, nullptr, Name))
									{
										tmp += L", " + Name;
									}
								}
								CloseHandle(hProcess);
							}
							tmp += L")";
							Msg.emplace_back(tmp);
						}
					}
				}
				Imports().RmEndSession(dwSession);
			}
		}
	}
	int ButtonCount = (AllowSkip? 4 : 2) + (SwitchBtn? 1 : 0);
	size_t LineCount = 1 + 1 + (Msg.empty()? 0 : Msg.size() + 1) + ButtonCount;
	std::vector<string> Msgs;
	Msgs.resize(LineCount);
	Msgs[0] = Description;
	string qObj(Object);
	QuoteLeadingSpace(qObj);
	Msgs[1] = qObj;
	LangString strReason(MObjectLockedReason);
	strReason << MSG(Reason);
	if(!Msg.empty())
	{
		auto s = Msg.begin();
		Msgs[2] = strReason;
		for (size_t i = 3; i < LineCount - ButtonCount; ++i)
		{
			Msgs[i] = *s;
			++s;
		}
	}
	if(SwitchBtn)
	{
		Msgs[LineCount - ButtonCount] = MSG(MObjectLockedSwitchTo);
	}
	Msgs[LineCount - (AllowSkip? 4 : 2)] = CloseBtn? MSG(MObjectLockedClose) : MSG(MDeleteRetry);
	if(AllowSkip)
	{
		Msgs[LineCount-3] = MSG(MDeleteSkip);
		Msgs[LineCount-2] = MSG(MDeleteFileSkipAll);
	}
	Msgs[LineCount-1] = MSG(MDeleteCancel);

	int Result = -1;
	for(;;)
	{
		Result = Message(MSG_WARNING|MSG_ERRORTYPE, ButtonCount, MSG(Title), Msgs);

		if(SwitchBtn)
		{
			if(Result == 0)
			{
				HWND Wnd = nullptr;
				if (pfiu && SUCCEEDED(pfiu->GetSwitchToHWND(&Wnd)))
				{
					SetForegroundWindow(Wnd);
					if (IsIconic(Wnd))
						ShowWindow(Wnd, SW_RESTORE);
				}
				continue;
			}
			else if(Result > 0)
			{
				--Result;
			}
		}

		if(CloseBtn && Result == 0)
		{
			// close & retry
			if (pfiu)
			{
				pfiu->CloseFile();
			}
		}
		break;
	}

	if (pfiu)
	{
		pfiu->Release();
	}

	return Result;
}
