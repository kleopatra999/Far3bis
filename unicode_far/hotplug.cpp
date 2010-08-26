/*
hotplug.cpp

���������� Hotplug-���������
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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

#include "hotplug.hpp"
#include "lang.hpp"
#include "language.hpp"
#include "keys.hpp"
#include "help.hpp"
#include "vmenu.hpp"
#include "imports.hpp"
#include "message.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "interf.hpp"
#include "window.hpp"

struct DeviceInfo
{
	DEVINST hDevInst; // device instance
	DWORD dwDriveMask; // mask of associated drives
};

static int  GetHotplugDevicesInfo(DeviceInfo **pInfo);
static void FreeHotplugDevicesInfo(DeviceInfo *pInfo);
static bool GetDeviceProperty(DEVINST hDevInst, int nProperty, string &strValue, bool bSearchChild);
static int __RemoveHotplugDevice(DEVINST hDevInst);
static int RemoveHotplugDevice(DEVINST hDevInst,DWORD DevMasks,DWORD Flags);
static DeviceInfo *EnumHotPlugDevice(LPARAM lParam);


DeviceInfo *EnumHotPlugDevice(LPARAM lParam)
{
	VMenu *HotPlugList=(VMenu *)lParam;
	DeviceInfo *pInfo=nullptr;
	int nCount = GetHotplugDevicesInfo(&pInfo);

	if (nCount)
	{
		MenuItemEx ListItem;

		for (int I = 0; I < nCount; I++)
		{
			string strFriendlyName;
			string strDescription;
			DEVINST hDevInst=pInfo[I].hDevInst;
			ListItem.Clear();

			if (GetDeviceProperty(hDevInst,CM_DRP_DEVICEDESC,strDescription,true))
			{
				if (!strDescription.IsEmpty())
				{
					RemoveExternalSpaces(strDescription);
					ListItem.strName = strDescription;
				}
			}

			if (GetDeviceProperty(hDevInst,CM_DRP_FRIENDLYNAME,strFriendlyName,true))
			{
				RemoveExternalSpaces(strFriendlyName);

				if (!strDescription.IsEmpty())
				{
					if (!strFriendlyName.IsEmpty() && StrCmpI(strDescription,strFriendlyName))
					{
						ListItem.strName += L" \"" + strFriendlyName + L"\"";
					}
				}
				else if (!strFriendlyName.IsEmpty())
				{
					ListItem.strName = strFriendlyName;
				}
			}

			if (!ListItem.strName.IsEmpty())
				HotPlugList->SetUserData((void*)(INT_PTR)I,sizeof(I),HotPlugList->AddItem(&ListItem));
		}
	}

	return pInfo;
}

static void RefreshHotplugMenu(DeviceInfo*& pInfo,VMenu& HotPlugList)
{
	if (pInfo) FreeHotplugDevicesInfo(pInfo);

	pInfo=nullptr;
	HotPlugList.Hide();
	HotPlugList.DeleteItems();
	HotPlugList.SetPosition(-1,-1,0,0);
	pInfo=EnumHotPlugDevice((LPARAM)&HotPlugList);
	HotPlugList.Show();
}

void ShowHotplugDevice()
{
	if (!ifn.bSetupAPIFunctions)
	{
		SetLastError(ERROR_INVALID_FUNCTION);
		return;
	}

	Events.DeviceArivalEvent.Reset();
	Events.DeviceRemoveEvent.Reset();

	DeviceInfo *pInfo=nullptr;
	VMenu HotPlugList(MSG(MHotPlugListTitle),nullptr,0,ScrY-4);
	HotPlugList.SetFlags(VMENU_WRAPMODE|VMENU_AUTOHIGHLIGHT);
	HotPlugList.SetPosition(-1,-1,0,0);
	pInfo=EnumHotPlugDevice((LPARAM)&HotPlugList);
	HotPlugList.AssignHighlights(TRUE);
	HotPlugList.SetBottomTitle(MSG(MHotPlugListBottom));

	HotPlugList.Show();

	while (!HotPlugList.Done())
	{
		int Key=Events.DeviceArivalEvent.Signaled() || Events.DeviceRemoveEvent.Signaled()?KEY_CTRLR:HotPlugList.ReadInput();
		switch (Key)
		{
			case KEY_F1:
			{
				Help Hlp(L"HotPlugList");
				break;
			}
			case KEY_CTRLR:
			{
				RefreshHotplugMenu(pInfo,HotPlugList);
				break;
			}
			case KEY_NUMDEL:
			case KEY_DEL:
			{
				if (HotPlugList.GetItemCount() > 0)
				{
					int bResult;
					int I=(int)(INT_PTR)HotPlugList.GetUserData(nullptr,0);

					if ((bResult=RemoveHotplugDevice(pInfo[I].hDevInst,pInfo[I].dwDriveMask,EJECT_NOTIFY_AFTERREMOVE)) == 1)
					{
						HotPlugList.Hide();

						if (pInfo)
							FreeHotplugDevicesInfo(pInfo);

						pInfo=nullptr;
						RefreshHotplugMenu(pInfo,HotPlugList);
					}
					else if (bResult != -1)
					{
						SetLastError(ERROR_DRIVE_LOCKED); // ...� "The disk is in use or locked by another process."
						Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),
						        MSG(MChangeCouldNotEjectHotPlugMedia2),HotPlugList.GetItemPtr(I)->strName,MSG(MOk));
					}
				}

				break;
			}
			default:
				HotPlugList.ProcessInput();
				break;
		}
	}

	if (pInfo)
		FreeHotplugDevicesInfo(pInfo);
}


int ProcessRemoveHotplugDevice(wchar_t Drive, DWORD Flags)
{
	int bResult = -1;
	DeviceInfo *pInfo;
	DWORD dwDriveMask = (1 << (Upper(Drive)-L'A'));
	DWORD SavedLastError=ERROR_SUCCESS;

	if (!ifn.bSetupAPIFunctions)
	{
		SetLastError(ERROR_INVALID_FUNCTION);
		return -1; //???
	}

	int nCount = GetHotplugDevicesInfo(&pInfo);

	if (nCount)
	{
		for (int i = 0; i < nCount; i++)
		{
			if (pInfo[i].dwDriveMask & dwDriveMask)
			{
				bResult=RemoveHotplugDevice(pInfo[i].hDevInst,pInfo[i].dwDriveMask,Flags);
				// ??? break; ???
			}
		}

		FreeHotplugDevicesInfo(pInfo);
		pInfo=nullptr;
	}

	SetLastError(SavedLastError);
	return bResult;
}



/**+
A device is considered a HotPlug device if the following are TRUE:
- does NOT have problem CM_PROB_DEVICE_NOT_THERE
- does NOT have problem CM_PROB_HELD_FOR_EJECT
- does NOT have problem CM_PROB_DISABLED
- has Capability CM_DEVCAP_REMOVABLE
- does NOT have Capability CM_DEVCAP_SURPRISEREMOVALOK
- does NOT have Capability CM_DEVCAP_DOCKDEVICE

Returns:
TRUE if this is a HotPlug device
FALSE if this is not a HotPlug device.
-**/

