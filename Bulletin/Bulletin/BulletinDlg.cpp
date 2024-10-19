// BulletinDlg.cpp : implementation file
//

#include "pch.h"
#include "BulletinApp.h"
#include "BulletinDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef	UNICODE
#define	strcmp		wcscmp
#define	strcpy		wcscpy
#define	strcpy_s	wcscpy_s
#define	atoi		_wtoi
#define	strtol		wcstol
#endif//UNICODE

#define	CX_MEM	16384

#define	TID_SCROLL	1
#define	TID_PAUSE	2

#define	WM_WTIMER	(WM_APP+10)

///////////////////////////////////////////////////////////////////////////////////////
// Constructor & Destructor

CBulletinDlg::CBulletinDlg( CWnd* pParent )
	: CDialog( IDD_BULLETIN, pParent )
{
	m_hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );

	LoadSettings();

	SelectScrollSpeed();
}

CBulletinDlg::~CBulletinDlg( void )
{
	SaveSettings();
}

///////////////////////////////////////////////////////////////////////////////////////
// Interface Functions

CString
CBulletinDlg::CheckSource( CString strURL, bool& bDesc )
{
	CString	strTitle;
	bDesc = false;

	CString	strContents = LoadWeb( strURL );
	int	x;
	
	x = strContents.Find( _T("<title>") );
	if	( x >= 0 ){
		strContents = strContents.Mid( x+7 );
		x = strContents.Find( _T("</title>") );
		if	( x >= 0 ){
			strTitle = strContents.Left( x );
			strContents = strContents.Mid( x+8 );
		}
	}
	x = strContents.Find( _T("<item>") );
	if	( x >= 0 ){
		strContents = strContents.Mid( x+6 );
		x = strContents.Find( _T("<description></description>") );
		if	( x >= 0 )
			bDesc = false;
		else{
			x = strContents.Find( _T("<description>") );
			if	( x >= 0 )
				bDesc = true;
			x = strContents.Find( _T("<description/>") );
			if	( x >= 0 )
				bDesc = false;
		}
	}

	DeCdata( strTitle );

	return	strTitle;
}

void
CBulletinDlg::SortNews( CArray <CNews, CNews&>& aNews, int iKey, int iOrder )
{
	int	aiContext[2];
	aiContext[0] = iKey;
	aiContext[1] = iOrder;

	qsort_s( aNews.GetData(), aNews.GetCount(), sizeof( CNews ), CompareNews, aiContext );
}

void
CBulletinDlg::GetDaily( CString strFile, CArray <CDaily, CDaily&>& aDaily )
{
	CString	strLines = LoadText( strFile );

	if	( strLines.IsEmpty() )
		return;

	while	( !strLines.IsEmpty() ){
		CDaily	daily;
		TCHAR*	pch = strLines.GetBuffer();
		daily.m_bHour = (BYTE)strtol( pch, &pch, 10 );
		if	( *pch != ':' ){
			TRACE( _T("wrong hour delimiter '%c' in '%s'\n"), *pch, strLines );
			break;
		}
		daily.m_bMinute = (BYTE)strtol( ++pch, &pch, 10 );
		if	( *pch <= ' ' ){
			daily.m_bSecond = 0;
		}
		else if	( *pch == ':' ){
			daily.m_bSecond = (BYTE)strtol( ++pch, &pch, 10 );
		}
		else{
			TRACE( _T("wrong second delimiter '%c' in '%s'\n"), *pch, strLines );
			break;
		}
		for	( pch++; *pch <= ' '; pch++ )
			;
		TCHAR*	pchText = pch;
		for	( ; *pch; pch++ )
			if	( *pch == '\r' )
				break;
		if	( *pch ){
			*pch++ = '\0';
			*pch++ = '\0';
		}
		daily.m_strText = pchText;
		aDaily.Add( daily );
		strLines = pch;
	}

	qsort( aDaily.GetData(), aDaily.GetCount(), sizeof( CDaily ), CompareDaily );
}

int
CBulletinDlg::SeekDaily( CTime tFrom, CArray <CDaily, CDaily&>& aDaily )
{
	int	n = (int)aDaily.GetCount();
	int	i = 0;

	int	iFrom = MAKESEC( tFrom.GetHour(), tFrom.GetMinute(), tFrom.GetSecond() );

	for	( ; i < n; i++ ){
		CDaily&	daily = aDaily[i];
		int	iDaily = MAKESEC( daily.m_bHour, daily.m_bMinute, daily.m_bSecond );
		if	( iDaily >= iFrom )
			return	i;
	}
	return	-1;
}

CFont*
CBulletinDlg::GetListFont( void )
{
	return	&m_aFont[1];
}

///////////////////////////////////////////////////////////////////////////////////////
// Overridden Functions

BOOL
CBulletinDlg::OnInitDialog( void )
{
	CDialog::OnInitDialog();

	SetIcon( m_hIcon, TRUE );
	SetIcon( m_hIcon, FALSE );

	SetWindowText( AfxGetApp()->m_pszAppName );

	PlaceWindow();

	SetBitmapDC();

	ReadDaily();

	SetScroll( true );

	m_timer.SetOwnerWnd( this );
	m_timer.SetNotifyMsg( WM_WTIMER );

	{
		CRect	rectClient;
		GetClientRect( &rectClient );
		CDC*	pDC = GetDC();
		pDC->FillSolidRect( &rectClient, m_crBack );
		ReleaseDC( pDC );
	}

	return	TRUE;
}

void
CBulletinDlg::OnOK( void )
{	// Do nothing for OK ( pressing the Enter key ).
}

///////////////////////////////////////////////////////////////////////////////////////
// Message Handlers

BEGIN_MESSAGE_MAP( CBulletinDlg, CDialog )
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_KEYUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_SYSKEYDOWN()
	ON_WM_TIMER()
	ON_COMMAND( ID_MENU_SOURCE,   OnMenuSource )
	ON_COMMAND( ID_MENU_NEWS,     OnMenuNews )
	ON_COMMAND( ID_MENU_DAILY,    OnMenuDaily )
	ON_COMMAND( ID_MENU_POSITION, OnMenuPosition )
	ON_COMMAND( ID_MENU_VIEW,     OnMenuView )
	ON_COMMAND( ID_MENU_INFO,     OnMenuInfo )
	ON_COMMAND( ID_MENU_EXIT,     OnMenuExit )
	ON_MESSAGE( WM_POPUP,  OnPopup )
	ON_MESSAGE( WM_WTIMER, OnWTimer )
END_MESSAGE_MAP()

BOOL
CBulletinDlg::OnEraseBkgnd( CDC* pDC )
{	// Do nothing to avoid flicker.
	return	TRUE;
}

void
CBulletinDlg::OnPaint( void )
{
	PAINTSTRUCT ps;
	CDC*	pDC = BeginPaint( &ps );

	pDC->SetBkMode( TRANSPARENT );

	Draw( pDC, ps.rcPaint );

	EndPaint( &ps );
}

void
CBulletinDlg::OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	if	( nChar == VK_APPS )
		PostMessage( WM_RBUTTONDOWN, 0, NULL );
	else if	( nChar == VK_F1 )
		OnMenuInfo();
	else if	( nChar == VK_F5 )
		ReloadNews();

	CDialog::OnKeyUp( nChar, nRepCnt, nFlags );
}

void
CBulletinDlg::OnLButtonDown( UINT nFlags, CPoint point )
{
	if	( nFlags & MK_CONTROL )
		if	( m_iNews < m_aNews.GetCount() ){
			CString	strLink = m_aNews[m_iNews].m_strLink;
			if	( !strLink.IsEmpty() )
				ShellExecute( NULL, _T("open"), strLink, NULL, NULL, SW_SHOWNORMAL );
		}
}

