#pragma once

#include "Core.hpp"

namespace core
{

enum class AllocDetails : uint32_t
{
	// Size in bytes of the allocation
	SIZE,
	// Address of next free allocation of the same size
	// Can be used for something else while the memory is allocated -
	// make sure to reset the value to zero before freeing though
	// The address points to the usable location, so to get AllocDetail from there,
	// you must do: (char*)loc - sizeof(AllocDetail)
	NEXT,

	_LAST,
};

using AllocDetail = size_t[static_cast<uint32_t>(AllocDetails::_LAST)];

constexpr size_t MAX_ROUNDUP	    = 2048;
constexpr size_t DEFAULT_POOL_SIZE  = 8 * 1024;
constexpr size_t MAX_ALIGNMENT	    = alignof(std::max_align_t);
constexpr size_t ALLOC_DETAIL_BYTES = sizeof(AllocDetail);

// Extra allocation bytes must be a multiple of MAX_ALIGNMENT.
// That way, when we allocate the memory, and move forward by ALLOC_DETAIL_BYTES,
// we are still on an address that is a multiple of MAX_ALIGNMENT.
static_assert(ALLOC_DETAIL_BYTES % MAX_ALIGNMENT == 0,
	      "sizeof(AllocDetail) must be a multiple of max alignment");

struct MemPool
{
	char *head;
	char *mem;
};

class MemoryManager
{
	// The size_t at freechunks[sz] is an address which holds an allocation.
	// This address is after ALLOC_DETAIL_BYTES bytes.
	Map<size_t, size_t> freechunks;
	Vector<MemPool> pools;
	// TODO: have multiple arenas and then use mutexes individual to those arenas
	// then, in a mutithreaded environment, the manager should be able to choose one of the
	// available arenas and allocate memory from that (therefore increasing speed, compared to a
	// global mutex).
	RecursiveMutex mtx;
	String name;
	size_t poolSize;

	// works upto MAX_ROUNDUP
	size_t nextPow2(size_t sz);
	void allocPool();

public:
	MemoryManager(StringRef name, size_t poolSize = DEFAULT_POOL_SIZE);
	~MemoryManager();

	void *alloc(size_t size, size_t align);
	void free(void *data);

	// Helper function - only use if seeing memory issues.
	void dumpMem(char *pool);

	template<typename T, typename... Args> T *alloc(Args &&...args)
	{
		void *m = alloc(sizeof(T), alignof(T));
		return new(m) T(std::forward<Args>(args)...);
	}

	// alloc address must be AFTER sizeof(AllocDetail)
	inline void setAllocDetail(size_t alloc, AllocDetails field, size_t value)
	{
		(*(AllocDetail *)((char *)alloc -
				  ALLOC_DETAIL_BYTES))[static_cast<uint32_t>(field)] = value;
	}
	// alloc address must be AFTER sizeof(AllocDetail)
	inline size_t getAllocDetail(size_t alloc, AllocDetails field)
	{
		return (
		*(AllocDetail *)((char *)alloc - ALLOC_DETAIL_BYTES))[static_cast<uint32_t>(field)];
	}

	inline size_t getPoolSize() { return poolSize; }
	inline size_t getPoolCount() { return pools.size(); }
};

// Base class for anything that uses the allocator
class IAllocated
{
public:
	IAllocated();
	virtual ~IAllocated();
};

// Can be a static object - any allocation is expected to be free'd manually.
class SimpleAllocator
{
	MemoryManager &mem;
	String name;

public:
	SimpleAllocator(MemoryManager &mem, StringRef name);

	inline StringRef getName() { return name; }

	// alloc address must be AFTER sizeof(AllocDetail)
	inline void setAllocDetail(size_t alloc, AllocDetails field, size_t value)
	{
		return mem.setAllocDetail(alloc, field, value);
	}
	// alloc address must be AFTER sizeof(AllocDetail)
	inline size_t getAllocDetail(size_t alloc, AllocDetails field)
	{
		return mem.getAllocDetail(alloc, field);
	}

	template<typename T, typename... Args>
	typename std::enable_if<std::is_base_of<IAllocated, T>::value, T *>::type
	alloc(Args &&...args)
	{
		T *res = mem.alloc<T>(std::forward<Args>(args)...);
		return res;
	}
	void free(IAllocated *data)
	{
		data->~IAllocated();
		mem.free(data);
	}
};

// Cannot be a static object - as it uses the static variable `logger` in destructor.
// RAII based - does not allow freeing of the memory unless it goes out of scope.
class ManagedAllocator
{
	SimpleAllocator allocator;
	IAllocated *start;

public:
	ManagedAllocator(MemoryManager &mem, StringRef name);
	~ManagedAllocator();

	inline StringRef getName() { return allocator.getName(); }

	template<typename T, typename... Args>
	typename std::enable_if<std::is_base_of<IAllocated, T>::value, T *>::type
	alloc(Args &&...args)
	{
		T *res = allocator.alloc<T>(std::forward<Args>(args)...);
		allocator.setAllocDetail((size_t)res, AllocDetails::NEXT, (size_t)start);
		start = res;
		return res;
	}
};

} // namespace core