#pragma once

#include <cstddef>
#include <exception>
#include <functional>
#include <iterator>
#include <list>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>

// A thread-safe least-recently-used cache.
//
// get() returns a value copy and therefore requires Value to be
// copy-constructible. size() and capacity() are O(1). put() and get() are
// average O(1) and worst-case O(n), due to unordered_map operations, excluding
// the cost of constructing, copying, assigning, hashing, or destroying values
// and keys.
//
// Template requirements:
// - Key must be copy-constructible and destructible because the list and map
//   each store a key. std::equal_to<Key> must compare keys consistently.
// - Value must be constructible from Value&& and move-assignable for put().
//   Value must also be copy-constructible for get(), and destructible.
// - Hash must be copy-constructible, destructible, and callable with const Key&
//   to produce a std::size_t. Equal keys must produce equal hashes, and a
//   stored key's hash and equality behavior must remain stable while it is in
//   the cache. By default, Hash is std::hash<Key>.
template <typename Key, typename Value, typename Hash = std::hash<Key>>
class LRUCache
{
public:
    explicit LRUCache(std::size_t capacity)
        : capacity_(capacity)
    {
    }

    LRUCache(const LRUCache &) = delete;
    LRUCache &operator=(const LRUCache &) = delete;
    LRUCache(LRUCache &&) = delete;
    LRUCache &operator=(LRUCache &&) = delete;

    // For a new key, insertion has the strong guarantee: if list/map insertion
    // or eviction lookup throws, the cache contents and recency order are
    // unchanged. When full, the candidate is inserted first; if finding the
    // LRU map entry then throws, the candidate is removed by its map iterator.
    // The LRU entry is erased by iterator, avoiding a throwing hash/equality
    // call after the candidate has been accepted.
    //
    // For an existing key, a throwing Value move assignment leaves the map
    // and list consistent and recency unchanged. The stored Value has the
    // guarantee provided by its move-assignment operator (which may leave it
    // partially modified).
    void put(Key key, Value value)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (capacity_ == 0)
            return;

        const auto existing = index_.find(key);
        if (existing != index_.end())
        {
            // Keep the recency order unchanged if assigning the new value throws.
            existing->second->second = std::move(value);
            entries_.splice(entries_.begin(), entries_, existing->second);
            return;
        }

        entries_.emplace_front(std::move(key), std::move(value));
        auto inserted_map_entry = index_.end();
        try
        {
            const auto [inserted_entry, inserted] = index_.emplace(
                entries_.front().first, entries_.begin());
            if (!inserted)
                throw std::logic_error("LRUCache key unexpectedly already exists");
            inserted_map_entry = inserted_entry;
        }
        catch (...)
        {
            // The list insertion succeeded but indexing it failed.
            entries_.pop_front();
            throw;
        }

        if (entries_.size() > capacity_)
        {
            const auto least_recent = std::prev(entries_.end());
            auto least_recent_map_entry = index_.end();
            try
            {
                // Find after insertion because insertion may have rehashed and
                // invalidated all pre-existing map iterators.
                least_recent_map_entry = index_.find(least_recent->first);
            }
            catch (...)
            {
                // Roll back the new item if hashing/equality fails during
                // eviction lookup. Iterator erase does not hash the key.
                index_.erase(inserted_map_entry);
                entries_.pop_front();
                throw;
            }

            if (least_recent_map_entry == index_.end())
            {
                index_.erase(inserted_map_entry);
                entries_.pop_front();
                throw std::logic_error("LRUCache list/map invariant is broken");
            }

            // Erasing by iterator does not invoke user-supplied hashing or
            // equality and does not throw.
            index_.erase(least_recent_map_entry);
            entries_.erase(least_recent);
        }
    }

    // A missing key returns nullopt without changing recency. A hit copies
    // Value before changing recency, so a copy failure leaves the cache
    // unchanged. If constructing the returned optional (including its
    // move/copy from the local result) throws, recency is also left unchanged.
    std::optional<Value> get(const Key &key)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        const auto found = index_.find(key);
        if (found == index_.end())
            return std::nullopt;

        // Copy before changing recency so a throwing Value copy leaves the
        // cache's order untouched.
        std::optional<Value> result(std::in_place, found->second->second);

        // Commit recency only after the return value has been materialized.
        // The guard runs during normal return after result construction, and
        // skips the splice if returning result throws.
        struct RecencyCommit
        {
            EntryList &entries;
            typename EntryList::iterator accessed;
            int exception_count;

            ~RecencyCommit() noexcept
            {
                if (std::uncaught_exceptions() == exception_count)
                    entries.splice(entries.begin(), entries, accessed);
            }
        } recency_commit{entries_, found->second, std::uncaught_exceptions()};

        return result;
    }

    std::size_t size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return entries_.size();
    }

    std::size_t capacity() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return capacity_;
    }

private:
    using Entry = std::pair<Key, Value>;
    using EntryList = std::list<Entry>;

    const std::size_t capacity_;
    mutable std::mutex mutex_;
    EntryList entries_; // Most recently used entry is at the front.
    std::unordered_map<Key, typename EntryList::iterator, Hash> index_;
};