void
CBulletinDlg::OnRButtonDown( UINT nFlags, CPoint point )
{
	CMenu menu;
	menu.LoadMenu( IDR_POPUP );
	CMenu* pPopupMenu = menu.GetSubMenu( 0 );

	ClientToScreen( &point );
	pPopupMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this );

	PostMessage( WM_NULL, 0, 0 );
}

void
CBulletinDlg::OnSysKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	if	( nChar != VK_MENU ){
		UINT	uID = 0;
		if	( nChar == 'P' )
			uID = ID_MENU_POSITION;
		else if	( nChar == 'S' )
			uID = ID_MENU_SOURCE;
		else if	( nChar == 'N' )
			uID = ID_MENU_NEWS;
		else if	( nChar == 'D' )
			uID = ID_MENU_DAILY;
		else if	( nChar == 'V' )
			uID = ID_MENU_VIEW;

		if	( uID )
			PostMessage( WM_COMMAND, uID, NULL );
	}

	CDialog::OnSysKeyDown( nChar, nRepCnt, nFlags );
}

void
CBulletinDlg::OnTimer( UINT_PTR nIDEvent )
{
	if	( nIDEvent == TID_SCROLL ){
		ScrollBulletin();
	}
	else if	( nIDEvent == TID_PAUSE ){
		KillTimer( nIDEvent );
		if	( m_bRepeat )
			m_bRepeat = false;
		else{
			SetScroll( true );
			m_xOffset = m_cxText;
		}
		Invalidate( TRUE );
	}
	else
		CDialog::OnTimer( nIDEvent );
}

#include "SourceDlg.h"

void
CBulletinDlg::OnMenuSource( void )
{
	CSourceDlg	dlg;

	dlg.SetOwner( this );
	dlg.SetSources( m_aSource );

	if	( dlg.DoModal() != IDOK )
		return;

	dlg.GetSources( m_aSource );

	ReloadNews();
}

#include "NewsDlg.h"

void
CBulletinDlg::OnMenuNews( void )
{
	CNewsDlg	dlg;

	dlg.SetOwner( this );
	dlg.SetNews( m_aNews, m_iNewsKey, m_iNewsOrder );

	if	( dlg.DoModal() != IDOK )
		return;

	int	iKey   = m_iNewsKey;
	int	iOrder = m_iNewsOrder;
	dlg.GetSort( m_iNewsKey, m_iNewsOrder );

	if	( m_iNewsKey   != iKey ||
		  m_iNewsOrder != iOrder ){
		m_bUpdating = true;
		SortNews( m_aNews, m_iNewsKey, m_iNewsOrder );
		m_bUpdating = false;
	}
}

#include "DailyDlg.h"

void
CBulletinDlg::OnMenuDaily( void )
{
	CDailyDlg	dlg;

	dlg.SetOwner( this );
	dlg.SetFiles( m_aFile );

	if	( dlg.DoModal() != IDOK )
		return;

	bool	bUpdate = false;

	CArray	<CDailyFile, CDailyFile&>
		aFile;
	dlg.GetFiles( aFile );

	int	n = (int)aFile.GetCount();
	if	( n != (int)m_aFile.GetCount() )
		bUpdate = true;
	else
		for	( int i = 0; i < n; i++ )
			if	( aFile[i] == m_aFile[i] )
				;
			else{
				bUpdate = true;
				break;
			}
	if	( bUpdate ){
		m_aFile.RemoveAll();
		m_aFile.Append( aFile );
		ReadDaily();
	}
}

#include "PositionDlg.h"

void
CBulletinDlg::OnMenuPosition( void )
{
	CPositionDlg	dlg;

	dlg.SetOwner( this );
	dlg.SetArgs( m_iCorner, m_iMonitor, m_nMonitor );

	if	( dlg.DoModal() != IDOK )
		return;

	int	iCorner  = m_iCorner;
	int	iMonitor = m_iMonitor;
	dlg.GetArgs( iCorner, iMonitor );

	if	( iCorner  != m_iCorner ||
		  iMonitor != m_iMonitor  ){
		m_iCorner  = iCorner;
		m_iMonitor = iMonitor;
		PlaceWindow();
	}
}

#include "ViewDlg.h"

void
CBulletinDlg::OnMenuView( void )
{
	CViewDlg	dlg;

	dlg.SetOwner( this );
	dlg.SetArgs( m_nDot, m_nChar, m_cxDot, m_bWide );
	for	( int i = 0; i < 2; i++ ){
		LOGFONT	lf = { 0 };
		m_aFont[i].GetLogFont( &lf );
		dlg.SetFont( i, &lf );
	}

	if	( dlg.DoModal() != IDOK )
		return;

	int	nDot  = m_nDot;
	int	nChar = m_nChar;
	int	cxDot = m_cxDot;
	bool	bWide = m_bWide;

	bool	bFont   = false;
	bool	bBitmap = false;
	bool	bPlace  = false;

	dlg.GetArgs( m_nDot, m_nChar, m_cxDot, m_bWide );

	if	( m_nDot != nDot ){
		bFont   = true;
		bBitmap = true;
	}
	if	( m_nChar != nChar )
		bPlace  = true;
	if	( m_cxDot != cxDot )
		bPlace  = true;
	if	( m_bWide != bWide )
		bPlace  = true;

	LOGFONT	lf = { 0 };
	if	( !bFont ){
		for	( int i = 0; i < _countof( m_aFont ); i++ ){
			m_aFont[i].GetLogFont( &lf );
			CString	strFont = lf.lfFaceName;
			dlg.GetFont( i, &lf );
			if	( strFont != lf.lfFaceName )
				bFont = true;
		}
	}

	if	( bFont ){
		for	( int i = 0; i < _countof( m_aFont ); i++ ){
			dlg.GetFont( i, &lf );
			lf.lfHeight  = ( i == 0 )? ( m_nDot >= 32 )? m_nDot-2: m_nDot: m_cyFont;
			m_aFont[i].DeleteObject();
			m_aFont[i].CreateFontIndirect( &lf );
		}
		m_dcMem.SelectObject( &m_aFont[0] );

		SelectScrollSpeed();
	}
	if	( bBitmap ){
		SetBitmapDC();
		bPlace = true;
	}
	if	( bPlace ){
		PlaceWindow();
	}

	m_xOffset = m_cxText;
	Invalidate( TRUE );
}

void
CBulletinDlg::OnMenuInfo( void )
{
	CString	strPath = _T("https://github.com/inhouse-tool/Bulletin");
	ShellExecute( NULL, _T("open"), strPath, NULL, NULL, SW_SHOWNORMAL );
}

void
CBulletinDlg::OnMenuExit( void )
{
	SetScroll( false );
	CDialog::OnOK();
}

LRESULT
CBulletinDlg::OnPopup( WPARAM wParam, LPARAM lParam )
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
CBulletinDlg::OnWTimer( WPARAM wParam, LPARAM lParam )
{
	OnTimer( (UINT_PTR)wParam );

	return	0;
}

///////////////////////////////////////////////////////////////////////////////////////
// Specific Functions

#include <mmsystem.h>			// for timeBeginPeriod, timeEndPeriod and PlaySound
#pragma comment( lib, "winmm.lib" )

