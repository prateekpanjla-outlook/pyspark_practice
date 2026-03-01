# catalog_entry__type_catalog_entry.cpp.md

## File Location
`duckdb/src/catalog/catalog_entry/type_catalog_entry.cpp`

## Purpose
Implements TypeCatalogEntry - catalog entry for user-defined types (ENUM, composite types).

## Key Classes and Their Purpose

### TypeCatalogEntry
- **Purpose**: Catalog entry for user-defined types
- **Key Members**:
  - `user_type` - LogicalType representation of the user type
  - `bind_function` - Optional bind function for type resolution
  - Inherits from StandardEntry

## Key Methods with Brief Descriptions

- Constructor - Creates entry from CreateTypeInfo
  - Stores the user-defined type
  - Stores optional bind function
  - Sets dependencies, comment, tags

- `Copy()` - Creates copy of the entry
- `GetInfo()` - Returns CreateTypeInfo for serialization
- `ToSQL()` - Generates CREATE TYPE SQL statement
  - Strips alias before calling ToString()
  - Outputs type as "CREATE TYPE name AS type;"

## Important Concepts
- **User-Defined Types**: Custom types like ENUM or composite types
- **Type Alias**: Stripped during SQL generation to avoid redundant output
- **Bind Function**: Optional callback for type resolution during binding
- **CREATE TYPE**: SQL command for defining custom types
