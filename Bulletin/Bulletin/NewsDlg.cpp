// NewsDlg.cpp : implementation file
//

#include "pch.h"
#include "Resource.h"
#include "BulletinDlg.h"
#include "NewsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef	UNICODE
#define	strcpy		wcscpy
#define	strcpy_s	wcscpy_s
#endif//UNICODE

#define	TID_SELCHANGE	1	// to get the last result of the selection

#define	COL_HEADER0	_T("Published")
#define	COL_HEADER1	_T("Title")

IMPLEMENT_DYNAMIC( CNewsDlg, CDialog )

///////////////////////////////////////////////////////////////////////////////////////
// Constructor

CNewsDlg::CNewsDlg( CWnd* pParent )
	: CDialog( IDD_NEWS, pParent )
{
	TIME_ZONE_INFORMATION	tzi = { 0 };
	GetTimeZoneInformation( &tzi );
	CTimeSpan	tsBias( 0, 0, -tzi.Bias, 0 );
	m_tsBias = tsBias;

	m_iSortCol  = -1;
	m_iSortOrder = 0;
}

///////////////////////////////////////////////////////////////////////////////////////
// Interface Functions

void
CNewsDlg::SetNews( CArray <CNews, CNews&>& aNews, int iKey, int iOrder )
{
	m_aNews.RemoveAll();
	m_aNews.Append( aNews );
}

void
CNewsDlg::GetSort( int& iCol, int& iOrder )
{
	iCol   = m_iSortCol;
	iOrder = m_iSortOrder;
}

///////////////////////////////////////////////////////////////////////////////////////
// Overridden Functions

BOOL
CNewsDlg::OnInitDialog( void )
{
	CDialog::OnInitDialog();

	HICON	hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );
	SetIcon( hIcon, TRUE );
	SetIcon( hIcon, FALSE );

	GetOwner()->PostMessage( WM_POPUP, 0, (LPARAM)this );

	CListCtrl*	pList = (CListCtrl*)GetDlgItem( IDC_NEWS_LIST );

	DWORD	dwExStyle = ListView_GetExtendedListViewStyle( pList->m_hWnd );
	dwExStyle |= LVS_EX_FULLROWSELECT;
	dwExStyle |= LVS_EX_GRIDLINES;
	ListView_SetExtendedListViewStyle( pList->m_hWnd , dwExStyle );

	int	i = 0;
	pList->InsertColumn( i++, COL_HEADER0, LVCFMT_RIGHT, 130 );
	pList->InsertColumn( i++, COL_HEADER1, LVCFMT_LEFT,  470 );

	pList->DeleteAllItems();
	for	( int i = 0; i < m_aNews.GetCount(); i++ ){
		pList->InsertItem(  i, _T("") );
		CTime	tDate = m_aNews[i].m_tPub;
		tDate += m_tsBias;
		CString	strDate = tDate.Format( _T("%Y/%m/%d %H:%M:%S") );
		pList->SetItemText( i, 0, strDate );
		pList->SetItemText( i, 1, m_aNews[i].m_strTitle );
	}

	CBulletinDlg*	pOwner = (CBulletinDlg*)GetOwner();
	CFont*	pFont = pOwner->GetListFont();

	pList->SetFont( pFont );
	GetDlgItem( IDC_NEWS_EDIT_SOURCE )->SetFont( pFont );
	GetDlgItem( IDC_NEWS_EDIT_TITLE  )->SetFont( pFont );
	GetDlgItem( IDC_NEWS_EDIT_DESC   )->SetFont( pFont );

	pList->SetColumnWidth( 0, LVSCW_AUTOSIZE );
	pList->SetColumnWidth( 1, LVSCW_AUTOSIZE );

	return	TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////
// Message Handlers

BEGIN_MESSAGE_MAP( CNewsDlg, CDialog )
	ON_WM_TIMER()
	ON_BN_CLICKED( IDC_NEWS_BUTTON_LINK, OnButtonLink )
	ON_NOTIFY( HDN_ITEMCLICK, 0, OnClickHeader )
	ON_NOTIFY( LVN_ITEMCHANGED, IDC_NEWS_LIST, OnChangList )
END_MESSAGE_MAP()

void
CNewsDlg::OnTimer( UINT_PTR nIDEvent )
{
	if	( nIDEvent == TID_SELCHANGE ){
		KillTimer( nIDEvent );
		OnSelectItem();
	}
	else
		CDialog::OnTimer( nIDEvent );
}

