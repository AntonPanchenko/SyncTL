#include "Synchronization.h"

#if defined QT_VERSION
#include <QReadWRiteLock>
#elif defined (WINDOWS)
#elif defined (POSIX)
#endif //(QT_VERSION)
#include "Timer.h"

using namespace SyncTL;

#if defined QT_VERSION

QtReadWriteLock::QtReadWriteLock(bool recursive) :
	m_lock(NULL)
{
	QReadWriteLock::RecursionMode recursion_mode = QReadWriteLock::NonRecursive;
	if (recursive)
	{
		recursion_mode = QReadWriteLock::Recursive;
	}
	m_lock = new QReadWriteLock(recursion_mode);
	if (m_lock == NULL)
	{
		throw Exception(SYNCHRONIZATION_ERROR_CANNOT_CREATE_LOCK,
			L"Cannot create QtReadWriteLock",
			EXC_HERE);
	}
}

QtReadWriteLock::~QtReadWriteLock()
{
	if (m_lock != NULL)
	{
		delete m_lock;
	}
}

bool QtReadWriteLock::LockForRead()
{
	ASSERT(m_lock != NULL);
	m_lock->lockForRead();
	return true;
}

bool QtReadWriteLock::LockForWrite()
{
	ASSERT(m_lock != NULL);
	m_lock->lockForWrite();
	return true;
}

bool QtReadWriteLock::TryLockForRead(unsigned int timeout_milliseconds)
{
	ASSERT(m_lock != NULL);
	return m_lock->tryLockForRead(timeout_milliseconds);
}

bool QtReadWriteLock::TryLockForWrite(unsigned int timeout_milliseconds)
{
	ASSERT(m_lock != NULL);
	return m_lock->tryLockForWrite(timeout_milliseconds);
}

void QtReadWriteLock::Unlock()
{
	ASSERT(m_lock != NULL);
	m_lock->unlock();
}

#endif //QT_VERSION

#ifdef WINDOWS
unsigned int SynchronizationObject::MapSystemErrorToError(unsigned int sys_error)
{
	switch (sys_error)
	{
	case WAIT_OBJECT_0: return SYNCH_WAIT_OK;
	case WAIT_ABANDONED: return SYNCH_WAIT_ABANDONED;
	case WAIT_TIMEOUT: return SYNCH_WAIT_TIMEOUT;
	case WAIT_FAILED: return SYNCH_WAIT_FAILED;
	default:
		ASSERT(false); return UNDEFINED_ERROR;
	}
}
#endif //WINDOWS

#ifdef WINDOWS
Event::Event(bool is_manual_reset, bool initial_state):
	m_handle(NULL)
{
	m_handle = CreateEvent(NULL, is_manual_reset, initial_state, NULL);
	if(m_handle == NULL)
	{
		throw Exception(SYNCHRONIZATION_ERROR_CANNOT_CREATE_EVENT,
			L"Cannot create synchronization event",
			EXC_HERE);
	}
}
#endif //WINDOWS

#ifdef WINDOWS
Event::~Event()
{
	if(m_handle != NULL)
	{
		CloseHandle(m_handle);
	}
}
#endif //WINDOWS

#ifdef WINDOWS
void Event::SetEvent()
{
	ASSERT(m_handle != NULL);
	::SetEvent(m_handle);
}
#endif //WINDOWS

void Event::ClearEvent()
{
	ASSERT(m_handle != NULL);
#ifdef WINDOWS
	::ResetEvent(m_handle);
#endif //WINDOWS
}

#ifdef WINDOWS
unsigned int /*error code*/ Event::Wait(unsigned int timeout_milliseconds)
{
	unsigned int sys_ret_val = WaitForSingleObject(m_handle, timeout_milliseconds);
	return MapSystemErrorToError(sys_ret_val);
}
#endif //WINDOWS

