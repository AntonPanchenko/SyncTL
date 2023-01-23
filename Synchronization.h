#ifndef SYNCHRONIZATION_H_INCLUDED
#define SYNCHRONIZATION_H_INCLUDED

#if ((defined _WIN64) || (defined _WIN32))
#define WINDOWS
#endif	//_WIN32 || _WIN64

#ifdef WINDOWS
#include <Windows.h>
#include <synchapi.h>
#endif //WIN32

#include "Utils.h"
#ifdef QT_VERSION
#include <qglobal.h>
#endif //WT_VERSION

#include <atomic>

namespace SyncTL
{

enum
{
	SYNCHRONIZATION_ERROR_OK = 0,
	SYNCHRONIZATION_ERROR_OPERATION_CANCELLED  = SYNCHRONIZATION_ERROR_BASE,
	SYNCHRONIZATION_ERROR_CANNOT_GET_LOCK_FOR_READ,
	SYNCHRONIZATION_ERROR_CANNOT_GET_LOCK_FOR_WRITE,
	SYNCHRONIZATION_ERROR_CANNOT_RELEASE_LOCK,
	SYNCHRONIZATION_ERROR_CANNOT_CREATE_LOCK,
	SYNCHRONIZATION_ERROR_CANNOT_CREATE_EVENT,
	SYNCHRONIZATION_ERROR_NO_RW_LOCK
};

class BasicReadWriteLock
{
public:
	virtual ~BasicReadWriteLock()
	{}
	virtual bool LockForRead() = 0;
	virtual bool LockForWrite() = 0;
	virtual bool TryLockForRead(unsigned int timeout_millisoconds = 0) = 0;
	virtual bool TryLockForWrite(unsigned int timeout_milliseconds = 0) = 0;
	virtual void Unlock() = 0;
};

#ifdef QT_VERSION

class QReadWriteLock;

class QtReadWriteLock : public BasicReadWriteLock
{
public:
	QtReadWriteLock(bool recursive = true);
	virtual ~QtReadWriteLock();
	bool LockForRead();
	bool LockForWrite();
	bool TryLockForRead(unsigned int timeout_milliseconds = 0);
	bool TryLockForWrite(unsigned int timeout_milliseconds = 0);
	void Unlock();
protected:
	QReadWriteLock* m_lock;
};

#define ReadWriteLock QtReadWriteLock

#elif defined WINDOWS
//there is a srvlock in Windows starting from vista/2008
//https://docs.microsoft.com/en-us/windows/win32/sync/slim-reader-writer--srw--locks

#pragma message ("there MUST be SWReadwriteLock !!!!!!!!!!!")

class SRWReadWriteLock : public BasicReadWriteLock
{
public:
	SRWReadWriteLock();
	virtual ~SRWReadWriteLock();
	virtual bool LockForRead();
	virtual bool LockForWrite();
	virtual bool TryLockForRead(unsigned int timeout_millisoconds = 0);
	virtual bool TryLockForWrite(unsigned int timeout_milliseconds = 0);
	virtual void Unlock();
protected:
	enum
	{
		UNLOCKED = 0,
		EXCLUSIVE,
		SHARED
	};
	SRWLOCK m_lock;
	unsigned int m_locking_type;
};

typedef SRWReadWriteLock ReadWriteLock;

#elif defined POSIX
//there is posix rwlock
//https://pubs.opengroup.org/onlinepubs/007904975/functions/pthread_rwlock_rdlock.html
#else //no rwlock, must implement on my own
#endif // rwlokcs implementation





enum
{
	SYNCH_WAIT_OK = 0,
	SYNCH_WAIT_TIMEOUT = 1,
	SYNCH_WAIT_ABANDONED = 2,
	SYNCH_WAIT_FAILED = 3
};

enum Synch
{
	WaitInfinite = INT_MAX
};

class SynchronizationObject
{
public:
	virtual ~SynchronizationObject() {}
	virtual unsigned int /*error code*/ Wait(unsigned int timeout_milliseconds = Synch::WaitInfinite) = 0;
	unsigned int MapSystemErrorToError(unsigned int sys_error);
};

class ReleasableSynchronizationObject : public SynchronizationObject
{
public:
	virtual unsigned int /*error code*/ Release() = 0;
};

class Event: public SynchronizationObject
{
public:
	Event(bool is_manual_reset = true, bool initial_state = false);
	virtual ~Event();
	void SetEvent();
	void ClearEvent();
	unsigned int /*error code*/ Wait(unsigned int timeout_milliseconds = Synch::WaitInfinite);
protected:
#ifdef WINDOWS
	typedef HANDLE EventHandle;
	EventHandle m_handle;
#endif //WINDOWS
};

class Mutex: public ReleasableSynchronizationObject
{
public:
	Mutex();
	virtual ~Mutex();
	unsigned int /*error code*/ Wait(unsigned int timeout_milliseconds = Synch::WaitInfinite);
	unsigned int /*error code*/ Release();
protected:
#ifdef WINDOWS
	typedef HANDLE MutexHandle;
	MutexHandle m_handle;
#endif //WINDOW
};

//class CriticalSection : public ReleasableSynchronizationObject
//{
//public:
//	CriticalSection();
//	virtual ~CriticalSection();
//	//timeout_milliseconds on Windows is ignored because WinAPI functions for critical section dont support it.
//	unsigned int /*error code*/ Wait(unsigned int timeout_milliseconds = Synch::WaitInfinite);
//	unsigned int /*error code*/ Release();
//protected:
//	CRITICAL_SECTION m_cs;
//};

class Synchronizer
{
public:
	Synchronizer(ReleasableSynchronizationObject* so) : m_so(so)
	{
		ASSERT(m_so != NULL);
		m_so->Wait();
	}
	virtual ~Synchronizer()
	{
		ASSERT(m_so != NULL);
		m_so->Release();
	}
protected:
	ReleasableSynchronizationObject* m_so;
};

class BasicRWSynchronizer
{
public:
	BasicRWSynchronizer(BasicReadWriteLock* rwl):
		m_rw_lock(rwl)
	{}
	virtual ~BasicRWSynchronizer()
	{
		if (m_rw_lock != NULL)
		{
			m_rw_lock->Unlock();
		}
	}
protected:
	BasicReadWriteLock* m_rw_lock;
};

class ReadSynchronizer: public BasicRWSynchronizer
{
public:
	ReadSynchronizer(BasicReadWriteLock* rwl) :
		BasicRWSynchronizer(rwl)
	{
		if (m_rw_lock != NULL)
		{
			bool ok = m_rw_lock->LockForRead();
			if (ok == false)
			{
				throw Exception(SYNCHRONIZATION_ERROR_CANNOT_GET_LOCK_FOR_READ,
					L"Cannot lock for read",
					EXC_HERE);
			}
		}
	}
};

class TryReadSynchronizer: public BasicRWSynchronizer
{
public:
	TryReadSynchronizer(BasicReadWriteLock* rwl, unsigned int timeout_milliseconds = 0) :
		BasicRWSynchronizer(rwl)
	{
		if (m_rw_lock != NULL)
		{
			bool ok = m_rw_lock->TryLockForRead(timeout_milliseconds);
			if (ok == false)
			{
				throw Exception(SYNCHRONIZATION_ERROR_CANNOT_GET_LOCK_FOR_READ,
					L"Cannot lock for read",
					EXC_HERE);
			}
		}
	}
};

class WriteSynchronizer: public BasicRWSynchronizer
{
public:
	WriteSynchronizer(BasicReadWriteLock* rwl) :
		BasicRWSynchronizer(rwl)
	{
		if (m_rw_lock != NULL)
		{
			bool ok = m_rw_lock->LockForWrite();
			if (ok == false)
			{
				throw Exception(SYNCHRONIZATION_ERROR_CANNOT_GET_LOCK_FOR_READ,
					L"Cannot lock for write",
					EXC_HERE);
			}
		}
	}
};

class TryWriteSynchronizer: public BasicRWSynchronizer
{
public:
	TryWriteSynchronizer(BasicReadWriteLock* rwl, unsigned int timeout_milliseconds = 0) :
		BasicRWSynchronizer(rwl)
	{
		if (m_rw_lock != NULL)
		{
			bool ok = m_rw_lock->TryLockForWrite(timeout_milliseconds);
			if (ok == false)
			{
				throw Exception(SYNCHRONIZATION_ERROR_CANNOT_GET_LOCK_FOR_READ,
					L"Cannot lock for write",
					EXC_HERE);
			}
		}
	}
};

template <class Synchronizeable>
class BasicTemplateSynchronizer
{
public:
	BasicTemplateSynchronizer(Synchronizeable* object) :
		m_object(object)
	{
		//ASSERT(m_object != NULL);
	}
	virtual ~BasicTemplateSynchronizer()
	{
		//ASSERT(m_object != NULL);
		if (m_object != NULL)
		{
			m_object->Unlock();
		}
	}
protected:
	Synchronizeable* m_object;
};

/*
template <class Synchronizeable>
class TestSynchronizer : public BasicTemplateSynchronizer<Synchronizeable>
{
public:
	typedef BasicTemplateSynchronizer<Synchronizeable> BaseClass; //this works
	TestSynchronizer(Synchronizeable* s):
		BasicTemplateSynchronizer(s)
	{
		//m_object = s;
		BaseClass::m_object->LockForRead();
	}
};*/

template <typename Synchronizeable>
class TemplateReadSynchronizer : public virtual BasicTemplateSynchronizer<Synchronizeable>
{
public:
	typedef BasicTemplateSynchronizer<Synchronizeable> BaseClass;
	TemplateReadSynchronizer(Synchronizeable* object) :
		BasicTemplateSynchronizer<Synchronizeable>(object)
	{
		if (BaseClass::m_object != NULL)
		{
			BaseClass::m_object->LockForRead();
		}
	}
};

template <typename Synchronizeable>
class TemplateWriteSynchronizer: public virtual BasicTemplateSynchronizer<Synchronizeable>
{
public:
	typedef BasicTemplateSynchronizer<Synchronizeable> BaseClass;
	TemplateWriteSynchronizer(Synchronizeable* object) :
		BasicTemplateSynchronizer<Synchronizeable>(object)
	{
		if (BaseClass::m_object != NULL)
		{
			BaseClass::m_object->LockForWrite();
		}
	}
};

template <class Synchronizeable>
class TemplateRWSynchronizer : public TemplateReadSynchronizer<Synchronizeable>,
							   public TemplateWriteSynchronizer<Synchronizeable>
{
public:
	//typedef TemplateReadSynchronizer<Synchronizeable> ReadSyncBaseClass;
	//typedef TemplateWriteSynchronizer<Synchronizeable> WriteSyncBaseClass;
	TemplateRWSynchronizer(Synchronizeable* object):
		BasicTemplateSynchronizer<Synchronizeable>(object),
		TemplateWriteSynchronizer<Synchronizeable>(object),	//because this lock is exclusive it comes first.
		TemplateReadSynchronizer<Synchronizeable>(object)
		
	{}
};

template <bool is_exclusive>
class LockGuard
{
public:
	LockGuard(BasicReadWriteLock* rw_lock) :
		m_rw_lock(rw_lock)
	{
		if (m_rw_lock == NULL)
		{
			throw Exception(SYNCHRONIZATION_ERROR_NO_RW_LOCK,
				L"no rw lock provided for lock guard",
				EXC_HERE);
		}
		if (is_exclusive)
		{
			m_rw_lock->LockForWrite();
		}
		else {
			m_rw_lock->LockForRead();
		}
	}
	virtual ~LockGuard()
	{
		if (m_rw_lock != NULL)
		{
			m_rw_lock->Unlock();
		}
	}
protected:
	BasicReadWriteLock* m_rw_lock;
};

class FastLockGuard
{
public:
	FastLockGuard(std::atomic_flag* atomic)	//no need for timeouts here - this is FAST spin lock.
		:m_atomic(atomic)
	{
		if (m_atomic != NULL)
		{
			while (m_atomic->test_and_set(std::memory_order_acquire) == false)
			{ /*do nothing*/
			}
		}
	}
	~FastLockGuard()
	{
		if (m_atomic != NULL)
		{
			m_atomic->clear(std::memory_order_acquire);
		}
	}
protected:
	std::atomic_flag* m_atomic;
};

//#endif //QT_VERSION

} //end namespace SyncTL

#endif //SYNCHRONIZATION_H_INCLUDED