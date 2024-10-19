// DailyDlg.cpp : implementation file
//

#include "pch.h"
#include "Resource.h"
#include "BulletinDlg.h"
#include "DailyDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef	UNICODE
#define	strcpy		wcscpy
#define	strcpy_s	wcscpy_s
#endif//UNICODE

#define	TID_SELCHANGE	1	// to get the last result of the selection

IMPLEMENT_DYNAMIC( CDailyDlg, CDialog )

///////////////////////////////////////////////////////////////////////////////////////
// Constructor

CDailyDlg::CDailyDlg( CWnd* pParent )
	: CDialog( IDD_DAILY, pParent )
{
}

///////////////////////////////////////////////////////////////////////////////////////
// Interface Functions

void
CDailyDlg::SetFiles( CArray <CDailyFile, CDailyFile&>& aFile )
{
	m_aFile.RemoveAll();
	m_aFile.Append( aFile );
}

void
CDailyDlg::GetFiles( CArray <CDailyFile, CDailyFile&>& aFile )
{
	aFile.RemoveAll();
	aFile.Append( m_aFile );
}

///////////////////////////////////////////////////////////////////////////////////////
// Overridden Functions

BOOL
CDailyDlg::OnInitDialog( void )
{
	CDialog::OnInitDialog();

	HICON	hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );
	SetIcon( hIcon, TRUE );
	SetIcon( hIcon, FALSE );

	GetOwner()->PostMessage( WM_POPUP, 0, (LPARAM)this );

	CListCtrl*	pList = (CListCtrl*)GetDlgItem( IDC_DAILY_LIST_FILES );

	DWORD	dwExStyle = ListView_GetExtendedListViewStyle( pList->m_hWnd );
	dwExStyle |= LVS_EX_FULLROWSELECT;
	dwExStyle |= LVS_EX_GRIDLINES;
	dwExStyle |= LVS_EX_CHECKBOXES;
	ListView_SetExtendedListViewStyle( pList->m_hWnd , dwExStyle );

	DWORD	dwStyle = GetWindowLong( pList->m_hWnd, GWL_STYLE );
	dwStyle |= LVS_NOCOLUMNHEADER;
	SetWindowLong( pList->m_hWnd, GWL_STYLE, dwStyle );

	int	i = 0;
	pList->InsertColumn( i++, _T(""), LVCFMT_RIGHT,  20 );
	pList->InsertColumn( i++, _T(""), LVCFMT_LEFT,  323 );

	pList->DeleteAllItems();
	for	( int i = 0; i < m_aFile.GetCount(); i++ ){
		pList->InsertItem(  i, _T("") );
		pList->SetCheck(    i,    m_aFile[i].m_bRead );
		pList->SetItemText( i, 1, m_aFile[i].m_strFile );
	}
	pList->SetColumnWidth( 1, LVSCW_AUTOSIZE );

	pList = (CListCtrl*)GetDlgItem( IDC_DAILY_LIST_ITEMS );

	dwExStyle = ListView_GetExtendedListViewStyle( pList->m_hWnd );
	dwExStyle |= LVS_EX_FULLROWSELECT;
	dwExStyle |= LVS_EX_GRIDLINES;
	ListView_SetExtendedListViewStyle( pList->m_hWnd , dwExStyle );

	pList->InsertColumn( i++, _T("Start"),   LVCFMT_CENTER, 58 );
	pList->InsertColumn( i++, _T("Message"), LVCFMT_LEFT,  300 );

	CBulletinDlg*	pOwner = (CBulletinDlg*)GetOwner();
	CFont*	pFont = pOwner->GetListFont();

	pList->SetFont( pFont );

	return	TRUE;
}

void
CDailyDlg::OnOK( void )
{
	CListCtrl*	pList = (CListCtrl*)GetDlgItem( IDC_DAILY_LIST_FILES );

	CString	strFile;
	GetDlgItem( IDC_DAILY_EDIT_FILE )->GetWindowText( strFile );

	int	i, n;
	
	if	( !strFile.IsEmpty() ){
		n = pList->GetItemCount();
		for	( i = 0; i < n; i++ )
			if	( pList->GetItemText( i, 1 ) == strFile )
				break;
		if	( i >= n ){
			int	iAnswer = MessageBox( _T("New file is not added. Add it?"), _T("Not Added"), MB_YESNOCANCEL );
			if	( iAnswer == IDCANCEL )
				return;
			if	( iAnswer == IDYES )
				OnButtonAdd();
		}
	}

	n = pList->GetItemCount();
	m_aFile.RemoveAll();
	for	( i = 0; i < n; i++ ){
		CDailyFile	daily;
		daily.m_bRead   = pList->GetCheck( i );
		daily.m_strFile = pList->GetItemText( i, 1 );
		CFileStatus	fs;
		if	( CFile::GetStatus( daily.m_strFile, fs ) )
			daily.m_tLast = fs.m_mtime;
		m_aFile.Add( daily );
	}

	CDialog::OnOK();
}

