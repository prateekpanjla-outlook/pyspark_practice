# catalog_entry__copy_function_catalog_entry.cpp.md

## File Location
`duckdb/src/catalog/catalog_entry/copy_function_catalog_entry.cpp`

## Purpose
Implements CopyFunctionCatalogEntry - catalog entry for COPY functions used in data import/export.

## Key Classes and Their Purpose

### CopyFunctionCatalogEntry
- **Purpose**: Catalog entry for COPY functions
- **Key Members**:
  - `function` - The CopyFunction implementation
  - Inherits from StandardEntry

## Key Methods with Brief Descriptions

- Constructor - Creates entry from CreateCopyFunctionInfo
  - Stores the copy function implementation

## Important Concepts
- **COPY Functions**: Handle import/export from various formats (CSV, Parquet, JSON, etc.)
- **Standard Entry Pattern**: Uses StandardEntry base for common catalog entry functionality
