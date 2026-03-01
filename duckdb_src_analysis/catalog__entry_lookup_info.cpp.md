# entry_lookup_info.cpp

## File Location
`duckdb/src/catalog/entry_lookup_info.cpp`

## Purpose
Implementation of EntryLookupInfo - encapsulates information for looking up catalog entries.

## Key Class: EntryLookupInfo

### Purpose
Simple data structure that holds all information needed to look up a catalog entry.

### Member Variables:
- `catalog_type` - The type of catalog entry to look up (CatalogType enum)
- `name` - The name of the entry to look up
- `at_clause` - Optional time travel clause (BoundAtClause)
- `error_context` - Query error context for better error messages

### Constructors:

**Basic Lookup:**
```cpp
EntryLookupInfo(CatalogType catalog_type_p, const string &name_p,
                QueryErrorContext error_context_p)
```
Creates basic lookup info without time travel.

**With Time Travel:**
```cpp
EntryLookupInfo(CatalogType catalog_type_p, const string &name_p,
                optional_ptr<BoundAtClause> at_clause_p,
                QueryErrorContext error_context_p)
```
Creates lookup info with time travel clause.

**Copy Constructors:**
```cpp
EntryLookupInfo(const EntryLookupInfo &parent, const string &name_p)
```
Copies all info except changes the name.

```cpp
EntryLookupInfo(const EntryLookupInfo &parent, optional_ptr<BoundAtClause> at_clause)
```
Copies all info but changes/sets the AT clause.

### Key Methods:

**Static Factory:**
- `SchemaLookup()` - Creates EntryLookupInfo for schema lookup
  - Used internally when resolving schema names
  - Sets catalog_type to SCHEMA_ENTRY

**Accessors:**
- `GetCatalogType()` - Returns the catalog type being looked up
- `GetEntryName()` - Returns the entry name
- `GetErrorContext()` - Returns error context for error messages
- `GetAtClause()` - Returns optional AT clause for time travel

## Important Concepts

### Time Travel Support
The `at_clause` field enables querying historical data:
- `AS OF <timestamp>` - Query data at specific time
- `BETWEEN <start> AND <end>` - Query data in time range
- Catalog must support time travel for this to work

### Error Context
When a lookup fails, `error_context` provides:
- Query location information
- Helpful error messages
- Context about where the error occurred

### Schema Lookup Pattern
```cpp
// Looking up a table
EntryLookupInfo lookup(CatalogType::TABLE_ENTRY, "employees", context);

// Looking up a schema (static method)
EntryLookupInfo schema_lookup = EntryLookupInfo::SchemaLookup(parent, "main");
```

## Usage Example
```cpp
// Create lookup info for table
EntryLookupInfo lookup(CatalogType::TABLE_ENTRY, "mytable", error_context);

// Create with time travel
BoundAtClause at_clause(...);
EntryLookupInfo lookup_tt(CatalogType::TABLE_ENTRY, "mytable", at_clause, error_context);

// Use for lookup
auto entry = catalog.GetEntry(retriever, schema, lookup);
```

## Notes
- Very lightweight data structure (mostly pass-through)
- Used extensively in catalog lookup code paths
- Enables consistent error reporting across catalog operations
- Time travel support is key to DuckDB's versioning features
