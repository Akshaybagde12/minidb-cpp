#include "storage.h"

#include <cstring>
#include <stdexcept>

namespace minidb {

LRURecordCache::LRURecordCache(std::size_t capacity) : capacity_(capacity) {}

bool LRURecordCache::get(int id, Record& out) {
    auto it = lookup_.find(id);
    if (it == lookup_.end()) return false;
    order_.splice(order_.begin(), order_, it->second);
    out = it->second->second;
    return true;
}

void LRURecordCache::put(const Record& record) {
    if (capacity_ == 0) return;
    auto it = lookup_.find(record.id);
    if (it != lookup_.end()) {
        it->second->second = record;
        order_.splice(order_.begin(), order_, it->second);
        return;
    }
    order_.push_front({record.id, record});
    lookup_[record.id] = order_.begin();
    if (order_.size() > capacity_) {
        auto last = std::prev(order_.end());
        lookup_.erase(last->first);
        order_.pop_back();
    }
}

void LRURecordCache::erase(int id) {
    auto it = lookup_.find(id);
    if (it == lookup_.end()) return;
    order_.erase(it->second);
    lookup_.erase(it);
}

void LRURecordCache::clear() {
    order_.clear();
    lookup_.clear();
}

TableStorage::TableStorage(std::string table_name, std::filesystem::path file_path, std::size_t cache_capacity)
    : table_name_(std::move(table_name)), file_path_(std::move(file_path)), index_(15), cache_(cache_capacity) {
    open_or_create();
    rebuild_index();
}

TableStorage::~TableStorage() {
    if (file_.is_open()) file_.close();
}

void TableStorage::open_or_create() {
    std::filesystem::create_directories(file_path_.parent_path());
    file_.open(file_path_, std::ios::in | std::ios::out | std::ios::binary);
    if (!file_.is_open()) {
        std::ofstream create(file_path_, std::ios::binary);
        create.close();
        file_.open(file_path_, std::ios::in | std::ios::out | std::ios::binary);
    }
    if (!file_.is_open()) throw std::runtime_error("Unable to open table file: " + file_path_.string());
}

void TableStorage::rebuild_index() {
    index_.clear();
    cache_.clear();
    live_rows_ = 0;
    file_.clear();
    file_.seekg(0, std::ios::beg);
    Record record;
    std::uint64_t offset = 0;
    while (file_.read(reinterpret_cast<char*>(&record), sizeof(Record))) {
        if (!record.deleted) {
            index_.insert(record.id, offset);
            ++live_rows_;
        }
        offset += sizeof(Record);
    }
    file_.clear();
}

bool TableStorage::read_record(std::uint64_t offset, Record& record) {
    file_.clear();
    file_.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    return static_cast<bool>(file_.read(reinterpret_cast<char*>(&record), sizeof(Record)));
}

bool TableStorage::write_record(std::uint64_t offset, const Record& record) {
    file_.clear();
    file_.seekp(static_cast<std::streamoff>(offset), std::ios::beg);
    file_.write(reinterpret_cast<const char*>(&record), sizeof(Record));
    file_.flush();
    return static_cast<bool>(file_);
}

bool TableStorage::insert(int id, const std::string& name, std::string& error) {
    if (index_.search(id)) {
        error = "PRIMARY KEY violation: id " + std::to_string(id) + " already exists";
        return false;
    }
    if (name.size() > kMaxNameLength) {
        error = "name exceeds " + std::to_string(kMaxNameLength) + " characters";
        return false;
    }

    Record record;
    record.id = id;
    std::strncpy(record.name, name.c_str(), kMaxNameLength);
    record.name[kMaxNameLength] = '\0';
    record.deleted = 0;

    file_.clear();
    file_.seekp(0, std::ios::end);
    const auto pos = file_.tellp();
    if (pos < 0) {
        error = "failed to seek table file";
        return false;
    }
    const auto offset = static_cast<std::uint64_t>(pos);
    file_.write(reinterpret_cast<const char*>(&record), sizeof(Record));
    file_.flush();
    if (!file_) {
        error = "failed to write record";
        return false;
    }

    index_.insert(id, offset);
    cache_.put(record);
    ++live_rows_;
    return true;
}

std::optional<LookupResult> TableStorage::select_by_id(int id) {
    Record record;
    if (cache_.get(id, record)) return LookupResult{record, true};

    auto offset = index_.search(id);
    if (!offset) return std::nullopt;
    if (!read_record(*offset, record) || record.deleted) return std::nullopt;
    cache_.put(record);
    return LookupResult{record, false};
}

std::vector<Record> TableStorage::select_all() {
    std::vector<Record> rows;
    rows.reserve(live_rows_);
    file_.clear();
    file_.seekg(0, std::ios::beg);
    Record record;
    while (file_.read(reinterpret_cast<char*>(&record), sizeof(Record))) {
        if (!record.deleted) rows.push_back(record);
    }
    file_.clear();
    return rows;
}

bool TableStorage::update_name(int id, const std::string& new_name, std::string& error) {
    if (new_name.size() > kMaxNameLength) {
        error = "name exceeds " + std::to_string(kMaxNameLength) + " characters";
        return false;
    }
    auto offset = index_.search(id);
    if (!offset) {
        error = "row not found";
        return false;
    }
    Record record;
    if (!read_record(*offset, record) || record.deleted) {
        error = "row not found";
        return false;
    }
    std::memset(record.name, 0, sizeof(record.name));
    std::strncpy(record.name, new_name.c_str(), kMaxNameLength);
    if (!write_record(*offset, record)) {
        error = "failed to persist update";
        return false;
    }
    cache_.put(record);
    return true;
}

bool TableStorage::erase(int id, std::string& error) {
    auto offset = index_.search(id);
    if (!offset) {
        error = "row not found";
        return false;
    }
    Record record;
    if (!read_record(*offset, record) || record.deleted) {
        error = "row not found";
        return false;
    }
    record.deleted = 1;
    if (!write_record(*offset, record)) {
        error = "failed to persist delete";
        return false;
    }
    index_.erase(id);
    cache_.erase(id);
    if (live_rows_ > 0) --live_rows_;
    return true;
}

} // namespace minidb
