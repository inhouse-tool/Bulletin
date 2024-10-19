// SourceDlg.cpp : implementation file
//

#include "pch.h"
#include "Resource.h"
#include "BulletinDlg.h"
#include "SourceDlg.h"

#define	TID_SELCHANGE	1	// to get the last result of the selection

IMPLEMENT_DYNAMIC( CSourceDlg, CDialog )

///////////////////////////////////////////////////////////////////////////////////////
// Constructor

CSourceDlg::CSourceDlg( CWnd* pParent )
	: CDialog( IDD_SOURCE, pParent )
{
	m_bDeselect = false;
}

///////////////////////////////////////////////////////////////////////////////////////
// Interface Functions

void
CSourceDlg::SetSources( CArray <CSource, CSource&>& aSource )
{
	m_aSource.RemoveAll();
	m_aSource.Append( aSource );
}

void
CSourceDlg::GetSources( CArray <CSource, CSource&>& aSource )
{
	aSource.RemoveAll();
	aSource.Append( m_aSource );
}

///////////////////////////////////////////////////////////////////////////////////////
// Overridden Functions

BOOL
CSourceDlg::OnInitDialog( void )
{
	CDialog::OnInitDialog();

	HICON	hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );
	SetIcon( hIcon, TRUE );
	SetIcon( hIcon, FALSE );

	GetOwner()->PostMessage( WM_POPUP, 0, (LPARAM)this );

	CListCtrl*	pList = (CListCtrl*)GetDlgItem( IDC_SOURCE_LIST_URL );

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
	for	( int i = 0; i < m_aSource.GetCount(); i++ ){
		pList->InsertItem(  i, _T("") );
		pList->SetItemText( i, 1, m_aSource[i].m_strURL );
		pList->SetCheck(    i, m_aSource[i].m_bRead );
	}

	CBulletinDlg*	pOwner = (CBulletinDlg*)GetOwner();
	CFont*	pFont = pOwner->GetListFont();

	GetDlgItem( IDC_SOURCE_EDIT_NAME  )->SetFont( pFont );
	GetDlgItem( IDC_SOURCE_EDIT_ALIAS )->SetFont( pFont );

	return	TRUE;
}

void
CSourceDlg::OnOK( void )
{
	CListCtrl*	pList = (CListCtrl*)GetDlgItem( IDC_SOURCE_LIST_URL );

	CString	strURL;
	GetDlgItem( IDC_SOURCE_EDIT_URL )->GetWindowText( strURL );

	bool	bUnadded = !strURL.IsEmpty();
	int	n = pList->GetItemCount();
	for	( int i = 0; i < n; i++ ){
		m_aSource[i].m_strURL = pList->GetItemText( i, 1 );
		m_aSource[i].m_bRead  = pList->GetCheck( i );

		if	( m_aSource[i].m_strURL == strURL ){
			CString	strAlias;
			GetDlgItem( IDC_SOURCE_EDIT_ALIAS )->GetWindowText( strAlias );
			m_aSource[i].m_strAlias = strAlias;
			m_aSource[i].m_bDesc =
				((CButton*)GetDlgItem( IDC_SOURCE_RADIO_DESC )   )->GetCheck() == BST_CHECKED;
			m_aSource[i].m_bSrc =
				((CButton*)GetDlgItem( IDC_SOURCE_CHECK_SOURCE ) )->GetCheck() == BST_CHECKED;
			bUnadded = false;
		}
	}

	if	( bUnadded ){
		int	iAnswer = MessageBox( _T("New URL is not added. Add it?"), _T("Not Added"), MB_YESNOCANCEL );
		if	( iAnswer == IDCANCEL )
			return;
		if	( iAnswer == IDYES )
			OnButtonAdd();
	}

	CDialog::OnOK();
}

///////////////////////////////////////////////////////////////////////////////////////
// Message Handlers

BEGIN_MESSAGE_MAP( CSourceDlg, CDialog )
	ON_WM_TIMER()
	ON_EN_CHANGE( IDC_SOURCE_EDIT_URL, OnChangeEditURL )
	ON_BN_CLICKED( IDC_SOURCE_BUTTON_ADD,    OnButtonAdd )
	ON_BN_CLICKED( IDC_SOURCE_BUTTON_DELETE, OnButtonDelete )
	ON_BN_CLICKED( IDC_SOURCE_BUTTON_UP,     OnButtonUp )
	ON_BN_CLICKED( IDC_SOURCE_BUTTON_DOWN,   OnButtonDown )
	ON_NOTIFY( LVN_ITEMCHANGED, IDC_SOURCE_LIST_URL, OnChangeItem )
