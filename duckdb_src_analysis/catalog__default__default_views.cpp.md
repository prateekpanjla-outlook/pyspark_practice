# catalog__default__default_views.cpp.md

## File Location
`duckdb/src/catalog/default/default_views.cpp`

## Purpose
Implements DefaultViewGenerator - creates default system views for metadata access.

## Key Classes and Their Purpose

### DefaultView (struct)
- **Purpose**: Defines default view
- **Key Members**:
  - `schema` - Schema name (main, pg_catalog, information_schema)
  - `name` - View name
  - `sql` - View SQL definition

### DefaultViewGenerator
- **Purpose**: Generates default view catalog entries
- Inherits from DefaultGenerator

## Key Methods with Brief Descriptions

- `GetDefaultView()` - Gets CreateViewInfo for schema and view name
  - Returns nullptr if not a default view
  - Calls CreateViewInfo::FromSelect to parse SQL

- `CreateDefaultEntry()` - Creates ViewCatalogEntry for default view
- `GetDefaultEntries()` - Lists all default views in schema

## Important Concepts
- **System Views**: Pre-defined views for metadata access
- **SQL Standard Views**: information_schema for standard compliance
- **PostgreSQL Compatibility**: pg_catalog views for PG tools
- **Dynamic Views**: Most views wrap PRAGMA functions for dynamic data

## Notable Default Views

**main schema:**
- `sqlite_master`, `sqlite_schema` - SQLite compatibility
- `duckdb_tables`, `duckdb_columns`, `duckdb_views` - Catalog metadata
- `duckdb_databases`, `duckdb_schemas`, `duckdb_types` - Database structure
- `duckdb_indexes`, `duckdb_constraints` - Index/constraint metadata
- `duckdb_logs` - Query logs

**pg_catalog schema:**
- `pg_class` - Tables, views, indexes, sequences
- `pg_attribute` - Column metadata
- `pg_namespace` - Schema metadata
- `pg_type` - Type metadata
- `pg_proc` - Function metadata
- `pg_constraint` - Constraint metadata
- `pg_index` - Index metadata
- `pg_database` - Database metadata
- `pg_tables`, `pg_views` - Table/view listings
- `pg_settings` - Configuration settings
- `pg_prepared_statements` - Prepared statements

**information_schema schema:**
- `columns` - Column metadata
- `tables` - Table and view metadata
- `schemata` - Schema metadata
- `views` - View metadata
- `table_constraints` - Table constraints
- `key_column_usage` - Primary/unique/foreign key columns
- `referential_constraints` - Foreign key constraints
- `check_constraints` - CHECK constraints
- `constraint_column_usage` - Constraints using columns
- `constraint_table_usage` - Tables with constraints
- `character_sets` - Character set info
