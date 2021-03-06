// SignatureTree.cpp : implementation file
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

#include "eudora.h"
#include "guiutils.h"
#include "ClipboardMgr.h"
#include "QCCommandActions.h"
#include "QCSignatureCommand.h"
#include "QCSignatureDirector.h"
#include "rs.h"
#include "SignatureTree.h"
#include "QCOleDropSource.h"

extern QCSignatureDirector	g_theSignatureDirector;

#include "DebugNewHelpers.h"

extern CString EudoraDir;

/////////////////////////////////////////////////////////////////////////////
// CSignatureTree

CSignatureTree::CSignatureTree()
{
//	EnableAutomation();
	g_theSignatureDirector.Register( this );
}

CSignatureTree::~CSignatureTree()
{
	g_theSignatureDirector.UnRegister( this );
}

BEGIN_MESSAGE_MAP(CSignatureTree, QCTreeCtrl)
	//{{AFX_MSG_MAP(CSignatureTree)
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginLabelEdit)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndLabelEdit)
	ON_WM_CHAR()
	ON_WM_KEYUP()
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBeginDrag)
	ON_WM_CONTEXTMENU()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CSignatureTree::Init()
{
	QCSignatureCommand*	pCommand;
	POSITION				pos;

	if (! QCTreeCtrl::Init())
		return FALSE;

	//
	// Add the bitmap images for tree control.
	//
	if (! m_ImageList.Load())
	{
		return FALSE;		// mangled resources?
	}

	SetImageList(&m_ImageList, TVSIL_NORMAL);
	
	//
	// Set "thin" font for old Win 3.x GUI.
	//
	if (! IsVersion4())
	{
		SetFont(CFont::FromHandle(HFONT(::GetStockObject(ANSI_VAR_FONT))));
	}

	pos = g_theSignatureDirector.GetFirstSignaturePosition();

	while( pos != NULL )
	{
		pCommand = g_theSignatureDirector.GetNext( pos );
		AddItem( pCommand->GetName() );
	}
	
	
	SortChildren( NULL );

	return TRUE;
}



DROPEFFECT CSignatureTree::OnDragEnter(
COleDataObject*	pDataObject,
DWORD, 
CPoint)
{
	if( g_theClipboardMgr.IsInstanceDataAvailable( pDataObject, QCClipboardMgr::CF_TRANSFER ) )
	{
		StartAutoScrollTimer();
	}

	return DROPEFFECT_NONE;
}


void CSignatureTree::OnDragLeave()
{
	KillAutoScrollTimer();
}


DROPEFFECT CSignatureTree::OnDragOver(
COleDataObject*	pDataObject,
DWORD, 
CPoint			thePoint)
{
	if( g_theClipboardMgr.IsInstanceDataAvailable( pDataObject, QCClipboardMgr::CF_TRANSFER ) )
	{
		UINT flags = TVHT_ONITEM | TVHT_ONITEMBUTTON | TVHT_ONITEMINDENT | TVHT_ONITEMRIGHT;
		HTREEITEM h_target = HitTest( thePoint, &flags);
		if (h_target != NULL)
		{
			SelectDropTarget(h_target);
			return DROPEFFECT_MOVE;
		}
#ifdef OK1
		QCFTRANSFER qcfTransfer;
		if (! g_theClipboardMgr.GetGlobalData(pDataObject, qcfTransfer))
		{
			ASSERT(0);
			return DROPEFFECT_NONE;
		}

		//
		// Okay, clipboard has usable drop data from our instance,
		// so let's do a hit test to see if we're over a valid 
		// mailbox target.
		//
		UINT flags = TVHT_ONITEM | TVHT_ONITEMBUTTON | TVHT_ONITEMINDENT | TVHT_ONITEMRIGHT;
		HTREEITEM h_target = HitTest(point, &flags);
		if (h_target != NULL)
		{
			CMboxTreeItemData* p_itemdata = (CMboxTreeItemData *) GetItemData(h_target);
			ASSERT(p_itemdata != NULL);
			switch (p_itemdata->m_itemType)
			{
			case ITEM_ROOT:
			case ITEM_FOLDER:
				{
					//
					// Hit folder item, so select it using bogus "link" cursor
					// to give user visual feedback regarding "auto-open"
					// capability.
					//
					SelectDropTarget(h_target);
					return DROPEFFECT_LINK;
				}
				break;
			case ITEM_IN_MBOX:
			case ITEM_OUT_MBOX:
			case ITEM_TRASH_MBOX:
			case ITEM_JUNK_MBOX:
			case ITEM_USER_MBOX:
				SelectDropTarget(h_target);
				if ((dwKeyState & MK_CONTROL) || ShiftDown())
					return DROPEFFECT_COPY;
				else
					return DROPEFFECT_MOVE;
				break;
			default:
				ASSERT(0);
				break;
			}
		}
#endif
	}

	SelectDropTarget(NULL);
	return DROPEFFECT_NONE;
}