void
CBulletinDlg::LoadSettings( void )
{
	CBulletinApp*	pApp = (CBulletinApp*)AfxGetApp();

	m_nDot  = m_nDot1st  = pApp->GetProfileInt( _T("Settings"), _T("Height"), 32 );
	m_nChar = m_nChar1st = pApp->GetProfileInt( _T("Settings"), _T("Width"),   8 );
	m_cxDot = m_cxDot1st = pApp->GetProfileInt( _T("Settings"), _T("Magnify"), 4 );

	m_iCorner   = m_iCorner1st  = pApp->GetProfileInt( _T("Settings"), _T("Corner"),  1 );
	m_iMonitor  = m_iMonitor1st = pApp->GetProfileInt( _T("Settings"), _T("Monitor"), 0 );
	m_bWide     = m_bWide1st    = pApp->GetProfileInt( _T("Settings"), _T("Wide"), 0 )? true: false;

	m_iNewsKey   = m_iNewsKey1st   = pApp->GetProfileInt( _T("News"), _T("Key"),   0 );
	m_iNewsOrder = m_iNewsOrder1st = pApp->GetProfileInt( _T("News"), _T("Order"), 0 );

	m_nInterval = pApp->GetProfileInt( _T("Tune"), _T("Interval"), 15 );
	m_nPeriod   = pApp->GetProfileInt( _T("Tune"), _T("Period"), 0 );

	GetProfileFonts();

	GetProfileSources();
	m_aSource1st.Append( m_aSource );

	GetProfileDaily();
	m_aFile1st.Append( m_aFile );

	m_nMonitor  = 0;
	m_rcMonitor = { 0, 0, 0, 0 };

	m_crBack = RGB(  16,  16,  16 );
	m_crFore = RGB( 255, 255, 255 );
	m_crPlus = RGB( 255, 200,   0 );

	m_xOffset = 0;
	m_cxText  = 0;

	m_bRepeat = false;
	m_bUpdating = false;

	m_iNews   = 0;
	m_iDaily  = 0;

	m_tLast   = { 0 };
	m_tNews   = { 0 };
}

void
CBulletinDlg::SaveSettings( void )
{
	CBulletinApp*	pApp = (CBulletinApp*)AfxGetApp();

	if	( m_nDot     != m_nDot1st )
		pApp->WriteProfileInt( _T("Settings"), _T("Height"),  m_nDot );
	if	( m_nChar    != m_nChar1st )
		pApp->WriteProfileInt( _T("Settings"), _T("Width"),   m_nChar );
	if	( m_cxDot    != m_cxDot1st )
		pApp->WriteProfileInt( _T("Settings"), _T("Magnify"), m_cxDot );

	if	( m_iCorner  != m_iCorner1st )
		pApp->WriteProfileInt( _T("Settings"), _T("Corner"),  m_iCorner );
	if	( m_iMonitor != m_iMonitor1st )
		pApp->WriteProfileInt( _T("Settings"), _T("Monitor"), m_iMonitor );
	if	( m_bWide    != m_bWide1st )
		pApp->WriteProfileInt( _T("Settings"), _T("Wide"),    m_bWide );
	
	WriteProfileFonts();

	if	( m_iNewsKey != m_iNewsKey1st )
		pApp->WriteProfileInt( _T("News"), _T("Key"),   m_iNewsKey );
	if	( m_iNewsOrder != m_iNewsOrder1st )
		pApp->WriteProfileInt( _T("News"), _T("Order"), m_iNewsOrder );

	INT_PTR	n;
	bool	bUpdate;
		
	bUpdate = false;
	n = m_aSource.GetCount();
	if	( n != m_aSource1st.GetCount() )
		bUpdate = true;
	else
		for	( INT_PTR i = 0; i < n; i++ )
			if	( !( m_aSource[i] == m_aSource1st[i] ) ){
				bUpdate = true;
				break;
			}
	if	( bUpdate )
		WriteProfileSources();

	bUpdate = false;
	n = m_aFile.GetCount();
	if	( n != m_aFile1st.GetCount() )
		bUpdate = true;
	else
		for	( INT_PTR i = 0; i < n; i++ )
			if	( !( m_aFile[i] == m_aFile1st[i] ) ){
				bUpdate = true;
				break;
			}
	if	( bUpdate )
		WriteProfileDaily();
}

void
CBulletinDlg::PlaceWindow( void )
{
	// Count Monitors.

	SelectMonitor( false );

	// Place on the selected monitor.

	SelectMonitor( true );

	// Select the position.

	int	cy = m_nDot*m_cxDot;
	int	cx = cy*m_nChar;
	int	cxMonitor = ( m_rcMonitor.right - m_rcMonitor.left );
	int	dx = ( cxMonitor - cx ) / 2;
	if	( m_bWide ){
		cx = cxMonitor;
		dx = 0;
	}

	CRect	rectClient;
	if	( m_iCorner == 0 )
		rectClient.SetRect( m_rcMonitor.left,     m_rcMonitor.top,       m_rcMonitor.left +cx, m_rcMonitor.top+cy );
	else if	( m_iCorner == 1 )
		rectClient.SetRect( m_rcMonitor.left +dx, m_rcMonitor.top,       m_rcMonitor.right-dx, m_rcMonitor.top+cy );
	else if	( m_iCorner == 2 )
		rectClient.SetRect( m_rcMonitor.right-cx, m_rcMonitor.top,       m_rcMonitor.right,    m_rcMonitor.top+cy );
	else if	( m_iCorner == 3 )
		rectClient.SetRect( m_rcMonitor.left,     m_rcMonitor.bottom-cy, m_rcMonitor.left +cx, m_rcMonitor.bottom );
	else if	( m_iCorner == 4 )
		rectClient.SetRect( m_rcMonitor.left +dx, m_rcMonitor.bottom-cy, m_rcMonitor.right-dx, m_rcMonitor.bottom );
	else if	( m_iCorner == 5 )
		rectClient.SetRect( m_rcMonitor.right-cx, m_rcMonitor.bottom-cy, m_rcMonitor.right,    m_rcMonitor.bottom );

	SetWindowPos( &CWnd::wndTop, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE );
}

	typedef	struct{
		int	iMonitor;
		int	nMonitor;
		RECT	rcMonitor;
	}	MonitorArg;

void
CBulletinDlg::SelectMonitor( bool bSelect )
{
	MonitorArg	arg = { 0 };

	arg.iMonitor = bSelect? m_iMonitor: -1;

	EnumDisplayMonitors( NULL, NULL, OnEnumDisplay, (LPARAM)&arg );

	if	( bSelect )
		m_rcMonitor = arg.rcMonitor;
	else
		m_nMonitor  = arg.nMonitor;
}

BOOL	CALLBACK
CBulletinDlg::OnEnumDisplay( HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData )
{
	MonitorArg*	pArg = (MonitorArg*)dwData;
	
	MONITORINFO	mi = { sizeof( mi ) };
	GetMonitorInfo( hMonitor, &mi );
	pArg->rcMonitor = mi.rcWork;

	if	( pArg->nMonitor == pArg->iMonitor ){
		return	FALSE;
	}
	else{
		pArg->nMonitor++;
		return	TRUE;
	}
}

void
CBulletinDlg::GetProfileFonts( void )
{
	CBulletinApp*	pApp = (CBulletinApp*)AfxGetApp();

	m_cyFont = pApp->GetProfileInt( _T("Font"), _T("Height"), 14 );

	static	TCHAR*	apchProfile[] = { _T("Bulletin"), _T("List") };

	for	( int i = 0; i < _countof( m_aFont ); i++ ){
		CString	strFont;
		TCHAR*	pch;
		int	iFamily;

		LOGFONT	lf = { 0 };
		lf.lfHeight  = ( i == 0 )? ( m_nDot >= 32 )? m_nDot-2: m_nDot: m_cyFont;
		lf.lfQuality = ( i == 0 )? NONANTIALIASED_QUALITY: DEFAULT_QUALITY;

		strFont = pApp->GetProfileString( _T("Font"), apchProfile[i], NULL );
		if	( !strFont.IsEmpty() ){
			iFamily = strtol( strFont.GetBuffer(), &pch, 10 );
			pch++;
			strcpy_s( lf.lfFaceName, pch );
			lf.lfCharSet = iFamily;
			m_aFont[i].DeleteObject();
			if	( m_aFont[i].CreateFontIndirect( &lf ) ){
				m_aFont1st[i].CreateFontIndirect( &lf );
				continue;
			}
		}

		CString strDefault;
		(void)strDefault.LoadString( IDS_FONT_BULLETIN+i );
		for	( ;; ){
			int	x = strDefault.Find( _T("\n") );
			strFont = strDefault.Left( x );

			iFamily = strtol( strFont.GetBuffer(), &pch, 10 );
			pch++;
			strcpy_s( lf.lfFaceName, pch );
			lf.lfCharSet = iFamily;
			m_aFont[i].DeleteObject();
			if	( m_aFont[i].CreateFontIndirect( &lf ) ){
				m_aFont1st[i].CreateFontIndirect( &lf );
				break;
			}

			strDefault.Delete( 0, x+1 );
		}	
	}
}

