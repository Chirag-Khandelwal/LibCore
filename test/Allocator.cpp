#include "Allocator.hpp"

#include <catch2/catch_all.hpp>

using namespace core;

TEST_CASE("Basic", "Allocator")
{
	MemoryManager mgr("Basic");

	REQUIRE(mgr.getPoolSize() == DEFAULT_POOL_SIZE);
	int *alloc = mgr.alloc<int>(5);
	REQUIRE(alloc != nullptr);
	REQUIRE(((uintptr_t)alloc) % MAX_ALIGNMENT == 0);
	REQUIRE(*alloc == 5);
	REQUIRE(mgr.getPoolCount() == 1);
	mgr.free(alloc);
	REQUIRE(mgr.getPoolCount() == 1);
}