END_MESSAGE_MAP()

void
CSourceDlg::OnTimer( UINT_PTR nIDEvent )
{
	if	( nIDEvent == TID_SELCHANGE ){
		KillTimer( nIDEvent );
		OnSelectItem();
	}
	else
		CDialog::OnTimer( nIDEvent );
}

void
CSourceDlg::OnChangeEditURL( void )
{
	CString	strURL;
	GetDlgItem( IDC_SOURCE_EDIT_URL )->GetWindowText( strURL );

	CString	strExt = strURL.Right( 4 );
	if	( strExt == _T(".xml") ||
		  strExt == _T(".rdf") ||
		  strExt == _T(".rss") ||
		  strExt.Right( 1 ) == _T("/") ){
		CheckSource( strURL );
		GetDlgItem( IDC_SOURCE_EDIT_ALIAS   )->SetWindowText( _T("") );
		GetDlgItem( IDC_SOURCE_CHECK_SOURCE )->EnableWindow( TRUE );
	}
	else{
		GetDlgItem( IDC_SOURCE_EDIT_NAME  )->SetWindowText( _T("") );
		GetDlgItem( IDC_SOURCE_EDIT_ALIAS )->SetWindowText( _T("") );
		((CButton*)GetDlgItem( IDC_SOURCE_RADIO_TITLE )  )->EnableWindow( FALSE );
		((CButton*)GetDlgItem( IDC_SOURCE_RADIO_TITLE )  )->SetCheck( BST_UNCHECKED );
		((CButton*)GetDlgItem( IDC_SOURCE_RADIO_DESC )   )->EnableWindow( FALSE );
		((CButton*)GetDlgItem( IDC_SOURCE_RADIO_DESC )   )->SetCheck( BST_UNCHECKED );
		((CButton*)GetDlgItem( IDC_SOURCE_BUTTON_ADD )   )->EnableWindow( FALSE );
		((CButton*)GetDlgItem( IDC_SOURCE_BUTTON_DELETE ))->EnableWindow( FALSE );
	}
}

void
CSourceDlg::OnButtonAdd( void )
{
	CSource	url;

	url.m_bRead = true;
	url.m_bDesc = ((CButton*)GetDlgItem( IDC_SOURCE_RADIO_DESC ))->GetCheck() == BST_CHECKED;
	url.m_bSrc  = ((CButton*)GetDlgItem( IDC_SOURCE_CHECK_SOURCE  ))->GetCheck() == BST_CHECKED;
	GetDlgItem( IDC_SOURCE_EDIT_URL   )->GetWindowText( url.m_strURL );
	GetDlgItem( IDC_SOURCE_EDIT_ALIAS )->GetWindowText( url.m_strAlias );

	m_aSource.Add( url );

	CListCtrl*	pList = (CListCtrl*)GetDlgItem( IDC_SOURCE_LIST_URL );
	int	i = pList->GetItemCount();

	pList->InsertItem(   i, _T("") );
	pList->SetCheck(     i,    url.m_bRead );
	pList->SetItemText(  i, 1, url.m_strURL );
	pList->SetItemState( i, LVNI_SELECTED, LVNI_SELECTED );

	pList->SetFocus();
}

void
CSourceDlg::OnButtonDelete( void )
{
	CListCtrl*	pList = (CListCtrl*)GetDlgItem( IDC_SOURCE_LIST_URL );
	int	i = pList->GetNextItem( -1, LVNI_ALL | LVNI_SELECTED );

	pList->DeleteItem( i );
	m_aSource.RemoveAt( i );
}

void
CSourceDlg::OnButtonUp( void )
{
	CListCtrl*	pList = (CListCtrl*)GetDlgItem( IDC_SOURCE_LIST_URL );
	int	i = pList->GetNextItem( -1, LVNI_ALL | LVNI_SELECTED );

	if	( i > 0 ){
		CSource	url = m_aSource[i];
		m_aSource.RemoveAt( i );
		pList->DeleteItem( i );
		m_aSource.InsertAt( --i, url );

		pList->InsertItem(  i, _T("") );
		pList->SetItemText( i, 1, url.m_strURL );
		pList->SetCheck(    i, url.m_bRead );
		pList->SetItemState( i, LVNI_SELECTED, LVNI_SELECTED );
		pList->SetFocus();
	}
}

