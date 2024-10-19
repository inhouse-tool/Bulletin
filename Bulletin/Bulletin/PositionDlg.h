// PositionDlg.h : header file
//

#pragma once

class CPositionDlg : public CDialog
{
	DECLARE_DYNAMIC( CPositionDlg )

public:
	CPositionDlg( CWnd* pParent = nullptr );

		void	SetArgs( int  iCorner, int  iMonitor, int  nMonitor );
		void	GetArgs( int& iCorner, int& iMonitor  );

protected:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_POSITION };
#endif
		int	m_iCorner;
		int	m_iMonitor,
			m_nMonitor;

	virtual	BOOL	OnInitDialog( void );
	virtual	void	OnOK( void );

	DECLARE_MESSAGE_MAP()
};
