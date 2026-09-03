#pragma once

#include "bplustree.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <list>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace minidb {

constexpr std::size_t kMaxNameLength = 63;

#pragma pack(push, 1)
struct Record {
    std::int32_t id = 0;
    char name[kMaxNameLength + 1]{};
    std::uint8_t deleted = 0;
};
#pragma pack(pop)

class LRURecordCache {
public:
    explicit LRURecordCache(std::size_t capacity = 64);
    bool get(int id, Record& out);
    void put(const Record& record);
    void erase(int id);
    void clear();

private:
    using Entry = std::pair<int, Record>;
    std::size_t capacity_;
    std::list<Entry> order_;
    std::unordered_map<int, std::list<Entry>::iterator> lookup_;
};

struct LookupResult {
    Record record{};
    bool cache_hit = false;
};

class TableStorage {
public:
    TableStorage(std::string table_name, std::filesystem::path file_path, std::size_t cache_capacity = 64);
    ~TableStorage();

    const std::string& name() const { return table_name_; }
    bool insert(int id, const std::string& name, std::string& error);
    std::optional<LookupResult> select_by_id(int id);
    std::vector<Record> select_all();
    bool update_name(int id, const std::string& new_name, std::string& error);
    bool erase(int id, std::string& error);
    std::size_t live_rows() const { return live_rows_; }

private:
    std::string table_name_;
    std::filesystem::path file_path_;
    std::fstream file_;
    BPlusTree index_;
    LRURecordCache cache_;
    std::size_t live_rows_ = 0;

    void open_or_create();
    void rebuild_index();
    bool read_record(std::uint64_t offset, Record& record);
    bool write_record(std::uint64_t offset, const Record& record);
};

} // namespace minidb