bool CheckChild(DEVINST hDevInst)
{
	bool Result=false;
	DEVINST hDevChild;

	if (ifn.pfnGetChild(&hDevChild,hDevInst,0)==CR_SUCCESS)
	{
		DWORD Capabilities;
		ULONG Length=sizeof(Capabilities);

		if (ifn.pfnGetDevNodeRegistryProperty(hDevChild,CM_DRP_CAPABILITIES,nullptr,&Capabilities,&Length,0)==CR_SUCCESS)
		{
			Result=!(Capabilities&CM_DEVCAP_SURPRISEREMOVALOK)&&(Capabilities&CM_DEVCAP_UNIQUEID);
		}
	}

	return Result;
}

BOOL IsHotPlugDevice(DEVINST hDevInst)
{
	DWORD Capabilities;
	DWORD Len;
	DWORD Status,Problem;
	Capabilities = 0;
	Status = 0;
	Problem = 0;
	Len = sizeof(Capabilities);

	if (ifn.pfnGetDevNodeRegistryProperty(
	            hDevInst,
	            CM_DRP_CAPABILITIES,
	            nullptr,
	            (PVOID)&Capabilities,
	            &Len,
	            0
	        ) == CR_SUCCESS)
	{
		if (ifn.pfnGetDevNodeStatus(
		            &Status,
		            &Problem,
		            hDevInst,
		            0
		        ) == CR_SUCCESS)
		{
			if ((Problem != CM_PROB_DEVICE_NOT_THERE) &&
			        (Problem != CM_PROB_HELD_FOR_EJECT) && //��������, ���� ��������� �� ������� ������� ������
			        (Problem != CM_PROB_DISABLED) &&
			        (Capabilities & CM_DEVCAP_REMOVABLE) &&
			        (!(Capabilities & CM_DEVCAP_SURPRISEREMOVALOK) || CheckChild(hDevInst)) &&
			        !(Capabilities & CM_DEVCAP_DOCKDEVICE))
				return TRUE;
		}
	}

	return FALSE;
}