///////////////////////////////////////////////////////////////////////////////////////
// Message Handlers

BEGIN_MESSAGE_MAP( CDailyDlg, CDialog )
	ON_WM_TIMER()
	ON_EN_CHANGE( IDC_DAILY_EDIT_FILE, OnChangeEditFile )
	ON_BN_CLICKED( IDC_DAILY_BUTTON_REF,    OnButtonRef )
	ON_BN_CLICKED( IDC_DAILY_BUTTON_ADD,    OnButtonAdd )
	ON_BN_CLICKED( IDC_DAILY_BUTTON_DELETE, OnButtonDelete )
	ON_BN_CLICKED( IDC_DAILY_BUTTON_EDIT,   OnButtonEdit )
	ON_NOTIFY( LVN_ITEMCHANGED, IDC_DAILY_LIST_FILES, OnChangeFile )
END_MESSAGE_MAP()

void
CDailyDlg::OnTimer( UINT_PTR nIDEvent )
{
	if	( nIDEvent == TID_SELCHANGE ){
		KillTimer( nIDEvent );
		OnSelectItem();
	}
	else
		CDialog::OnTimer( nIDEvent );
}

void
CDailyDlg::OnChangeEditFile( void )
{
	CString	strFile;
	GetDlgItem( IDC_DAILY_EDIT_FILE )->GetWindowText( strFile );

	GetDaily( strFile );

	if	( strFile.IsEmpty() ){
		((CButton*)GetDlgItem( IDC_DAILY_BUTTON_ADD )   )->EnableWindow( FALSE );
		((CButton*)GetDlgItem( IDC_DAILY_BUTTON_DELETE ))->EnableWindow( FALSE );
		((CButton*)GetDlgItem( IDC_DAILY_BUTTON_EDIT )  )->EnableWindow( FALSE );
	}
	else{
		((CButton*)GetDlgItem( IDC_DAILY_BUTTON_ADD )   )->EnableWindow( TRUE );
		((CButton*)GetDlgItem( IDC_DAILY_BUTTON_DELETE ))->EnableWindow( FALSE );
		((CButton*)GetDlgItem( IDC_DAILY_BUTTON_EDIT )  )->EnableWindow( FALSE );
	}
}

void
CDailyDlg::OnChangeFile( NMHDR* pNMHDR, LRESULT* pResult )
{
//	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>( pNMHDR );
	SetTimer( TID_SELCHANGE, 0, NULL );
	*pResult = 0;
}

void
CDailyDlg::OnButtonRef( void )
{
	CString	strExt = _T("txt");
	TCHAR*	pchFilter = _T("Text file\0*.txt\0Any file\0*.*\0");

	// Open the file dialog.

	TCHAR	szFile[MAX_PATH] = {};

	CFileDialog	dlg( FALSE );
	dlg.m_ofn.lpstrFilter     = pchFilter;
	dlg.m_ofn.lpstrFile       = szFile;
	dlg.m_ofn.lpstrDefExt     = strExt.GetBuffer();
	dlg.m_ofn.lpstrInitialDir = NULL;
	dlg.m_ofn.lpstrTitle      = _T("Open a Daily schedule file");
	dlg.m_ofn.Flags          &= ~OFN_OVERWRITEPROMPT;
	*szFile = '\0';

	// Quit unless OK.

	if	( dlg.DoModal() != IDOK )
		return;

	CString	strFile = dlg.m_ofn.lpstrFile;
	CListCtrl*	pList = (CListCtrl*)GetDlgItem( IDC_DAILY_LIST_FILES );

	int	n = pList->GetItemCount();
	for	( int i = 0; i < n; i++ ){
		CString	strInList = pList->GetItemText( i, 1 );
		if	( strInList == strFile ){
			pList->SetFocus();
			pList->SetItemState( i, LVNI_SELECTED, LVNI_SELECTED );
			return;
		}
	}

	GetDlgItem( IDC_DAILY_EDIT_FILE )->SetWindowText( strFile );
}

