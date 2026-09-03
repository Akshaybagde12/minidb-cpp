#include "database.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <stdexcept>

namespace minidb {

Database::Database(std::filesystem::path root_directory)
    : root_(std::move(root_directory)), catalog_path_(root_ / "catalog.meta") {
    std::filesystem::create_directories(root_);
    load_catalog();
}

bool Database::valid_identifier(const std::string& name) {
    if (name.empty() || !(std::isalpha(static_cast<unsigned char>(name[0])) || name[0] == '_')) return false;
    for (char c : name) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_')) return false;
    }
    return true;
}

void Database::load_catalog() {
    std::ifstream in(catalog_path_);
    std::string table;
    while (std::getline(in, table)) {
        if (table.empty()) continue;
        tables_[table] = std::make_unique<TableStorage>(table, root_ / (table + ".tbl"));
    }
}

void Database::persist_catalog() const {
    std::vector<std::string> names;
    names.reserve(tables_.size());
    for (const auto& [name, _] : tables_) names.push_back(name);
    std::sort(names.begin(), names.end());

    std::ofstream out(catalog_path_, std::ios::trunc);
    for (const auto& name : names) out << name << '\n';
}

bool Database::create_table(const std::string& table_name, std::string& error) {
    if (!valid_identifier(table_name)) {
        error = "invalid table name";
        return false;
    }
    if (has_table(table_name)) {
        error = "table already exists";
        return false;
    }
    tables_[table_name] = std::make_unique<TableStorage>(table_name, root_ / (table_name + ".tbl"));
    persist_catalog();
    return true;
}

bool Database::has_table(const std::string& table_name) const {
    return tables_.find(table_name) != tables_.end();
}

std::vector<std::string> Database::list_tables() const {
    std::vector<std::string> result;
    result.reserve(tables_.size());
    for (const auto& [name, _] : tables_) result.push_back(name);
    std::sort(result.begin(), result.end());
    return result;
}

TableStorage* Database::get_table(const std::string& name) {
    auto it = tables_.find(name);
    return it == tables_.end() ? nullptr : it->second.get();
}

bool Database::insert(const std::string& table, int id, const std::string& name, std::string& error) {
    auto* storage = get_table(table);
    if (!storage) {
        error = "unknown table '" + table + "'";
        return false;
    }
    return storage->insert(id, name, error);
}

std::optional<LookupResult> Database::select_by_id(const std::string& table, int id, std::string& error) {
    auto* storage = get_table(table);
    if (!storage) {
        error = "unknown table '" + table + "'";
        return std::nullopt;
    }
    auto result = storage->select_by_id(id);
    if (!result) error = "row not found";
    return result;
}

std::vector<Record> Database::select_all(const std::string& table, std::string& error) {
    auto* storage = get_table(table);
    if (!storage) {
        error = "unknown table '" + table + "'";
        return {};
    }
    return storage->select_all();
}

bool Database::update_name(const std::string& table, int id, const std::string& name, std::string& error) {
    auto* storage = get_table(table);
    if (!storage) {
        error = "unknown table '" + table + "'";
        return false;
    }
    return storage->update_name(id, name, error);
}

bool Database::erase(const std::string& table, int id, std::string& error) {
    auto* storage = get_table(table);
    if (!storage) {
        error = "unknown table '" + table + "'";
        return false;
    }
    return storage->erase(id, error);
}

} // namespace minidb
