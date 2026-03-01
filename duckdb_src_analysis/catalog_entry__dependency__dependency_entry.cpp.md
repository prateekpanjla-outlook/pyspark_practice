# catalog_entry__dependency__dependency_entry.cpp.md

## File Location
`duckdb/src/catalog/catalog_entry/dependency/dependency_entry.cpp`

## Purpose
Implements DependencyEntry - base class for dependency relationship catalog entries.

## Key Classes and Their Purpose

### DependencyEntry
- **Purpose**: Base class for dependency tracking entries
- **Key Members**:
  - `dependent_name` - Mangled name of dependent entry
  - `subject_name` - Mangled name of subject entry
  - `dependent` - Dependent side info
  - `subject` - Subject side info
  - `side` - DEPENDENT or SUBJECT (which side this entry represents)
- Inherits from InCatalogEntry

## Key Methods with Brief Descriptions

- `DependencyEntry()` - Constructor from DependencyInfo
  - Stores mangled names of both dependent and subject
  - Sets side to DEPENDENT or SUBJECT
  - Marks as temporary if catalog is temporary

- `SubjectMangledName()` - Returns subject's mangled name
- `Subject()` - Returns subject info
- `DependentMangledName()` - Returns dependent's mangled name
- `Dependent()` - Returns dependent info
- `Side()` - Returns which side this entry represents

## Important Concepts
- **Dependency Tracking**: Catalog-level tracking of object dependencies
- **Mangled Names**: Names are mangled for unique identification
- **Two-Sided Entries**: Each dependency creates two entries (dependent + subject)
- **Subject**: The object being depended on (e.g., a table)
- **Dependent**: The object that depends (e.g., a view referencing the table)
- **Temporary Catalogs**: Dependencies in temp catalogs also marked temporary
