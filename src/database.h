#pragma once

#include "storage.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace minidb {

class Database {
public:
    explicit Database(std::filesystem::path root_directory = "data");

    bool create_table(const std::string& table_name, std::string& error);
    bool has_table(const std::string& table_name) const;
    std::vector<std::string> list_tables() const;

    bool insert(const std::string& table, int id, const std::string& name, std::string& error);
    std::optional<LookupResult> select_by_id(const std::string& table, int id, std::string& error);
    std::vector<Record> select_all(const std::string& table, std::string& error);
    bool update_name(const std::string& table, int id, const std::string& name, std::string& error);
    bool erase(const std::string& table, int id, std::string& error);

private:
    std::filesystem::path root_;
    std::filesystem::path catalog_path_;
    std::unordered_map<std::string, std::unique_ptr<TableStorage>> tables_;

    static bool valid_identifier(const std::string& name);
    void load_catalog();
    void persist_catalog() const;
    TableStorage* get_table(const std::string& name);
};

} // namespace minidb
