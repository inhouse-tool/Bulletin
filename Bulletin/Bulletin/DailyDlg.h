// DailyDlg.h : header file
//

#pragma once

class CDailyDlg : public CDialog
{
	DECLARE_DYNAMIC( CDailyDlg )

public:
	CDailyDlg( CWnd* pParent = nullptr );

		void	SetFiles( CArray <CDailyFile, CDailyFile&>& aFile );
		void	GetFiles( CArray <CDailyFile, CDailyFile&>& aFile );

protected:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DAILY };
#endif
		CArray	<CDailyFile, CDailyFile&>
			m_aFile;

	virtual	BOOL	OnInitDialog( void );
	virtual	void	OnOK( void );

	afx_msg	void	OnTimer( UINT_PTR nIDEvent );
	afx_msg	void	OnChangeEditFile( void );
	afx_msg	void	OnButtonRef( void );
	afx_msg	void	OnButtonAdd( void );
	afx_msg	void	OnButtonDelete( void );
	afx_msg	void	OnButtonEdit( void );
	afx_msg	void	OnChangeFile( NMHDR* pNMHDR, LRESULT* pResult );
	DECLARE_MESSAGE_MAP()

		void	OnSelectItem( void );
		void	GetDaily( CString strFile );
};
