# duck_catalog.cpp

## File Location
`duckdb/src/catalog/duck_catalog.cpp`

## Purpose
Implementation of DuckCatalog - the main DuckDB catalog implementation.

## Key Class: DuckCatalog

### Purpose
Concrete implementation of the Catalog interface for DuckDB's native catalog.

### Member Variables:
- `dependency_manager` - Manages object dependencies
- `schemas` - CatalogSet containing schema entries
- `encryption_key_id` - ID of encryption key for encrypted databases
- `is_encrypted` - Whether catalog is encrypted

### Key Methods:

**Initialization:**
- `Initialize()` - Initializes the catalog
  - Creates default schema
  - Loads built-in functions if requested
  - Calls BuiltinFunctions::Initialize()
  - Calls FunctionList::RegisterFunctions()

**Schema Management:**
- `CreateSchema()` - Creates new schema
  - Validates internal schema conflicts
  - Handles OnCreateConflict (ERROR/REPLACE/IGNORE)
  - Uses CreateSchemaInternal() for actual creation

- `CreateSchemaInternal()` - Internal schema creation logic
  - Creates DuckSchemaEntry
  - Adds to schemas CatalogSet

- `DropSchema()` - Drops schema by name
- `ScanSchemas()` - Iterates all schemas
- `LookupSchema()` - Looks up schema by name

**Storage Information:**
- `GetDatabaseSize()` - Returns database size information
  - Acquires shared checkpoint lock
  - Delegates to StorageManager

- `GetMetadataInfo()` - Returns metadata block information
- `GetCatalogVersion()` - Returns current catalog version

**Catalog Type:**
- `IsDuckCatalog()` - Returns true (identifies this as DuckCatalog)

**Encryption:**
- `SetEncryptionKeyId()` / `GetEncryptionKeyId()` - Encryption key ID management
- `SetIsEncrypted()` / `GetIsEncrypted()` - Encryption flag management
- `IsEncrypted()` - Checks if catalog is encrypted
- `GetEncryptionCipher()` - Returns cipher type used

**Database Properties:**
- `InMemory()` - Returns whether database is in-memory
- `GetDBPath()` - Returns database file path

**Dependency Manager:**
- `GetDependencyManager()` - Returns the dependency manager

**Verification:**
- `Verify()` - Verifies catalog integrity (DEBUG builds only)

## Important Concepts

### Default Schema
The default schema (usually "main") is:
- Created during initialization
- Marked as internal
- Uses IGNORE_ON_CONFLICT to avoid errors if already exists

### Schema CatalogSet
The `schemas` member is a CatalogSet containing:
- All schemas in this catalog
- Default schema generator for system catalog
- Transactional access via MVCC

### Encryption Support
DuckCatalog tracks encryption state:
- `is_encrypted` - Whether catalog content is encrypted
- `encryption_key_id` - Which key to use for encryption/decryption
- Delegates actual encryption to StorageManager

### Built-in Functions
When `load_builtin` is true during Initialize():
- Scalar functions (math, string, date, etc.)
- Aggregate functions (sum, avg, count, etc.)
- Table functions (glob, range, etc.)
- Pragma functions
- Copy functions

### System Catalog vs User Catalog
- System catalog: Built-in functions, types, internal objects
- User catalog: User-defined tables, views, functions
- System catalog creates default schema generator for lazy initialization

## Usage Example
```cpp
// Get DuckCatalog from AttachedDatabase
DuckCatalog &catalog = db.GetCatalog().Cast<DuckCatalog>();

// Create schema
CreateSchemaInfo info;
info.schema = "myschema";
catalog.CreateSchema(transaction, info);

// Get dependency manager
auto &dep_manager = catalog.GetDependencyManager();
```

## Notes
- DuckCatalog is the standard catalog implementation for DuckDB databases
- Other implementations exist (e.g., for PostgreSQL catalog in ATTACH)
- Heavily integrated with DependencyManager for referential integrity
