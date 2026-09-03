#pragma once

#include <optional>
#include <string>

namespace minidb {

enum class StatementType {
    CreateTable,
    Insert,
    SelectAll,
    SelectById,
    Update,
    Delete,
    ShowTables,
    Help,
    Exit,
    Invalid
};

struct Statement {
    StatementType type = StatementType::Invalid;
    std::string table;
    int id = 0;
    std::string value;
    std::string error;
};

class SQLParser {
public:
    static Statement parse(std::string sql);

private:
    static std::string trim(std::string s);
};

} // namespace minidb
