/*
scrobj.cpp

Parent class ��� ���� screen objects
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

#include "scrobj.hpp"
#include "savescr.hpp"
#include "interf.hpp"

SimpleScreenObject::SimpleScreenObject():
	pOwner(),
	nLockCount(),
	m_X1(),
	m_Y1(),
	m_X2(),
	m_Y2()
{
}

SimpleScreenObject::~SimpleScreenObject()
{
}

void SimpleScreenObject::Lock()
{
	nLockCount++;
}

void SimpleScreenObject::Unlock()
{
	if (nLockCount > 0)
		nLockCount--;
	else
		nLockCount = 0;
}

bool SimpleScreenObject::Locked()
{
	return (nLockCount > 0) || (pOwner?pOwner->Locked():false);
}

void SimpleScreenObject::SetPosition(int X1,int Y1,int X2,int Y2)
{
	m_X1 = X1;
	m_Y1 = Y1;
	m_X2 = X2;
	m_Y2 = Y2;
	m_Flags.Set(FSCROBJ_SETPOSITIONDONE);
}

void SimpleScreenObject::SetScreenPosition()
{
	m_Flags.Clear(FSCROBJ_SETPOSITIONDONE);
}


void SimpleScreenObject::GetPosition(int& X1, int& Y1, int& X2, int& Y2) const
{
	X1 = m_X1;
	Y1 = m_Y1;
	X2 = m_X2;
	Y2 = m_Y2;
}

void SimpleScreenObject::Hide()
{
	m_Flags.Clear(FSCROBJ_VISIBLE);
}

void SimpleScreenObject::Show()
{
	if (Locked())
		return;

	if (!m_Flags.Check(FSCROBJ_SETPOSITIONDONE))
		return;

	m_Flags.Set(FSCROBJ_VISIBLE);

	DisplayObject();
}

void SimpleScreenObject::Redraw()
{
	if (m_Flags.Check(FSCROBJ_VISIBLE))
		Show();
}

ScreenObject::ScreenObject()
{
}

ScreenObject::~ScreenObject()
{
	if (!m_Flags.Check(FSCROBJ_ENABLERESTORESCREEN))
	{
		if (SaveScr)
			SaveScr->Discard();
	}
}

void ScreenObject::Show()
{
	if (Locked())
		return;

	if (!m_Flags.Check(FSCROBJ_SETPOSITIONDONE))
		return;

	if (!IsVisible())
	{
		if (m_Flags.Check(FSCROBJ_ENABLERESTORESCREEN) && !SaveScr)
			SaveScr = std::make_unique<SaveScreen>(m_X1, m_Y1, m_X2, m_Y2);
	}

	SimpleScreenObject::Show();
}
void ScreenObject::Hide()
{
	SimpleScreenObject::Hide();
	SaveScr.reset();
}

void ScreenObject::HideButKeepSaveScreen()
{
	SimpleScreenObject::Hide();
}

void ScreenObject::SetPosition(int X1, int Y1, int X2, int Y2)
{
	/* $ 13.04.2002 KM
	- ��� ������ ������� ������� �� ������, �� �����
	����� ���� ����������� ����������� ��� ��� ���
	�������������� �������������� ����� ������������
	����������� � ����� �����.
	*/
	SaveScr.reset();

	SimpleScreenObject::SetPosition(X1, Y1, X2, Y2);
}


ScreenObjectWithShadow::ScreenObjectWithShadow()
{
}

ScreenObjectWithShadow::~ScreenObjectWithShadow()
{
	if (!m_Flags.Check(FSCROBJ_ENABLERESTORESCREEN))
	{
		if (ShadowSaveScr)
			ShadowSaveScr->Discard();
	}
}

void ScreenObjectWithShadow::Hide()
{
	if (!m_Flags.Check(FSCROBJ_VISIBLE))
		return;

	ShadowSaveScr.reset();

	ScreenObject::Hide();
}

void ScreenObjectWithShadow::Shadow(bool Full)
{
	if (m_Flags.Check(FSCROBJ_VISIBLE))
	{
		if(Full)
		{
			if (!ShadowSaveScr)
				ShadowSaveScr = std::make_unique<SaveScreen>(0,0,ScrX,ScrY);

			MakeShadow(0,0,ScrX,ScrY);
		}
		else
		{
			if (!ShadowSaveScr)
				ShadowSaveScr = std::make_unique<SaveScreen>(m_X1, m_Y1, m_X2 + 2, m_Y2 + 1);

			MakeShadow(m_X1+2,m_Y2+1,m_X2+1,m_Y2+1);
			MakeShadow(m_X2+1,m_Y1+1,m_X2+2,m_Y2+1);
		}
	}
}