DWORD DriveMaskFromVolumeName(const wchar_t *lpwszVolumeName)
{
	string strCurrentVolumeName;
	wchar_t wszMountPoint[]=L"\\\\?\\A:\\";

	for (wchar_t Letter = L'A'; Letter <= L'Z'; Letter++)
	{
		wszMountPoint[4] = Letter;
		if(apiGetVolumeNameForVolumeMountPoint(wszMountPoint,strCurrentVolumeName) && strCurrentVolumeName.Equal(0,lpwszVolumeName))
			return (1 << (Letter-L'A'));
	}

	return 0;
}

DWORD GetDriveMaskFromMountPoints(DEVINST hDevInst)
{
	DWORD dwMask = 0;
	wchar_t szDeviceID[MAX_DEVICE_ID_LEN];

	if (ifn.pfnGetDeviceID(
	            hDevInst,
	            szDeviceID,
	            ARRAYSIZE(szDeviceID),
	            0
	        ) == CR_SUCCESS)
	{
		DWORD dwSize = 0;


		GUID GUID_DEVINTERFACE_VOLUME = {0x53f5630dL,0xb6bf,0x11d0,0x94,0xf2,0x00,0xa0,0xc9,0x1e,0xfb,0x8b};

		if (ifn.pfnGetDeviceInterfaceListSize(
		            &dwSize,
		            &GUID_DEVINTERFACE_VOLUME,
		            szDeviceID,
		            0
		        ) == CR_SUCCESS)
		{
			if (dwSize > 1)
			{
				wchar_t *lpwszDeviceInterfaceList = (wchar_t*)xf_malloc(dwSize*sizeof(wchar_t));

				if (ifn.pfnGetDeviceInterfaceList(
				            &GUID_DEVINTERFACE_VOLUME,
				            szDeviceID,
				            lpwszDeviceInterfaceList,
				            dwSize,
				            0
				        ) == CR_SUCCESS)
				{
					wchar_t *p = lpwszDeviceInterfaceList;

					while (*p)
					{
						string strMountPoint(p);
						size_t Length = strMountPoint.GetLength();
						AddEndSlash(strMountPoint);
						string strVolumeName;
						if (apiGetVolumeNameForVolumeMountPoint(strMountPoint,strVolumeName))
						{
								dwMask |= DriveMaskFromVolumeName(strVolumeName);
						}
						p += Length + 1;
					}
				}

				xf_free(lpwszDeviceInterfaceList);
			}
		}
	}

	return dwMask;
}

DWORD GetRelationDrivesMask(DEVINST hDevInst)
{
	DWORD dwMask = 0;
	DEVINST hRelationDevInst;
	wchar_t szDeviceID [MAX_DEVICE_ID_LEN];

	if (ifn.pfnGetDeviceID(
	            hDevInst,
	            szDeviceID,
	            sizeof(szDeviceID)/sizeof(wchar_t),
	            0
	        ) == CR_SUCCESS)
	{
		DWORD dwSize = 0;

		if (ifn.pfnGetDeviceIDListSize(
		            &dwSize,
		            szDeviceID,
		            CM_GETIDLIST_FILTER_REMOVALRELATIONS
		        ) == CR_SUCCESS)
		{
			if (dwSize)
			{
				wchar_t *lpDeviceIdList = (wchar_t*)xf_malloc(dwSize*sizeof(wchar_t));

				if (ifn.pfnGetDeviceIDList(
				            szDeviceID,
				            lpDeviceIdList,
				            dwSize,
				            CM_GETIDLIST_FILTER_REMOVALRELATIONS
				        ) == CR_SUCCESS)
				{
					wchar_t *p = lpDeviceIdList;

					while (*p)
					{
						if (ifn.pfnLocateDevNode(
						            &hRelationDevInst,
						            p,
						            0
						        ) == CR_SUCCESS)
							dwMask = GetDriveMaskFromMountPoints(hRelationDevInst);

						p += wcslen(p)+1;
					}
				}

				xf_free(lpDeviceIdList);
			}
		}
	}

	return dwMask;
}

