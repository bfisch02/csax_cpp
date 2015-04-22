/*
  The contents of this file are dedicated by all of its authors, including

    Michael S. Gashler,
    Eric Moyer,
    anonymous contributors,

  to the public domain (http://creativecommons.org/publicdomain/zero/1.0/).

  Note that some moral obligations still exist in the absence of legal ones.
  For example, it would still be dishonest to deliberately misrepresent the
  origin of a work. Although we impose no legal requirements to obtain a
  license, it is beseeming for those who build on the works of others to
  give back useful improvements, or pay it forward in their own field. If
  you would like to cite us, a published paper about Waffles can be found
  at http://jmlr.org/papers/volume12/gashler11a/gashler11a.pdf. If you find
  our code to be useful, the Waffles team would love to hear how you use it.
*/

#ifndef __GHOLDERS_H__
#define __GHOLDERS_H__

#include <stdio.h>
#include <string>
#ifdef WINDOWS
#	include <malloc.h>
#	pragma warning(disable: 4996)
#else
# ifdef __FreeBSD__
#  include <stdlib.h>
# else
#  include <alloca.h>
# endif
#endif
#include <vector>

namespace GClasses {

/// The threshold over which a temporary buffer will be placed on the heap instead of the stack.
#define MAX_STACK_TEMP_BUFFER 1024

/// A helper class used by the GTEMPBUF macro
class GTempBufHelper
{
public:
	char* m_pBuf;
	GTempBufHelper(size_t nSize)
	{
		m_pBuf = ((nSize > MAX_STACK_TEMP_BUFFER) ? new char[nSize] : NULL);
	}