void
CBulletinDlg::WriteProfileFonts( void )
{
	CBulletinApp*	pApp = (CBulletinApp*)AfxGetApp();

	static	TCHAR*	apchProfile[] = { _T("Bulletin"), _T("List") };

	for	( int i = 0; i < _countof( m_aFont ); i++ ){
		LOGFONT	lf = { 0 };
		LOGFONT	lf1st = { 0 };
		m_aFont   [i].GetLogFont( &lf );
		m_aFont1st[i].GetLogFont( &lf1st );
		if	( ( lf.lfCharSet != lf1st.lfCharSet ) ||
			  strcmp( lf.lfFaceName, lf1st.lfFaceName ) ){
			CString	strFont;
			strFont.Format( _T("%d,%s"), lf.lfCharSet, lf.lfFaceName );
			pApp->WriteProfileString( _T("Font"), apchProfile[i], strFont.GetBuffer() );
		}
	}
}

void
CBulletinDlg::GetProfileSources( void )
{
	CBulletinApp*	pApp = (CBulletinApp*)AfxGetApp();

	m_aSource.RemoveAll();

	for	( int i = 0; ; i++ ){
		CString	strName, strValue;
		strName.Format( _T("URL%d"), i+1 );
		strValue = pApp->GetProfileString( _T("News"), strName, NULL );
		if	( strValue.IsEmpty() )
			break;

		CSource	source;
		TCHAR*	pch = strValue.GetBuffer();
		source.m_bRead = strtol( pch, &pch, 10 );
		pch++;
		source.m_bDesc = strtol( pch, &pch, 10 );
		pch++;
		source.m_bSrc  = strtol( pch, &pch, 10 );
		pch++;
		source.m_strURL = pch;
		int	x = source.m_strURL.Find( '|' );
		if	( x >= 0 ){
			source.m_strAlias = source.m_strURL.Mid( x+1 );
			source.m_strURL   = source.m_strURL.Left( x );
		}
		m_aSource.Add( source );
	}
}

void
CBulletinDlg::WriteProfileSources( void )
{
	CBulletinApp*	pApp = (CBulletinApp*)AfxGetApp();

	int	n = (int)m_aSource.GetCount();
	for	( int i = 0; ; i++ ){
		CString	strName, strValue;
		strName.Format( _T("URL%d"), i+1 );
		BOOL	bIn = pApp->DeleteProfileValue( _T("News"), strName );

		if	( i < n ){
			strValue.Format( _T("%d,%d,%d:%s"),
				m_aSource[i].m_bRead, m_aSource[i].m_bDesc, m_aSource[i].m_bSrc, m_aSource[i].m_strURL.GetBuffer() );
			if	( !m_aSource[i].m_strAlias.IsEmpty() ){
				strValue += _T("|");
				strValue += m_aSource[i].m_strAlias;
			}
			pApp->WriteProfileString( _T("News"), strName, strValue );
		}
		else
			if	( bIn )
				;
			else
				break;
	}
}

void
CBulletinDlg::ReloadNews( void )
{
	m_xOffset = m_cxText = 0;
	m_bRepeat = false;
	m_iNews = m_aNews.GetUpperBound();
	m_tNews = 0;

	Invalidate( TRUE );

	ScrollBulletin();
}

void
CBulletinDlg::ReadNews( void )
{
	m_bUpdating = true;

	m_aNews.RemoveAll();
	int	n = (int)m_aSource.GetCount();
	if	( n ){
		int	iIndex = 0;
		for	( int i = 0; i < n; i++ )
			ReadNewsSite( i, iIndex );
		SortNews( m_aNews, m_iNewsKey, m_iNewsOrder );
	}

	m_iNews   = m_aNews.GetCount();
	m_cxText  = 0;
	m_xOffset = 0;
	Invalidate( TRUE );

	if	( !m_iNews ){
		long	lBias;
		{
			TIME_ZONE_INFORMATION	tzi;
			GetTimeZoneInformation( &tzi );
			lBias = tzi.Bias;
		}
		CNews	news;
		news.m_tPub = CTime::GetCurrentTime() + CTimeSpan( 0, 0, lBias, 0 );
		news.m_iIndex = 0;
		news.m_iSource = -1;
		(void)news.m_strSrc  .LoadString( IDS_EMPTY_SRC );
		(void)news.m_strTitle.LoadString( IDS_EMPTY_TITLE );
		(void)news.m_strDesc .LoadString( IDS_EMPTY_DESC );
		(void)news.m_strLink .LoadString( IDS_EMPTY_LINK );
		m_aNews.Add( news );
	}

	m_bUpdating = false;
}

void
CBulletinDlg::ReadNewsSite( int iSource, int& iIndex )
{
	CSource&	source = m_aSource[iSource];
	if	( !source.m_bRead )
		return;

	// Load the RSS.

	CString	strText = LoadWeb( source.m_strURL );

	// Add the news source.

	{
		CString	strSrc;
		int	x = strText.Find( _T("<title>") );
		strSrc = strText.Mid( x+7 );
		x = strSrc.Find( _T("</title>") );
		strSrc = strSrc.Left( x );
		DeCdata( strSrc );
		source.m_strSrc = strSrc;
	}

	// Pick up a description or a title of the items.

	for	( ;; ){
		int	x;
		CNews	news;

		news.m_strSrc = source.m_strSrc;
		news.m_iSource = iSource;

		x = strText.Find( _T("<item") );
		if	( x < 0 )
			break;
		if	( strText.Mid( x, 6 ) == _T("<items" ) ){
			strText = strText.Mid( x+6 );
			continue;
		}
		strText = strText.Mid( x+5 );
		x = strText.Find( _T("</item>") );
		CString	strItem = strText.Left( x+7 );
		strText = strText.Mid( x+7 );

		x = strItem.Find( _T("<title>") );
		if	( x < 0 )
			break;

		CString	strTitle = strItem.Mid( x+7 );
		x = strTitle.Find( _T("</title>") );
		strTitle = strTitle.Left( x );
		DeCdata( strTitle );
		news.m_strTitle = strTitle;

		x = strItem.Find( _T("<description>") );
		if	( x >= 0 ){
			CString	strDesc = strItem.Mid( x+13 );
			x = strDesc.Find( _T("</description>") );
			strDesc = strDesc.Left( x );
			if	( !DeCdata( strDesc ) ){
				x = strDesc.Find( _T("&lt;") );
				if	( x >= 0 )
					strDesc = strDesc.Left( x );
			}
			news.m_strDesc = strDesc;
		}

		CString	strLink;
		x = strItem.Find( _T("<link>") );
		if	( x >= 0 ){
			strLink = strItem.Mid( x+6 );
			x = strLink.Find( _T("</link>") );
			strLink = strLink.Left( x );
			DeCdata( strLink );
			news.m_strLink = strLink;
		}

		do{
			if	( ( x = strItem.Find( _T("<pubDate>") ) ) >= 0 )
				break;
			else if	( ( x = strItem.Find( _T("<dc:date>") ) ) >= 0 )
				break;
		}while	( 0 );
		if	( x >= 0 ){
			CString	strDate = strItem.Mid( x );
			x = strDate.Find( _T(">" ) );
			x = strDate.Find( _T("<" ), x+1 );
			strDate = strDate.Left( x );
			news.m_tPub = TakeDate( strDate );
		}

		news.m_iIndex = iIndex++;
		m_aNews.Add( news );
	}

	m_tNews = CTime::GetCurrentTime();
}

