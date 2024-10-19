// PositionDlg.cpp : implementation file
//

#include "pch.h"
#include "Resource.h"
#include "BulletinDlg.h"
#include "PositionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC( CPositionDlg, CDialog )

///////////////////////////////////////////////////////////////////////////////////////
// Constructor

CPositionDlg::CPositionDlg( CWnd* pParent )
	: CDialog( IDD_POSITION, pParent )
{
	m_iCorner  = 0;
	m_iMonitor = 0;
	m_nMonitor = 0;
}

///////////////////////////////////////////////////////////////////////////////////////
// Interface Functions

void
CPositionDlg::SetArgs( int iCorner, int iMonitor, int nMonitor )
{
	m_iCorner  = iCorner;
	m_iMonitor = iMonitor;
	m_nMonitor = nMonitor;
}

void
CPositionDlg::GetArgs( int& iCorner, int& iMonitor )
{
	iCorner  = m_iCorner;
	iMonitor = m_iMonitor;
}

///////////////////////////////////////////////////////////////////////////////////////
// Overridden Functions

BOOL
CPositionDlg::OnInitDialog( void )
{
	CDialog::OnInitDialog();

	HICON	hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );
	SetIcon( hIcon, TRUE );
	SetIcon( hIcon, FALSE );

	GetOwner()->PostMessage( WM_POPUP, 0, (LPARAM)this );

	((CButton*)GetDlgItem( IDC_POSITION_RADIO_POS0+m_iCorner ))->SetCheck( BST_CHECKED );
	CComboBox*	pComombo = (CComboBox*)GetDlgItem( IDC_POSITION_COMBO_MONITOR );
	for	( int i = 0;  i < m_nMonitor; i++ ){
		CString	str;
		if	( i == 0 )
			str = _T("1st");
		else if	( i == 1 )
			str = _T("2nd");
		else if	( i == 2 )
			str = _T("3rd");
		else
			str.Format(_T("%dth"), i );
		pComombo->AddString( str );
	}
	pComombo->SetCurSel( m_iMonitor );

	return	TRUE;
}

void
CPositionDlg::OnOK( void )
{
	for	( int i = 0; i < 6; i++ )
		if	( ((CButton*)GetDlgItem( IDC_POSITION_RADIO_POS0+i ))->GetCheck() == BST_CHECKED ){
			m_iCorner = i;
			break;
		}

	m_iMonitor = ((CComboBox*)GetDlgItem( IDC_POSITION_COMBO_MONITOR ))->GetCurSel();

	CDialog::OnOK();
}

///////////////////////////////////////////////////////////////////////////////////////
// Message Handlers

BEGIN_MESSAGE_MAP( CPositionDlg, CDialog )
END_MESSAGE_MAP()
