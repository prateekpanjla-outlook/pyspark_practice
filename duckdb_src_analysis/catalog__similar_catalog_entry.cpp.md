# similar_catalog_entry.cpp

## File Location
`duckdb/src/catalog/similar_catalog_entry.cpp`

## Purpose
Implementation of SimilarCatalogEntry - represents a catalog entry that is similar to a requested name (for suggestions).

## Key Class: SimilarCatalogEntry

### Purpose
Stores information about a catalog entry that is similar to what the user was looking for, used for "did you mean?" suggestions.

### Member Variables:
- `name` - The name of the similar entry
- `score` - Similarity rating (0.0 to 1.0, higher is more similar)
- `schema` - Pointer to the SchemaCatalogEntry containing the entry

### Key Methods:

**GetQualifiedName()**
```cpp
string GetQualifiedName(bool qualify_catalog, bool qualify_schema) const
```
Returns the qualified name based on what's needed:
- `qualify_catalog=true, qualify_schema=true` → `catalog.schema.name`
- `qualify_catalog=false, qualify_schema=true` → `schema.name`
- `qualify_catalog=false, qualify_schema=false` → `name`

## Important Concepts

### Similarity Scoring
The `score` field indicates how similar the entry is to the requested name:
- 1.0 = Exact match
- 0.8+ = Very similar (minor typo)
- 0.5+ = Somewhat similar
- <0.5 = Not very similar

### Use Case in Error Messages
When a catalog entry lookup fails:
```cpp
vector<SimilarCatalogEntry> similar = catalog.GetSimilarEntries(...);

if (!similar.empty()) {
    string message = "Table 'emplouees' does not exist. Did you mean 'employees'?";
    throw CatalogException(message);
}
```

### Qualification Strategy
`GetQualifiedName()` helps with minimal qualification:
- If schema is in search path → no qualification needed
- If only one catalog has the schema → schema.qualification sufficient
- Otherwise → full catalog.schema.name qualification

## Usage Example
```cpp
// Find similar entries
auto similar_entries = catalog.SimilarEntriesInSchemas(
    context,
    EntryLookupInfo(CatalogType::TABLE_ENTRY, "emplouees"), // typo
    schemas
);

// Build suggestion message
if (!similar_entries.empty()) {
    auto &entry = similar_entries[0];
    string qualified = entry.GetQualifiedName(qualify_db, qualify_schema);
    string error = "Did you mean " + qualified + "?";
}
```

## Notes
- Simple utility class for user-friendly error messages
- Used by Catalog::CreateMissingEntryException()
- Levenshtein distance or similar algorithm for scoring
- Key to DuckDB's helpful error messages