/////////////////////////////////////////////////////////////////////////////
// CSignatureTree message handlers

HTREEITEM CSignatureTree::AddItem(LPCSTR szName)
{
	return InsertItem( szName, QCMailboxImageList::IMAGE_SIGNATURE, QCMailboxImageList::IMAGE_SIGNATURE );
}

void CSignatureTree::OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	//	Only allow the edit if we have the focus (setting *pResult to 1 cancels the edit).
	//	We will always have the focus when the user is actually trying to change the label.
	//	This fixes the bug where the user double clicked changing the focus only to have
	//	it stolen back by the label editing timer triggering (the first LButtonDown of a
	//	double click seems to install the timer).
	if (GetFocus() == this)
		*pResult = 0;
	else
		*pResult = 1;
}

void CSignatureTree::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO*			pTVDispInfo;
	CString					szOldName;
	CEdit*					pEdit;
	CString					szName;
	QCSignatureCommand*	pCommand;

	pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	*pResult = 0;

	if( ( pTVDispInfo->item.mask & TVIF_TEXT ) == 0 )
	{
		// the user pressed escape
		return;
	}

	szName = pTVDispInfo->item.pszText;
	szName.TrimLeft();
	szName.TrimRight();
	
	if( szName == "" )
	{
		// don't let the user set this to a blank name 
		// go back to editing

		MessageBeep( MB_OK );
		SendMessage( TVM_EDITLABEL, 0, LPARAM( pTVDispInfo->item.hItem ) );
		
		pEdit = GetEditControl();
		
		if( pEdit != NULL )
		{
			pEdit->SetWindowText( szName );
			pEdit->SetSel( 0, -1 );
		}
		
		return;
	}

	szOldName = GetItemText( pTVDispInfo->item.hItem );
	
	pCommand = g_theSignatureDirector.Find( szOldName );

	if( pCommand != NULL )
	{
		pCommand->Execute( CA_RENAME, ( void* ) (LPCSTR) szName );
	}

	if( szOldName.CompareNoCase( szName ) != 0 )
	{
		pCommand = g_theSignatureDirector.Find( szOldName );

		if( pCommand )
		{
			// the old name is still around -- an error occurred
			// keep editing after failure
			
			SendMessage( TVM_EDITLABEL, 0, LPARAM( pTVDispInfo->item.hItem ) );
			
			pEdit = GetEditControl();
			
			if( pEdit != NULL )
			{
				pEdit->SetWindowText( szOldName );
				pEdit->SetSel( 0, -1 );
			}
		}
	}			
}

void CSignatureTree::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if( nChar == VK_RETURN )
	{
		if (GetEditControl())
			SendMessage( TVM_ENDEDITLABELNOW, FALSE, 0 );
		else
			OnLButtonDblClk(0, CPoint(-1, -1));		// fake a double-click on the selected item
	}
	else
	if( nChar == VK_ESCAPE )
	{
		SendMessage( TVM_ENDEDITLABELNOW, TRUE, 0 );	
	}
	else
	{
		QCTreeCtrl::OnChar(nChar, nRepCnt, nFlags);
	}
}


////////////////////////////////////////////////////////////////////////
// OnKeyUp [protected]
//
////////////////////////////////////////////////////////////////////////
void CSignatureTree::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case VK_DELETE:
		{
			// The Del key is a shortcut for the Delete command.
			CWnd* pParentWnd = GetParent();
			ASSERT(pParentWnd != NULL);
			if(pParentWnd) // Control Container
				pParentWnd = pParentWnd->GetParent();

			ASSERT(pParentWnd != NULL);

			if(pParentWnd)
				pParentWnd->PostMessage(WM_COMMAND, IDM_DELETE_SIGNATURE, LPARAM(GetSafeHwnd()));
		}
		break;
	case VK_F2:
		{
			//
			// Shortcut for the Rename command.
			//
			HTREEITEM hSelectedItem = GetSelectedItem();
			if (hSelectedItem != NULL)
				PostMessage(TVM_EDITLABEL, 0, LPARAM(hSelectedItem)); 
			QCTreeCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
		}
		break;
	default:
		QCTreeCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
		break;
	}
}




