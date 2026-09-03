<<<<<<< HEAD
# MiniDB — Lightweight SQL Storage Engine

A small C++17 database project built to understand how a SQL-facing interface connects to persistent storage, indexing, and caching.

**Author:** Akshay Bagde

## What it implements

- A small SQL parser for a deliberately limited SQL subset
- Persistent binary table files
- Multiple table catalog persisted on disk
- Fixed-size records with soft deletion
- B+ Tree primary-key index (`id -> byte offset`)
- LRU record cache for recently accessed rows
- CRUD operations and full-table scans
- Index reconstruction when the database restarts
- Unit-style persistence/CRUD tests
- A simple benchmark executable

## Supported SQL

MiniDB currently uses one MVP schema for every table:

```sql
(id INT PRIMARY KEY, name TEXT)
```

Supported commands:

```sql
CREATE TABLE users (id INT PRIMARY KEY, name TEXT);
INSERT INTO users VALUES (1, 'Akshay');
SELECT * FROM users;
SELECT * FROM users WHERE id = 1;
UPDATE users SET name = 'Akshay Bagde' WHERE id = 1;
DELETE FROM users WHERE id = 1;
SHOW TABLES;
EXIT;
```

This is intentionally **not** a full SQL implementation. The parser is a small regex-based front end designed for learning the database execution path.

## Architecture

```text
SQL command
    |
    v
SQLParser
    |
    v
Database / Catalog
    |
    v
TableStorage
    |------------------------|
    v                        v
LRU Record Cache        B+ Tree Index
                             |
                             v
                     byte offset in .tbl
                             |
                             v
                     Persistent Binary File
```

For `SELECT ... WHERE id = ?`, MiniDB checks the LRU cache first. On a cache miss, the B+ Tree locates the row's byte offset and the storage layer seeks directly to that record on disk. `SELECT *` performs a sequential scan.

## Project structure

```text
akshay-minidb/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── main.cpp
│   ├── sql_parser.{h,cpp}
│   ├── database.{h,cpp}
│   ├── storage.{h,cpp}
│   └── bplustree.{h,cpp}
├── tests/
│   └── basic_test.cpp
├── benchmarks/
│   └── benchmark.cpp
└── data/
```

## Build and run

```bash
mkdir -p build
cd build
cmake ..
cmake --build . -j
./minidb
```

Run tests:

```bash
ctest --output-on-failure
```

Run the benchmark:

```bash
./minidb_benchmark 20000
```

## Design choices

### Fixed-size records
The current storage format uses a fixed record layout (`id`, 64-byte name buffer, deletion flag). This makes direct byte-offset addressing simple and keeps the first version understandable.

### B+ Tree index
The primary key is indexed in memory using a B+ Tree. Leaf entries map IDs to byte offsets in the table file. At startup, MiniDB scans the table file and rebuilds this in-memory index.

### LRU cache
Frequently accessed records are stored in a bounded LRU cache. It is a **record cache**, not a full page-oriented DBMS buffer pool; page-based storage is listed as future work.

### Soft deletion
`DELETE` marks a record deleted in the table file and removes its key from the in-memory index. Space reclamation/compaction is not implemented yet.

## Current limitations / future work

- General schema support instead of the fixed `(id, name)` schema
- Page-oriented storage and a real buffer pool
- Persisted B+ Tree pages instead of rebuilding the index on startup
- Range queries through linked B+ Tree leaves
- Free-space management / compaction
- Transactions, WAL, and crash recovery
- More expressive `WHERE` predicates and query planning

These limitations are intentional: the current version focuses on a clean end-to-end path from SQL input to indexed persistent storage.
=======
# minidb-cpp
>>>>>>> 187783a8b4e711f42137a4834e883641620d099d
