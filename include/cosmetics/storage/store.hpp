#pragma once

#include <map>
#include <utility>

namespace cosmetics {

template <typename Key, typename Value>
class Store {
public:
    Value get(const Key& key) const {
        auto it = records.find(key);
        return it == records.end() ? Value{} : it->second;
    }

    void set(const Key& key, Value value) {
        records.insert_or_assign(key, std::move(value));
    }

    bool contains(const Key& key) const { return records.find(key) != records.end(); }
    std::size_t size() const { return records.size(); }

private:
    std::map<Key, Value> records;
};

}
