// FontDlg.cpp : implementation file
//

#include "pch.h"
#include "Resource.h"
#include "BulletinDlg.h"
#include "FontDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef	UNICODE
#define	strcpy_s	wcscpy_s
#define	atoi		_wtoi
#endif//UNICODE

IMPLEMENT_DYNAMIC( CFontDlg, CDialog )

///////////////////////////////////////////////////////////////////////////////////////
// Constructor

CFontDlg::CFontDlg( CWnd* pParent )
	: CDialog( IDD_FONT, pParent)
{
	m_nCharSet = 0;
	m_bPitch   = 0;

	memset( &m_lf, 0, sizeof( m_lf ) );
}

///////////////////////////////////////////////////////////////////////////////////////
// Interface Functions

void
CFontDlg::SetFont( LOGFONT* plf )
{
	m_lf = *plf;
}

void
CFontDlg::GetFont( LOGFONT* plf )
{
	*plf = m_lf;
}

///////////////////////////////////////////////////////////////////////////////////////
// Overridden Functions

BOOL
CFontDlg::OnInitDialog( void )
{
	CDialog::OnInitDialog();

	HICON	hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );
	SetIcon( hIcon, TRUE );
	SetIcon( hIcon, FALSE );

	GetOwner()->PostMessage( WM_POPUP, 0, (LPARAM)this );

	InitControls();
	SetControls();

	return	TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////
// Message Handlers

BEGIN_MESSAGE_MAP( CFontDlg, CDialog )
	ON_WM_SHOWWINDOW()
	ON_WM_SYSCOMMAND()
	ON_CBN_SELCHANGE( IDC_FONT_COMBO_CHARSET, OnSelectCharset )
	ON_CBN_SELCHANGE( IDC_FONT_COMBO_NAME,    OnSelectName )
	ON_BN_CLICKED( IDC_FONT_CHECK_PROPORTIONAL, OnClickWidth )
	ON_BN_CLICKED( IDC_FONT_CHECK_FIXED,        OnClickWidth )
END_MESSAGE_MAP()

void
CFontDlg::OnShowWindow( BOOL bShow, UINT nStatus )
{
	CDialog::OnShowWindow( bShow, nStatus );

	GotoDlgCtrl( GetDlgItem( IDC_FONT_COMBO_NAME ) );
	GetDlgItem( IDC_FONT_COMBO_NAME )->SetFocus();
}

void
CFontDlg::OnSelectCharset( void )
{
	EnumFontGroup();
}

void
CFontDlg::OnSelectName( void )
{
	SeekFontByName();
}

void
CFontDlg::OnClickWidth( void )
{
	EnumFontGroup();
}

///////////////////////////////////////////////////////////////////////////////////////
// Specific Functions

#define	LOGICALDPI	72	// It's seems to be fixed at 72dpi ( not 96dpi ) for the compatibility with Notepad.exe.

void
CFontDlg::InitControls( void )
{
	CComboBox*	pCombo;

	struct{
		TCHAR*	pchName;
		DWORD	dwData;
	}	astCharset[] = {
		{	_T("ANSI"),		ANSI_CHARSET		},
		{	_T("Baltic"),		BALTIC_CHARSET		},
		{	_T("Chinese"),		CHINESEBIG5_CHARSET	},
		{	_T("East Europe"),	EASTEUROPE_CHARSET	},
		{	_T("GB2312"),		GB2312_CHARSET		},
		{	_T("Greek"),		GREEK_CHARSET		},
		{	_T("Hangul"),		HANGUL_CHARSET		},
		{	_T("Mac"),		MAC_CHARSET		},
		{	_T("OEM"),		OEM_CHARSET		},
		{	_T("Russian"),		RUSSIAN_CHARSET		},
		{	_T("Shift JIS"),	SHIFTJIS_CHARSET	},
		{	_T("Symbol"),		SYMBOL_CHARSET		},
		{	_T("Turkish"),		TURKISH_CHARSET		},
		{	_T("Vietnamese"),	VIETNAMESE_CHARSET	},
		{	_T("Johab"),		JOHAB_CHARSET		},
		{	_T("Arabic"),		ARABIC_CHARSET		},
		{	_T("Hebrew"),		HEBREW_CHARSET		},
		{	_T("Thai"),		THAI_CHARSET		},
		{	NULL,			0			}
	};

	pCombo = (CComboBox*)GetDlgItem( IDC_FONT_COMBO_CHARSET );
	for	( int i = 0; astCharset[i].pchName; i++ ){
		pCombo->AddString( astCharset[i].pchName );
		pCombo->SetItemData( i, astCharset[i].dwData );
	}

	GotoDlgCtrl( GetDlgItem( IDC_FONT_COMBO_NAME ) );
}

void
CFontDlg::SetControls( void )
{
	CComboBox*	pCombo;

	pCombo = (CComboBox*)GetDlgItem( IDC_FONT_COMBO_CHARSET );
	int	n = pCombo->GetCount();
	for	( int i = 0; i < n; i++ )
		if	( pCombo->GetItemData( i ) == m_lf.lfCharSet ){
			pCombo->SetCurSel( i );
			break;
		}

	if	( m_lf.lfPitchAndFamily == 0x00 ){
		((CButton*)GetDlgItem( IDC_FONT_CHECK_PROPORTIONAL ))->SetCheck( BST_CHECKED );
		((CButton*)GetDlgItem( IDC_FONT_CHECK_FIXED )       )->SetCheck( BST_CHECKED );
	}
	else{
		bool	b;
		b = m_lf.lfPitchAndFamily & VARIABLE_PITCH;
		((CButton*)GetDlgItem( IDC_FONT_CHECK_PROPORTIONAL ))->SetCheck( b? BST_CHECKED: BST_UNCHECKED );
		b = m_lf.lfPitchAndFamily & FIXED_PITCH;
		((CButton*)GetDlgItem( IDC_FONT_CHECK_FIXED )       )->SetCheck( b? BST_CHECKED: BST_UNCHECKED );
	}

	CString	strFaceName = m_lf.lfFaceName;
	EnumFontGroup();

	pCombo = (CComboBox*)GetDlgItem( IDC_FONT_COMBO_NAME );
	pCombo->SelectString( 0, strFaceName.GetBuffer() );
	SeekFontByName();
}

