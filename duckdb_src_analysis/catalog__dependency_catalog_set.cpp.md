# catalog__dependency_catalog_set.cpp.md

## File Location
`duckdb/src/catalog/dependency_catalog_set.cpp`

## Purpose
Implements DependencyCatalogSet - a wrapper around CatalogSet that provides prefix-based filtering for dependency entries.

## Key Classes and Their Purpose

### DependencyCatalogSet
- **Purpose**: Filtered view of catalog entries for dependency management
- **Key Members**:
  - `set` - Reference to underlying CatalogSet
  - `mangled_name` - Mangled name used for prefix filtering

## Key Methods with Brief Descriptions

- `ApplyPrefix()` - Combines source mangled name with entry name for unique storage
- `CreateEntry()` - Creates entry with prefixed name and empty dependencies
- `GetEntryDetailed()` - Gets detailed entry lookup with prefix applied
- `GetEntry()` - Gets entry by prefixed name
- `Scan()` - Scans entries matching the prefix (filters by source name)
- `DropEntry()` - Drops entry by prefixed name

## Important Concepts
- **Prefix Filtering**: Only entries matching the source name are returned during scans
- **Mangled Names**: Combines source + target names for unique storage
- **Bidirectional Storage**: Each dependency creates two entries (subject and dependent)
