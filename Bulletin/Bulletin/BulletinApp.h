// Bulletin.h : main header file for the Bulletin application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"

class CBulletinApp : public CWinApp
{
public:
	virtual	BOOL	InitInstance( void );

		int	GetProfileInt(      CString strSection, CString strName, int nDefault );
		CString	GetProfileString(   CString strSection, CString strName, CString strDefault );
		BOOL	WriteProfileInt(    CString strSection, CString strName, int nValue );
		BOOL	WriteProfileString( CString strSection, CString strName, CString strValue );
		BOOL	DeleteProfileValue( CString strSection, CString strName );
};

extern	CBulletinApp	theApp;