void
CFontDlg::EnumFontGroup( void )
{
	CComboBox*	pCombo;

	// Specify a 'CharSet'.

	pCombo = (CComboBox*)GetDlgItem( IDC_FONT_COMBO_CHARSET );
	int	iCharSet = pCombo->GetCurSel();
	m_lf.lfCharSet = (BYTE)pCombo->GetItemData( iCharSet );

	// Unspecify 'Font name'.

	m_strFaceName.Empty();
	strcpy_s( m_lf.lfFaceName, m_strFaceName.GetBuffer() );

	// Enumerate the fonts filtered by the condition.

	pCombo = (CComboBox*)GetDlgItem( IDC_FONT_COMBO_NAME );
	pCombo->ResetContent();
	EnumFontFamiliesEx( GetDC()->m_hDC, &m_lf, EnumFontProc, (LPARAM)this, 0 );

	// Select the first 'Font name' as default.

	if	( pCombo->GetCount() > 0 ){
		pCombo->SetCurSel( 0 );
		SeekFontByName();
	}
}

void
CFontDlg::SeekFontByName( void )
{
	// Get the selected 'Font name'.

	CComboBox*	pCombo = (CComboBox*)GetDlgItem( IDC_FONT_COMBO_NAME );
	int	iName = pCombo->GetCurSel();
	if	( iName >= 0 )
		pCombo->GetLBText( iName, m_strFaceName );

	// Apply the font with 'Font name'.

	if	( !m_strFaceName.IsEmpty() ){
		strcpy_s( m_lf.lfFaceName, m_strFaceName.GetBuffer() );
		EnumFontFamiliesEx( GetDC()->m_hDC, &m_lf, EnumFontProc, (LPARAM)this, 0 );
	}
}

int	CALLBACK
CFontDlg::EnumFontProc( const LOGFONT* plf, const TEXTMETRIC* ptm, DWORD dwType, LPARAM lparam )
{
	BOOL	bContinue = TRUE;
	CFontDlg*	pdlg = (CFontDlg*)lparam;

	if	( plf->lfFaceName[0] == '@' )
		;
	else if	( dwType & TRUETYPE_FONTTYPE )
		bContinue = pdlg->OnEnumFont( plf );

	return	bContinue;
}

BOOL
CFontDlg::OnEnumFont( const LOGFONT* plf )
{
	// When the 'Font name' is not specified, collect the fonts for the specified conditions.

	if	( m_strFaceName.IsEmpty() ){
		BYTE	bPitch = 0;
		if	( ((CButton*)GetDlgItem( IDC_FONT_CHECK_PROPORTIONAL ))->GetCheck() == BST_CHECKED )
			bPitch |= VARIABLE_PITCH;
		if	( ((CButton*)GetDlgItem( IDC_FONT_CHECK_FIXED )       )->GetCheck() == BST_CHECKED )
			bPitch |= FIXED_PITCH;
		if	( ( plf->lfPitchAndFamily & bPitch ) )
			((CComboBox*)GetDlgItem( IDC_FONT_COMBO_NAME ))->AddString( plf->lfFaceName );
		return	TRUE;
	}

	// When the 'Font name' is specified, seek the LOGFONT of the specified font and select it.

	else{
		if	( m_strFaceName == plf->lfFaceName ){
			m_strFaceName.Empty();
			int	nHeight = m_lf.lfHeight;
			int	nWidth  = m_lf.lfWidth;
			*(&m_lf) = *plf;
			m_lf.lfHeight = nHeight;
			m_lf.lfWidth  = 0;
			SelectFont();
			return	FALSE;
		}
		else
			return	TRUE;
	}
}

void
CFontDlg::SelectFont( void )
{
	CWnd*	pwnd = GetOwner();
	if	( pwnd )
		pwnd->PostMessage( WM_FONT, 0, (LPARAM)&m_lf );

	CString	str, strProp;
	str =
		( ( m_lf.lfPitchAndFamily & 0x03 ) == FIXED_PITCH )?	"Fixed width":
		( ( m_lf.lfPitchAndFamily & 0x03 ) == VARIABLE_PITCH )?	"Proportional width":
									"Default width";
	strProp += str;
	strProp += " ";
	str =
		( ( m_lf.lfPitchAndFamily & 0xf0 ) == FF_DECORATIVE )?	"decorative":
		( ( m_lf.lfPitchAndFamily & 0xf0 ) == FF_MODERN )?	"without serifs":
		( ( m_lf.lfPitchAndFamily & 0xf0 ) == FF_ROMAN )?	"with serifs":
		( ( m_lf.lfPitchAndFamily & 0xf0 ) == FF_SCRIPT )?	"script":
		( ( m_lf.lfPitchAndFamily & 0xf0 ) == FF_SWISS )?	"without serifs":
									"default";
	strProp += str;

	GetDlgItem( IDC_FONT_STATIC_PROPERTIES )->SetWindowText( strProp );
}
