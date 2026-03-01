# catalog.cpp

## File Location
`duckdb/src/catalog/catalog.cpp`

## Purpose
Core catalog implementation providing the interface for managing database objects (tables, views, functions, schemas, etc.) in DuckDB.

## Key Classes and Functions

### Catalog Class
The main class representing a catalog - a container for all database metadata.

#### Key Methods:

**Schema Management:**
- `GetSchema()` - Retrieves schema entries by name with various overloads
- `CreateSchema()` - Creates new schemas

**Table Management:**
- `CreateTable()` - Creates tables with BoundCreateTableInfo
- `SupportsCreateTable()` - Validates if table creation is supported (checks partition keys, sort keys, options)

**View Management:**
- `CreateView()` - Creates view entries

**Sequence Management:**
- `CreateSequence()` - Creates sequence entries

**Type Management:**
- `CreateType()` - Creates custom type entries

**Function Management:**
- `CreateFunction()` - Creates scalar/aggregate functions
- `CreateTableFunction()` - Creates table functions
- `CreatePragmaFunction()` - Creates PRAGMA functions
- `AddFunction()` - Adds function with ALTER_ON_CONFLICT behavior

**Index Management:**
- `CreateIndex()` - Creates indexes
- `BindCreateIndex()` - Binds CREATE INDEX statements

**Entry Lookup:**
- `GetEntry()` - Retrieves catalog entries by type and name
- `TryLookupEntry()` - Attempts lookup with detailed error handling
- `LookupEntry()` - Main lookup method that throws on failure
- `TryLookupDefaultTable()` - Special handling for catalogs with default tables

**Drop Operations:**
- `DropEntry()` - Drops catalog entries

**Alter Operations:**
- `Alter()` - Alters catalog entries

## Important Concepts

### CatalogEntryRetriever
Helper class for retrieving catalog entries with:
- Search path management
- Callback support
- AT clause support (for time travel)

### Catalog Lookup Process
1. Resolve catalog name (or use default)
2. Resolve schema name (or search path)
3. Look up entry by name and type
4. Auto-load extensions if entry not found
5. Provide helpful error messages with suggestions

### Time Travel Support
- EntryLookupInfo can contain an AT clause for querying historical data
- `SupportsTimeTravel()` checks if catalog supports this feature

### Auto-loading Extensions
The catalog can automatically load extensions when:
- Functions from extensions are referenced
- Types from extensions are used
- Copy functions or collations from extensions are needed

### Error Handling
- `CreateMissingEntryException()` - Creates detailed error messages with:
  - Similar entry suggestions
  - Extension hints ("this exists in X extension")
  - Qualification hints (need to specify catalog.schema)

### Search Path
`GetCatalogEntries()` resolves which catalogs/schemas to search based on:
- Explicit catalog/schema provided
- Search path configuration
- Default catalog/schema

## Dependencies
- `AttachedDatabase` - The database this catalog belongs to
- `DatabaseManager` - Manages multiple attached databases
- `DependencyManager` - Tracks object dependencies
- `CatalogSearchPath` - Search path configuration
- `ExtensionHelper` - For auto-loading extensions

## Notes
- The catalog is the central metadata repository for DuckDB
- Supports multiple attached databases with unified catalog access
- Provides consistent API across different catalog types (DuckDB, PostgreSQL, etc.)
