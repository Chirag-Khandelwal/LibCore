#include "Allocator.hpp"

#include "Logger.hpp"

// aligned_alloc doesn't exist on Windows, so we use _aligned_malloc and _aligned_free instead.
#if defined(CORE_OS_WINDOWS)
#include <malloc.h>
#define AlignedAlloc(align, sz) _aligned_malloc(sz, align)
#define AlignedFree(ptr) _aligned_free(ptr)
#else
#define AlignedAlloc(align, sz) std::aligned_alloc(align, sz)
#define AlignedFree(ptr) std::free(ptr)
#endif

namespace core
{

static Atomic<size_t> totalAllocRequests = 0, totalAllocBytes = 0, totalPoolAlloc = 0,
		      chunkReuseCount = 0;

MemoryManager::MemoryManager(StringRef name, size_t poolSize) : name(name), poolSize(poolSize)
{
	allocPool();
}
MemoryManager::~MemoryManager()
{
	// clear out the allocations that are larger than MAX_ROUNDUP
	for(auto &it : freechunks) {
		if(it.first > poolSize || it.second == 0) continue;
		size_t allocAddr = it.second;
		while(allocAddr > 0) {
			if(getAllocDetail(allocAddr, AllocDetails::SIZE) <= poolSize) break;
			AlignedFree((char *)allocAddr - ALLOC_DETAIL_BYTES);
			allocAddr = getAllocDetail(allocAddr, AllocDetails::NEXT);
		}
	}
	freechunks.clear();
	for(auto &p : pools) AlignedFree(p.mem);
	logger.info("=============== ", name, " memory manager stats: ===============");
	logger.info("-- Total allocated bytes (pools + otherwise): ", totalAllocBytes.load());
	logger.info("--                Allocated bytes from pools: ", totalPoolAlloc.load());
	logger.info("--                             Request count: ", totalAllocRequests.load());
	logger.info("--                         Chunk Reuse count: ", chunkReuseCount.load());
}

size_t MemoryManager::nextPow2(size_t sz)
{
	if(sz > MAX_ROUNDUP) return sz;
	--sz;
	sz |= sz >> 1;
	sz |= sz >> 2;
	sz |= sz >> 4;
	sz |= sz >> 8;
	sz |= sz >> 16;
	return ++sz;
}

void MemoryManager::allocPool()
{
	char *alloc = (char *)AlignedAlloc(MAX_ALIGNMENT, poolSize);
	totalAllocBytes += poolSize;
	pools.emplace_back(alloc, alloc);
}

void *MemoryManager::alloc(size_t size, size_t align)
{
	// align is unused for now.
	if(size == 0) return nullptr;

	// Add ALLOC_DETAIL_BYTES to the size since it is guaranteed
	// (static_assert) to be a multiple of MAX_ALIGNMENT.
	size_t requiredSz = size + ALLOC_DETAIL_BYTES;
	size_t allocSz	  = nextPow2(requiredSz);

	logger.trace("Allocating: ", allocSz, " (required size: ", requiredSz,
		     ") (original size: ", size, ")");

	char *loc = nullptr;

	++totalAllocRequests;
	if(allocSz > poolSize) {
		totalAllocBytes += allocSz;
		loc = (char *)AlignedAlloc(MAX_ALIGNMENT, allocSz);
		logger.trace("Allocated ", allocSz,
			     " using malloc as it exceeds pool size: ", poolSize);
	} else {
		totalPoolAlloc += allocSz;
		LockGuard<RecursiveMutex> mtxlock(mtx);
		// there is a free chunk available in the chunk list
		size_t addr = 0;
		auto it	    = freechunks.find(allocSz);
		if(it != freechunks.end()) addr = it->second;
		if(addr != 0) {
			loc	   = (char *)addr;
			it->second = getAllocDetail(addr, AllocDetails::NEXT);
			setAllocDetail(addr, AllocDetails::NEXT, 0);
			++chunkReuseCount;
			logger.trace("Allocated ", allocSz, " using chunk list");
			// No need to size size bytes here because they would have already been set
			// when they were taken from the pool.
			return loc;
		}

		// fetch a chunk from the pool
		for(auto &p : pools) {
			size_t freespace = poolSize - (p.head - p.mem);
			if(freespace >= allocSz) {
				loc = p.head;
				p.head += allocSz;
				logger.trace("Allocated ", allocSz, " using existing pool");
				break;
			}
		}
		if(!loc) {
			allocPool();
			auto &p = pools.back();
			loc	= p.head;
			p.head += allocSz;
			logger.trace("Allocated ", allocSz, " using a newly generated pool");
		}
	}
	loc += ALLOC_DETAIL_BYTES;
	setAllocDetail((size_t)loc, AllocDetails::SIZE, allocSz);
	setAllocDetail((size_t)loc, AllocDetails::NEXT, 0);
	return loc;
}

void MemoryManager::free(void *data)
{
	if(data == nullptr) return;
	char *loc = (char *)data;
	size_t sz = getAllocDetail((size_t)loc, AllocDetails::SIZE);
	if(sz > poolSize) {
		AlignedFree(loc - ALLOC_DETAIL_BYTES);
		return;
	}
	LockGuard<RecursiveMutex> mtxlock(mtx);
	auto mapLoc = freechunks.find(sz);
	if(mapLoc == freechunks.end()) {
		freechunks.insert({sz, (size_t)loc});
		return;
	}
	setAllocDetail((size_t)loc, AllocDetails::NEXT, mapLoc->second);
	mapLoc->second = (size_t)loc;
}

void MemoryManager::dumpMem(char *pool)
{
	constexpr size_t charSize     = 2; // in bytes
	constexpr size_t charsPerLine = 64 * charSize;
	for(size_t i = 0; i < poolSize; i += charSize) {
		if(i % charsPerLine == 0) std::cout << "\n" << (void *)(pool + i) << " :: ";
		std::cout << std::hex << (*(uint16_t *)(pool + i)) << " ";
	}
	std::cout << std::dec << "\n";
}

IAllocated::IAllocated() {}
IAllocated::~IAllocated() {}

SimpleAllocator::SimpleAllocator(MemoryManager &mem, StringRef name) : mem(mem), name(name) {}

ManagedAllocator::ManagedAllocator(MemoryManager &mem, StringRef name)
	: allocator(mem, name), start(0)
{}
ManagedAllocator::~ManagedAllocator()
{
	size_t count = 0;
	while((size_t)start > 0) {
		++count;
		IAllocated *next =
		(IAllocated *)allocator.getAllocDetail((size_t)start, AllocDetails::NEXT);
		allocator.setAllocDetail((size_t)start, AllocDetails::NEXT, 0);
		allocator.free(start);
		start = next;
	}
	logger.trace(getName(), " allocator had ", count, " allocations");
}

} // namespace core