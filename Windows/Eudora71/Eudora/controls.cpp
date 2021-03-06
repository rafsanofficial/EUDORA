/////////////////////////////////////////////////////////////////////////////
// controls.cpp
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//

#include "stdafx.h"

#include "rs.h"
#include "utils.h"
#include "eudora.h"
#include "mainfrm.h"
#include "newmbox.h"
#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"
#include "QCStationeryDirector.h"
#include "QCLabelDirector.h"
#include "QCImapMailboxCommand.h"

#include "controls.h"


#include "DebugNewHelpers.h"


extern QCCommandStack		g_theCommandStack;
extern QCMailboxDirector	g_theMailboxDirector;

/////////////////////////////////////////////////////////////////////////////
void EnableAllItems(CMenu* menu)
{
	for (int i = 0; (UINT)i < menu->GetMenuItemCount(); i++)
	{
		if (menu->GetMenuItemID(i) == -1)
			EnableAllItems(menu->GetSubMenu(i) );
		else
			menu->EnableMenuItem( i, MF_BYPOSITION | MF_ENABLED );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CLabelButton

CLabelButton::CLabelButton()
{
	m_Label = 0;
}

CLabelButton::~CLabelButton()
{
}

void CLabelButton::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	CRect rect(lpDIS->rcItem);
	
	// Draw focus rect
	if ((lpDIS->itemAction & ODA_FOCUS) == ODA_FOCUS)
	{
		pDC->DrawFocusRect(&rect);
		return;
	}
	
	// The control doesn't change its look for selection
	if (!(lpDIS->itemAction & ODA_DRAWENTIRE))
		return;

	// Background color is the selected color with a black frame
	COLORREF clr;
	if (m_Label)
		ASSERT(QCLabelDirector::GetLabelDirector()->GetLabel(m_Label, &clr));
	else
		clr = GetSysColor(COLOR_WINDOW);

	CBrush Brush(clr);
	
	pDC->SelectObject(&Brush);
	pDC->SelectStockObject(BLACK_PEN);
	pDC->Rectangle(&rect);
	pDC->SelectStockObject(BLACK_BRUSH);
	
	// Text color is window background color
	if (m_Label)
	{
		LPCSTR Text = QCLabelDirector::GetLabelDirector()->GetLabelText(m_Label);
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(GetSysColor(COLOR_WINDOW));
		pDC->DrawText((LPCSTR)Text, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
	}

	// Draw focus rect
	if (lpDIS->itemState & ODS_FOCUS)
		pDC->DrawFocusRect(&rect);
}

//BEGIN_MESSAGE_MAP(CLabelButton, CButton)
//	//{{AFX_MSG_MAP(CLabelColorButton)
//		// NOTE - the ClassWizard will add and remove mapping macros here.
//	//}}AFX_MSG_MAP
//END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLabelButton message handlers

/////////////////////////////////////////////////////////////////////////////
// CLabelMenu

CLabelMenu::CLabelMenu()
{
//	LoadMenu(IDR_CONTEXT_POPUPS);
}

CLabelMenu::~CLabelMenu()
{
}

void CLabelMenu::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
	UINT MenuID = lpMIS->itemID;

	// Is This A Label
	if ((MenuID >= ID_MESSAGE_LABEL_1) && (MenuID <= ID_MESSAGE_LABEL_7))
	{
		UINT nLabelIdx = (lpMIS->itemID - ID_MESSAGE_LABEL_1) + 1;
		CString Text;
		VERIFY(QCLabelDirector::GetLabelDirector()->GetLabel(nLabelIdx, &Text));
		
		CDC theDC;
		
		theDC.CreateCompatibleDC( NULL );
		
		if (Text.IsEmpty())
			Text = "dummy";
		
		CSize size(theDC.GetTextExtent(Text, ::SafeStrlenMT(Text)));

		lpMIS->itemWidth = size.cx + LOWORD(GetMenuCheckMarkDimensions());
		lpMIS->itemHeight = size.cy + 2;
	}

}

void CLabelMenu::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	UINT MenuID = lpDIS->itemID & 0xFFFF;
	
	// Is This A Label
	if ((MenuID >= ID_MESSAGE_LABEL_1) && (MenuID <= ID_MESSAGE_LABEL_7))
	{
		CDC* pDC = CDC::FromHandle(lpDIS->hDC);
		UINT nLabelIdx = (MenuID - ID_MESSAGE_LABEL_1) + 1;

		BOOL Grayed = (lpDIS->itemState & ODS_GRAYED) == ODS_GRAYED;
		CRect rect(lpDIS->rcItem);
		COLORREF BackColor, ForeColor;

		if (lpDIS->itemState & ODS_SELECTED)
		{
			BackColor = Grayed? GetSysColor(COLOR_HIGHLIGHT) : (QCLabelDirector::GetLabelDirector()->GetLabelColor(nLabelIdx)); //GetIniLong(IniColorID);
			ForeColor = Grayed? GetSysColor(COLOR_GRAYTEXT) : GetSysColor(COLOR_MENU);
		}
		else
		{
			BackColor = GetSysColor(COLOR_MENU);
			ForeColor = Grayed? GetSysColor(COLOR_GRAYTEXT) : (QCLabelDirector::GetLabelDirector()->GetLabelColor(nLabelIdx)); //GetIniLong(IniColorID);
		}
		
		// Draw background
		CBrush BackBrush(BackColor);
		pDC->FillRect(&rect, &BackBrush);
		
		// Set text colors
		pDC->SetTextColor(ForeColor);
		pDC->SetBkMode(TRANSPARENT);
		
		rect.left += LOWORD(GetMenuCheckMarkDimensions()) * 3 / 2;

		LPCSTR pText = (QCLabelDirector::GetLabelDirector()->GetLabelText(nLabelIdx));
		if (IsVersion4())
			pDC->DrawState(rect.TopLeft(), rect.Size(), pText,
				DST_PREFIXTEXT | (Grayed? DSS_DISABLED : DSS_NORMAL), TRUE, 0, (HBRUSH)NULL);
		else
			pDC->DrawText(pText, -1, &rect, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
	}
}

//BEGIN_MESSAGE_MAP(CLabelMenu, CMenu)
//	//{{AFX_MSG_MAP(CLabelMenu)
//		// NOTE - the ClassWizard will add and remove mapping macros here.
//	//}}AFX_MSG_MAP
//END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLabelMenu message handlers

/////////////////////////////////////////////////////////////////////////////
// CMyBitmapButton

CMyBitmapButton::CMyBitmapButton()
{
}

CMyBitmapButton::~CMyBitmapButton()
{
}


BEGIN_MESSAGE_MAP(CMyBitmapButton, CBitmapButton)
	//{{AFX_MSG_MAP(CMyBitmapButton)
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyBitmapButton message handlers

LRESULT CMyBitmapButton::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class

//	if (message == BM_SETSTATE)
//		return OnSetState(wParam);

	return CBitmapButton::DefWindowProc(message, wParam, lParam);
}

