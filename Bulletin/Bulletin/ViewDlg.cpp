// ViewDlg.cpp : implementation file
//

#include "pch.h"
#include "Resource.h"
#include "BulletinDlg.h"
#include "ViewDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef	UNICODE
#define	strcpy		wcscpy
#define	strcpy_s	wcscpy_s
#define	atoi		_wtoi
#define	strtol		wcstol
#endif//UNICODE

IMPLEMENT_DYNAMIC( CViewDlg, CDialog )

///////////////////////////////////////////////////////////////////////////////////////
// Constructor

CViewDlg::CViewDlg( CWnd* pParent )
	: CDialog( IDD_VIEW, pParent)
{
	m_nDot  = 0;
	m_nChar = 0;
	m_cxDot = 0;
	m_bWide = false;
	m_iFont = 0;

	for	( int i = 0; i < _countof( m_anHeight ); i++ )
		m_anHeight[i] = 0;
	for	( int i = 0; i < _countof( m_anQuality ); i++ )
		m_anQuality[i] = 0;
}

///////////////////////////////////////////////////////////////////////////////////////
// Interface Functions

void
CViewDlg::SetArgs( int nDot, int nChar, int cxDot, bool bWide )
{
	m_nDot  = nDot;
	m_nChar = nChar;
	m_cxDot = cxDot;
	m_bWide = bWide;
}

void
CViewDlg::GetArgs( int& nDot, int& nChar, int& cxDot, bool& bWide )
{
	nDot  = m_nDot;
	nChar = m_nChar;
	cxDot = m_cxDot;
	bWide = m_bWide;
}

void
CViewDlg::SetFont( int iIndex, LOGFONT* plf )
{
	m_aFont[iIndex].DeleteObject();
	LOGFONT	lf = *plf;
	m_anHeight [iIndex] = lf.lfHeight;
	m_anQuality[iIndex] = lf.lfQuality;
	lf.lfHeight = 20;
	m_aFont[iIndex].CreateFontIndirect( &lf );
}

void
CViewDlg::GetFont( int iIndex, LOGFONT* plf )
{
	m_aFont[iIndex].GetLogFont( plf );
	plf->lfHeight  = m_anHeight [iIndex];
	plf->lfQuality = m_anQuality[iIndex];
}

///////////////////////////////////////////////////////////////////////////////////////
// Overridden Functions

BOOL
CViewDlg::OnInitDialog( void )
{
	CDialog::OnInitDialog();

	HICON	hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );
	SetIcon( hIcon, TRUE );
	SetIcon( hIcon, FALSE );

	GetOwner()->PostMessage( WM_POPUP, 0, (LPARAM)this );

	CString	str;
	CComboBox*	pCombo = (CComboBox*)GetDlgItem( IDC_VIEW_COMBO_HEIGHT );
	int	n = pCombo->GetCount();
	for	( int i = 0; i < n; i++ ){
		pCombo->GetLBText( i, str );
		if	( atoi( str ) == m_nDot ){
			pCombo->SetCurSel( i );
			break;
		}
	}

	str.Format( _T("%d"), m_nChar );
	GetDlgItem( IDC_VIEW_EDIT_WIDTH )->SetWindowText( str.GetBuffer() );

	str.Format( _T("%d"), m_cxDot );
	GetDlgItem( IDC_VIEW_EDIT_MAGNIFY )->SetWindowText( str.GetBuffer() );

	((CButton*)GetDlgItem( IDC_VIEW_CHECK_WIDE ))->SetCheck( m_bWide? BST_CHECKED: BST_UNCHECKED );

	for	( int i = 0; i < 2; i++ ){
		LOGFONT	lf;
		m_aFont[i].GetLogFont( &lf );
		GetDlgItem( IDC_VIEW_STATIC_BULLETIN+i )->SetFont( &m_aFont[i], FALSE );
		GetDlgItem( IDC_VIEW_STATIC_BULLETIN+i )->SetWindowText( lf.lfFaceName );
	}

	CSpinButtonCtrl*	pSpin;
	pSpin = (CSpinButtonCtrl*)GetDlgItem( IDC_VIEW_SPIN_WIDTH );
	pSpin->SetRange32( 1, 1000 );
	pSpin = (CSpinButtonCtrl*)GetDlgItem( IDC_VIEW_SPIN_MAGNIFY );
	pSpin->SetRange32( 1, 100 );

	return	TRUE;
}

