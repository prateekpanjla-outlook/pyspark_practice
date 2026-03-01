# catalog__default__default_table_functions.cpp.md

## File Location
`duckdb/src/catalog/default/default_table_functions.cpp`

## Purpose
Implements DefaultTableFunctionGenerator - creates internal table macro functions.

## Key Classes and Their Purpose

### DefaultTableMacro (struct)
- **Purpose**: Defines internal table macro
- **Key Members**:
  - `schema` - Schema name
  - `name` - Function name
  - `parameters` - Parameter names array
  - `named_parameters` - Default parameter values
  - `macro` - SQL SELECT statement

### DefaultTableFunctionGenerator
- **Purpose**: Generates internal table macro catalog entries
- Inherits from DefaultGenerator

## Key Methods with Brief Descriptions

- `CreateInternalTableMacroInfo()` - Creates CreateMacroInfo from macro definition
  - Adds positional parameters
  - Adds named parameters with defaults

- `CreateTableMacroInfo()` - Parses SQL and creates TableMacroFunction
  - Validates SQL is a SELECT statement
  - Extracts query node

- `GetDefaultTableFunction()` - Finds table macro by schema and name
- `CreateDefaultEntry()` - Creates TableMacroCatalogEntry
- `GetDefaultEntries()` - Lists all table macros in schema

## Important Concepts
- **Table Macros**: Functions that return tabular results defined as SQL
- **Internal Table Macros**: Pre-defined macros for common operations

## Notable Internal Table Macros

**Histogram:**
- `histogram_values` - Raw histogram bin data
- `histogram` - Formatted histogram with bar visualization
  - Parameters: source, col_name, bin_count (default 10), technique (auto/equi-width/equi-width-nice/sample)

**Logging:**
- `duckdb_logs_parsed` - Parsed structured log data
- `duckdb_profiling_settings` - Profiling-related settings
