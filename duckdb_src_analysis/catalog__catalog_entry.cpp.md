# catalog_entry.cpp

## File Location
`duckdb/src/catalog/catalog_entry.cpp`

## Purpose
Base implementations for catalog entries - the fundamental objects stored in the catalog (tables, views, functions, etc.).

## Key Classes

### CatalogEntry
Base class for all catalog entries.

#### Member Variables:
- `oid` - Object identifier
- `type` - CatalogType enum value
- `name` - Entry name
- `set` - Pointer to owning CatalogSet
- `deleted` - Whether entry is deleted (tombstone)
- `temporary` - Whether this is a temporary entry
- `internal` - Whether this is an internal system entry
- `parent` - Pointer to parent entry (for version chain)
- `child` - Pointer to child entry (for version chain)

#### Key Methods:

**Version Chain Management:**
- `SetChild()` / `TakeChild()` - Manage child entries in version chain
- `HasChild()` / `HasParent()` - Check chain relationships
- `Child()` / `Parent()` - Access chain members

**Altering:**
- `AlterEntry()` - Creates a new version of this entry (throws by default, overridden by subclasses)
- `UndoAlter()` - Undoes an alter operation

**Serialization:**
- `Serialize()` - Serializes entry to serializer
- `GetInfo()` - Returns CreateInfo for this entry
- `Deserialize()` - Creates entry from deserialized data

**Validation:**
- `Verify()` - Verifies entry integrity

**Lifecycle:**
- `OnDrop()` - Called when entry is dropped
- `Rollback()` - Rollback to previous state

**Parent Catalog Access:**
- `ParentCatalog()` - Returns parent catalog (throws in base class)
- `ParentSchema()` - Returns parent schema (throws in base class)

### InCatalogEntry
Derived class for entries that are part of a catalog.

#### Key Features:
- Stores reference to parent Catalog
- `Verify()` method ensures catalog consistency

## Important Concepts

### Version Chain (MVCC)
DuckDB uses Multi-Version Concurrency Control for catalog entries:
```
Head Entry (newest)
    ↓
    v (child pointer)
Parent Entry
    ↓
    v (child pointer)
Older Entry
    ↓
    ...
```

- Each modification creates a new version
- Old versions are kept for transaction isolation
- Child pointers go from newer → older versions

### Tombstone Entries
When an entry is deleted:
- A "tombstone" entry is created with `deleted = true`
- The tombstone replaces the entry in the map
- The old entry becomes the child of the tombstone

### Entry Types
CatalogType enum includes:
- `TABLE_ENTRY`
- `VIEW_ENTRY`
- `SCHEMA_ENTRY`
- `TYPE_ENTRY`
- `SCALAR_FUNCTION_ENTRY`
- `AGGREGATE_FUNCTION_ENTRY`
- `TABLE_FUNCTION_ENTRY`
- `PRAGMA_FUNCTION_ENTRY`
- `MACRO_ENTRY`
- `INDEX_ENTRY`
- `SEQUENCE_ENTRY`
- `COLLATION_ENTRY`
- `COPY_FUNCTION_ENTRY`
- `DEPENDENCY_ENTRY`
- `RENAMED_ENTRY`
- `DELETED_ENTRY`

## Usage Pattern
When a catalog entry is modified:
1. Create new version with `AlterEntry()`
2. Set new version's child to old version
3. Update catalog map with new version
4. Old version kept for transaction rollback

## Notes
- Base class provides default implementations that throw exceptions
- Subclasses override specific methods for their behavior
- The version chain is key to DuckDB's transaction handling
