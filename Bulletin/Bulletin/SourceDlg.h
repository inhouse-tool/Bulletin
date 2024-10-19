// SourceDlg.h : header file
//

#pragma once

class CSourceDlg : public CDialog
{
	DECLARE_DYNAMIC( CSourceDlg )

public:
	CSourceDlg( CWnd* pParent = nullptr );

		void	SetSources( CArray <CSource, CSource&>& aSource );
		void	GetSources( CArray <CSource, CSource&>& aSource );

protected:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_NEWS };
#endif
		CArray	<CSource, CSource&>
			m_aSource;
		bool	m_bDeselect;

	virtual	BOOL	OnInitDialog( void );
	virtual	void	OnOK( void );

	afx_msg	void	OnTimer( UINT_PTR nIDEvent );
	afx_msg	void	OnChangeEditURL( void );
	afx_msg	void	OnButtonAdd( void );
	afx_msg	void	OnButtonDelete( void );
	afx_msg	void	OnButtonUp( void );
	afx_msg	void	OnButtonDown( void );
	afx_msg	void	OnChangeItem( NMHDR* pNMHDR, LRESULT* pResult );
	DECLARE_MESSAGE_MAP()

		void	OnSelectItem( void );
		void	CheckSource( CString strURL );
};
