# catalog_entry__pragma_function_catalog_entry.cpp.md

## File Location
`duckdb/src/catalog/catalog_entry/pragma_function_catalog_entry.cpp`

## Purpose
Implements PragmaFunctionCatalogEntry - catalog entry for PRAGMA functions that configure database behavior.

## Key Classes and Their Purpose

### PragmaFunctionCatalogEntry
- **Purpose**: Catalog entry for PRAGMA functions
- **Key Members**:
  - `functions` - PragmaFunctionSet with PRAGMA implementations
  - Inherits from FunctionEntry

## Key Methods with Brief Descriptions

- Constructor - Creates entry from CreatePragmaFunctionInfo
  - Stores the pragma function set

## Important Concepts
- **PRAGMA Functions**: Configuration functions like `PRAGMA table_info`
- **Named Constant**: Entry name defined as "PragmaFunction"
- **Function Set**: Can contain multiple PRAGMA implementations
