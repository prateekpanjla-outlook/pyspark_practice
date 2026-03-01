# catalog_entry__table_function_catalog_entry.cpp.md

## File Location
`duckdb/src/catalog/catalog_entry/table_function_catalog_entry.cpp`

## Purpose
Implements TableFunctionCatalogEntry - catalog entry for table functions that return result sets.

## Key Classes and Their Purpose

### TableFunctionCatalogEntry
- **Purpose**: Catalog entry for table functions
- **Key Members**:
  - `functions` - TableFunctionSet containing overloaded implementations
  - Inherits from FunctionEntry

## Key Methods with Brief Descriptions

- Constructor - Creates entry from CreateTableFunctionInfo
  - Validates function set is non-empty
  - Sets catalog_name and schema_name for all functions
  - D_ASSERT ensures at least one function exists

- `AlterEntry()` - Supports adding new function overloads
  - Only accepts ALTER_TABLE_FUNCTION type
  - Only accepts ADD_FUNCTION_OVERLOADS operation
  - Merges new overloads with existing set
  - Validates no duplicate overloads
  - Creates new entry with merged function set

## Important Concepts
- **Table Functions**: Functions that return tabular result sets (e.g., UNNEST, GENERATE_SERIES)
- **Function Overloading**: Multiple implementations with different parameter types
- **Dynamic Extension**: New overloads can be added via ALTER FUNCTION
