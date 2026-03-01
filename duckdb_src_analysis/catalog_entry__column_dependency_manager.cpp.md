# catalog_entry__column_dependency_manager.cpp.md

## File Location
`duckdb/src/catalog/catalog_entry/column_dependency_manager.cpp`

## Purpose
Implements ColumnDependencyManager - tracks dependencies between columns, especially for generated columns.

## Key Classes and Their Purpose

### ColumnDependencyManager
- **Purpose**: Manages column dependencies for generated/computed columns
- **Key Members**:
  - `dependents_map` - Maps column → set of columns that depend on it
  - `dependencies_map` - Maps column → set of columns it depends on
  - `direct_dependencies` - Maps generated column → direct dependencies only
  - `deleted_columns` - Tracks deleted columns during cleanup

## Key Methods with Brief Descriptions

- `AddGeneratedColumn()` - Registers generated column with its dependencies
  - Extracts referenced columns from column definition
  - Validates referenced columns exist
  - Inherits transitive dependencies
  - Detects circular dependencies

- `RemoveColumn()` - Removes column and updates all dependencies
  - Returns adjusted indices for remaining columns
  - Handles both generated and standard columns

- `IsDependencyOf()` - Checks if column B depends on column A
- `HasDependencies()` / `GetDependencies()` - Query what a column depends on
- `HasDependents()` / `GetDependents()` - Query what depends on a column

- `GetBindOrder()` - Returns topological order for binding columns
  - Leaves (no dependencies) bound first
  - Generated columns bound after their dependencies

## Important Concepts
- **Circular Dependency Detection**: Prevents infinite loops in generated column references
- **Transitive Dependencies**: Generated columns inherit dependencies of their dependencies
- **Index Adjustment**: When columns are removed, indices are adjusted for remaining columns
- **Bind Order**: Topological sort ensures columns are bound in correct order