void
CSourceDlg::OnButtonDown( void )
{
	CListCtrl*	pList = (CListCtrl*)GetDlgItem( IDC_SOURCE_LIST_URL );
	int	i = pList->GetNextItem( -1, LVNI_ALL | LVNI_SELECTED );

	if	( i >= 0 && i < pList->GetItemCount()-1 ){
		CSource	url = m_aSource[i];
		m_aSource.RemoveAt(   i );
		pList->DeleteItem( i );
		m_aSource.InsertAt( ++i, url );

		pList->InsertItem(   i, _T("") );
		pList->SetItemText(  i, 1, url.m_strURL );
		pList->SetCheck(     i, url.m_bRead );
		pList->SetItemState( i, LVNI_SELECTED, LVNI_SELECTED );
		pList->SetFocus();
	}
}

void
CSourceDlg::OnChangeItem( NMHDR* pNMHDR, LRESULT* pResult )
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>( pNMHDR );

	SetTimer( TID_SELCHANGE, 0, NULL );
	*pResult = 0;
}

///////////////////////////////////////////////////////////////////////////////////////
// Specific Functions

void
CSourceDlg::OnSelectItem( void )
{
	CListCtrl*	pList = (CListCtrl*)GetDlgItem( IDC_SOURCE_LIST_URL );
	int	i = pList->GetNextItem( -1, LVNI_ALL | LVNI_SELECTED );

	// Selected by the list:

	if	( i >= 0 ){
		GetDlgItem( IDC_SOURCE_EDIT_URL   )->SetWindowText( m_aSource[i].m_strURL );
		GetDlgItem( IDC_SOURCE_EDIT_ALIAS )->SetWindowText( m_aSource[i].m_strAlias );

		if	( m_aSource[i].m_bDesc ){
			((CButton*)GetDlgItem( IDC_SOURCE_RADIO_TITLE ))->SetCheck( BST_UNCHECKED );
			((CButton*)GetDlgItem( IDC_SOURCE_RADIO_DESC ) )->SetCheck( BST_CHECKED );
		}
		else{
			((CButton*)GetDlgItem( IDC_SOURCE_RADIO_TITLE ))->SetCheck( BST_CHECKED );
			((CButton*)GetDlgItem( IDC_SOURCE_RADIO_DESC ) )->SetCheck( BST_UNCHECKED );
		}
		((CButton*)GetDlgItem( IDC_SOURCE_CHECK_SOURCE ))->SetCheck( m_aSource[i].m_bSrc? BST_CHECKED: BST_UNCHECKED );
		((CButton*)GetDlgItem( IDC_SOURCE_BUTTON_ADD )   )->EnableWindow( FALSE );
		((CButton*)GetDlgItem( IDC_SOURCE_BUTTON_DELETE ))->EnableWindow( TRUE );

		BOOL	bUppable = i != 0;
		BOOL	bDownable = i < pList->GetItemCount()-1;

		((CButton*)GetDlgItem( IDC_SOURCE_BUTTON_UP )    )->EnableWindow( bUppable );
		((CButton*)GetDlgItem( IDC_SOURCE_BUTTON_DOWN )  )->EnableWindow( bDownable );
		((CButton*)GetDlgItem( IDC_SOURCE_CHECK_SOURCE )    )->EnableWindow( TRUE );
		m_bDeselect = false;
	}

	// Deselected by the edit:

	else if	( m_bDeselect )
		;

	// Deselected by the list:

	else{
		GetDlgItem( IDC_SOURCE_EDIT_URL  )->SetWindowText( _T("") );
		GetDlgItem( IDC_SOURCE_EDIT_NAME  )->SetWindowText( _T("") );
		GetDlgItem( IDC_SOURCE_EDIT_ALIAS )->SetWindowText( _T("") );
		((CButton*)GetDlgItem( IDC_SOURCE_RADIO_TITLE ))->SetCheck( BST_UNCHECKED );
		((CButton*)GetDlgItem( IDC_SOURCE_RADIO_DESC ) )->SetCheck( BST_UNCHECKED );
		((CButton*)GetDlgItem( IDC_SOURCE_BUTTON_ADD )   )->EnableWindow( FALSE );
		((CButton*)GetDlgItem( IDC_SOURCE_BUTTON_DELETE ))->EnableWindow( FALSE );
		((CButton*)GetDlgItem( IDC_SOURCE_BUTTON_UP )    )->EnableWindow( FALSE );
		((CButton*)GetDlgItem( IDC_SOURCE_BUTTON_DOWN )  )->EnableWindow( FALSE );
		((CButton*)GetDlgItem( IDC_SOURCE_CHECK_SOURCE )    )->EnableWindow( FALSE );
	}
}