void CSignatureTree::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW*				pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	QCSignatureCommand*			pCommand;
	HTREEITEM					hItem;
	CString						szName;

	hItem = pNMTreeView->itemNew.hItem;
	szName = GetItemText( hItem );

	// find the command
	pCommand = g_theSignatureDirector.Find( szName );

	if( pCommand == NULL )
	{
		ASSERT( 0 );
		return;
	}

	// allocate space
	QCFSIGNATURE qcfSignature;
	ASSERT(pCommand->GetPathname().GetLength() < sizeof(qcfSignature.m_szPathName));
	strcpy(qcfSignature.m_szPathName, pCommand->GetPathname() );
	g_theClipboardMgr.CacheGlobalData(m_OleDataSource, qcfSignature);
	
	//
	// Do the drag and drop
	//
	QCOleDropSource oleDropSource;
	m_OleDataSource.DoDragDrop( DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK, NULL, &oleDropSource);
  
	// Clear the Data Source's cache
	m_OleDataSource.Empty();
	
	*pResult = 0;
}


HTREEITEM	CSignatureTree::FindItem(
HTREEITEM	hItem,
LPCSTR		szName )
{
	HTREEITEM	hChild;
	CString		szItemName;

	if( hItem == NULL )
	{
		return NULL;
	}
	
	szItemName = GetItemText( hItem );

	if( szItemName.CompareNoCase( szName ) == 0 )
	{
		return hItem;
	}

	if( ItemHasChildren( hItem ) )
	{
		// do a depth first search
		hChild = GetChildItem( hItem );

		if( hChild != NULL )
		{
			hChild = FindItem( hChild, szName );
			if( hChild != NULL )
			{
				// it's a child of this
				return hChild;
			}
		}
	}
	
	// check sibling items 
	return FindItem( GetNextSiblingItem( hItem ), szName );
}


void CSignatureTree::Notify( 
QCCommandObject*	pObject,
COMMAND_ACTION_TYPE	theAction,
void*				pData )
{
	HTREEITEM	hItem;

	if( ( pObject->IsKindOf( RUNTIME_CLASS( QCSignatureCommand ) ) == FALSE ) ||
		( ( theAction != CA_RENAME ) && ( theAction != CA_NEW ) && ( theAction != CA_DELETE ) ))
	{
		return;
	}

	if( ( theAction == CA_RENAME ) || ( theAction == CA_DELETE ) )
	{
		hItem = FindItem( GetRootItem(), ( ( QCSignatureCommand* ) pObject )->GetName() );
		
		if( hItem )
		{
			if( theAction == CA_DELETE ) 
			{
				DeleteItem( hItem );
			}
			else
			{
				SetItemText( hItem, ( LPCSTR ) pData );
				SortChildren( NULL );
			}
		}
	}

	if( theAction == CA_NEW ) 
	{
		AddItem( ( ( QCSignatureCommand* ) pObject )->GetName () );
		SortChildren( NULL );
	}
}


void CSignatureTree::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	HTREEITEM	hGetSelectedItem;
	HTREEITEM	hHitTest;
	UINT		uFlags;
	CPoint		thePoint( point );

	hGetSelectedItem = GetSelectedItem();
	ScreenToClient( &thePoint );
	hHitTest = HitTest( thePoint, &uFlags );

	if( ( hHitTest != hGetSelectedItem ) && ( hHitTest != NULL ) )
	{
		if( uFlags & TVHT_ONITEM )
		{
			SelectItem( hHitTest );	
		}
	}

	CTreeCtrl::OnContextMenu( pWnd, point );
}


void CSignatureTree::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	HTREEITEM			hSelected;
	HTREEITEM			hHitItem;
	TV_HITTESTINFO		tvHit;
	CString				szName;
	QCSignatureCommand*	pCommand;

	hSelected = GetSelectedItem();

#ifdef _DEBUG
	if (0 == nFlags)
	{
		// nFlags should only be zero for "fake" mouse double-click cases
		ASSERT(-1 == point.x && -1 == point.y);
	}
#endif // DEBUG

	if( hSelected != NULL )
	{
		//
		// Do a hit test to see where the user landed the double click.
		// Only do this if we're handling a bona-fide mouse click.  The
		// pNMHDR is NULL if we're handling an Enter key shortcut.
		//
		hHitItem = NULL;
		
		if (nFlags)
		{
			tvHit.flags = 0;
			tvHit.hItem = 0;
			tvHit.pt = point;		// client coordinates
			HitTest(&tvHit);

			if ( ( tvHit.flags & TVHT_ONITEM ) && ( tvHit.hItem != NULL ) )
			{
				hHitItem = tvHit.hItem;
			}
		}

		if ( ( 0 == nFlags ) || ( hHitItem == hSelected ) )
		{
			szName = GetItemText( hSelected );
			pCommand = g_theSignatureDirector.Find( szName );
			ASSERT( pCommand );
			pCommand->Execute( CA_OPEN );
			
			return;
		}
	}

	QCTreeCtrl::OnLButtonDblClk(nFlags, point);
}
