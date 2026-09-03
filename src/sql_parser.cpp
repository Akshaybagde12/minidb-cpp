#include "sql_parser.h"

#include <algorithm>
#include <cctype>
#include <regex>

namespace minidb {

std::string SQLParser::trim(std::string s) {
    auto not_space = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
    s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
    return s;
}

Statement SQLParser::parse(std::string sql) {
    sql = trim(std::move(sql));
    if (!sql.empty() && sql.back() == ';') sql.pop_back();
    sql = trim(std::move(sql));

    if (sql.empty()) return {StatementType::Invalid, {}, 0, {}, "empty statement"};

    static const std::regex exit_re(R"(^\s*(EXIT|QUIT)\s*$)", std::regex::icase);
    static const std::regex help_re(R"(^\s*HELP\s*$)", std::regex::icase);
    static const std::regex show_re(R"(^\s*SHOW\s+TABLES\s*$)", std::regex::icase);
    static const std::regex create_re(
        R"(^\s*CREATE\s+TABLE\s+([A-Za-z_][A-Za-z0-9_]*)\s*\(\s*id\s+INT\s+PRIMARY\s+KEY\s*,\s*name\s+TEXT\s*\)\s*$)",
        std::regex::icase);
    static const std::regex insert_re(
        R"(^\s*INSERT\s+INTO\s+([A-Za-z_][A-Za-z0-9_]*)\s+VALUES\s*\(\s*(-?\d+)\s*,\s*'([^']*)'\s*\)\s*$)",
        std::regex::icase);
    static const std::regex select_all_re(
        R"(^\s*SELECT\s+\*\s+FROM\s+([A-Za-z_][A-Za-z0-9_]*)\s*$)",
        std::regex::icase);
    static const std::regex select_id_re(
        R"(^\s*SELECT\s+\*\s+FROM\s+([A-Za-z_][A-Za-z0-9_]*)\s+WHERE\s+id\s*=\s*(-?\d+)\s*$)",
        std::regex::icase);
    static const std::regex update_re(
        R"(^\s*UPDATE\s+([A-Za-z_][A-Za-z0-9_]*)\s+SET\s+name\s*=\s*'([^']*)'\s+WHERE\s+id\s*=\s*(-?\d+)\s*$)",
        std::regex::icase);
    static const std::regex delete_re(
        R"(^\s*DELETE\s+FROM\s+([A-Za-z_][A-Za-z0-9_]*)\s+WHERE\s+id\s*=\s*(-?\d+)\s*$)",
        std::regex::icase);

    std::smatch match;
    if (std::regex_match(sql, exit_re)) return {StatementType::Exit, {}, 0, {}, {}};
    if (std::regex_match(sql, help_re)) return {StatementType::Help, {}, 0, {}, {}};
    if (std::regex_match(sql, show_re)) return {StatementType::ShowTables, {}, 0, {}, {}};
    if (std::regex_match(sql, match, create_re)) return {StatementType::CreateTable, match[1].str(), 0, {}, {}};
    if (std::regex_match(sql, match, insert_re)) {
        return {StatementType::Insert, match[1].str(), std::stoi(match[2].str()), match[3].str(), {}};
    }
    if (std::regex_match(sql, match, select_id_re)) {
        return {StatementType::SelectById, match[1].str(), std::stoi(match[2].str()), {}, {}};
    }
    if (std::regex_match(sql, match, select_all_re)) return {StatementType::SelectAll, match[1].str(), 0, {}, {}};
    if (std::regex_match(sql, match, update_re)) {
        return {StatementType::Update, match[1].str(), std::stoi(match[3].str()), match[2].str(), {}};
    }
    if (std::regex_match(sql, match, delete_re)) {
        return {StatementType::Delete, match[1].str(), std::stoi(match[2].str()), {}, {}};
    }

    Statement invalid;
    invalid.type = StatementType::Invalid;
    invalid.error = "unsupported SQL syntax; type HELP for examples";
    return invalid;
}

} // namespace minidb
