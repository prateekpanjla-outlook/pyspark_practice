# catalog_entry__dependency__dependency_subject_entry.cpp.md

## File Location
`duckdb/src/catalog/catalog_entry/dependency/dependency_subject_entry.cpp`

## Purpose
Implements DependencySubjectEntry - catalog entry representing the subject side of a dependency relationship.

## Key Classes and Their Purpose

### DependencySubjectEntry
- **Purpose**: Represents the subject (dependency target) in a relationship
- **Key Members**:
  - `dependent_name` - Mangled name of dependent entry
  - `subject_name` - Mangled name of subject entry
- Inherits from DependencyEntry

## Key Methods with Brief Descriptions

- `DependencySubjectEntry()` - Constructor from DependencyInfo
  - Entry name is mangled combination: "subject→dependent"
  - Entry type is SUBJECT

- `EntryMangledName()` - Returns subject's mangled name (this entry)
- `EntryInfo()` - Returns subject's catalog entry info
- `SourceMangledName()` - Returns dependent's mangled name (dependency source)
- `SourceInfo()` - Returns dependent's catalog entry info

## Important Concepts
- **Dependency Direction**: Subject is what the dependent depends on (subject → dependent)
- **Reverse Lookup**: Enables finding what depends on a given object
- **Cascade Detection**: Used to determine what to invalidate when subject changes
- **Mangled Names**: Names combined for unique identification
