# catalog_entry__table_catalog_entry.cpp.md

## File Location
`duckdb/src/catalog/catalog_entry/table_catalog_entry.cpp`

## Purpose
Base class implementation for table catalog entries - provides common functionality used by all table implementations.

## Key Class: TableCatalogEntry

### Purpose
Abstract base class representing a table in the catalog. Provides common table operations and metadata.

### Member Variables:
- `columns` - ColumnList containing column definitions
- `constraints` - Vector of table constraints

### Key Methods:

**Column Access:**
- `GetColumn()` - Gets column by index or name
- `GetColumns()` - Returns all columns
- `ColumnExists()` - Checks if column exists
- `GetColumnIndex()` - Gets column index by name
- `GetTypes()` - Returns vector of column types

**Storage:**
- `GetStorage()` - Returns DataTable (throws in base class)
- `GetStorageIndex()` - Converts ColumnIndex to StorageIndex
- `GetSample()` - Returns sample for query planning (returns nullptr in base)

**Constraints:**
- `GetConstraints()` - Returns all constraints
- `GetPrimaryKey()` - Returns primary key constraint if exists
- `HasPrimaryKey()` - Checks if table has primary key

**Metadata:**
- `GetInfo()` - Returns CreateTableInfo for serialization
- `ToSQL()` - Generates CREATE TABLE SQL statement
- `ColumnsToSQL()` - Generates column list SQL

**Update Binding:**
- `BindUpdateConstraints()` - Binds constraints for UPDATE operations
- Adds extra columns to UPDATE for CHECK constraints
- Converts to DELETE+INSERT for index column updates

**Utilities:**
- `HasGeneratedColumns()` - Checks if table has generated columns
- `ColumnNamesToSQL()` - Generates column name list SQL
- `GetVirtualColumns()` - Returns virtual columns (rowid)
- `GetRowIdColumns()` - Returns rowid column identifiers

**Scan Function:**
- `GetScanFunction()` - Returns TableFunction for scanning table

## Important Concepts

### Generated Columns
- **HasGeneratedColumns()** - Logical count ≠ Physical count
- Generated columns computed from other columns
- Not stored in physical storage
- Affects UPDATE operations (cannot be directly updated)

### StorageIndex vs ColumnIndex
- **ColumnIndex** - Logical column position
- **StorageIndex** - Physical storage position
- Conversion needed because generated columns don't have storage
- `GetStorageIndex()` performs this mapping

### Virtual Columns
- `rowid` is always available as virtual column
- `GetVirtualColumns()` returns map of virtual columns
- `GetRowIdColumns()` returns rowid identifiers

### UPDATE Operation Handling
**BindUpdateConstraints** handles:**
1. **CHECK constraints** - Adds required columns to UPDATE
2. **Index columns** - Converts UPDATE to DELETE+INSERT if indexed columns change
3. **LIST columns** - Converts to DELETE+INSERT (LISTs don't support regular updates)
4. **RETURNING** - Requires all columns available

### SQL Generation
**ColumnsToSQL** handles:
- Column type declarations
- COLLATION clauses
- DEFAULT values
- GENERATED ALWAYS AS expressions
- NOT NULL constraints
- PRIMARY KEY (single column)
- UNIQUE (single column)
- Multi-column constraints deferred to end

## Usage Example
```cpp
// Access columns
auto &col = table.GetColumn("name");
auto col_type = col.Type();

// Get column index
auto idx = table.GetColumnIndex("age");

// Check constraints
if (table.HasPrimaryKey()) {
    auto &pk = table.GetPrimaryKey();
    // Process primary key
}

// Generate SQL
string sql = table.ToSQL();
```

## Notes
- Base class - DuckDB uses DuckTableEntry for most tables
- Provides common functionality for all table types
- Throws InternalException if GetStorage() called on non-DuckDB table
- UPDATE constraint binding is complex due to index and generated column handling
