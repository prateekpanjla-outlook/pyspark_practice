# catalog__dependency_list.cpp.md

## File Location
`duckdb/src/catalog/dependency_list.cpp`

## Purpose
Implements LogicalDependency and LogicalDependencyList for tracking dependencies between catalog entries.

## Key Classes and Their Purpose

### LogicalDependency
- **Purpose**: Represents a single dependency from one catalog entry to another
- **Key Members**:
  - `entry` - CatalogEntryInfo (type, schema, name)
  - `catalog` - Catalog name string

### LogicalDependencyList
- **Purpose**: Container for dependencies with automatic deduplication
- **Key Members**:
  - `set` - Unordered set of LogicalDependency objects

## Key Methods with Brief Descriptions

### LogicalDependency
- Constructor from CatalogEntry - Extracts dependency info
- Constructor from catalog/schema info - Manual construction
- `operator==()` - Equality comparison

### LogicalDependencyList
- `AddDependency()` - Adds dependency to set (automatic deduplication)
- `Contains()` - Checks if dependency exists
- `VerifyDependencies()` - Ensures all dependencies are in same catalog
- `Set()` - Returns internal set
- `operator==()` - Compares two dependency lists

## Important Concepts
- **Automatic Deduplication**: Set prevents duplicate dependencies
- **Cross-Catalog Validation**: Ensures dependencies stay within same catalog
- **Hash-Based Storage**: Uses custom hash and equality functions
