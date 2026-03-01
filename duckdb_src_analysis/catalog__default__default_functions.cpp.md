# catalog__default__default_functions.cpp.md

## File Location
`duckdb/src/catalog/default/default_functions.cpp`

## Purpose
Implements DefaultFunctionGenerator - creates internal scalar macro functions (SQL macros).

## Key Classes and Their Purpose

### DefaultMacro (struct)
- **Purpose**: Defines internal scalar macro
- **Key Members**:
  - `schema` - Schema name (main or pg_catalog)
  - `name` - Function name
  - `parameters` - Parameter names array
  - `named_parameters` - Default parameter values
  - `macro` - Macro expression SQL

### DefaultFunctionGenerator
- **Purpose**: Generates internal scalar macro catalog entries
- Inherits from DefaultGenerator

## Key Methods with Brief Descriptions

- `CreateInternalMacroInfo()` - Creates CreateMacroInfo from macro definition(s)
  - Parses macro expression
  - Adds positional parameters
  - Adds named parameters with defaults

- `GetDefaultFunction()` - Finds internal macro by schema and name
  - Supports function overloading (multiple macros with same name)
  - Returns CreateMacroInfo for matching macro

- `CreateDefaultEntry()` - Creates ScalarMacroCatalogEntry for requested function
- `GetDefaultEntries()` - Lists all internal macros in schema

## Important Concepts
- **Internal Scalar Macros**: SQL functions implemented as macros
- **PostgreSQL Compatibility**: Many pg_catalog functions for PG compatibility
- **Function Overloading**: Same function name with different parameter counts

## Notable Internal Macros

**User/Session Info:**
- `current_user`, `current_role`, `session_user` - Returns 'duckdb'
- `current_database`, `current_schema` - Current catalog/schema

**PostgreSQL Compatibility Functions:**
- `pg_typeof` - Get data type of value
- `pg_sleep` - Sleep for specified seconds
- `pg_size_pretty` - Format bytes as readable size
- `pg_get_viewdef` - Get view definition

**Privilege Functions:** (all return true)
- `has_table_privilege`, `has_column_privilege`, etc.

**Array/List Functions:**
- `array_append`, `array_prepend` - Add element to list
- `array_pop_back`, `array_pop_front` - Remove elements
- `array_to_string` - Join list elements
- `list_reverse` - Reverse list
- `list_*` - Various list aggregate functions

**Date Functions:**
- `date_add` - Add interval to date
- `days_in_month` - Get days in month
- `ago` - Subtract interval from current_timestamp

**Math Functions:**
- `round_even` - Banker's rounding
- `roundbankers` - Alias for round_even
- `fdiv` - Floor division
- `fmod` - Floor modulo
- `geomean` - Geometric mean

**String Functions:**
- `split_part` - Split string and get part
- `nullif` - Return NULL if arguments equal
