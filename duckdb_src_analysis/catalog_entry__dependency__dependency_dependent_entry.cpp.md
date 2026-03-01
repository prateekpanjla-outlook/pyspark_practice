# catalog_entry__dependency__dependency_dependent_entry.cpp.md

## File Location
`duckdb/src/catalog/catalog_entry/dependency/dependency_dependent_entry.cpp`

## Purpose
Implements DependencyDependentEntry - catalog entry representing the dependent side of a dependency relationship.

## Key Classes and Their Purpose

### DependencyDependentEntry
- **Purpose**: Represents the dependent (observer) in a dependency relationship
- **Key Members**:
  - `dependent_name` - Mangled name of dependent entry
  - `subject_name` - Mangled name of subject (dependency target)
- Inherits from DependencyEntry

## Key Methods with Brief Descriptions

- `DependencyDependentEntry()` - Constructor from DependencyInfo
  - Entry name is mangled combination: "subject→dependent"
  - Entry type is DEPENDENT

- `EntryMangledName()` - Returns dependent's mangled name (this entry)
- `EntryInfo()` - Returns dependent's catalog entry info
- `SourceMangledName()` - Returns subject's mangled name (dependency source)
- `SourceInfo()` - Returns subject's catalog entry info

## Important Concepts
- **Dependency Direction**: Dependent depends on subject (subject ← dependent)
- **Mangled Names**: Names mangled for unique identification
- **Bidirectional Tracking**: Each dependency has both subject and dependent entries
- **Cascade Detection**: Used to detect what needs update/recompilation when subject changes