CTime
CBulletinDlg::TakeDate( CString strDate )
{
	int	nYear   = 0,
		nMonth  = 0,
		nDay    = 0,
		nHour   = 0,
		nMinute = 0,
		nSecond = 0;
	int	nBias   = 0;
	int	x;

	if	( strDate.Left( 9 ) == _T("<pubDate>") ){
		TCHAR*	apchMonth[] = { _T("Jan"), _T("Feb"), _T("Mar"), _T("Apr"), _T("May"), _T("Jun"),
					_T("Jul"), _T("Aug"), _T("Sep"), _T("Oct"), _T("Nov"), _T("Dec"), NULL };
		strDate = strDate.Mid( 9 );
		x = strDate.FindOneOf( _T("0123456789") );
		strDate = strDate.Mid( x );
		nDay = atoi( strDate.GetBuffer() );
		x = strDate.Find( _T(" ") );
		strDate = strDate.Mid( x+1 );
		x = strDate.Find( _T(" ") );
		CString	strMonth = strDate.Left( x );
		for	( int i = 0; apchMonth[i]; i++ )
			if	( strMonth == apchMonth[i] ){
				nMonth = i+1;
				break;
			}
		strDate = strDate.Mid( x+1 );
		nYear = atoi( strDate.GetBuffer() );
		x = strDate.Find( _T(" ") );

		strDate = strDate.Mid( x+1 );
		nHour = atoi( strDate.GetBuffer() );
		x = strDate.Find( _T(":") );
		strDate = strDate.Mid( x+1 );
		nMinute = atoi( strDate.GetBuffer() );
		x = strDate.Find( _T(":") );
		strDate = strDate.Mid( x+1 );
		nSecond = atoi( strDate.GetBuffer() );
		x = strDate.Find( _T(" ") );
		strDate = strDate.Mid( x+1 );

		if	( strDate[0] == '+' )
			nBias = -atoi( strDate.Mid( 1 ).GetBuffer() );
		else if	( strDate[0] == '-' )
			nBias = +atoi( strDate.Mid( 1 ).GetBuffer() );
		else if	( !strDate.CompareNoCase( _T("GMT") ) )
			nBias = 0;
		else
			TRACE( _T("Unknown timezone '%s'\n"), strDate );
	}
	else if	( strDate.Left( 9 ) == _T("<dc:date>") ){	// Dublin Core
		strDate = strDate.Mid( 9 );
		nYear = atoi( strDate.GetBuffer() );
		x = strDate.Find( _T("-") );
		strDate = strDate.Mid( x+1 );
		nMonth = atoi( strDate.GetBuffer() );
		x = strDate.Find( _T("-") );
		strDate = strDate.Mid( x+1 );
		nDay = atoi( strDate.GetBuffer() );

		x = strDate.Find( _T("T") );
		strDate = strDate.Mid( x+1 );
		nHour = atoi( strDate.GetBuffer() );
		x = strDate.Find( _T(":") );
		strDate = strDate.Mid( x+1 );
		nMinute = atoi( strDate.GetBuffer() );
		x = strDate.Find( _T(":") );
		strDate = strDate.Mid( x+1 );
		nSecond = atoi( strDate.GetBuffer() );
		strDate = strDate.Mid( 2 );

		if	( strDate[0] == '+' ){
			nBias = -atoi( strDate.Mid( 1 ).GetBuffer() );
			nBias *= 100;
			nBias -= atoi( strDate.Mid( 4 ).GetBuffer() );
		}
		else if	( strDate[0] == '-' ){
			nBias = +atoi( strDate.Mid( 1 ).GetBuffer() );
			nBias *= 100;
			nBias += atoi( strDate.Mid( 4 ).GetBuffer() );
		}
		else
			TRACE( _T("Unknown timezone '%s'\n"), strDate );
	}
	else
		return	CTime( 0 );

	CTimeSpan	tsBias( 0, nBias/100, nBias%100, 0 );
	CTime	tDate( nYear, nMonth, nDay, nHour, nMinute, nSecond );
	tDate += tsBias;

	return	tDate;
}

bool
CBulletinDlg::DeCdata( CString& strText )
{
	CString	strTag = _T("<![CDATA[");
	int	x = strText.Find( strTag );
	if	( x >= 0 ){
		strText = strText.Mid( x+strTag.GetLength() );
		x = strText.Find( _T("]]>") );
		strText = strText.Left( x );
		return	true;
	}
	else
		return	false;
}

int
CBulletinDlg::CompareNews( void* pContext, const void *pElem1, const void *pElem2 )
{
	CNews*	p1 = (CNews*)pElem1;
	CNews*	p2 = (CNews*)pElem2;
	int*	piContext = (int*)pContext;
	int	iKey   = piContext[0];
	int	iOrder = piContext[1];

	// Sort by published date:

	if	( iKey == 1 ){
		if	( iOrder == 1 )
			return	( p1->m_tPub < p2->m_tPub )? -1: +1;
		else if	( iOrder == 2 )
			return	( p1->m_tPub > p2->m_tPub )? -1: +1;
	}

	// Sort by title:

	else if	( iKey == 2 ){
		if	( iOrder == 1 )
			return	( p1->m_strTitle < p2->m_strTitle )? -1: +1;
		else if	( iOrder == 2 )
			return	( p1->m_strTitle > p2->m_strTitle )? -1: +1;
	}

	// Sort by the original order:

	{
		if	( iOrder == 0 )
			return	( p1->m_iIndex < p2->m_iIndex )? -1: +1;
		else
			return	( p1->m_iIndex > p2->m_iIndex )? -1: +1;
	}
}

void
CBulletinDlg::GetProfileDaily( void )
{
	CBulletinApp*	pApp = (CBulletinApp*)AfxGetApp();

	m_aFile.RemoveAll();

	for	( int iFile = 0; ; iFile++ ){
		CString	strName, strValue;
		strName.Format( _T("File%d"), iFile+1 );
		strValue = pApp->GetProfileString( _T("Daily"), strName, NULL );
		if	( strValue.IsEmpty() )
			break;

		CDailyFile	daily;
		TCHAR*	pch = strValue.GetBuffer();
		daily.m_bRead = strtol( pch, &pch, 10 );
		pch++;
		daily.m_strFile = pch;
		CFileStatus	fs;
		if	( CFile::GetStatus( daily.m_strFile, fs ) )
			daily.m_tLast = fs.m_mtime;

		m_aFile.Add( daily );
	}
}

void
CBulletinDlg::WriteProfileDaily( void )
{
	CBulletinApp*	pApp = (CBulletinApp*)AfxGetApp();

	int	n = (int)m_aFile.GetCount();
	for	( int i = 0; ; i++ ){
		CString	strName, strValue;
		strName.Format( _T("File%d"), i+1 );
		BOOL	bIn = pApp->DeleteProfileValue( _T("Daily"), strName );

		if	( i < n ){
			strValue.Format( _T("%d,%s"),
				m_aFile[i].m_bRead, m_aFile[i].m_strFile.GetBuffer() );
			pApp->WriteProfileString( _T("Daily"), strName, strValue );
		}
		else
			if	( bIn )
				;
			else
				break;
	}
}

void
CBulletinDlg::ReadDaily( void )
{
	m_bUpdating = true;

	m_aDaily.RemoveAll();

	int	n = (int)m_aFile.GetCount();
	for	( int i = 0; i < n; i++ )
		if	( m_aFile[i].m_bRead )
			GetDaily( m_aFile[i].m_strFile, m_aDaily );
	
	qsort( m_aDaily.GetData(), m_aDaily.GetCount(), sizeof( CDaily ), CompareDaily );

	m_iDaily = SeekDaily( CTime::GetCurrentTime(), m_aDaily );

	m_bUpdating = false;
}

