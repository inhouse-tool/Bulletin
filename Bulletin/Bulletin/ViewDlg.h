// ViewDlg.h : header file
//

#pragma once

class CViewDlg : public CDialog
{
	DECLARE_DYNAMIC( CViewDlg )

public:
	CViewDlg( CWnd* pParent = nullptr );

		void	SetArgs( int  nDot, int  nChar, int  cxDot, bool  bWide );
		void	GetArgs( int& nDot, int& nChar, int& cxDot, bool& bWide );
		void	SetFont( int iIndex, LOGFONT* plf );
		void	GetFont( int iIndex, LOGFONT* plf );

protected:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VIEW };
#endif
		int	m_nDot,
			m_nChar,
			m_cxDot;
		bool	m_bWide;
		CFont	m_aFont[2];
		int	m_anHeight[2];
		int	m_anQuality[2];
		int	m_iFont;

	virtual	BOOL	OnInitDialog( void );
	virtual	void	OnOK( void );

	afx_msg	void	OnButtonFont( UINT uID );
	afx_msg LRESULT	OnPopup( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT	OnFont( WPARAM wParam, LPARAM lParam );
	DECLARE_MESSAGE_MAP()
};
