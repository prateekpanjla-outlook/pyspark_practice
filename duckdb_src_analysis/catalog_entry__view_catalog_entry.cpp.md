# catalog_entry__view_catalog_entry.cpp.md

## File Location
`duckdb/src/catalog/catalog_entry/view_catalog_entry.cpp`

## Purpose
Implements ViewCatalogEntry - catalog entry for views (stored SQL queries).

## Key Classes and Their Purpose

### ViewCatalogEntry
- **Purpose**: Catalog entry for VIEW objects
- **Key Members**:
  - `query` - SelectStatement defining the view
  - `aliases` - View column aliases
  - `view_columns` - Atomic pointer to ViewColumnInfo (names, types)
  - `bind_state` - UNBOUND, BINDING, or BOUND
  - `bind_thread` - Thread ID for recursive binding detection
  - `bind_lock` - Mutex protecting binding operations
  - `sql` - Original CREATE VIEW SQL
  - `column_comments` - Map of column comments

### ViewBindState (enum)
- **UNBOUND**: View not yet bound
- **BINDING**: View currently being bound (recursive detection)
- **BOUND**: View successfully bound

### ViewColumnInfo
- **Purpose**: Stores bound view column information
- **Key Members**:
  - `names` - Column names
  - `types` - Column types

## Key Methods with Brief Descriptions

- `Initialize()` - Sets up view from CreateViewInfo
  - Stores query, aliases, SQL
  - Sets to BOUND if types/names provided

- `BindView()` - Binds the view query to determine columns
  - Thread-safe with mutex
  - Detects recursive binding attempts
  - Calls Binder::BindView to determine types/names
  - Atomic store of view columns

- `UpdateBinding()` - Updates binding with new types/names
  - No-op if already bound with same info
  - Atomic store of new columns

- `GetColumnInfo()` - Atomic load of view columns
- `GetColumnComment()` - Returns comment for specific column

- `AlterEntry()` - Handles view alterations
  - RENAME_VIEW: Renames the view
  - SET_COLUMN_COMMENT: Adds/updates column comments

- `GetInfo()` - Returns CreateViewInfo for serialization
- `ToSQL()` - Generates CREATE VIEW SQL statement
- `Copy()` - Creates copy of the entry (internal views cannot be copied)

## Important Concepts
- **Lazy Binding**: Views are bound on first use, not at creation
- **Atomic Column Storage**: View columns stored atomically for thread-safe access
- **Recursive Detection**: Detects recursive view definitions during binding
- **View Persistence**: Original SQL stored for serialization