int
CBulletinDlg::CompareDaily( const void *pElem1, const void *pElem2 )
{
	CDaily*	p1 = (CDaily*)pElem1;
	CDaily*	p2 = (CDaily*)pElem2;

	if	( p1->m_bHour != p2->m_bHour )
		return	( p1->m_bHour   < p2->m_bHour   )? -1: +1;

	else if	( p1->m_bMinute != p2->m_bMinute )
		return	( p1->m_bMinute < p2->m_bMinute )? -1: +1;

	else if	( p1->m_bSecond != p2->m_bSecond )
		return	( p1->m_bSecond < p2->m_bSecond )? -1: +1;

	else
		return	( p1->m_strText < p2->m_strText )? -1: +1;
}

void
CBulletinDlg::ScrollBulletin( void )
{
	if	( m_bUpdating )
		return;

	CTime	tNow = CTime::GetCurrentTime();

	// Turn to draw the next daily item:

	if	( DrawDaily( tNow ) )
		;

	// Turn to shift the current text:

	else if	( m_xOffset < m_cxText ){
		int	cxScroll = m_nScrollPix * m_cxDot;
		ScrollWindow( -cxScroll, 0, NULL, NULL );
		m_xOffset += m_nScrollPix;
	}

	// Turn to repeat the same text.
	
	else if	( m_bRepeat )
		m_xOffset = 0;

	// Turn to draw the next news item:

	else
		DrawNews( tNow );

	m_tLast = tNow;
}

bool
CBulletinDlg::DrawDaily( CTime tNow )
{
	if	( m_aDaily.GetCount() <= m_iDaily )
		return	false;

	if	( m_iDaily < 0 )
		return	false;

	if	( tNow.GetHour()   == m_tLast.GetHour()   && 
		  tNow.GetSecond() == m_tLast.GetSecond() &&
		  tNow.GetMinute() == m_tLast.GetMinute()    )
		return	false;

	CDaily&	daily = m_aDaily[m_iDaily];
	int	iDaily = MAKESEC( daily.m_bHour, daily.m_bMinute, daily.m_bSecond );
	int	iNow   = MAKESEC( tNow.GetHour(), tNow.GetMinute(), tNow.GetSecond() );
	if	( iDaily != iNow )
		return	false;

	CString	strDaily = daily.m_strText;
	m_iDaily++;
	if	( m_iDaily >= m_aDaily.GetCount() )
		m_iDaily = 0;
				
	bool	bScroll = true;
	bool	bContinue = false;

	while	( !strDaily.IsEmpty() ){
		if	( strDaily[0] == '!' ){
			strDaily.Delete( 0, 1 );
			bContinue = true;
		}
		else if	( strDaily[0] == '|' ){
			strDaily.Delete( 0, 1 );
			bScroll = false;		
		}
		else if	( strDaily[0] == '#' ){
			CString	strSound = strDaily.MakeLower();
			strSound.Delete( 0, 1 );
			int	x = strSound.Find( _T(".wav") );
			if	( x >= 0 ){
				strSound = strSound.Left( x+4 );
				strDaily.Delete( 0, 1+x+4 );
				strDaily.TrimLeft();
				PlaySound( strSound, NULL, SND_FILENAME | SND_ASYNC );
			}
		}
		else
			break;
	}
	if	( bScroll )
		m_bRepeat = true;
	else{
		m_bRepeat = false;
		SetScroll( false );
	}

	CDaily&	dNext = m_aDaily[m_iDaily];
	int	iNext = MAKESEC( dNext.m_bHour, dNext.m_bMinute, dNext.m_bSecond );
	int	nPause = iNext - iDaily;
	if	( nPause < 0 )
		nPause += 24*60*60;
	if	( bContinue )
		;
	else if	( nPause > 1*60 )
		nPause = 1*60;

	SetTimer( TID_PAUSE, nPause*1000, NULL );
	DrawBulletin( strDaily, bScroll );

	Invalidate( TRUE );

	return	true;
}

bool
CBulletinDlg::DrawNews( CTime tNow )
{
	bool	bRead = false;

	if	( m_iNews >= m_aNews.GetUpperBound() ){
		if	( m_tNews == 0 ){
			Invalidate( TRUE );
			bRead = true;
		}
		else if	( m_tNews.GetHour() != tNow.GetHour() )
			bRead = true;
		if	( bRead )
			ReadNews();
		m_iNews = 0;
	}
	else
		m_iNews++;

	CNews&	news = m_aNews[m_iNews];
	bool	bDesc = false;
	bool	bSrc  = false;
	CString	strSource;
	if	( m_aSource.GetCount() ){
		if	( news.m_iSource >= 0 ){
			CSource& source = m_aSource[news.m_iSource];
			bDesc = source.m_bDesc;
			bSrc  = source.m_bSrc;
			if	( bSrc )
				strSource = source.m_strAlias.IsEmpty()? source.m_strSrc: source.m_strAlias;
		}
		else{
			strSource = news.m_strSrc;
			bDesc = false;
			bSrc  = true;
		}
	}
	else{
		bDesc = false;
		bSrc  = true;
	}

	CString	strText;
	if	( bSrc )
		strText.Format( _T("\x25c7%s\x25c7"), strSource.GetBuffer() );
	strText += bDesc? news.m_strDesc: news.m_strTitle;

	DrawBulletin( strText, true );

	return	bRead;
}

void
CBulletinDlg::DrawBulletin( CString strText, bool bScroll )
{
	BITMAP	bm = { 0 };
	m_bmMem.GetBitmap( &bm );

	CRect	rectMem;
	rectMem.SetRect( 0, 0, bm.bmWidth, bm.bmHeight );
	m_dcMem.FillSolidRect( &rectMem, m_crBack );
	m_dcMem.SetTextColor( m_crFore );

	CRect	rectClient;
	GetClientRect( &rectClient );

	CUIntArray	iaTag;
	CString	strDetagged = DeTag( strText, iaTag );
	CSize	size = m_dcMem.GetTextExtent( strDetagged );

	int	xMargin = rectClient.Width() / m_cxDot;
	if	( !bScroll ){
		xMargin -= size.cx;
		xMargin /= 2;
	}
	rectMem.left += xMargin;

	DrawTagged( &m_dcMem, rectMem, strText, iaTag );

	m_cxText = xMargin + size.cx;
	m_xOffset = 0;
}

CString
CBulletinDlg::DeTag( CString strTagged, CUIntArray& iaTag )
{
	CString	strDetagged;
	iaTag.RemoveAll();

	int	x = 0;
	for	( ;; ){
		int	xOpen = strTagged.Find( '{', x );
		if	( xOpen < 0 ){
			strDetagged += strTagged.Mid( x );
			break;
		}
		else{
			iaTag.Add( xOpen );
			int	xClose = strTagged.Find( '}', xOpen );
			if	( xClose < 0 ){
				xClose = strTagged.GetLength()-1;
				break;
			}
			else{
				strDetagged += strTagged.Mid( x, xOpen-x );
				xOpen += 1;
				int	cchTag = 1;
				if	( strTagged[xOpen] == '#' )
					cchTag = 6;
				else if	( strTagged[xOpen] == '-' )
					cchTag = 1;
				else if	( strTagged[xOpen] == '+' )
					cchTag = 1;
				else
					return	_T("");

				xOpen += cchTag;
				strDetagged += strTagged.Mid( xOpen, xClose-xOpen );
				x = xClose+1;
			}
			iaTag.Add( xClose );
		}
	}

	return	strDetagged;
}

