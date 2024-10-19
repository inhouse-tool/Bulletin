// BulletinDlg.h : header file
//

#pragma once

#include "WTimer.h"

#define	WM_POPUP		(WM_APP+1)
#define	MAKESEC( h, m, s )	((h*60*60)+(m*60)+s)

class	CSource{
public:
		bool	m_bRead;
		bool	m_bDesc;
		bool	m_bSrc;
		CString	m_strURL;
		CString	m_strAlias;
		CString	m_strSrc;
		CSource( void )
		{
			m_bRead = true;
			m_bDesc = false;
			m_bSrc  = true;
		}
		bool	operator ==( const CSource& src )
		{
			return
			m_bRead    == src.m_bRead    &&
			m_bDesc    == src.m_bDesc    &&
			m_bSrc     == src.m_bSrc     &&
			m_strURL   == src.m_strURL   &&
			m_strAlias == src.m_strAlias;
		}
};

class	CNews{
public:
		int	m_iSource;
		int	m_iIndex;
		CString	m_strSrc;
		CString	m_strTitle;
		CString	m_strDesc;
		CString	m_strLink;
		CTime	m_tPub;
		CNews( void )
		{
			m_iSource = 0;
			m_iIndex = 0;
		}
};

class	CDailyFile{
public:
		bool	m_bRead;
		CString	m_strFile;
		CTime	m_tLast;
		CDailyFile( void )
		{
			m_bRead  = true;
		}
		bool	operator ==( const CDailyFile& src )
		{
			return
			m_bRead    == src.m_bRead    &&
			m_strFile  == src.m_strFile;
		}
};

class	CDaily{
public:
		BYTE	m_bHour,
			m_bMinute,
			m_bSecond;
		CString	m_strText;
		CDaily( void )
		{
			m_bHour   = 0;
			m_bMinute = 0;
			m_bSecond = 0;
		}
};

class CBulletinDlg : public CDialog
{
public:
	 CBulletinDlg( CWnd* pParent = nullptr );
	~CBulletinDlg( void );

		CString	CheckSource( CString strURL, bool& bDesc );
		void	SortNews( CArray <CNews, CNews&>& aNews, int iKey, int iOrder );
		void	GetDaily( CString strFile, CArray <CDaily, CDaily&>& aDaily );
		int	SeekDaily( CTime tFrom, CArray <CDaily, CDaily&>& aDaily );
		CFont*	GetListFont( void );

protected:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BULLETIN_DIALOG };
#endif
		HICON	m_hIcon;

		int	m_nDot,		m_nDot1st,
			m_cxDot,	m_cxDot1st,
			m_nChar,	m_nChar1st;			

		int	m_iCorner,	m_iCorner1st;
		int	m_iMonitor,	m_iMonitor1st,
			m_nMonitor;
		RECT	m_rcMonitor;
		bool	m_bWide,	m_bWide1st;

		int	m_nInterval;
		int	m_nPeriod;

		int	m_cyFont;
		CFont	m_aFont[2],	m_aFont1st[2];

		CArray	<CSource, CSource&>
			m_aSource,	m_aSource1st;
		CArray	<CNews, CNews&>
			m_aNews;
		INT_PTR	m_iNews;
		int	m_iNewsKey,	m_iNewsKey1st,
			m_iNewsOrder,	m_iNewsOrder1st;

		CArray	<CDailyFile, CDailyFile&>
			m_aFile,	m_aFile1st;
		CArray	<CDaily, CDaily&>
			m_aDaily;
		INT_PTR	m_iDaily;

		bool	m_bRepeat;
		bool	m_bUpdating;

		CWTimer	m_timer;
		CTime	m_tLast,
			m_tNews;

		int	m_nScrollTime;
		int	m_nScrollPix;

		int	m_xOffset,
			m_cxText;

	    COLORREF	m_crBack,
			m_crFore,
			m_crPlus;

		CDC	m_dcMem;
		CBitmap	m_bmMem;

	virtual	BOOL	OnInitDialog( void );
	virtual	void	OnOK( void );

	afx_msg	BOOL	OnEraseBkgnd( CDC* pDC );
	afx_msg	void	OnPaint( void );
	afx_msg	void	OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg	void	OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg	void	OnRButtonDown( UINT nFlags, CPoint point );
	afx_msg	void	OnSysKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg	void	OnTimer( UINT_PTR nIDEvent );
	afx_msg	void	OnMenuSource( void );
	afx_msg	void	OnMenuNews( void );
	afx_msg	void	OnMenuDaily( void );
	afx_msg	void	OnMenuPosition( void );
	afx_msg	void	OnMenuView( void );
	afx_msg	void	OnMenuInfo( void );
	afx_msg	void	OnMenuExit( void );

	afx_msg LRESULT	OnPopup(  WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT	OnWTimer(  WPARAM wParam, LPARAM lParam );
	DECLARE_MESSAGE_MAP()

		void	LoadSettings( void );
		void	SaveSettings( void );

		void	PlaceWindow( void );
		void	SelectMonitor( bool bSelect );
	static	BOOL	CALLBACK
			OnEnumDisplay( HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData );

		void	GetProfileFonts( void );
		void	WriteProfileFonts( void );

		void	GetProfileSources( void );
		void	WriteProfileSources( void );
		void	ReloadNews( void );
		void	ReadNews( void );
		void	ReadNewsSite( int iSource, int& iIndex );
		CTime	TakeDate( CString strDate );
		bool	DeCdata( CString& strText );
	static	int	CompareNews( void* pContext, const void *pElem1, const void *pElem2 );

		void	GetProfileDaily( void );
		void	WriteProfileDaily( void );
		void	ReadDaily( void );
	static	int	CompareDaily( const void *pElem1, const void *pElem2 );

		void	ScrollBulletin( void );
		bool	DrawDaily( CTime tNow );
		bool	DrawNews( CTime tNow );
		void	DrawBulletin( CString strText, bool bScroll );
		CString	DeTag( CString strTagged, CUIntArray& iaTag );
		void	DrawTagged( CDC* pDC, CRect rect, CString strTagged, CUIntArray& iaTag );
		void	Draw( CDC* pDC, CRect rectUpdate );
		void	DrawGrid( CDC* pDC, CRect rectUpdate );

		CString	LoadWeb( CString strURL );
		CString	LoadText( CString strFile );
		int	GetTextEncode( BYTE* pbData, QWORD cbData, int& cbBOM );

		void	SelectScrollSpeed( void );
		void	SetScroll( bool bScroll );
		void	SetBitmapDC( void );
};
