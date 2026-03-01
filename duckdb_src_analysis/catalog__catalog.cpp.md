# catalog__catalog.cpp.md

## File Location
`duckdb/src/catalog/catalog.cpp`

## Purpose
Implements the Catalog class - central interface for managing database objects (tables, views, functions, types, etc.).

## Key Classes and Their Purpose

### Catalog
- **Purpose**: Abstract base class for catalog implementations
- **Key Members**:
  - `db` - AttachedDatabase reference
  - `default_table` - Default table name for catalog-level queries
  - `default_table_schema` - Schema of default table

### CatalogLookup (internal struct)
- **Purpose**: Stores catalog lookup information
- **Key Members**:
  - `catalog` - Reference to catalog
  - `schema` - Schema name
  - `name` - Entry name
  - `lookup_info` - Entry lookup details

## Key Methods with Brief Descriptions

**Catalog Access:**
- `GetCatalog()` - Gets catalog by name, throws if not found
- `GetCatalogEntry()` - Gets optional catalog by name
- `GetSystemCatalog()` - Gets the system catalog
- `GetName()` - Returns catalog name from attached database

**Schema Operations:**
- `GetSchema()` - Gets schema by name
- `GetSchemas()` - Returns all schemas
- `GetAllSchemas()` - Returns all schemas across all databases
- `CreateSchema()` - Creates new schema

**Table Operations:**
- `CreateTable()` - Creates table from bound or unbound info
- `TryLookupDefaultTable()` - Looks up catalog's default table

**View Operations:**
- `CreateView()` - Creates a view

**Function Operations:**
- `CreateFunction()` - Creates scalar/aggregate function
- `CreateTableFunction()` - Creates table function
- `AddFunction()` - Adds function with ALTER_ON_CONFLICT
- `CreatePragmaFunction()` - Creates PRAGMA function

**Type Operations:**
- `CreateType()` - Creates user-defined type

**Sequence Operations:**
- `CreateSequence()` - Creates sequence

**Index Operations:**
- `CreateIndex()` - Creates index
- `BindCreateIndex()` - Binds CREATE INDEX to logical operator

**Entry Lookup:**
- `GetEntry()` - Gets catalog entry by type and name
- `LookupEntry()` - Looks up entry with error handling
- `TryLookupEntry()` - Attempts lookup, returns error info
- `SimilarEntriesInSchemas()` - Finds similar entries for suggestions

**Drop/Alter:**
- `DropEntry()` - Drops catalog entry
- `Alter()` - Alters catalog entry

**Extension Autoloading:**
- `TryAutoLoad()` - Attempts to autoload extension
- `AutoLoadExtensionByCatalogEntry()` - Autoloads based on entry type
- `UnrecognizedConfigurationError()` - Creates helpful error with extension hint

**Utilities:**
- `GetMetadataInfo()` - Returns metadata block info
- `SupportsCreateTable()` - Validates table creation options
- `HasDefaultTable()` - Whether catalog has default table
- `SetDefaultTable()` - Sets default table for catalog-level queries

## Important Concepts
- **Multi-Catalog System**: DuckDB supports multiple attached databases (catalogs)
- **Search Path**: Entry resolution follows search path (catalog, schema)
- **Extension Autoloading**: Missing functions/types can trigger extension load
- **Default Tables**: Catalogs can have default tables (queried by catalog name)
- **Entry Qualification**: Minimal qualification for unambiguous entry reference
- **Similar Entries**: Error messages suggest similar entries when lookup fails
- **Time Travel**: Some catalogs support AT clause for time travel queries