void
CDailyDlg::OnButtonAdd( void )
{
	CDailyFile	file;

	GetDlgItem( IDC_DAILY_EDIT_FILE )->GetWindowText( file.m_strFile );

	file.m_bRead = true;
	m_aFile.Add( file );

	CListCtrl*	pList = (CListCtrl*)GetDlgItem( IDC_DAILY_LIST_FILES );
	int	i = pList->GetItemCount();

	pList->InsertItem(  i, _T("") );
	pList->SetCheck(    i,    file.m_bRead );
	pList->SetItemText( i, 1, file.m_strFile );
	pList->SetItemState( i, LVNI_SELECTED, LVNI_SELECTED );

	pList->SetFocus();
	pList->SetColumnWidth( 1, LVSCW_AUTOSIZE );
}

void
CDailyDlg::OnButtonDelete( void )
{
	CListCtrl*	pList = (CListCtrl*)GetDlgItem( IDC_DAILY_LIST_FILES );
	int	i = pList->GetNextItem( -1, LVNI_ALL | LVNI_SELECTED );

	pList->DeleteItem( i );
	m_aFile.RemoveAt( i );
}

void
CDailyDlg::OnButtonEdit( void )
{
	CString	strFile;
	GetDlgItem( IDC_DAILY_EDIT_FILE )->GetWindowText( strFile );

	if	( !strFile.IsEmpty() )
		ShellExecute( NULL, _T("open"), strFile, NULL, NULL, SW_SHOWNORMAL );
}

///////////////////////////////////////////////////////////////////////////////////////
// Specific Functions

void
CDailyDlg::OnSelectItem( void )
{
	CListCtrl*	pList = (CListCtrl*)GetDlgItem( IDC_DAILY_LIST_FILES );
	int	i = pList->GetNextItem( -1, LVNI_ALL | LVNI_SELECTED );
	int	n = (int)m_aFile.GetCount();

	if	( i >= 0 && i < n ){
		CDailyFile&	file = m_aFile[i];
		GetDlgItem( IDC_DAILY_EDIT_FILE )->SetWindowText( file.m_strFile );

		((CButton*)GetDlgItem( IDC_DAILY_BUTTON_ADD )   )->EnableWindow( FALSE );
		((CButton*)GetDlgItem( IDC_DAILY_BUTTON_DELETE ))->EnableWindow( TRUE );
		((CButton*)GetDlgItem( IDC_DAILY_BUTTON_EDIT )  )->EnableWindow( TRUE );
	}
	else{
		GetDlgItem( IDC_DAILY_EDIT_FILE )->SetWindowText( _T("") );

		((CButton*)GetDlgItem( IDC_DAILY_BUTTON_ADD )   )->EnableWindow( FALSE );
		((CButton*)GetDlgItem( IDC_DAILY_BUTTON_DELETE ))->EnableWindow( FALSE );
		((CButton*)GetDlgItem( IDC_DAILY_BUTTON_EDIT )  )->EnableWindow( FALSE );
	}
}

void
CDailyDlg::GetDaily( CString strFile )
{
	CBulletinDlg*	pOwner = (CBulletinDlg*)GetOwner();
	if	( !pOwner )
		return;

	CArray <CDaily, CDaily&> aDaily;

	// If not a file is specified, get schediles out of all files.

	if	( strFile.IsEmpty() ){
		CListCtrl*	pList = (CListCtrl*)GetDlgItem( IDC_DAILY_LIST_FILES );
		int	n = pList->GetItemCount();
		for	( int i = 0; i < n; i++ ){
			if	( pList->GetCheck( i ) ){
				CString	strFile = pList->GetItemText( i, 1 );
				pOwner->GetDaily( strFile, aDaily );
			}
		}
	}

	// If a file is specified, get schediles in the file.

	else
		pOwner->GetDaily( strFile, aDaily );

	// Set the daily schedules into the list control.

	CListCtrl*	pList = (CListCtrl*)GetDlgItem( IDC_DAILY_LIST_ITEMS );
	pList->DeleteAllItems();
	int	n = (int)aDaily.GetCount();
	for	( int i = 0; i < n; i++ ){
		pList->InsertItem(  i, _T("") );
		CDaily&	daily = aDaily[i];
		CString	str;
		str.Format( _T("%02d:%02d:%02d"), daily.m_bHour, daily.m_bMinute, daily.m_bSecond );
		pList->SetItemText( i, 0, str );
		pList->SetItemText( i, 1, daily.m_strText );
	}

	// Seek the next daily schedule.

	int	iNext = pOwner->SeekDaily( CTime::GetCurrentTime(), aDaily );
	if	( iNext >= 0 ){
		pList->SetItemState(  iNext, LVNI_SELECTED, LVNI_SELECTED );
		pList->EnsureVisible( iNext, TRUE );
		pList->SetFocus();
	}

	// Adjust the width of columns.

	pList->SetColumnWidth( 0, LVSCW_AUTOSIZE );
	pList->SetColumnWidth( 1, LVSCW_AUTOSIZE );
}