void
CBulletinDlg::DrawTagged( CDC* pDC, CRect rect, CString strTagged, CUIntArray& iaTag )
{
	UINT	uFormat = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP | DT_NOPREFIX;

	CString	str = strTagged;
	int	xPre = 0;
	int	nTag = (int)iaTag.GetCount();
	for	( int iTag = 0; iTag < nTag; ){
		// Draw text in front of a tag.

		str = strTagged.Left( iaTag[iTag] );
		str.Delete( 0, xPre );
		CSize	size = pDC->GetTextExtent( str );
		pDC->DrawText( str, &rect, uFormat );
		rect.left += size.cx;

		// Decorate the text according to the tag.

		bool	bColor = false;
		bool	bReverse = false;
		int	xFrom = iaTag[iTag++]+1;
		int	xTo   = iaTag[iTag++];

		if	( strTagged[xFrom] == '#' ){
			xFrom++;
			CString	strColor = strTagged.Mid( xFrom, 6 );
			COLORREF	cr = strtol( strColor, NULL, 16 );
			pDC->SetTextColor( cr );
			xFrom += 6;
			bColor = true;
		}
		else if	( strTagged[xFrom] == '+' ){
			xFrom++;
			pDC->SetTextColor( m_crPlus );
			bColor = true;
		}
		else if	( strTagged[xFrom] == '-' ){
			xFrom++;
			bReverse = true;
		}

		str = strTagged.Mid( xFrom, xTo-xFrom );
		size = pDC->GetTextExtent( str );
		rect.right = rect.left + size.cx;

		if	( bReverse ){
			pDC->SetTextColor( m_crBack );
			pDC->FillSolidRect( &rect, m_crFore );
			bColor = true;
		}
		
		// Draw text between tags.

		pDC->DrawText( str, &rect, uFormat );

		if	( bColor )
			pDC->SetTextColor( m_crFore );

		// Go advance to the next text.

		rect.left += size.cx;
		xPre = xTo+1;
		str = strTagged.Mid( xPre );
	}
	if	( !str.IsEmpty() )
		pDC->DrawText( str, &rect, uFormat );
}

void
CBulletinDlg::Draw( CDC* pDC, CRect rectUpdate )
{
	int	cxUpdate = rectUpdate.Width();
	int	cxBitmap = cxUpdate/m_cxDot;
	int	cyUpdate = m_nDot*m_cxDot;
	int	cyBitmap = m_nDot;
	int	xUpdate  = rectUpdate.left;
	int	xBitmap  = xUpdate/m_cxDot;

	pDC->StretchBlt( xUpdate, 0, cxUpdate, cyUpdate, &m_dcMem, xBitmap+m_xOffset, 0, cxBitmap, cyBitmap, SRCCOPY );

	DrawGrid( pDC, rectUpdate );
}

void
CBulletinDlg::DrawGrid( CDC* pDC, CRect rectUpdate )
{
	CRect	rectClient;
	GetClientRect( &rectClient );

	int	nPen = m_cxDot / 3;
	if	( nPen ){
		CPen	pen( PS_SOLID, nPen, RGB( 0, 0, 0 ) );
		pDC->SelectObject( &pen );
		int	xUpdate = rectUpdate.left;
		for	( int x = xUpdate; x < rectClient.right;  x += m_cxDot ){
			pDC->MoveTo( x, 0 );
			pDC->LineTo( x, rectClient.bottom );
		}
		for	( int y = 0; y < rectClient.bottom; y += m_cxDot ){
			pDC->MoveTo( xUpdate,          y );
			pDC->LineTo( rectClient.right, y );
		}
	}
}

#include <wininet.h>
#define	URLSIZE	2048

#define	CP_UNKNOWN	-1
#define	CP_SHIFT_JIS	932
#define	CP_UTF16_LE	1200
#define	CP_UTF16_BE	1201
#define	CP_UTF32_LE	12000
#define	CP_UTF32_BE	12001
#define	CP_ASCII	20127

CString
CBulletinDlg::LoadWeb( CString strURL )
{
	// Try to open as a web page.

	TCHAR*	pchHost = new	TCHAR[URLSIZE];
	TCHAR*	pchPath = new	TCHAR[URLSIZE];

	URL_COMPONENTS	url = { 0 };
	url.dwStructSize = sizeof( url );
	url.lpszHostName = pchHost;
	url.dwHostNameLength = URLSIZE;
	url.lpszUrlPath  = pchPath;
	url.dwUrlPathLength  = URLSIZE;

	DWORD	dwFlags = 0;

	BOOL	b = InternetCrackUrl( strURL, strURL.GetLength(), 0, &url );
	if	( b ){
		dwFlags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE;
		if	( url.nScheme == INTERNET_SCHEME_HTTP )
			;
		else if	( url.nScheme == INTERNET_SCHEME_HTTPS )
			dwFlags |= INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_IGNORE_CERT_CN_INVALID;
		else
			b = FALSE;
	}

	char*	pchLines = NULL;
	if	( b ){
		HINTERNET	hInternet = NULL,
				hConnect  = NULL,
				hRequest  = NULL;

		do{
			CString	strAppName = AfxGetApp()->m_pszAppName;
			hInternet = InternetOpen( strAppName, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0 );
			if	( !hInternet )
				break;

			hConnect = InternetConnect( hInternet, url.lpszHostName, url.nPort, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0 );
			if	( !hConnect )
				break;

			hRequest = HttpOpenRequest( hConnect, _T("GET"), url.lpszUrlPath, NULL, NULL, NULL, dwFlags, NULL );
			if	( !hRequest )
				break;

			if	( !HttpSendRequest( hRequest, _T(""), 0, NULL, 0 ) )
				break;

			DWORD	dwStatus = 0;
			DWORD	dwLength = sizeof( dwStatus );
			if	( !HttpQueryInfo( hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwStatus, &dwLength, 0 ) )
				break;

			if	( dwStatus != HTTP_STATUS_OK ){
				TRACE( _T("%d: %s\n"), dwStatus, strURL );
				break;
			}

			DWORD	cbLine = 0;

			BYTE*	pbContents = new	BYTE[URLSIZE+1];
			for	( ;; ){
				dwLength = 0;
				if	( !InternetReadFile( hRequest, pbContents, URLSIZE, &dwLength ) )
					break;
				else if	( !dwLength )
					break;
				else{
					pbContents[dwLength] = '\0';
					char*	pchNew = (char*)realloc( pchLines, (size_t)( cbLine+dwLength+1 ) );
					if	( pchNew ){
						pchLines = pchNew;
						memcpy( pchLines+cbLine, pbContents, (size_t)( dwLength+1 ) );
					}
					cbLine += dwLength;
				}
			}
			delete[]	pbContents;
		}while	( 0 );

		if	( hInternet )
			InternetCloseHandle( hInternet );
		if	( hConnect )
			InternetCloseHandle( hConnect );
		if	( hRequest )
			InternetCloseHandle( hRequest );
	}
	delete[]	pchHost;
	delete[]	pchPath;

	// Not on the web, then try as a file.

	if	( !b ){
		CFileStatus	fs;
		if	( CFile::GetStatus( strURL, fs ) ){
			CFile	f;
			if	( f.Open( strURL, CFile::modeRead | CFile::shareDenyNone ) ){
				pchLines = (char*)malloc( fs.m_size );
				f.Read( pchLines, (DWORD)fs.m_size );
			}
		}
	}

	// Decode it.

	CString	strLines;

	if	( pchLines ){
		int	iCodePage = CP_UNKNOWN;
		CString	strEncode;
		{
			char*		pch = (char*)pchLines;
			CStringA	str = pch;
			int	x = str.Find( "encoding=" );
			if	( x >= 0 ){
				str = str.Mid( x+9 );
				if	( str[0] == '"' ){
					str.Delete( 0, 1 );
					x = str.Find( '"' );
					str = str.Left( x );
				}
				else{
					x = str.Find( '\r' );
					str = str.Left( x );
				}
				strEncode = str;
			}
		}
		if	( !strEncode.CompareNoCase( _T("UTF-8" ) ) )
			iCodePage = CP_UTF8;
		else if	( !strEncode.CompareNoCase( _T("Shift_JIS" ) ) )
			iCodePage = CP_SHIFT_JIS;

		if	( iCodePage != CP_UNKNOWN ){
			int	cwch =
			MultiByteToWideChar( iCodePage, 0, pchLines, -1, NULL, 0 );
			WCHAR*	pwch = new WCHAR[cwch+1];
			MultiByteToWideChar( iCodePage, 0, pchLines, -1, pwch, cwch );
#ifdef	UNICODE
			strLines = pwch;
#else //UNICODE
			int	cmch =
			WideCharToMultiByte( CP_ACP, 0, pwch,     -1, NULL,    0, NULL, NULL );
			TCHAR*	pmch = new TCHAR[cmch+1];
			WideCharToMultiByte( CP_ACP, 0, pwch, cwch+1, pmch, cmch, NULL, NULL );
			strLines = pmch;
			delete[]	pmch;
#endif//UNICODE
			delete[]	pwch;
		}
		free( pchLines );
	}

	return	strLines;
}