#ifdef WINDOWS
Mutex::Mutex()
	: m_handle(NULL)
{
	m_handle = CreateMutex(NULL, false, NULL);
	if(m_handle == NULL)
	{
		throw Exception(SYNCHRONIZATION_ERROR_CANNOT_CREATE_EVENT,
			L"Cannot create mutex",
			EXC_HERE);
	}
}
#endif //WINDOWS

#ifdef WINDOWS
Mutex::~Mutex()
{
	if(m_handle != NULL)
	{
		::CloseHandle(m_handle);
	}
}
#endif //WINDOWS

#ifdef WINDOWS
unsigned int /*error code*/ Mutex::Wait(unsigned int timeout_milliseconds)
{
	unsigned int sys_ret_val = WaitForSingleObject(m_handle, timeout_milliseconds);
	return MapSystemErrorToError(sys_ret_val);
}
#endif //WINDOWS

#ifdef WINDOWS
unsigned int /*error code*/ Mutex::Release()
{
	if(ReleaseMutex(m_handle) == false)
	{
		unsigned int sys_error = GetLastError();
		return MapSystemErrorToError(sys_error);
	}
	return ERR_OK;
}
#endif //WINDOWS

#ifdef WINDOWS
/*
CriticalSection::CriticalSection()
{
	InitializeCriticalSection(&m_cs);
}

CriticalSection::~CriticalSection()
{
	DeleteCriticalSection(&m_cs);
}

unsigned int CriticalSection::Wait(unsigned int timeout_milliseconds)
{
	EnterCriticalSection(&m_cs);
	return SYNCH_WAIT_OK;
}

unsigned int CriticalSection::Release()
{
	LeaveCriticalSection(&m_cs);
	return SYNCH_WAIT_OK;
}
*/
#endif //WINDOWS

#ifdef WINDOWS

//Windows RWLock implementation

SRWReadWriteLock::SRWReadWriteLock()
{
	InitializeSRWLock(&m_lock);
}

SRWReadWriteLock::~SRWReadWriteLock()
{
	//looks like nothing..
}

bool SRWReadWriteLock::LockForRead()
{
	AcquireSRWLockShared(&m_lock);
	return true;
}

bool SRWReadWriteLock::LockForWrite()
{
	AcquireSRWLockExclusive(&m_lock);
	return true;
}

bool SRWReadWriteLock::TryLockForRead(unsigned int timeout_milliseconds)
{
	bool ret_val = false;
	Timeout timeout(timeout_milliseconds);
	unsigned int sleep_time(1000);
	if (timeout_milliseconds < sleep_time)
	{
		sleep_time = timeout_milliseconds;
	}
	while (timeout.IsElapsed() == false)
	{
		if (TryAcquireSRWLockShared(&m_lock) == 0)
		{
			Sleep(sleep_time);
		}
		else {
			m_locking_type = SHARED;
		}
	}
	return ret_val;
}

bool SRWReadWriteLock::TryLockForWrite(unsigned int timeout_milliseconds)
{
	bool ret_val = false;
	Timeout timeout(timeout_milliseconds);
	unsigned int sleep_time(1000);
	if (timeout_milliseconds < sleep_time)
	{
		sleep_time = timeout_milliseconds;
	}
	while (timeout.IsElapsed() == false)
	{
		if (TryAcquireSRWLockShared(&m_lock) == 0)
		{
			Sleep(sleep_time);
		}
		else {
			m_locking_type = EXCLUSIVE;
		}
	}
	return ret_val;
}

void SRWReadWriteLock::Unlock()
{
	switch (m_locking_type)
	{
	case EXCLUSIVE:
		ReleaseSRWLockExclusive(&m_lock);
		m_locking_type = UNLOCKED;
		break;
	case SHARED:
		ReleaseSRWLockShared(&m_lock);
		m_locking_type = UNLOCKED;
		break;
	default:;
	}
}

#endif //WINDOWS

//#endif //QT_VERSION