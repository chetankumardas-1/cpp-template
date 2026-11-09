#include "lru_cache.h"

#include <gtest/gtest.h>

#include <atomic>
#include <barrier>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

TEST(LRUCacheTest, InsertsAndRetrievesElements)
{
    LRUCache<int, std::string> cache(3);

    cache.put(1, "one");
    cache.put(2, "two");

    EXPECT_EQ(cache.get(1), "one");
    EXPECT_EQ(cache.get(2), "two");
    EXPECT_EQ(cache.size(), 2U);
    EXPECT_EQ(cache.capacity(), 3U);
}

TEST(LRUCacheTest, UpdatesExistingKey)
{
    LRUCache<int, std::string> cache(2);

    cache.put(1, "old");
    cache.put(1, "new");

    EXPECT_EQ(cache.get(1), "new");
    EXPECT_EQ(cache.size(), 1U);
}

TEST(LRUCacheTest, UpdatingExistingKeyPromotesItToMostRecentlyUsed)
{
    LRUCache<int, std::string> cache(2);

    cache.put(1, "one");
    cache.put(2, "two");
    cache.put(1, "updated one");
    cache.put(3, "three");

    EXPECT_FALSE(cache.get(2).has_value());
    EXPECT_EQ(cache.get(1), "updated one");
    EXPECT_EQ(cache.get(3), "three");
}

TEST(LRUCacheTest, EvictsLeastRecentlyUsedElement)
{
    LRUCache<int, int> cache(2);

    cache.put(1, 10);
    cache.put(2, 20);
    cache.put(3, 30);

    EXPECT_FALSE(cache.get(1).has_value());
    EXPECT_EQ(cache.get(2), 20);
    EXPECT_EQ(cache.get(3), 30);
}

TEST(LRUCacheTest, AccessChangesEvictionOrder)
{
    LRUCache<int, int> cache(2);

    cache.put(1, 10);
    cache.put(2, 20);
    ASSERT_EQ(cache.get(1), 10);
    cache.put(3, 30);

    EXPECT_EQ(cache.get(1), 10);
    EXPECT_FALSE(cache.get(2).has_value());
    EXPECT_EQ(cache.get(3), 30);
}

TEST(LRUCacheTest, MissingKeyReturnsEmptyOptional)
{
    LRUCache<int, int> cache(2);

    EXPECT_FALSE(cache.get(42).has_value());
    EXPECT_EQ(cache.size(), 0U);
}

TEST(LRUCacheTest, ZeroCapacityDoesNotStoreElements)
{
    LRUCache<int, int> cache(0);

    cache.put(1, 10);

    EXPECT_EQ(cache.capacity(), 0U);
    EXPECT_EQ(cache.size(), 0U);
    EXPECT_FALSE(cache.get(1).has_value());
}

TEST(LRUCacheTest, CapacityOneRetainsOnlyMostRecentElement)
{
    LRUCache<int, int> cache(1);

    cache.put(1, 10);
    cache.put(2, 20);

    EXPECT_FALSE(cache.get(1).has_value());
    EXPECT_EQ(cache.get(2), 20);
    EXPECT_EQ(cache.size(), 1U);
}

namespace
{
struct ThrowOnCopy
{
    explicit ThrowOnCopy(int input)
        : value(input)
    {
    }

    ThrowOnCopy(const ThrowOnCopy &other)
        : value(other.value)
    {
        if (throw_on_copy)
            throw std::runtime_error("copy failed");
    }

    ThrowOnCopy(ThrowOnCopy &&) noexcept = default;
    ThrowOnCopy &operator=(ThrowOnCopy &&) noexcept = default;

    int value;
    static bool throw_on_copy;
};

bool ThrowOnCopy::throw_on_copy = false;
} // namespace

TEST(LRUCacheTest, FailedGetCopyDoesNotChangeRecency)
{
    LRUCache<int, ThrowOnCopy> cache(2);
    cache.put(1, ThrowOnCopy(10));
    cache.put(2, ThrowOnCopy(20));

    ThrowOnCopy::throw_on_copy = true;
    EXPECT_THROW(cache.get(1), std::runtime_error);
    ThrowOnCopy::throw_on_copy = false;

    cache.put(3, ThrowOnCopy(30));
    EXPECT_FALSE(cache.get(1).has_value());
    ASSERT_TRUE(cache.get(2).has_value());
    EXPECT_EQ(cache.get(2)->value, 20);
}

namespace
{
struct ThrowOnEvictionHash
{
    std::size_t operator()(int key) const
    {
        if (key == throw_for_key)
            throw std::runtime_error("hash failed");
        return std::hash<int>{}(key);
    }

    static int throw_for_key;
};

int ThrowOnEvictionHash::throw_for_key = -1;
} // namespace

TEST(LRUCacheTest, FailedEvictionLookupRollsBackNewEntry)
{
    LRUCache<int, int, ThrowOnEvictionHash> cache(2);
    cache.put(1, 10);
    cache.put(2, 20);

    ThrowOnEvictionHash::throw_for_key = 1;
    EXPECT_THROW(cache.put(3, 30), std::runtime_error);
    ThrowOnEvictionHash::throw_for_key = -1;

    EXPECT_EQ(cache.size(), 2U);
    EXPECT_EQ(cache.get(1), 10);
    EXPECT_EQ(cache.get(2), 20);
    EXPECT_FALSE(cache.get(3).has_value());
}

TEST(LRUCacheTest, SupportsConcurrentAccessAndInsertion)
{
    constexpr int thread_count = 8;
    constexpr int operations_per_thread = 500;
    LRUCache<int, int> cache(64);
    std::atomic<bool> values_are_consistent{true};
    std::barrier start_barrier(thread_count);
    std::vector<std::jthread> threads;
    threads.reserve(thread_count);

    for (int thread_id = 0; thread_id < thread_count; ++thread_id)
    {
        threads.emplace_back(
            [thread_id, &cache, &values_are_consistent, &start_barrier] {
                start_barrier.arrive_and_wait();
                for (int i = 0; i < operations_per_thread; ++i)
                {
                    const int key = thread_id * operations_per_thread + i;
                    cache.put(key, key * 2);
                    const auto own_value = cache.get(key);
                    if (own_value && *own_value != key * 2)
                        values_are_consistent.store(false);

                    const auto previous_value = cache.get(key - 1);
                    if (previous_value && *previous_value != (key - 1) * 2)
                        values_are_consistent.store(false);
                }
            });
    }

    // Clearing the jthreads joins them before checking final cache state.
    threads.clear();

    EXPECT_EQ(cache.size(), cache.capacity());
    EXPECT_TRUE(values_are_consistent.load());

    for (int key = 0; key < thread_count * operations_per_thread; ++key)
    {
        const auto value = cache.get(key);
        if (value)
        {
            EXPECT_EQ(*value, key * 2);
        }
    }
}
