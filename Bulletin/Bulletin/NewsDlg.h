// NewsDlg.h : header file
//

#pragma once

class CNewsDlg : public CDialog
{
	DECLARE_DYNAMIC( CNewsDlg )

public:
	CNewsDlg( CWnd* pParent = nullptr );

		void	SetNews( CArray <CNews, CNews&>& aNews, int iKey, int iOrder );
		void	GetSort( int& iCol, int& iOrder );

protected:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_NEWS };
#endif
	    CTimeSpan	m_tsBias;
		CArray	<CNews, CNews&>
			m_aNews;
		int	m_iSortCol;
		int	m_iSortOrder;

	virtual	BOOL	OnInitDialog( void );

	afx_msg	void	OnTimer( UINT_PTR nIDEvent );
	afx_msg	void	OnButtonLink( void );
	afx_msg	void	OnClickHeader( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg	void	OnChangList( NMHDR* pNMHDR, LRESULT* pResult );
	DECLARE_MESSAGE_MAP()

		void	OnSelectItem( void );
};
