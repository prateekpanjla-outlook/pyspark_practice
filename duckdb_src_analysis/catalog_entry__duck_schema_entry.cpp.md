# catalog_entry__duck_schema_entry.cpp.md

## File Location
`duckdb/src/catalog/catalog_entry/duck_schema_entry.cpp`

## Purpose
Implementation of DuckSchemaEntry - the DuckDB-specific schema catalog entry that manages all objects within a schema.

## Key Class: DuckSchemaEntry

### Purpose
Represents a schema (namespace for database objects) in DuckDB.

### Member Variables (CatalogSets for different object types):
- `tables` - TABLE and VIEW entries
- `indexes` - INDEX entries
- `table_functions` - TABLE FUNCTION and TABLE MACRO entries
- `copy_functions` - COPY FUNCTION entries
- `pragma_functions` - PRAGMA FUNCTION entries
- `functions` - SCALAR, AGGREGATE, and MACRO function entries
- `sequences` - SEQUENCE entries
- `collations` - COLLATION entries
- `types` - TYPE entries

### Key Methods:

**Object Creation:**
- `CreateTable()` - Creates table with foreign key handling
- `CreateView()` - Creates view
- `CreateSequence()` - Creates sequence
- `CreateType()` - Creates custom type
- `CreateIndex()` - Creates index (validates uniqueness of index name)
- `CreateCollation()` - Creates collation
- `CreateTableFunction()` - Creates table function
- `CreateCopyFunction()` - Creates copy function
- `CreatePragmaFunction()` - Creates pragma function
- `CreateFunction()` - Creates scalar/aggregate/macro functions

**Entry Management:**
- `AddEntry()` - Adds entry with conflict handling
- `AddEntryInternal()` - Internal add logic with validation
- `Alter()` - Alters entry or changes ownership
- `DropEntry()` - Drops entry with foreign key cleanup

**Lookup:**
- `LookupEntry()` - Looks up entry by name and type
- `LookupEntryDetailed()` - Detailed lookup with failure reason
- `GetSimilarEntry()` - Finds similar entries for suggestions

**Scanning:**
- `Scan()` - Iterates entries of a given type

**Internal:**
- `GetCatalogSet()` - Returns appropriate CatalogSet for type
- `Verify()` - Verifies all catalog sets

## Important Concepts

### Default Generators
System catalog has default entries for:
- **Views** - DefaultViewGenerator for information_schema views
- **Table Functions** - DefaultTableFunctionGenerator for glob, range, etc.
- **Functions** - DefaultFunctionGenerator for built-in functions
- **Types** - DefaultTypeGenerator for LIST, MAP, STRUCT types

### CREATE OR REPLACE Handling
When `OnCreateConflict::REPLACE_ON_CONFLICT`:
1. Checks if entry exists
2. Validates entry is same type
3. Calls `OnDropEntry()` to clean up old entry
4. Drops old entry
5. Creates new entry

### Foreign Key Handling
**On CREATE TABLE:**
1. Finds all FK constraints
2. For each FK, alters referenced (primary key) table
3. Adds dependency from current table to referenced table

**On DROP TABLE:**
1. Finds all FK constraints
2. For primary key tables, validates no remaining FK references
3. Alters referenced tables to remove FK entries

### MetaTransaction Validation
`AddEntryInternal()` checks database is marked as modified in MetaTransaction for non-temporary, non-system databases.

### Index Name Uniqueness
`CreateIndex()` validates index names are unique across:
1. Other INDEX entries in catalog
2. Constraint-based indexes (PRIMARY KEY, UNIQUE, FOREIGN KEY)

## CatalogSet Type Mapping
| CatalogType | CatalogSet |
|-------------|------------|
| TABLE_ENTRY, VIEW_ENTRY | tables |
| INDEX_ENTRY | indexes |
| TABLE_FUNCTION_ENTRY, TABLE_MACRO_ENTRY | table_functions |
| SCALAR_FUNCTION_ENTRY, AGGREGATE_FUNCTION_ENTRY, MACRO_ENTRY | functions |
| COPY_FUNCTION_ENTRY | copy_functions |
| PRAGMA_FUNCTION_ENTRY | pragma_functions |
| SEQUENCE_ENTRY | sequences |
| COLLATION_ENTRY | collations |
| TYPE_ENTRY | types |

## Usage Example
```cpp
// Create schema
CreateSchemaInfo info;
info.schema = "myschema";
DuckSchemaEntry schema(catalog, info);

// Create table
BoundCreateTableInfo table_info;
schema.CreateTable(transaction, table_info);

// Create function
CreateScalarFunctionInfo func_info;
schema.CreateFunction(transaction, func_info);

// Scan tables
schema.Scan(context, CatalogType::TABLE_ENTRY, [](CatalogEntry &entry) {
    auto &table = entry.Cast<TableCatalogEntry>();
    // Process table
});
```

## Notes
- Central hub for all schema operations
- Organizes objects by type into separate CatalogSets
- Handles complex cross-object relationships (FKs, dependencies)
- Supports lazy initialization of default objects via generators
