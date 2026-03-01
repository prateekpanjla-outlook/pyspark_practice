# dependency_manager.cpp

## File Location
`duckdb/src/catalog/dependency_manager.cpp`

## Purpose
Implementation of DependencyManager - manages dependencies between catalog entries (tables, views, functions, etc.).

## Key Classes

### MangledEntryName
Represents a mangled (encoded) catalog entry name for dependency tracking.

#### Format: `CatalogType + '\0' + schema + '\0' + name`

Example: `"TABLE_ENTRY\0main\0employees"`

### MangledDependencyName
Represents a dependency relationship between two entries.

#### Format: `MangledEntryName + '\0' + MangledEntryName`

### DependencyManager
Manages all dependencies in the catalog.

#### Member Variables:
- `catalog` - Reference to owning DuckCatalog
- `subjects` - CatalogSet of what entries depend on (forward dependencies)
- `dependents` - CatalogSet of what depends on entries (backward dependencies)

### Key Methods:

**Dependency Creation:**
- `AddObject()` - Registers object with its dependencies
- `CreateDependencies()` - Creates dependency links for an object
- `CreateDependency()` - Creates bidirectional dependency links
- `CreateSubject()` - Creates entry in subjects (what object depends on)
- `CreateDependent()` - Creates entry in dependents (what depends on object)

**Dependency Removal:**
- `RemoveDependency()` - Removes dependency link
- `CleanupDependencies()` - Removes all dependencies for an object

**Drop Operations:**
- `DropObject()` - Drops object and all owned/cascaded objects
- `CheckDropDependencies()` - Validates DROP can proceed
  - Collects blocking dependents
  - Throws error if cascade needed but not specified
  - Returns set of objects to drop

**Dependency Scanning:**
- `ScanSubjects()` - Scans what entry depends on
- `ScanDependents()` - Scans what depends on entry
- `ScanSetInternal()` - Internal scanning logic

**Validation:**
- `VerifyExistence()` - Verifies dependency target still exists
- `VerifyCommitDrop()` - Verifies no new dependencies since transaction start

**Alter Operations:**
- `AlterObject()` - Updates dependencies after ALTER
  - Handles rename (recreates links)
  - Preserves existing dependencies
  - Validates dependent entries allow alteration

**Ownership:**
- `AddOwnership()` - Creates ownership relationship
  - Prevents circular ownership
  - Validates owner not already owned
  - Owned entries are dropped with owner

**Reordering:**
- `ReorderEntries()` - Topologically sorts entries for checkpoint
  - Dependencies written before dependents
  - Used for proper serialization order

**Utility:**
- `MangleName()` - Creates mangled name from entry
- `GetSchema()` - Extracts schema from entry
- `LookupEntry()` - Resolves dependency to actual entry
- `CollectDependents()` - Builds error message for blocked drops

## Important Concepts

### Bidirectional Dependencies
Dependencies are tracked in both directions:
```
subjects:     view → table (what "view" depends on)
dependents:   table → view (what depends on "table")
```

### Dependency Flags
Control drop behavior:
- **Blocking**: DROP blocks without CASCADE (tables, views, etc.)
- **Owned by**: DROP owner drops this entry too (indexes, sequences)
- **Ownership**: This entry owns the dependent

### CASCADE DROP
When dropping with CASCADE:
1. Find all dependents
2. Recursively drop those with blocking dependencies
3. Drop owned entries automatically
4. Clean up dependency links

### Mangled Names
Null-byte separated format for efficient lookup:
- Enables prefix scans in CatalogSet
- Type and schema included for uniqueness
- Example: `"TABLE_ENTRY\0main\0employees"`

### Cross-Catalog Validation
Dependencies must be within same catalog:
- `AddObject()` throws if cross-catalog dependency detected
- Prevents dangling references across databases

### Drop Validation
Before committing DROP:
1. Check for new dependencies created after transaction started
2. Verify no conflicting ownership changes
3. Ensure all dependencies can be satisfied

### System Entry Handling
System entries are handled specially:
- Internal entries don't track dependencies
- DEPENDENCY_ENTRY, DATABASE_ENTRY, RENAMED_ENTRY are internal

## Error Messages
DependencyManager provides detailed error messages:
```
Cannot drop entry "employees" because there are entries that depend on it.
view "employee_view" depends on table "employees".
materialized_view "mv_employees" depends on view "employee_view".
Use DROP...CASCADE to drop all dependents.
```

## Notes
- Central to referential integrity in DuckDB
- Enables CASCADE drops and dependency tracking
- Prevents orphaned objects
- Used during checkpoint for proper serialization order