	~GTempBufHelper()
	{
		delete[] m_pBuf;
	}
};


// Macro for allocating a temporary buffer
#ifdef _DEBUG
/// This is a helper class used by the debug version of the GTEMPBUF macro to help detect buffer overruns.
class GTempBufSentinel
{
	void* m_pBuf;

public:
	GTempBufSentinel(void* pBuf);
	~GTempBufSentinel();
};

/// A macro for allocating a temporary buffer. If the buffer is small, it will use alloca to put it on
/// the stack. If the buffer is big, it will allocate it on the heap and use a holder to ensure that it
/// is properly deleted.
#	define GTEMPBUF(typ, var, cnt)\
	GTempBufHelper var##__(sizeof(typ) * (cnt) + 1);\
	typ* var = (((sizeof(typ) * (cnt)) <= MAX_STACK_TEMP_BUFFER) ? (typ*)alloca(sizeof(typ) * (cnt) + 1) : (typ*)var##__.m_pBuf);\
	GTempBufSentinel var##_sentinel(&var[cnt]);
#else

/// A macro for allocating a temporary buffer. If the buffer is small, it will use alloca to put it on
/// the stack. If the buffer is big, it will allocate it on the heap and use a holder to ensure that it
/// is properly deleted.
#	define GTEMPBUF(typ, var, cnt)\
	GTempBufHelper var##__(sizeof(typ) * (cnt));\
	typ* var = (((sizeof(typ) * (cnt)) <= MAX_STACK_TEMP_BUFFER) ? (typ*)alloca(sizeof(typ) * (cnt)) : (typ*)var##__.m_pBuf);
#endif


void verboten();

/// This class is very similar to the standard C++ class auto_ptr,
/// except it throws an exception if you try to make a copy of it.
/// This way, it will fail early if you use it in a manner that
/// could result in non-deterministic behavior. (For example, if you
/// create a vector of auto_ptrs, wierd things happen if an oom exception
/// is thrown while resizing the buffer--part of the data will be lost
/// when it reverts back to the original buffer. But if you make a
/// vector of these, it will fail quickly, thus alerting you to the
/// issue.)
template <class T>
class Holder
{
private:
	T* m_p;

public:
	Holder(T* p = NULL)
	{
		//COMPILER_ASSERT(sizeof(T) > sizeof(double));
		m_p = p;
	}

private:
	/// Private copy constructor so that the attempts to copy a
	/// holder are caught at compile time rather than at run time
	Holder(const Holder& other)
	{
		verboten();
	}
public:
	/// Deletes the object that is being held
	~Holder()
	{
		delete(m_p);
	}
private:
	/// Private operator= so that the attempts to copy a
	/// holder are caught at compile time rather than at run time
	const Holder& operator=(const Holder& other)
	{
		verboten();
		return *this;
	}
public:
	/// Deletes the object that is being held, and sets the holder
	/// to hold p.  Will not delete the held pointer if the new
	/// pointer is the same.
	void reset(T* p = NULL)
	{
		if(p != m_p)
		{
			delete(m_p);
			m_p = p;
		}
	}

	/// Returns a pointer to the object being held
	T* get()
	{
		return m_p;
	}

	/// Releases the object. (After calling this method, it is your job to delete the object.)
	T* release()
	{
		T* pTmp = m_p;
		m_p = NULL;
		return pTmp;
	}

	T& operator*() const
	{
		return *m_p;
	}

	T* operator->() const
	{
		return m_p;
	}
};

/// Just like Holder, except for arrays
template <class T>
class ArrayHolder
{
private:
	T* m_p;

public:
	ArrayHolder(T* p = NULL)
	{
		m_p = p;
	}
private:	
	ArrayHolder(const ArrayHolder& other)
	{
		verboten();
	}
public:	
	/// Deletes the array of objects being held
	~ArrayHolder()
	{
		delete[] m_p;
	}
private:
	const ArrayHolder& operator=(const ArrayHolder& other)
	{
		verboten();
		return *this;
	}
public:
	/// Deletes the array of objects being held and sets this holder to hold NULL
	void reset(T* p = NULL)
	{
		if(p != m_p)
		{
			delete[] m_p;
			m_p = p;
		}
	}

	/// Returns a pointer to the first element of the array being held
	T* get()
	{
		return m_p;
	}

	/// Releases the array. (After calling this method, it is your job to delete the array.)
	T* release()
	{
		T* pTmp = m_p;
		m_p = NULL;
		return pTmp;
	}

	T& operator[](size_t n)
	{
		return m_p[n];
	}
};

/// Closes a file when this object goes out of scope
class FileHolder
{
private:
	FILE* m_pFile;

public:
	FileHolder()
	{
		m_pFile = NULL;
	}

	FileHolder(FILE* pFile)
	{
		m_pFile = pFile;
	}

	/// Close the file
	~FileHolder()
	{
		if(m_pFile)
			fclose(m_pFile);
	}

	/// Close the file and set this holder to hold NULL
	void reset(FILE* pFile = NULL);

	/// Returns a pointer to the FILE being held
	FILE* get()
	{
		return m_pFile;
	}

	/// Releases the FILE (it is now your job to close it) and sets this holder to hold NULL
	FILE* release()
	{
		FILE* pFile = m_pFile;
		m_pFile = NULL;
		return pFile;
	}
};

/// Deletes all of the pointers in a vector when this object goes out of scope.
template <class T>
class VectorOfPointersHolder
{
protected:
	std::vector<T*>& m_vec;

public:
	VectorOfPointersHolder(std::vector<T*>& vec)
	: m_vec(vec)
	{
	}

	/// Deletes all of the pointers in the vector
	~VectorOfPointersHolder()
	{
		for(typename std::vector<T*>::iterator it = m_vec.begin(); it != m_vec.end(); it++)
			delete(*it);
	}
};

/// A helper class used by the smart_ptr class.
template <class T>
class smart_ptr_ref_counter
{
public:
	T* m_p;
	size_t m_refCount;

	smart_ptr_ref_counter(T* p)
	: m_p(p), m_refCount(1)
	{
	}

	~smart_ptr_ref_counter()
	{
		delete(m_p);
	}
};

/// A reference-counting smart-pointer.
template <class T>
class smart_ptr
{
protected:
	smart_ptr_ref_counter<T>* m_pRefCounter;

	template<class S>
	friend class smart_ptr;
public:
	smart_ptr()
	: m_pRefCounter(NULL)
	{
	}

	smart_ptr(const smart_ptr& other)
	: m_pRefCounter(other.m_pRefCounter)
	{
		if(m_pRefCounter)
			m_pRefCounter->m_refCount++;
	}

	///\brief Make a smart pointer of a different type (T) that shares
	///referent and ownership with \a other
	///
	///This is mainly for creating smart_pointer<const T> from
	///smart_pointer<T>.
	///
	///The basic use is:\code
	///smart_ptr<int> original_ptr(new int);
	///int const* type_tag = NULL;
	///smart_ptr<const int> new_ptr(original_ptr, type_tag);
	///\endcode
	///
	///\warning Using this code for anything other than creating a
	///         smart_pointer<const T> from smart_pointer<T> is
	///         dangerous and may do bad things due to pointer
	///         aliasing.  You do it at your own risk.
	///
	///\post get() == (T*)other.get() and when \a other goes out of
	///      scope, it decreases the reference count for *this and
	///      vice-versa
	template <class S>
	smart_ptr(const smart_ptr<S>& other, T*):
		m_pRefCounter((smart_ptr_ref_counter<T>*)other.m_pRefCounter)
	{
		if(m_pRefCounter)
			m_pRefCounter->m_refCount++;
	}



	template <class S>
	smart_ptr(S* pThat)
	{
		m_pRefCounter = new smart_ptr_ref_counter<T>(pThat);
	}

	~smart_ptr()
	{
		reset();
	}

	smart_ptr& operator=(const smart_ptr& that)
	{
		if(that.m_pRefCounter == m_pRefCounter)
			return *this;
		reset();
		if(that.m_pRefCounter)
		{
			m_pRefCounter = that.m_pRefCounter;
			m_pRefCounter->m_refCount++;
		}
		return *this;
	}

	template <class S>
	smart_ptr& operator=(S* pThat)
	{
		reset();
		if(pThat)
			m_pRefCounter = new smart_ptr_ref_counter<T>(pThat);
		return *this;
	}

	T& operator*() const
	{
		return *m_pRefCounter->m_p;
	}

	T* operator->() const
	{
		return m_pRefCounter->m_p;
	}

	T* get() const
	{
		return m_pRefCounter ? m_pRefCounter->m_p : NULL;
	}

	void reset()
	{
		if(m_pRefCounter)
		{
			if(--m_pRefCounter->m_refCount == 0)
				delete(m_pRefCounter);
			m_pRefCounter = NULL;
		}
	}

	size_t refCount()
	{
		return m_pRefCounter->m_refCount;
	}
};

/// Placing these on the stack can help catch buffer overruns
class GOverrunSentinel
{
protected:
	unsigned int m_sentinel;

public:
	GOverrunSentinel() : m_sentinel(0x5e47143a)
	{
	}

	~GOverrunSentinel()
	{
		Check();
	}

	void Check();
};



} // namespace GClasses

#endif // __GHOLDERS_H__
