#pragma once

/*
filetype.hpp

������ � ������������ ������
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

// ������ � ������������ ������
enum FILETYPE_MODE
{
	FILETYPE_EXEC,       // Enter
	FILETYPE_ALTEXEC,    // Ctrl-PgDn
	FILETYPE_VIEW,       // F3
	FILETYPE_ALTVIEW,    // Alt-F3
	FILETYPE_EDIT,       // F4
	FILETYPE_ALTEDIT     // Alt-F4
};

void ProcessGlobalFileTypes(const string& Name, bool AlwaysWaitFinish, bool RunAs,bool FromPanel=false);
bool ProcessLocalFileTypes(const string& Name, const string& ShortName, FILETYPE_MODE Mode, bool AlwaysWaitFinish);
void ProcessExternal(const string& Command, const string& Name, const string& ShortName, bool AlwaysWaitFinish);
bool ExtractIfExistCommand(string &strCommandText);
void EditFileTypes();
