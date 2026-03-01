# catalog_entry_retriever.cpp

## File Location
`duckdb/src/catalog/catalog_entry_retriever.cpp`

## Purpose
Implementation of CatalogEntryRetriever - a helper class for retrieving catalog entries with configurable search behavior.

## Key Class: CatalogEntryRetriever

### Purpose
Provides a context-aware way to look up catalog entries with:
- Configurable search path
- Callback mechanism for retrieved entries
- AT clause support (time travel)

### Member Variables:
- `context` - ClientContext for the retrieval
- `callback` - Optional callback to call when entries are retrieved
- `search_path` - Optional override for search path
- `at_clause` - Optional AT clause for time travel queries

### Key Methods:

**Type Retrieval:**
- `GetType()` - Retrieves a custom type by name, returns LogicalType
  - Two overloads: one with Catalog reference, one with catalog name string
  - Returns LogicalType::INVALID if not found

**Entry Retrieval:**
- `GetEntry()` - Retrieves catalog entries
  - Multiple overloads for different parameter combinations
  - Can specify catalog/schema explicitly or use search path
  - Uses ReturnAndCallback() to invoke callback if set

**Schema Retrieval:**
- `GetSchema()` - Retrieves schema entries
  - Returns optional_ptr<SchemaCatalogEntry>
  - Invokes callback if set

**Configuration:**
- `Inherit()` - Copies configuration from parent retriever
  - Copies callback, search_path, and at_clause

- `SetSearchPath()` - Sets custom search path
  - Filters out invalid catalogs (SYSTEM_CATALOG, TEMP_CATALOG)
  - Merges with client's existing search path

- `SetAtClause()` / `GetAtClause()` - Manages time travel clause

- `SetCallback()` / `GetCallback()` - Manages callback function

**Accessors:**
- `GetSearchPath()` - Returns the search path (either custom or client's default)
- `GetContext()` - Returns the ClientContext

### ReturnAndCallback()
Helper method that:
1. Checks if result exists
2. If callback is set, invokes it with the result
3. Returns the result

## Important Concepts

### Search Path Resolution
The search path defines the order in which schemas are searched:
1. Temporary catalog (TEMP_CATALOG) - highest priority
2. User-configured schemas
3. Invalid catalog (resolves to default database)
4. System catalog (SYSTEM_CATALOG)
5. pg_catalog (PostgreSQL compatibility)

### AT Clause (Time Travel)
When querying historical data:
- `at_clause` contains the time point
- Passed through to lookup methods
- Catalog must support time travel for this to work

### Callback Pattern
The callback is invoked for every retrieved entry, allowing:
- Tracking which entries were accessed
- Building dependency information
- Custom logging or monitoring

## Usage Example
```cpp
CatalogEntryRetriever retriever(context);

// Set custom search path
retriever.SetSearchPath({
    {"my_db", "my_schema"},
    {"my_db", "public"}
});

// Set callback to track access
retriever.SetCallback([](CatalogEntry &entry) {
    // Log or track entry access
});

// Retrieve entry
auto entry = retriever.GetEntry(catalog, schema, lookup_info);
```

## Notes
- Provides a flexible interface for catalog lookups
- Used extensively in binding phase of query processing
- Essential for resolving unqualified object references
