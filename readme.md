## Far 3 bis ##

Far3bis is the patch for official Far Manager 3.0 latest builds.
This build has new API with Lua Macro language.

## Download binaries ##

[http://sourceforge.net/projects/conemu/files/FarManager/Far3bis/](http://sourceforge.net/projects/conemu/files/FarManager/Far3bis/ "7z, far.exe only")

## List of patches ##

- Far3bis. Run-Time Check Failure #1 - A cast to a smaller data type has caused a loss of data
- Far3bis. m_LastBottomFile fixup
- Far3bis. Multiline panel status line
- Far3bis. Temp commit. Remove '_ASSERTE(OpenFrom == OPEN_FROMMACRO)'
- Far3bis. Temp commit. debugger bitmask exceptions
- Far3bis. Support narrow display in viewer Search dialog
- Far3bis. Debug build. Asserts in viewer
- Far3bis. Show all Fn on KeyBar even on narrow display (touch-panels)
- Far3bis. Support narrow display in MkDir dialog
- Far3bis. PlugAPI. PPIF_HASNOTEXTENSION - say 'I has no extensions on panel'
- Far3bis. Debug build. skip force load of plugin LangData if it was not loaded yet
- Far3bis. FCTL_GETPANELITEMINFO (coords and color)
- Far3bis. '/co' plugins loading optimization
- Far3bis. Wider plugin info dialog
- Far3bis. Debug assers in plugins.cpp
- Far3bis. IsProcessAssignMacroKey (compilation fix)
- Far3bis. Restrict ProcessConsoleInput in IsProcessAssignMacroKey
- Far3bis. Debug assers and PlCacheCfgEnum in plugin loading
- Far3bis. Show diagnostic on plugin loading fail
- Far3bis. Compilation fixes
- Far3bis. PluginManager::IsPluginValid for debugging
- Far3bis. PluginManager::GetPluginVersion for ext plugin menu
- Far3bis. Detect Far3wrapper, '2' in menus, FPF_FAR2 in API
- Far3Bis. SetCurrentDirectory/c_str (compilation fix)
- Far3Bis. ECTL_DROPMODIFEDFLAG (compilation fix)
- Far3Bis. IsPluginValid (compilation fix)
- Far3bis. Debug. OutputDebugString in apiSetCurrentDirectory
- Far3bis. Debug assert STATUS_DATATYPE_MISALIGNMENT
- Far3bis. Debug asserts in synchro.cpp
- Far3bis. Don't allow ACTL_SYNCHO before init finished
- Far3bis. Support narrow display in SearchReplace dialog
- Far3bis. Executor. Suggest 'openas' when 'bad' path detected
- Far3bis. Editor. Seamless softlinebreaks, ECTL_DROPMODIFEDFLAG
- Far3bis. Editor,whitespaces. Show CRLF as one 'Para' symbol
- Far3bis. Show 'owner' from plugin panel in SetAttr dlg
- Far3bis. Support narrow display in FindFile dialog
- Far3bis. Debug asserts in interf.cpp
- Far3bis. Debug. Frame Manager check in apiPanelControl
- Far3bis. Debug. Plugin pointer check in apiGetMsgFn
- Far3bis. Return VS_BIS in ACTL_GETFARMANAGERVERSION
- Far3bis. Try to catch bad plugin calls in DEBUG build
- Far3bis. Show 'bis' after build no
- Far3bis. Debug global var gnMainThreadId initialized in main()
- Far3bis. ShellCopy::ShellCopy compilation fixup
- Far3bis. Support narrow display in Copy dialog
- Far3bis. Support narrow display in Masks dialog
- Far3bis. Debugging asserts in history.cpp
- Far3bis. Call NetworkBis for logon dlg when history goto fails
- Far3bis. DizList. Don't try elevation for SetHidden (compilation fixup)
- Far3bis. DizList. Some todo comments
- Far3bis. DizList. Don't try elevation for SetHidden
- Far3bis. DEBUG_PEEK_INPUT
- Far3bis. headers.hpp. Debug assertions (remove cl warning)
- Far3bis. CreatePath. Some todo comments
- Far3bis. RemoveToRecycleBin is used in ExtPluginMenu
- Far3bis. VS_BIS in version, "bis" in description
- Far3bis. headers.hpp. Debug assertions
- Far3bis. Editor. Some todo comments
- Far3bis. Editor. Restore strOldCurDir after save
- Far3bis. Editor. Remove excess save confirm if ShiftF10 already cancelled
- Far3bis. Editor. Support narrow display in dlgOpenEditor
- TmpPanelBis. Remove PF_PRELOAD flag
- NetworkBis. Add bis_changelog
- NetworkBis. Export to Far ability to logon
- NetworkBis. Disconnect servers on plugin exit
- NetworkBis. Allow retry login/password in GotoComputer
- NetworkBis. Debug GetLastError check
- NetworkBis. Allow NetBrowser::GetFindData with NULL args
- NetworkBis. Some todo comments
- NetworkBis. Pointer checks
- NetworkBis. There are currently no logon servers...
- NetworkBis. Check logon errors with IsLogonInvalid macro
- NetworkBis. Don't call FarApi if we already in GetFindData
