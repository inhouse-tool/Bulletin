// FontDlg.h : interface of the CFontDlg class
//

#pragma once

#define	WM_FONT		(WM_APP+300)

class CFontDlg : public CDialog
{
	DECLARE_DYNAMIC( CFontDlg )

public:
	CFontDlg( CWnd* pParent = nullptr );

		void	SetFont( LOGFONT* plf );
		void	GetFont( LOGFONT* plf );

protected:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FONT };
#endif
		int	m_nCharSet;
		BYTE	m_bPitch;
		CString	m_strFaceName;
		LOGFONT	m_lf;

	virtual	BOOL	OnInitDialog( void );

	afx_msg	void	OnShowWindow( BOOL bShow, UINT nStatus );
	afx_msg	void	OnSelectCharset( void );
	afx_msg	void	OnSelectName( void );
	afx_msg	void	OnClickWidth( void );
	DECLARE_MESSAGE_MAP()

		void	InitControls( void );
		void	SetControls( void );
		void	EnumFontGroup( void );
		void	SeekFontByName( void );
	static	int	CALLBACK
			EnumFontProc( const LOGFONT* plf, const TEXTMETRIC* ptm, DWORD dwType, LPARAM lparam );
		BOOL	OnEnumFont( const LOGFONT* plf );
		void	SelectFont( void );
};
