# catalog_entry__duck_index_entry.cpp.md

## File Location
`duckdb/src/catalog/catalog_entry/duck_index_entry.cpp`

## Purpose
Implements DuckIndexEntry - DuckDB's index catalog entry linking catalog metadata to physical index storage.

## Key Classes and Their Purpose

### IndexDataTableInfo
- **Purpose**: Links index entry to the table's physical storage
- **Key Members**:
  - `info` - Shared pointer to DataTableInfo (table metadata)
  - `index_name` - Name of the index

### DuckIndexEntry
- **Purpose**: DuckDB's index catalog entry implementation
- **Key Members**:
  - `info` - IndexDataTableInfo linking to table storage
  - `initial_index_size` - Size of index when created

## Key Methods with Brief Descriptions

- `Rollback()` - Removes index from table storage on transaction rollback
- `GetSchemaName()` - Returns schema name from linked table
- `GetTableName()` - Returns table name from linked table
- `GetDataTableInfo()` - Returns the DataTableInfo this index belongs to
- `CommitDrop()` - Commits index drop to storage

## Important Concepts
- **Storage Linkage**: Index entries are linked to actual table storage
- **Rollback Support**: Indexes are removed from storage on transaction rollback
- **Index Lifecycle**: Managed through table's index collection
