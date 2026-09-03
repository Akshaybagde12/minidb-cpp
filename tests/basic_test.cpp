#include "database.h"
#include "sql_parser.h"

#include <cassert>
#include <filesystem>
#include <iostream>

int main() {
    namespace fs = std::filesystem;
    const fs::path root = "test_data";
    fs::remove_all(root);

    {
        minidb::Database db(root);
        std::string error;
        assert(db.create_table("users", error));
        assert(db.insert("users", 10, "Akshay", error));
        assert(db.insert("users", 20, "IITH", error));
        assert(!db.insert("users", 10, "duplicate", error));

        error.clear();
        auto first = db.select_by_id("users", 10, error);
        assert(first && first->record.id == 10);
        auto second = db.select_by_id("users", 10, error);
        assert(second && second->cache_hit);

        assert(db.update_name("users", 10, "Akshay Bagde", error));
        auto updated = db.select_by_id("users", 10, error);
        assert(updated && std::string(updated->record.name) == "Akshay Bagde");

        auto rows = db.select_all("users", error);
        assert(rows.size() == 2);
        assert(db.erase("users", 20, error));
        rows = db.select_all("users", error);
        assert(rows.size() == 1);
    }

    // Persistence + index rebuild.
    {
        minidb::Database db(root);
        std::string error;
        assert(db.has_table("users"));
        auto row = db.select_by_id("users", 10, error);
        assert(row && std::string(row->record.name) == "Akshay Bagde");
        auto deleted = db.select_by_id("users", 20, error);
        assert(!deleted);
    }

    const auto stmt = minidb::SQLParser::parse("SELECT * FROM users WHERE id = 10;");
    assert(stmt.type == minidb::StatementType::SelectById);
    assert(stmt.table == "users" && stmt.id == 10);

    fs::remove_all(root);
    std::cout << "All MiniDB tests passed.\n";
    return 0;
}
