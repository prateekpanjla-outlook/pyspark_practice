# catalog_entry__scalar_function_catalog_entry.cpp.md

## File Location
`duckdb/src/catalog/catalog_entry/scalar_function_catalog_entry.cpp`

## Purpose
Implements ScalarFunctionCatalogEntry - catalog entry for scalar functions like abs, concat, etc.

## Key Classes and Their Purpose

### ScalarFunctionCatalogEntry
- **Purpose**: Catalog entry for scalar functions
- **Key Members**:
  - `functions` - ScalarFunctionSet containing overloaded implementations
  - Inherits from FunctionEntry

## Key Methods with Brief Descriptions

- Constructor - Creates entry from CreateScalarFunctionInfo
  - Sets catalog_name and schema_name for all functions

- `AlterEntry()` - Supports adding new function overloads
  - Merges new overloads with existing set
  - Validates no duplicate overloads
  - Creates new entry with merged function set

## Important Concepts
- **Function Overloading**: Same function name with different parameter types
- **Dynamic Overloading**: New overloads can be added via ALTER FUNCTION
- **Function Merging**: Overload sets are merged when adding new overloads