DWORD GetDriveMaskForDeviceInternal(DEVINST hDevInst)
{
	DWORD dwMask = 0;
	DEVINST hDevChild;

	do
	{
		if (!IsHotPlugDevice(hDevInst))
		{
			dwMask |= GetDriveMaskFromMountPoints(hDevInst);
			dwMask |= GetRelationDrivesMask(hDevInst);

			if (ifn.pfnGetChild(
			            &hDevChild,
			            hDevInst,
			            0
			        ) == CR_SUCCESS)
				dwMask |= GetDriveMaskForDeviceInternal(hDevChild);
		}
	}
	while (ifn.pfnGetSibling(&hDevInst, hDevInst, 0) == CR_SUCCESS);

	return dwMask;
}


DWORD GetDriveMaskForDevice(DEVINST hDevInst)
{
	DWORD dwMask = 0;
	DEVINST hDevChild;
	dwMask |= GetDriveMaskFromMountPoints(hDevInst);
	dwMask |= GetRelationDrivesMask(hDevInst);

	if (ifn.pfnGetChild(
	            &hDevChild,
	            hDevInst,
	            0
	        ) == CR_SUCCESS)
		dwMask |= GetDriveMaskForDeviceInternal(hDevChild);

	return dwMask;
}

int GetHotplugDriveDeviceInfoInternal(DEVINST hDevInst, DeviceInfo **pInfo, int nCount)
{
	DEVINST hDevChild;

	do
	{
		if (IsHotPlugDevice(hDevInst))
		{
			nCount++;
			*pInfo = (DeviceInfo*)xf_realloc(*pInfo, nCount*sizeof(DeviceInfo));
			DeviceInfo *pItem = &(*pInfo)[nCount-1];
			pItem->dwDriveMask = GetDriveMaskForDevice(hDevInst);
			pItem->hDevInst = hDevInst;
		}

		if (ifn.pfnGetChild(
		            &hDevChild,
		            hDevInst,
		            0
		        ) == CR_SUCCESS)
			nCount = GetHotplugDriveDeviceInfoInternal(hDevChild, pInfo, nCount);
	}
	while (ifn.pfnGetSibling(&hDevInst, hDevInst, 0) == CR_SUCCESS);

	return nCount;
}

int GetHotplugDevicesInfo(DeviceInfo **pInfo)
{
	if (pInfo)
	{
		*pInfo = nullptr;

		if (ifn.bSetupAPIFunctions)
		{
			DEVNODE hDevRoot;

			if (ifn.pfnLocateDevNode(
			            &hDevRoot,
			            nullptr,
			            CM_LOCATE_DEVNODE_NORMAL
			        ) == CR_SUCCESS)
			{
				DEVINST hDevChild;

				if (ifn.pfnGetChild(
				            &hDevChild,
				            hDevRoot,
				            0
				        ) == CR_SUCCESS)
					return GetHotplugDriveDeviceInfoInternal(hDevChild, pInfo, 0);
			}
		}
	}

	return 0;
}

void FreeHotplugDevicesInfo(DeviceInfo *pInfo)
{
	xf_free(pInfo);
}

bool GetDeviceProperty(
    DEVINST hDevInst,
    int nProperty,
    string &strValue,
    bool bSearchChild
)
{
	bool bResult = false;

	if (ifn.bSetupAPIFunctions)
	{
		DWORD dwSize = 0;
		CONFIGRET crResult;
		crResult = ifn.pfnGetDevNodeRegistryProperty(
		               hDevInst,
		               nProperty,//CM_DRP_FRIENDLYNAME,
		               nullptr,
		               nullptr,
		               &dwSize,
		               0
		           );

		if (!dwSize)
		{
			if (bSearchChild)
			{
				DEVINST hDevChild;

				if (ifn.pfnGetChild(&hDevChild, hDevInst, 0) == CR_SUCCESS)
					bResult = GetDeviceProperty(hDevChild, nProperty, strValue, bSearchChild);
			}
		}
		else
		{
			wchar_t *lpwszBuffer = strValue.GetBuffer(dwSize+1);
			crResult = ifn.pfnGetDevNodeRegistryProperty(
			               hDevInst,
			               nProperty,//CM_DRP_FRIENDLYNAME,
			               nullptr,
			               lpwszBuffer,
			               &dwSize,
			               0
			           );
			strValue.ReleaseBuffer();
			bResult = (crResult == ERROR_SUCCESS);
		}
	}

	return bResult;
}


