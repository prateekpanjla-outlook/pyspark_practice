# catalog_entry__duck_table_entry.cpp.md

## File Location
`duckdb/src/catalog/catalog_entry/duck_table_entry.cpp`

## Purpose
Implementation of DuckTableEntry - the DuckDB-specific table catalog entry that manages table metadata, storage, and schema modifications.

## Key Class: DuckTableEntry

### Purpose
Represents a table in DuckDB with full support for ALTER TABLE operations, indexing, and storage management.

### Member Variables:
- `storage` - Shared pointer to DataTable (physical storage)
- `column_dependency_manager` - Tracks dependencies between columns (for generated columns)

### Key Methods:

**Creation:**
- Constructor creates DataTable and indexes from BoundCreateTableInfo
- Handles UNIQUE, PRIMARY KEY, and FOREIGN KEY constraints
- Creates ART (Adaptive Radix Tree) indexes for constraints

**ALTER TABLE Operations:**
- `AlterEntry()` - Main dispatcher for ALTER operations
- `RenameColumn()` - Renames a column (updates generated expressions, constraints)
- `AddColumn()` - Adds new column with default value
- `RemoveColumn()` - Drops column (handles dependent generated columns with CASCADE)
- `SetDefault()` - Changes DEFAULT value
- `SetNotNull()` / `DropNotNull()` - Adds/removes NOT NULL constraint
- `ChangeColumnType()` - Changes column type with USING expression
- `AddForeignKeyConstraint()` / `DropForeignKeyConstraint()` - FK management
- `AddConstraint()` - Adds UNIQUE/PRIMARY KEY constraints
- `SetColumnComment()` - Sets COMMENT on column

**Nested Type Operations (STRUCT/LIST/MAP):**
- `AddField()` - Adds field to nested STRUCT type
- `RemoveField()` - Removes field from nested STRUCT type
- `RenameField()` - Renames field in nested STRUCT type
- Uses `remap_struct()` function for type transformations

**Statistics:**
- `GetStatistics()` - Returns column statistics for query optimization

**Storage:**
- `GetStorage()` - Returns DataTable reference
- `GetStorageInfo()` - Returns storage metadata
- `CommitDrop()` - Commits table drop to storage
- `SetAsRoot()` - Marks as main table for storage

## Important Concepts

### Index Creation
- **UNIQUE constraints** → Unique ART index
- **PRIMARY KEY** → Unique ART index marked as PRIMARY
- **FOREIGN KEY** → Index on foreign key columns

### Generated Column Dependencies
- ColumnDependencyManager tracks which generated columns reference which base columns
- DROP COLUMN with CASCADE removes dependent generated columns
- RENAME COLUMN updates references in generated expressions

### Constraint Validation
**DROP COLUMN validation:**
- CHECK constraints: Must not reference dropped column (unless CASCADE)
- UNIQUE constraints: Cannot drop columns in multi-column UNIQUE
- FOREIGN KEY: Cannot drop FK columns

### Nested Type Handling
**STRUCT columns with ALTER:**
- `ALTER TABLE tbl ADD COLUMN col.field TYPE` - Add nested field
- `ALTER TABLE tbl DROP COLUMN col.field` - Remove nested field
- `ALTER TABLE tbl RENAME COLUMN col.field TO new_name` - Rename nested field
- Uses `remap_struct()` for type transformation

### Storage Management
- DataTable created with TableIOManager
- Indexes stored within DataTable (not separate catalog entries)
- Storage modifications require creating new DataTable instance
- ON DROP marks storage as dropped (delayed cleanup)

## Usage Example
```cpp
// Create table
BoundCreateTableInfo info;
DuckTableEntry table(catalog, schema, info);

// Alter table
RenameColumnInfo rename("old_name", "new_name");
table.AlterEntry(context, rename_info);

// Get statistics
auto stats = table.GetStatistics(context, ColumnIndex(0));
```

## Notes
- Full ALTER TABLE support makes DuckDB very flexible
- Generated columns add significant complexity
- ART indexes used for all constraint types
- Storage modifications create new DataTable instances
- Foreign key constraints handled specially with bidirectional references
