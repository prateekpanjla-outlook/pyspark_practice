# catalog_entry__index_catalog_entry.cpp.md

## File Location
`duckdb/src/catalog/catalog_entry/index_catalog_entry.cpp`

## Purpose
Implements IndexCatalogEntry - base class for index catalog entries storing index metadata.

## Key Classes and Their Purpose

### IndexCatalogEntry
- **Purpose**: Base class for all index catalog entries
- **Key Members**:
  - `sql` - Original CREATE INDEX SQL statement
  - `options` - Index options (case insensitive, etc.)
  - `index_type` - Type of index (ART, BOUND, etc.)
  - `index_constraint_type` - Constraint type (UNIQUE, PRIMARY, FOREIGN)
  - `column_ids` - Columns included in the index
  - `expressions` - Bound expressions for expression-based indexes
  - `parsed_expressions` - Parsed expressions for serialization

## Key Methods with Brief Descriptions

- Constructor - Creates entry from CreateIndexInfo
  - Stores all index metadata
  - Copies expressions

- `GetInfo()` - Returns CreateIndexInfo for serialization
- `ToSQL()` - Generates CREATE INDEX SQL statement
- `IsUnique()` - Returns true if index enforces uniqueness
- `IsPrimary()` - Returns true if index is a primary key constraint

## Important Concepts
- **Constraint Types**: Indexes can represent UNIQUE, PRIMARY KEY, or FOREIGN KEY constraints
- **Expression Indexes**: Supports indexes on expressions (not just columns)
- **SQL Storage**: Original SQL preserved for serialization
