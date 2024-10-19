// WTimer.h : header file
//

#pragma once

class	CTimerEntry
{
public:
	      UINT_PTR	m_nIDEvent;
		HANDLE	m_hThread;
		HANDLE	m_hTimer;

	      CTimerEntry( void );
	      ~CTimerEntry( void );
};

class CWTimer
{
protected:
		CWnd*	m_pWnd;
		UINT	m_uMsg;
	CTypedPtrArray	<CPtrArray, CTimerEntry*>
			m_apEntry;
public:
		CWTimer( void );
		~CWTimer( void );

		void	SetOwnerWnd( CWnd* pWnd );
		void	SetNotifyMsg( UINT uMsg );

		void	SetTimer(  UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc = NULL );
		void	KillTimer( UINT_PTR nIDEvent );
protected:
	static	UINT	DoTimerThread( void* pArg );
		void	DoTimerLoop( CWnd* pWnd, UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc );
};