int __RemoveHotplugDevice(DEVINST hDevInst)
{
	PNP_VETO_TYPE pvtVeto = PNP_VetoTypeUnknown;
	CONFIGRET crResult;
	wchar_t wszDescription[MAX_PATH]={0}; //BUGBUG
	crResult = ifn.pfnRequestDeviceEject(
	               hDevInst,
	               &pvtVeto,
	               (wchar_t*)&wszDescription,
	               MAX_PATH,
	               0
	           );

	if ((crResult != CR_SUCCESS) || (pvtVeto != PNP_VetoTypeUnknown))   //M$ ���, ���� ���� szDecsription, �� ���� ��� ������ ������������ CR_SUCCESS
	{
		SetLastError((pvtVeto != PNP_VetoTypeUnknown)?ERROR_DRIVE_LOCKED:ERROR_UNABLE_TO_UNLOAD_MEDIA); // ...� "The disk is in use or locked by another process."
		return 0;
	}

	SetLastError(ERROR_SUCCESS);
	return 1;
}

int RemoveHotplugDevice(DEVINST hDevInst,DWORD dwDriveMask,DWORD Flags)
{
	int bResult = -1; // ����� �������� -1, �����, �� ������� HDD �������� Shift-Del ��������, ��� ��� ����� �������
	string strFriendlyName;
	string strDescription;
	GetDeviceProperty(hDevInst,CM_DRP_FRIENDLYNAME,strFriendlyName,true);
	RemoveExternalSpaces(strFriendlyName);
	GetDeviceProperty(hDevInst,CM_DRP_DEVICEDESC,strDescription,true);
	RemoveExternalSpaces(strDescription);
	int DoneEject=0;

	if (!(Flags&EJECT_NO_MESSAGE) && Opt.Confirm.RemoveHotPlug)
	{
		string strDiskMsg;
		wchar_t Disks[256], *pDisk=Disks;
		*pDisk=0;

		for (int Drive='A'; Drive <= 'Z'; ++Drive)
		{
			if (dwDriveMask & (1 << (Drive-'A')))
			{
				*pDisk++=(wchar_t)Drive;
				*pDisk++=L':';
				*pDisk++=L',';
			}
		}

		*pDisk=0;

		if (pDisk != Disks)
			*--pDisk=0;

		if (*Disks)
			strDiskMsg.Format(MSG(MHotPlugDisks),Disks);

		if (StrCmpI(strDescription,strFriendlyName) && !strFriendlyName.IsEmpty())
		{
			if (!strDiskMsg.IsEmpty())
				DoneEject=Message(MSG_WARNING,2,MSG(MChangeHotPlugDisconnectDriveTitle),MSG(MChangeHotPlugDisconnectDriveQuestion),strDescription,strFriendlyName,strDiskMsg,MSG(MHRemove),MSG(MHCancel));
			else
				DoneEject=Message(MSG_WARNING,2,MSG(MChangeHotPlugDisconnectDriveTitle),MSG(MChangeHotPlugDisconnectDriveQuestion),strDescription,strFriendlyName,MSG(MHRemove),MSG(MHCancel));
		}
		else
		{
			if (!strDiskMsg.IsEmpty())
				DoneEject=Message(MSG_WARNING,2,MSG(MChangeHotPlugDisconnectDriveTitle),MSG(MChangeHotPlugDisconnectDriveQuestion),strDescription,strDiskMsg,MSG(MHRemove),MSG(MHCancel));
			else
				DoneEject=Message(MSG_WARNING,2,MSG(MChangeHotPlugDisconnectDriveTitle),MSG(MChangeHotPlugDisconnectDriveQuestion),strDescription,MSG(MHRemove),MSG(MHCancel));
		}
	}

	if (!DoneEject)
		bResult = __RemoveHotplugDevice(hDevInst);
	else
		bResult = -1;

	if (bResult == 1 && (Flags&EJECT_NOTIFY_AFTERREMOVE))
	{
		Message(0,1,MSG(MChangeHotPlugDisconnectDriveTitle),MSG(MChangeHotPlugNotify1),strDescription,strFriendlyName,MSG(MChangeHotPlugNotify2),MSG(MOk));
	}

	return bResult;
}