void
CSourceDlg::CheckSource( CString strURL )
{
	CBulletinDlg*	pDlg = (CBulletinDlg*)GetOwner();
	if	( !pDlg )
		return;

	bool	bDesc = false;
	CString	strTitle = pDlg->CheckSource( strURL, bDesc );

	bool	bURL = strTitle.IsEmpty()? false: true;
	GetDlgItem( IDC_SOURCE_EDIT_NAME )->SetWindowText( strTitle );
	((CButton*)GetDlgItem( IDC_SOURCE_RADIO_TITLE ) )->EnableWindow( bURL?  TRUE: FALSE );
	((CButton*)GetDlgItem( IDC_SOURCE_RADIO_DESC  ) )->EnableWindow( bDesc? TRUE: FALSE );

	int	i, n = (int)m_aSource.GetCount();
	for	( i = 0; i < n; i++ )
		if	( m_aSource[i].m_strURL == strURL )
			break;
	if	( i < n ){
		if	( m_aSource[i].m_bDesc ){
			((CButton*)GetDlgItem( IDC_SOURCE_RADIO_TITLE ))->SetCheck( BST_UNCHECKED );
			((CButton*)GetDlgItem( IDC_SOURCE_RADIO_DESC ) )->SetCheck( BST_CHECKED );
		}
		else{
			((CButton*)GetDlgItem( IDC_SOURCE_RADIO_TITLE ))->SetCheck( BST_CHECKED );
			((CButton*)GetDlgItem( IDC_SOURCE_RADIO_DESC ) )->SetCheck( BST_UNCHECKED );
		}
		((CButton*)GetDlgItem( IDC_SOURCE_CHECK_SOURCE ))->SetCheck( m_aSource[i].m_bSrc? BST_CHECKED: BST_UNCHECKED );
	}
	else{
		if	( bDesc ){
			((CButton*)GetDlgItem( IDC_SOURCE_RADIO_TITLE ))->SetCheck( BST_UNCHECKED );
			((CButton*)GetDlgItem( IDC_SOURCE_RADIO_DESC ) )->SetCheck( BST_CHECKED );
		}
		else{
			((CButton*)GetDlgItem( IDC_SOURCE_RADIO_TITLE ))->SetCheck( BST_CHECKED );
			((CButton*)GetDlgItem( IDC_SOURCE_RADIO_DESC ) )->SetCheck( BST_UNCHECKED );
		}
		((CButton*)GetDlgItem( IDC_SOURCE_CHECK_SOURCE ) )->SetCheck( BST_CHECKED );
		((CButton*)GetDlgItem( IDC_SOURCE_BUTTON_ADD )   )->EnableWindow( TRUE );
		((CButton*)GetDlgItem( IDC_SOURCE_BUTTON_DELETE ))->EnableWindow( FALSE );
		((CButton*)GetDlgItem( IDC_SOURCE_BUTTON_UP )    )->EnableWindow( FALSE );
		((CButton*)GetDlgItem( IDC_SOURCE_BUTTON_DOWN )  )->EnableWindow( FALSE );

		// Deselect the current row since a new URL has entered manually.

		m_bDeselect = true;
		CListCtrl*	pList = (CListCtrl*)GetDlgItem( IDC_SOURCE_LIST_URL );
		int	iItem = pList->GetNextItem( -1, LVNI_ALL | LVNI_SELECTED );
		if	( iItem >= 0 )
			pList->SetItemState( iItem, 0, LVIS_SELECTED );
	}
}
