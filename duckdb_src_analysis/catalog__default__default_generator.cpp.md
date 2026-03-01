# catalog__default__default_generator.cpp.md

## File Location
`duckdb/src/catalog/default/default_generator.cpp`

## Purpose
Implements DefaultGenerator - base class for generating default catalog entries (schemas, functions, views, types).

## Key Classes and Their Purpose

### DefaultGenerator
- **Purpose**: Abstract base class for default entry generators
- **Key Members**:
  - `catalog` - Reference to catalog
  - `created_all_entries` - Whether all entries have been created

## Key Methods with Brief Descriptions

- `DefaultGenerator()` - Constructor initializing catalog reference
- `~DefaultGenerator()` - Virtual destructor

- `CreateDefaultEntry(ClientContext)` - Creates entry with client context
  - Throws InternalException if not overridden (pure virtual in derived classes)

- `CreateDefaultEntry(CatalogTransaction)` - Wrapper that extracts context from transaction
  - Returns nullptr if no context available

- `GetDefaultEntries()` - Pure virtual - returns list of default entry names

## Important Concepts
- **Lazy Creation**: Default entries created on-demand when referenced
- **Schema-Specific Generators**: Each schema has its own generator for functions, views, types
- **Transaction Context**: Can create entries within transaction context
- **Internal Entries**: Default entries are marked as internal and temporary
