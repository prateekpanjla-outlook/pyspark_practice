# catalog__default__default_schemas.cpp.md

## File Location
`duckdb/src/catalog/default/default_schemas.cpp`

## Purpose
Implements DefaultSchemaGenerator - creates default schemas (information_schema, pg_catalog).

## Key Classes and Their Purpose

### DefaultSchema (struct)
- **Purpose**: Defines default schema
- **Key Members**:
  - `name` - Schema name

### DefaultSchemaGenerator
- **Purpose**: Generates default schema catalog entries
- Inherits from DefaultGenerator

## Key Methods with Brief Descriptions

- `IsDefaultSchema()` - Static method checking if schema name is default
  - Returns true for: information_schema, pg_catalog

- `DefaultSchemaGenerator()` - Constructor
- `CreateDefaultEntry()` - Creates DuckSchemaEntry for default schema
  - Marks entry as internal
  - Returns nullptr for non-default schemas

- `GetDefaultEntries()` - Returns list of default schema names

## Important Concepts
- **Default Schemas**: Schemas automatically available in every database
- **information_schema**: SQL standard schema for metadata access
- **pg_catalog**: PostgreSQL compatibility schema
- **Internal Schemas**: Marked as internal to distinguish from user schemas
