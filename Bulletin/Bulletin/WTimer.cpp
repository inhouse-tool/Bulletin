// WTimer.cpp : implementation file
//

#include "pch.h"
#include "WTimer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////////////
// Constructor & Destructor of CTimerEntry

CTimerEntry::CTimerEntry( void )
{
	// Initialize all members ( to avoid C26495 :-).

	m_nIDEvent = 0;
	m_hThread  = NULL;
	m_hTimer   = NULL;
}

CTimerEntry::~CTimerEntry( void )
{
	// Signal the timer to wake up the timer thread ASAP.

	LARGE_INTEGER	li;
	li.QuadPart = -1;
	SetWaitableTimer( m_hTimer, &li, 0, NULL, NULL, FALSE );

	// Close the timer handle to let the timer thread end gracelully.

	CloseHandle( m_hTimer );
	m_hTimer = NULL;

	// Wait for the thread to end.

	WaitForSingleObject( m_hThread, INFINITE );
}

///////////////////////////////////////////////////////////////////////////////////////
// Constructor

CWTimer::CWTimer( void )
{
	// Initialize all members ( to avoid C26495 :-).

	m_pWnd   = NULL;
	m_uMsg   = WM_TIMER;
}

CWTimer::~CWTimer( void )
{
	// Delete remained entries.

	INT_PTR	n = m_apEntry.GetCount();
	for	( INT_PTR i = 0; i < n; i++ )
		if	( m_apEntry[i] )
			delete	m_apEntry[i];
}

///////////////////////////////////////////////////////////////////////////////////////
// Interface Functions

void
CWTimer::SetOwnerWnd( CWnd* pWnd )
{
	m_pWnd = pWnd;
}

void
CWTimer::SetNotifyMsg( UINT uMsg )
{
	m_uMsg = uMsg;
}

	typedef	struct{
		     CWTimer*	pThis;
			CWnd*	pWnd;
		    UINT_PTR	nIDEvent;
			UINT	uElapse;
		   TIMERPROC	lpTimerFunc;
	}	ThreadArg;

void
CWTimer::SetTimer( UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc )
{
	// If the owner windows is not assigned, select the main window.

	if	( !m_pWnd )
		m_pWnd = AfxGetMainWnd();

	// Remove the timer with specified ID.

	KillTimer( nIDEvent );

	// Allocate a timer entry with given arguments.

	CTimerEntry*	pEntry = new CTimerEntry;
	pEntry->m_nIDEvent = nIDEvent;

	INT_PTR	i, n = m_apEntry.GetCount();
	for	( i = 0; i < n; i++ )
		if	( !m_apEntry[i] )
			break;
	if	( i >= n )
		m_apEntry.Add( pEntry );
	else
		m_apEntry[i] = pEntry;

	// Make an inter-thread argument packet.

	ThreadArg*	pArg = new ThreadArg;
	pArg->pThis	  = this;
	pArg->pWnd	  = m_pWnd;
	pArg->nIDEvent	  = nIDEvent;
	pArg->uElapse	  = uElapse;
	pArg->lpTimerFunc = lpTimerFunc;

	// Launch a timer thread...

	CWinThread*	pth = AfxBeginThread( DoTimerThread, (void*)pArg, THREAD_PRIORITY_ABOVE_NORMAL, 0, CREATE_SUSPENDED, NULL );

	// ...with its thread handle ( for graceful end of the thread ( see ~CTimerEntry() ) ).

	pEntry->m_hThread = pth->m_hThread;

	pth->ResumeThread();
}

void
CWTimer::KillTimer( UINT_PTR nIDEvent )
{
	// Search and destroy the timer with the specified ID.

	INT_PTR	n = m_apEntry.GetCount();
	for	( INT_PTR i = 0; i < n; i++ )
		if	( m_apEntry[i] &&
			  m_apEntry[i]->m_nIDEvent == nIDEvent ){
			delete	m_apEntry[i];
			m_apEntry[i] = NULL;
			return;
		}
}

///////////////////////////////////////////////////////////////////////////////////////
// Internal Functions

UINT
CWTimer::DoTimerThread( void* pArg )
{
	// Retrieve the arguments given from the main thread.

	ThreadArg	arg = *(ThreadArg*)pArg;
	delete	pArg;

	// Enter into the loop of timer thread.

	arg.pThis->DoTimerLoop( arg.pWnd, arg.nIDEvent, arg.uElapse, arg.lpTimerFunc );

	return	0;
}

void
CWTimer::DoTimerLoop( CWnd* pWnd, UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc )
{
	// Search the entry with the specified ID.

	INT_PTR	i, n = m_apEntry.GetCount();
	for	( i = 0; i < n; i++ )
		if	( m_apEntry[i] &&
			  m_apEntry[i]->m_nIDEvent == nIDEvent )
			break;
	if	( i >= n )
		return;

	// Create a 'Waitable Timer' for the entry.

	CTimerEntry*	pEntry = m_apEntry[i];
	pEntry->m_hTimer = CreateWaitableTimer( NULL, FALSE, NULL );
	if	( pEntry->m_hTimer ){
		LARGE_INTEGER	li;
		li.QuadPart = -((long long)uElapse*10*1000);
		SetWaitableTimer( pEntry->m_hTimer, &li, uElapse, NULL, NULL, FALSE );
	}

	// Repeat the periodic timer until the timer has closed.

	while	( pEntry->m_hTimer )

		// When the timer has fired, tell the owner window.

		if	( WaitForSingleObject( pEntry->m_hTimer, INFINITE ) == WAIT_OBJECT_0 )
			if	( lpTimerFunc )
				(*lpTimerFunc)( pWnd->m_hWnd, m_uMsg, nIDEvent, uElapse );
			else
				pWnd->PostMessage( m_uMsg, nIDEvent, NULL );
}