void
CNewsDlg::OnButtonLink( void )
{
	CListCtrl*	pList = (CListCtrl*)GetDlgItem( IDC_NEWS_LIST );
	int	i = pList->GetNextItem( -1, LVNI_ALL | LVNI_SELECTED );
	if	( i < 0 )
		return;

	CNews&	news = m_aNews[i];
	CString	strLink = news.m_strLink;
	if	( !strLink.IsEmpty() )
		ShellExecute( NULL, _T("open"), strLink, NULL, NULL, SW_SHOWNORMAL );
}

void
CNewsDlg::OnClickHeader( NMHDR* pNMHDR, LRESULT* pResult )
{
	NMHEADER*	phdr = reinterpret_cast<NMHEADER*>( pNMHDR );

	int	i = phdr->iItem;
	int	iKey = i;
	int	iOrder = 0;

	CListCtrl*	pList = (CListCtrl*)GetDlgItem( IDC_NEWS_LIST );
	CHeaderCtrl*	pHeader = pList->GetHeaderCtrl();

	HDITEM	header = {};
	header.mask = HDI_FORMAT;
	pHeader->GetItem( i, &header );
	if	( header.fmt & HDF_SORTUP ){
		header.fmt &= ~HDF_SORTUP;
		header.fmt |=  HDF_SORTDOWN;
		iOrder = 2;
	}
	else if	( header.fmt & HDF_SORTDOWN ){
		header.fmt &= ~HDF_SORTDOWN;
		iOrder = 0;
	}
	else{
		header.fmt |= HDF_SORTUP;
		iOrder = 1;
	}
	pHeader->SetItem( i, &header );

	i++;
	i %= 2;
	pHeader->GetItem( i, &header );
	pHeader->GetItem( i, &header );
	header.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
	pHeader->SetItem( i, &header );

	CBulletinDlg*	pOwner = (CBulletinDlg*)GetOwner();
	if	( iOrder )
		iKey++;
	pOwner->SortNews( m_aNews, iKey, iOrder );

	pList->DeleteAllItems();
	for	( int i = 0; i < m_aNews.GetCount(); i++ ){
		pList->InsertItem(  i, _T("") );
		CTime	tDate = m_aNews[i].m_tPub;
		tDate += m_tsBias;
		CString	strDate = tDate.Format( _T("%Y/%m/%d %H:%M:%S") );
		pList->SetItemText( i, 0, strDate );
		pList->SetItemText( i, 1, m_aNews[i].m_strTitle );
	}

	*pResult = 0;
}

void
CNewsDlg::OnChangList( NMHDR* pNMHDR, LRESULT* pResult )
{
//	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>( pNMHDR );
	SetTimer( TID_SELCHANGE, 0, NULL );
	*pResult = 0;
}

///////////////////////////////////////////////////////////////////////////////////////
// Specific Functions

void
CNewsDlg::OnSelectItem( void )
{
	CListCtrl*	pList = (CListCtrl*)GetDlgItem( IDC_NEWS_LIST );
	int	i = pList->GetNextItem( -1, LVNI_ALL | LVNI_SELECTED );
	int	n = (int)m_aNews.GetCount();

	if	( i >= 0 && i < n ){
		CNews&	news = m_aNews[i];

		GetDlgItem( IDC_NEWS_EDIT_SOURCE )->SetWindowText( news.m_strSrc );
		GetDlgItem( IDC_NEWS_EDIT_TITLE  )->SetWindowText( news.m_strTitle );
		GetDlgItem( IDC_NEWS_EDIT_DESC   )->SetWindowText( news.m_strDesc );
		GetDlgItem( IDC_NEWS_EDIT_LINK   )->SetWindowText( news.m_strLink );
		GetDlgItem( IDC_NEWS_BUTTON_LINK )->EnableWindow( TRUE );
	}
	else{
		GetDlgItem( IDC_NEWS_EDIT_SOURCE )->SetWindowText( _T("") );
		GetDlgItem( IDC_NEWS_EDIT_TITLE  )->SetWindowText( _T("") );
		GetDlgItem( IDC_NEWS_EDIT_DESC   )->SetWindowText( _T("") );
		GetDlgItem( IDC_NEWS_EDIT_LINK   )->SetWindowText( _T("") );
		GetDlgItem( IDC_NEWS_BUTTON_LINK )->EnableWindow( FALSE );
	}
}