void CMyBitmapButton::OnKillFocus(CWnd* pNewWnd) 
{
//	CBitmapButton::OnKillFocus(pNewWnd);
	
	// TODO: Add your message handler code here
	
}



/////////////////////////////////////////////////////////////////////////////
// CMailboxButton

CMailboxButton::CMailboxButton()
{
	// Setup some default values
	m_ButtonDesc = CRString( IDS_IN_MBOX_NAME );
	m_MailboxPath.Empty();

	// For IMAP mbox.
	m_SelectedImapName.Empty();
	m_bIsImap = FALSE;
}

CMailboxButton::~CMailboxButton()
{
}


BEGIN_MESSAGE_MAP(CMailboxButton, CButton)
	//{{AFX_MSG_MAP(CMailboxButton)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_COMMAND_EX_RANGE( QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, OnDynamicCommand )
	ON_UPDATE_COMMAND_UI_RANGE( QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, OnUpdateDynamicCommand )
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////////////
// HISTORY
// 11/4/98 (JOK) - If "pPersonalityName" is non-NULL, show menu for this personality
// subtree only.
// Used to restrict mailbox choice to an IMAP mailbox in some cases.
// END HISTORY
/////////////////////////////////////////////////////////////////////////////////////
BOOL CMailboxButton::SelectMailbox(LPCSTR pPersonalityName /* = NULL */)
{
	BOOL bModified = false;

	QCCommandObject*		pCommand;
	COMMAND_ACTION_TYPE		theAction;	
	WORD					uID;
	UINT cmdID = 0;
	QCMailboxCommand* pC = NULL;	// set to the command object for the IMAP personality passed in

	CPtrList *pList = NULL;

	if (pPersonalityName && *pPersonalityName)
	{
		// Find the mailbox command object for this personality and extract it's
		// mailbox list.
		pC = g_theMailboxDirector.FindByName(g_theMailboxDirector.GetMailboxList(), 
															pPersonalityName);

		if (pC)
			pList = &pC->GetChildList();
	}

	CMenu* theTransferMenu = CMainFrame::QCGetMainFrame()->GetTransferMenu();
	EnableAllItems(theTransferMenu);

	CRect rect;
	GetWindowRect(&rect);
	
	CMainFrame* pMainFrame = CMainFrame::QCGetMainFrame();
	pMainFrame->SetTransferMenuBeingUsed(TRUE);
	if (cmdID = theTransferMenu->TrackPopupMenu(TPM_RETURNCMD, rect.left, rect.bottom, pMainFrame))
	{
		// The top of the stack will contain which menu item was selected
		g_theCommandStack.GetSavedCommand(&uID , &pCommand, &theAction);
		ASSERT_KINDOF( QCMailboxCommand, pCommand );
		if (theAction == CA_TRANSFER_NEW)
			pCommand = OnNewMailbox(pCommand);

		if (pCommand)
		{
			bModified = true;
			m_MailboxPath = ((QCMailboxCommand* )pCommand)->GetPathname();

			switch (GetIniShort(IDS_INI_FILTER_TRANSFER_NAME)) //seanr
			{
				case 1: // Full path using "Folder \ Folder \ Mailbox" style
				{
					m_ButtonDesc = g_theMailboxDirector.BuildNamedPath( (QCMailboxCommand* )pCommand);
					break;
				}


				case 2:	// Full path using ms-dos style
				{
					m_ButtonDesc = m_MailboxPath;
					break;
				}

				case 3: // One parent deep style (per sdorner request) "Folder\Mailbox" or just "Mailbox" if root
				{				

					if (g_theMailboxDirector.FindParent((QCMailboxCommand* )pCommand))  //Make sure we have a parent
					{
						m_ButtonDesc = (g_theMailboxDirector.FindParent((QCMailboxCommand* )pCommand))->GetName();
						m_ButtonDesc += "\\";
						m_ButtonDesc += ((QCMailboxCommand* )pCommand)->GetName();
							
					}

					else // No parent?  Cool, just get the name then.
					{
						m_ButtonDesc = ((QCMailboxCommand* )pCommand)->GetName();
					}

					break;
				} 

				case 0: // Default path using GetName() or "Mailbox" style
				default:
				{
					m_ButtonDesc = ((QCMailboxCommand* )pCommand)->GetName();	
					break;
				}
			}

			SetWindowText(m_ButtonDesc);

			// If this is an IMAP mailbox, set the foll:
			//
			if ( ((QCMailboxCommand* )pCommand)->IsImapType() )
			{
				m_SelectedImapName = ((QCImapMailboxCommand *)pCommand)->GetImapName();
				m_bIsImap = TRUE;
			}
		}
	}

	pMainFrame->SetTransferMenuBeingUsed(FALSE);

	return (bModified); // We return true if the mailbox was changed
}

