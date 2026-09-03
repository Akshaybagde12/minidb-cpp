#include "database.h"
#include "sql_parser.h"

#include <iomanip>
#include <iostream>
#include <string>

using minidb::Database;
using minidb::SQLParser;
using minidb::StatementType;

namespace {

void print_help() {
    std::cout
        << "Supported SQL (MVP schema: id INT PRIMARY KEY, name TEXT):\n"
        << "  CREATE TABLE users (id INT PRIMARY KEY, name TEXT);\n"
        << "  INSERT INTO users VALUES (1, 'Akshay');\n"
        << "  SELECT * FROM users;\n"
        << "  SELECT * FROM users WHERE id = 1;\n"
        << "  UPDATE users SET name = 'Akshay Bagde' WHERE id = 1;\n"
        << "  DELETE FROM users WHERE id = 1;\n"
        << "  SHOW TABLES;\n"
        << "  HELP;\n"
        << "  EXIT;\n";
}

void print_row(const minidb::Record& row) {
    std::cout << std::left << std::setw(12) << row.id << row.name << '\n';
}

} // namespace

int main(int argc, char** argv) {
    const std::string data_dir = argc > 1 ? argv[1] : "data";
    Database db(data_dir);

    std::cout << "MiniDB - Lightweight SQL Storage Engine\n"
              << "B+ Tree index | persistent binary storage | LRU record cache\n"
              << "Type HELP for supported SQL.\n\n";

    std::string input;
    while (true) {
        std::cout << "minidb> ";
        if (!std::getline(std::cin, input)) break;

        const auto stmt = SQLParser::parse(input);
        std::string error;

        switch (stmt.type) {
            case StatementType::CreateTable:
                if (db.create_table(stmt.table, error))
                    std::cout << "Table '" << stmt.table << "' created.\n";
                else
                    std::cout << "Error: " << error << '\n';
                break;

            case StatementType::Insert:
                if (db.insert(stmt.table, stmt.id, stmt.value, error))
                    std::cout << "1 row inserted.\n";
                else
                    std::cout << "Error: " << error << '\n';
                break;

            case StatementType::SelectById: {
                auto result = db.select_by_id(stmt.table, stmt.id, error);
                if (!result) {
                    std::cout << "Error: " << error << '\n';
                    break;
                }
                std::cout << "id          name\n------------ -------------------------------\n";
                print_row(result->record);
                std::cout << "(" << (result->cache_hit ? "cache hit" : "B+ tree + disk lookup") << ")\n";
                break;
            }

            case StatementType::SelectAll: {
                auto rows = db.select_all(stmt.table, error);
                if (!error.empty()) {
                    std::cout << "Error: " << error << '\n';
                    break;
                }
                std::cout << "id          name\n------------ -------------------------------\n";
                for (const auto& row : rows) print_row(row);
                std::cout << rows.size() << " row(s).\n";
                break;
            }

            case StatementType::Update:
                if (db.update_name(stmt.table, stmt.id, stmt.value, error))
                    std::cout << "1 row updated.\n";
                else
                    std::cout << "Error: " << error << '\n';
                break;

            case StatementType::Delete:
                if (db.erase(stmt.table, stmt.id, error))
                    std::cout << "1 row deleted (soft delete).\n";
                else
                    std::cout << "Error: " << error << '\n';
                break;

            case StatementType::ShowTables: {
                const auto tables = db.list_tables();
                if (tables.empty()) std::cout << "(no tables)\n";
                for (const auto& table : tables) std::cout << table << '\n';
                break;
            }

            case StatementType::Help:
                print_help();
                break;

            case StatementType::Exit:
                return 0;

            case StatementType::Invalid:
                std::cout << "Error: " << stmt.error << '\n';
                break;
        }
    }
    return 0;
}