CString
CBulletinDlg::LoadText( CString strFile )
{
	CString	strLines;

	CFileStatus	fs;
	if	( CFile::GetStatus( strFile, fs ) ){
		DWORD	cbData = (DWORD)fs.m_size;
		BYTE*	pbData = new BYTE[cbData+1];

		CFile	f;
		if	( f.Open( strFile, CFile::modeRead | CFile::shareDenyNone ) ){
			f.Read( pbData, cbData );
			pbData[cbData] = 0x00;

			int	cbBOM;
			int	iCodePage = GetTextEncode( pbData, cbData, cbBOM );

			if	( iCodePage == CP_UTF16_LE ){
				strLines = (TCHAR*)( pbData+cbBOM );
			}
			else if	( iCodePage == CP_UTF16_BE ){
				BYTE*	pbSwapped = new BYTE[cbData];
				_swab( (char*)( pbData+cbBOM ), (char*)pbSwapped, cbData-cbBOM );
				pbSwapped[cbData-cbBOM+0] = 0x00;
				pbSwapped[cbData-cbBOM+1] = 0x00;
				strLines = (TCHAR*)pbSwapped;
				delete[]	pbSwapped;
			}
			else if	( iCodePage != CP_UNKNOWN ){
				int	cwch =
				::MultiByteToWideChar( iCodePage, 0, (CHAR*)pbData+cbBOM, -1, NULL, 0 );
				WCHAR*	pwch = new WCHAR[cwch+2];
				::MultiByteToWideChar( iCodePage, 0, (CHAR*)pbData+cbBOM, -1, pwch, cwch );
				strLines = pwch;
				delete[]   pwch;
			}
		}

		delete[]	pbData;
	}

	return	strLines;
}

int
CBulletinDlg::GetTextEncode( BYTE* pbData, QWORD cbData, int& cbBOM )
{
	int	iCodePage = CP_UNKNOWN;
	cbBOM = 0;

	BYTE*	pb = pbData;

	// If the data is empty, encoding is unknown.

	if	( cbData == 0 )
		;

	// If the BOM exists, encoding is written in BOM.

	else if	( pb[0] == 0xef && pb[1] == 0xbb && pb[2] == 0xbf ){
		cbBOM = 3;
		iCodePage = CP_UTF8;
	}
	else if	( pb[0] == 0xff && pb[1] == 0xfe ){
		cbBOM = 2;
		iCodePage = CP_UTF16_LE;
	}
	else if	( pb[0] == 0xfe && pb[1] == 0xff ){
		cbBOM = 2;
		iCodePage = CP_UTF16_BE;
	}
	else if	( pb[0] == 0x00 && pb[1] == 0x00 && pb[2] == 0xfe && pb[3] == 0xff ){
		cbBOM = 4;
		iCodePage = CP_UTF32_BE;
	}
	else if	( pb[0] == 0xff && pb[1] == 0xfe && pb[2] == 0x00 && pb[3] == 0x00 ){
		cbBOM = 4;
		iCodePage = CP_UTF32_LE;
	}

	// If there's no BOM, judge encoding from some data.

	else{
		UINT	aiEncode[16] = {};
		int	nEncode = 0;

		for	( QWORD cb = cbData; cb > 0; ){
			if	( *pb < 0x7f ){
				cb--;
				pb++;
			}
			else if	( pb[0] >= 0xc2 && pb[0] <= 0xdf &&
				  pb[1] >= 0x80 && pb[1] <= 0xbf ){
				aiEncode[nEncode++] = CP_UTF8;	// 11bit code
				cb -= 2;
				pb += 2;
			}
			else if	( pb[0] >= 0xe0 && pb[0] <= 0xef &&
				  pb[1] >= 0x80 && pb[1] <= 0xbf &&
				  pb[2] >= 0x80 && pb[2] <= 0xbf ){
				aiEncode[nEncode++] = CP_UTF8;	// 16bit code
				cb -= 3;
				pb += 3;
			}
			else if	( pb[0] >= 0xf0 && pb[0] <= 0xf4 &&
				  pb[1] >= 0x80 && pb[1] <= 0xbf &&
				  pb[2] >= 0x80 && pb[2] <= 0xbf &&
				  pb[3] >= 0x80 && pb[3] <= 0xbf ){
				aiEncode[nEncode++] = CP_UTF8;	// 21bit code
				cb -= 4;
				pb += 4;
			}
			else if	( ( ( pb[0] >= 0x81 && pb[0] <= 0x9f ) ||
				    ( pb[0] >= 0xe0 && pb[0] <= 0xef )    ) &&
				  ( ( pb[1] >= 0x40 && pb[1] <= 0x7e ) ||
				    ( pb[0] >= 0x80 && pb[0] <= 0xfc )    )    ){
				aiEncode[nEncode++] = CP_SHIFT_JIS;
				cb -= 2;
				pb += 2;
			}
			else{
				aiEncode[nEncode++] = CP_UNKNOWN;
				break;
			}
			if	( nEncode >= _countof( aiEncode ) )
				break;
		}
		{
			int	i;
			for	( i = 1; i < nEncode; i++ )
				if	( aiEncode[i] != aiEncode[0] )
					break;	
			iCodePage =	( nEncode == 0 )?	CP_ACP:
					( i >= nEncode )?	aiEncode[0]:
								CP_UNKNOWN;
		}
	}

	return	iCodePage;
}

void
CBulletinDlg::SelectScrollSpeed( void )
{
	m_nScrollPix  = ( m_nDot >= 48 )? 2: 1;
	m_nScrollTime = m_nInterval;
	if	( m_nDot <= 16 )
		m_nInterval += 1;
	if	( IsWindow( m_hWnd ) )
		SetScroll( true );
}

void
CBulletinDlg::SetScroll( bool bScroll )
{
	if	( bScroll ){
		if	( m_nPeriod )
			timeBeginPeriod( m_nPeriod );
		m_timer.SetTimer( TID_SCROLL, m_nScrollTime );
	}
	else{
		if	( m_nPeriod )
			timeEndPeriod( m_nPeriod );
		m_timer.KillTimer( TID_SCROLL );
	}
}

void
CBulletinDlg::SetBitmapDC( void )
{
	m_bmMem.DeleteObject();
	m_bmMem.CreateBitmap( CX_MEM, m_nDot, 1, 32, NULL );
	m_dcMem.DeleteDC();
	m_dcMem.CreateCompatibleDC( NULL );
	m_dcMem.SelectObject( &m_bmMem );
	m_dcMem.SelectObject( &m_aFont[0] );
	m_dcMem.SetBkMode( TRANSPARENT );
}
