#include "Allocator.hpp"

#include <catch2/catch_all.hpp>

using namespace core;

class Test : public IAllocated
{
public:
    int p;
    Test(int p) : p(p) {}
};

TEST_CASE("MemoryManager", "Basic")
{
    MemoryManager mem("Basic");

    REQUIRE(mem.getPoolSize() == DEFAULT_POOL_SIZE);
    int *alloc = (int *)mem.allocRaw(sizeof(int), alignof(int));
    *alloc     = 5;
    REQUIRE(alloc != nullptr);
    REQUIRE(((uintptr_t)alloc) % MAX_ALIGNMENT == 0);
    REQUIRE(*alloc == 5);
    REQUIRE(mem.getPoolCount() == 1);
    mem.freeRaw(alloc);
    REQUIRE(mem.getPoolCount() == 1);
}

TEST_CASE("ManagedList", "Basic")
{
    MemoryManager mem("Basic");
    ManagedList allocator(mem, "ManagedList");

    Test *res = allocator.alloc<Test>(5);

    REQUIRE(res->p == 5);
}

TEST_CASE("ManagedRawList", "Basic")
{
    MemoryManager mem("Basic");
    ManagedRawList allocator1(mem, "ManagedRawList1");
    ManagedRawList *allocator2 = mem.allocInit<ManagedRawList>(mem, "ManagedRawList2");

    char *h  = allocator1.allocInit<char>("hello", 6);
    char *d  = allocator1.allocInit<char>("dear", 5);
    char *w  = allocator1.allocInit<char>("world", 6);
    char *ex = allocator1.allocInit<char>("!", 2);

    REQUIRE(allocator1.size() == 4);
    REQUIRE(!allocator1.empty());
    REQUIRE(StringRef(h) == "hello");
    char *iter = nullptr;
    while((iter = (char *)allocator1.next(iter))) {
        StringRef item = iter;
        REQUIRE((item == "hello" || item == "dear" || item == "world" || item == "!"));
    }
    iter = nullptr;
    while((iter = (char *)allocator1.prev(iter))) {
        StringRef item = iter;
        REQUIRE((item == "hello" || item == "dear" || item == "world" || item == "!"));
    }

    REQUIRE(allocator2->empty());
    char *h2 = (char *)allocator2->add(allocator1.remove(h));
    REQUIRE(allocator2->size() == 1);
    REQUIRE(allocator1.size() == 3);
    REQUIRE(h == h2);
    iter = nullptr;
    allocator1.free(d);
    allocator1.free(1);
    while((iter = (char *)allocator1.next(iter))) {
        StringRef item = iter;
        REQUIRE((item == "world"));
    }
    iter = nullptr;
    while((iter = (char *)allocator2->next(iter))) {
        StringRef item = iter;
        REQUIRE((item == "hello"));
    }
    REQUIRE((allocator2->clear() == 1));
    mem.freeDeinit(allocator2);
}