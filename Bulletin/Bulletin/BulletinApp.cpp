// BulletinApp.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "BulletinApp.h"
#include "BulletinDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CBulletinApp theApp;

///////////////////////////////////////////////////////////////////////////////////////
// Overridden Functions

BOOL
CBulletinApp::InitInstance( void )
{
	INITCOMMONCONTROLSEX InitCtrls = {};
	InitCtrls.dwSize = sizeof( InitCtrls );
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx( &InitCtrls );

	CWinApp::InitInstance();

	SetRegistryKey( _T("In-house Tool") );

	// Activate "Windows Native" visual manager for enabling themes in MFC controls.

	CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS( CMFCVisualManagerWindows ) );

	CString	strName = m_pszAppName;

	CBulletinDlg	dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	return	FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////
// Interface Functions

int
CBulletinApp::GetProfileInt( CString strSection, CString strName, int nDefault )
{
	CString	strKey;
	strKey.Format( _T("Software\\%s\\%s\\%s"), m_pszRegistryKey, m_pszAppName, strSection.GetBuffer() );

	HKEY	hKey = NULL;
	DWORD	dwResult = RegOpenKeyEx( HKEY_CURRENT_USER, strKey, 0, KEY_QUERY_VALUE, &hKey );

	if	( dwResult == ERROR_SUCCESS ){
		int	nValue = 0;
		ULONG	cbValue = sizeof( nValue );

		dwResult = RegGetValue( HKEY_CURRENT_USER, strKey, strName, RRF_RT_REG_DWORD, NULL, (PVOID)&nValue, &cbValue );
		if	( dwResult == ERROR_SUCCESS )
			nDefault = nValue;
		RegCloseKey( hKey );
	}

	return	nDefault;
}

CString
CBulletinApp::GetProfileString( CString strSection, CString strName, CString strDefault )
{
	CString	strKey;
	strKey.Format( _T("Software\\%s\\%s\\%s"), m_pszRegistryKey, m_pszAppName, strSection.GetBuffer() );

	HKEY	hKey = NULL;
	DWORD	dwResult = RegOpenKeyEx( HKEY_CURRENT_USER, strKey, 0, KEY_QUERY_VALUE, &hKey );

	if	( dwResult == ERROR_SUCCESS ){
		BYTE	abValue[2048] = { 0 };
		ULONG	cbValue = sizeof( abValue );

		dwResult = RegGetValue( HKEY_CURRENT_USER, strKey, strName, RRF_RT_REG_SZ, NULL, (PVOID)abValue, &cbValue );
		if	( dwResult == ERROR_SUCCESS )
			strDefault = (TCHAR*)abValue;
		RegCloseKey( hKey );
	}

	return	strDefault;
}

BOOL
CBulletinApp::WriteProfileInt( CString strSection, CString strName, int nValue )
{
	CString	strKey;
	strKey.Format( _T("Software\\%s\\%s\\%s"), m_pszRegistryKey, m_pszAppName, strSection.GetBuffer() );

	HKEY	hKey = NULL;
	DWORD	dwDisp = 0;
	DWORD	dwResult = RegCreateKeyEx( HKEY_CURRENT_USER, strKey, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisp );

	if	( dwResult == ERROR_SUCCESS ){
		DWORD	cbValue = sizeof( DWORD );
		dwResult = RegSetValueEx( hKey, strName, NULL, REG_DWORD, (BYTE*)&nValue, cbValue );
		RegCloseKey( hKey );
	}

	return	( dwResult == ERROR_SUCCESS )? TRUE: FALSE;
}

BOOL
CBulletinApp::WriteProfileString( CString strSection, CString strName, CString strValue )
{
	CString	strKey;
	strKey.Format( _T("Software\\%s\\%s\\%s"), m_pszRegistryKey, m_pszAppName, strSection.GetBuffer() );

	HKEY	hKey = NULL;
	DWORD	dwDisp = 0;
	DWORD	dwResult = RegCreateKeyEx( HKEY_CURRENT_USER, strKey, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisp );

	if	( dwResult == ERROR_SUCCESS ){
		DWORD	cbValue = ( strValue.GetLength()+1 ) * sizeof( TCHAR );
		dwResult = RegSetValueEx( hKey, strName, NULL, REG_SZ, (BYTE*)strValue.GetBuffer(), cbValue );
		RegCloseKey( hKey );
	}

	return	( dwResult == ERROR_SUCCESS )? TRUE: FALSE;
}

BOOL
CBulletinApp::DeleteProfileValue( CString strSection, CString strName )
{
	CString	strKey;
	strKey.Format( _T("Software\\%s\\%s\\%s"), m_pszRegistryKey, m_pszAppName, strSection.GetBuffer() );

	HKEY	hKey = NULL;
	DWORD	dwResult = RegOpenKeyEx( HKEY_CURRENT_USER, strKey, 0, KEY_WRITE, &hKey );

	if	( dwResult == ERROR_SUCCESS ){
		dwResult = RegDeleteValue( hKey, strName );
		RegCloseKey( hKey );
	}

	return	( dwResult == ERROR_SUCCESS )? TRUE: FALSE;
}