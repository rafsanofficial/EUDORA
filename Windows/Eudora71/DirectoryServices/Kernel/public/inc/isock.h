/******************************************************************************/
/*																										*/
/*	Name		:	ISOCK.H																			*/
/* Date     :  4/23/1997                                                      */
/* Author   :  Jim Susoy                                                      */
/* Notice   :  (C) 1997 Qualcomm, Inc. - All Rights Reserved                  */
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

/*	Desc.		:	ISock Class definition														*/
/*																										*/
/******************************************************************************/
#ifndef _ISOCK_H_
#define _ISOCK_H_

#include <winsock.h>
#include "IList.h"
#include "ISched.h"

typedef void (*ISOCKCB)(LPVOID pCtx);

/* ISock object state definitions															*/
typedef enum {
	ISOCK_UNINITIALIZED,
	ISOCK_INITIALIZED,
	ISOCK_ERROR,
	ISOCK_LOOKUPHOST,
	ISOCK_CONNECTING,
	ISOCK_CONNECTED,
} ISOCK_STATE;

/* ISock object dwFlags definitions															*/
#define ISOCK_FLAG_NOFLAGS	0x000000

typedef class ISock * LPISOCK;
class ISock:public IUnknown
{
public:
	/* These methods can be called before Initialize()									*/
	virtual HRESULT STDMETHODCALLTYPE GetWinsockVersion(WORD *pwVersion) PURE;
	virtual HRESULT STDMETHODCALLTYPE GetWinsockMaxVersion(WORD *pwVersion) PURE;
	virtual HRESULT STDMETHODCALLTYPE GetWinsockDesc(LPSTR pszBuffer,int nBufLen) PURE;
	virtual HRESULT STDMETHODCALLTYPE GetWinsockStatus(LPSTR pszBuffer,int nBufLen) PURE;
	virtual HRESULT STDMETHODCALLTYPE GetMaxSockets(unsigned short *pusMaxSock) PURE;
	virtual HRESULT STDMETHODCALLTYPE GetMaxUdpSize(int *pusMaxUdp) PURE;
	
	/* This method must be called before any of the following methods.			*/
	virtual HRESULT STDMETHODCALLTYPE Initialize(LPSTR pszHost,int nPort,DWORD dwFlags) PURE;
	
	/* Create a connection to the host that was specified in Initialize()		*/
	virtual HRESULT STDMETHODCALLTYPE Connect(ISOCKCB pCB,LPVOID pCtx) PURE;
	
	/* Schedule a read operation when data is available								*/
	virtual HRESULT STDMETHODCALLTYPE Readable(ISOCKCB pCB,LPVOID pCtx) PURE;
	
	/* Read data from socket																	*/
	virtual HRESULT STDMETHODCALLTYPE Read(LPSTR pBuf,int nLen,int *nBytesRead) PURE;
	
	/* Schedule a write operation																*/
	virtual HRESULT STDMETHODCALLTYPE Writable(ISOCKCB pCB,LPVOID pCtx) PURE;
	
	/* Write data to the socket																*/
	virtual HRESULT STDMETHODCALLTYPE Write(LPSTR pBuf,int nLen,int *bBytesWritten) PURE;
	
	/* Cancels any pending Async socket operations										*/
	virtual HRESULT STDMETHODCALLTYPE Cancel(void) PURE;
	
	/* Gets the winsock socket error assoiated with the last socket operation.	*/
	/* This value will remain valid until the next socket operation.				*/
	virtual HRESULT STDMETHODCALLTYPE GetSocketError(int *pErr) PURE;
};

/* 3f0dd7e0-c83a-11d0-8d34-00a02471d0b1 (Generated by uuidgen.exe)				*/
#define ISOCK_CLASS_STR		"{3f0dd7e0-c83a-11d0-8d34-00a02471d0b1}"
DEFINE_GUID(CLSID_ISock		,0x3f0dd7e0,0xc83a,0x11d0,0x8d,0x34,0x00,0xa0,0x24,0x71,0xd0,0xb1);

#define ISOCK_IID_STR 		SOCKET_CLASS_STR
DEFINE_GUID(IID_ISock		,0x3f0dd7e0,0xc83a,0x11d0,0x8d,0x34,0x00,0xa0,0x24,0x71,0xd0,0xb1);

#endif				

