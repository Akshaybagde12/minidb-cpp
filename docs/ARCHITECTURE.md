# Architecture

```text
+------------------+
|   CLI / main     |
+--------+---------+
         |
         v
+------------------+
|    SQLParser     |  regex-based MVP grammar
+--------+---------+
         |
         v
+------------------+
| Database/Catalog |  table discovery + dispatch
+--------+---------+
         |
         v
+------------------+
|   TableStorage   |
+---+----------+---+
    |          |
    v          v
+-------+   +---------+
|  LRU  |   | B+ Tree |
| Cache |   |  Index  |
+-------+   +----+----+
               |
               v
        byte offset
               |
               v
        +-------------+
        | .tbl binary |
        |    file     |
        +-------------+
```

## Complexity (MVP)

- Cache hit by primary key: expected `O(1)`
- B+ Tree point lookup: `O(log n)` in-memory index traversal + one direct record read
- Insert: append + B+ Tree insertion, `O(log n)` index work
- Full table scan: `O(n)`
- Startup index rebuild: `O(n log n)` with the current insertion-based rebuild