void CMailboxButton::OnNewMailboxInRoot()
{
	OnNewMailbox(NULL);
}

/////////////////////////////////////////////////////////////////////////////
// CMailboxButton message handlers
QCCommandObject* CMailboxButton::OnNewMailbox(QCCommandObject* pCommand)
{
	if (pCommand)
		ASSERT_KINDOF(QCMailboxCommand, pCommand);

	// This pops up a dialog asking for the mailbox name
	pCommand = g_theMailboxDirector.CreateTargetMailbox((QCMailboxCommand*)pCommand, FALSE);

	if (pCommand)
	{
		ASSERT_KINDOF( QCMailboxCommand, pCommand );

		// Include IMAP mailboxes in this!! (JOK, 10/8/97)
		ASSERT( ( ( QCMailboxCommand *) pCommand)->GetType() == MBT_REGULAR ||
				( ( QCMailboxCommand *) pCommand)->GetType() == MBT_IMAP_MAILBOX );

		pCommand->Execute(CA_NEW_MAILBOX, NULL);
	}

	return pCommand;
}


void CMailboxButton::OnUpdateDynamicCommand(CCmdUI* pCmdUI)
{
	QCCommandObject*	pCommand;
	COMMAND_ACTION_TYPE	theAction;	

	if( pCmdUI->m_pSubMenu == NULL )
	{
		if( g_theCommandStack.Lookup( ( WORD ) ( pCmdUI->m_nID ), &pCommand, &theAction ) )
		{
			if (theAction == CA_TRANSFER_TO || theAction == CA_TRANSFER_NEW)
			{
				pCmdUI->Enable( TRUE );
				return;
			}
		}
	}

	pCmdUI->ContinueRouting();
}

BOOL CMailboxButton::OnDynamicCommand(UINT uID )
{
	return TRUE;
}