void
CViewDlg::OnOK( void )
{
	CString	str;
	CComboBox*	pCombo = (CComboBox*)GetDlgItem( IDC_VIEW_COMBO_HEIGHT );

	int	i = pCombo->GetCurSel();
	pCombo->GetLBText( i, str );
	m_nDot = atoi( str );

	GetDlgItem( IDC_VIEW_EDIT_WIDTH )->GetWindowText( str );
	m_nChar = atoi( str );

	GetDlgItem( IDC_VIEW_EDIT_MAGNIFY )->GetWindowText( str );
	m_cxDot = atoi( str );

	m_bWide = ((CButton*)GetDlgItem( IDC_VIEW_CHECK_WIDE ))->GetCheck() == BST_CHECKED;

	CDialog::OnOK();
}

///////////////////////////////////////////////////////////////////////////////////////
// Message Handlers

#include "FontDlg.h"

BEGIN_MESSAGE_MAP( CViewDlg, CDialog )
	ON_CONTROL_RANGE( BN_CLICKED, IDC_VIEW_BUTTON_FONT_BULLETIN, IDC_VIEW_BUTTON_FONT_LIST, OnButtonFont )
	ON_MESSAGE( WM_POPUP, OnPopup )
	ON_MESSAGE( WM_FONT,  OnFont )
END_MESSAGE_MAP()

void
CViewDlg::OnButtonFont( UINT uID )
{
	int	i = uID - IDC_VIEW_BUTTON_FONT_BULLETIN;
	
	CFontDlg	dlg;
	LOGFONT	lf = { 0 };
	m_aFont[i].GetLogFont( &lf );
	dlg.SetFont( &lf );
	dlg.SetOwner( this );
	m_iFont = i;

	if	( dlg.DoModal() != IDOK )
		return;

	dlg.GetFont( &lf );
	m_aFont[i].DeleteObject();
	m_aFont[i].CreateFontIndirect( &lf );
	GetDlgItem( IDC_VIEW_STATIC_BULLETIN+i )->SetFont( &m_aFont[i], FALSE );
	GetDlgItem( IDC_VIEW_STATIC_BULLETIN+i )->SetWindowText( lf.lfFaceName );
}

LRESULT
CViewDlg::OnPopup( WPARAM wParam, LPARAM lParam )
{
	// Get rectangles of this window and the popup.

	CWnd*	pDialog = (CWnd*)lParam;

	CRect	rectOwner, rectDialog;
	GetWindowRect( &rectOwner );
	pDialog->GetWindowRect( &rectDialog );

	// Get info of the monitor this window belongs to.

	MONITORINFO mi;
	mi.cbSize = sizeof( mi );
	HMONITOR	hMonitor = MonitorFromRect( &rectOwner, MONITOR_DEFAULTTONEAREST );
	GetMonitorInfo( hMonitor, &mi );

	// Calculate margins between this window and the monitor.

	CRect	rectMonitor = mi.rcMonitor;
	int	cxLeft  = rectOwner.left     - rectMonitor.left;
	int	cxRight = rectMonitor.right  - rectOwner.right;
	int	cyUpper = rectOwner.top      - rectMonitor.top;
	int	cyLower = rectMonitor.bottom - rectOwner.bottom;

	// Select X and Y to palace the popup.

	int	x = ( cxLeft < cxRight  )? rectOwner.left: rectOwner.right - rectDialog.Width();
	int	y = ( cyUpper < cyLower )? rectOwner.bottom: rectOwner.top - rectDialog.Height();
	if	( cxLeft == cxRight )
		x = ( rectMonitor.Width()/2 ) - ( rectDialog.Width()/2 );

	// Place the popup.

	pDialog->SetWindowPos( &CWnd::wndTop, x, y, 0, 0, SWP_NOSIZE );

	return	0;
}

LRESULT
CViewDlg::OnFont( WPARAM wParam, LPARAM lParam )
{
	LOGFONT	lf = *(LOGFONT*)lParam;
	int	i = m_iFont;

	m_aFont[i].DeleteObject();
	m_aFont[i].CreateFontIndirect( &lf );
	GetDlgItem( IDC_VIEW_STATIC_BULLETIN+i )->SetFont( &m_aFont[i], FALSE );
	GetDlgItem( IDC_VIEW_STATIC_BULLETIN+i )->SetWindowText( lf.lfFaceName );

	return	0;
}