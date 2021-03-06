// TaskErrorWazooWnd.cpp : implementation file
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
#include "resource.h"
#include "utils.h"
#include "TaskErrorView.h"
#include "TaskErrorWazooWnd.h"
#include "mainfrm.h"
#include "QCTaskManager.h"

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CTaskErrorWazooWnd

IMPLEMENT_DYNCREATE(CTaskErrorWazooWnd, CWazooWnd)

BEGIN_MESSAGE_MAP(CTaskErrorWazooWnd, CWazooWnd)
	//{{AFX_MSG_MAP(CTaskErrorWazooWnd)
	ON_WM_SIZE()
	ON_MESSAGE(msgErrorViewNewError, OnMsgNewError)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CTaskErrorWazooWnd::CTaskErrorWazooWnd() : CWazooWnd(IDR_TASK_ERROR),
	m_pTaskErrorView(NULL)
{
	// Initially set up the Task Manager to send event messages here.  When (if) it gets
	// the first message posted to it, then the view will be created and from then on
	// the view will get the messages from the Task Manager.
	QCGetTaskManager()->SetDisplayWindow(CTaskObjectMT::TOBJ_ERROR, this);
}

CTaskErrorWazooWnd::~CTaskErrorWazooWnd()
{
	m_pTaskErrorView = NULL;			// deleted in CView::OnPostNcDestroy();
}


////////////////////////////////////////////////////////////////////////
// OnActivateWazoo [public, virtual]
//
// Redefine base class implementation to pass activation notification
// to Task Status view.
////////////////////////////////////////////////////////////////////////
void CTaskErrorWazooWnd::OnActivateWazoo()
{
	CreateView();

	CControlBar* pParentControlBar = GetParentControlBar();

	if (pParentControlBar && pParentControlBar->IsVisible())
	{
		if (m_pTaskErrorView)
			m_pTaskErrorView->SendMessage(umsgActivateWazoo);
	}
}

////////////////////////////////////////////////////////////////////////
// CreateView [protected]
//
////////////////////////////////////////////////////////////////////////
BOOL CTaskErrorWazooWnd::CreateView()
{
	if (m_pTaskErrorView == NULL)
	{
		m_pTaskErrorView = DEBUG_NEW_MFCOBJ_NOTHROW CTaskErrorView;
		if (m_pTaskErrorView == NULL)
			return FALSE;

		//
		// For some reason, MFC is really insistent that this CView class
		// get created as a Child window.
		//
		if (!m_pTaskErrorView->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), this, UINT(-1), NULL))
			return FALSE;

		// Need to notify the children to perform their OnInitialUpdate() sequence.			
		SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE, TRUE);

		CRect rect;
		GetClientRect(&rect);
		OnSize(SIZE_RESTORED, rect.Width(), rect.Height());
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CTaskErrorWazooWnd message handlers

////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void CTaskErrorWazooWnd::OnSize(UINT nType, int cx, int cy) 
{
	if (m_pTaskErrorView)
	{
		CRect rectClient;
		GetClientRect(rectClient);

		m_pTaskErrorView->SetWindowPos(NULL, rectClient.left, rectClient.top,
											rectClient.Width(), rectClient.Height(),
											SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);
	}

	CWazooWnd::OnSize(nType, cx, cy);
}

LONG CTaskErrorWazooWnd::OnMsgNewError( WPARAM wParam, LPARAM lParam ) // NEW ERROR
{
	// Create the view which will handle message from now on.  We don't need to
	// repost this message to the view because when the view is created it will
	// register itself with the task manager, which will cause existing tasks
	// to notify the view about this new info.
	CreateView();

	return 0;
}
