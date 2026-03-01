# catalog_entry__aggregate_function_catalog_entry.cpp.md

## File Location
`duckdb/src/catalog/catalog_entry/aggregate_function_catalog_entry.cpp`

## Purpose
Implements AggregateFunctionCatalogEntry - catalog entry for aggregate functions like SUM, COUNT, AVG.

## Key Classes and Their Purpose

### AggregateFunctionCatalogEntry
- **Purpose**: Catalog entry for aggregate functions
- **Key Members**:
  - `functions` - AggregateFunctionSet containing overloaded implementations
  - Inherits from FunctionEntry

## Key Methods with Brief Descriptions

- Constructor - Creates entry from CreateAggregateFunctionInfo
  - Sets catalog_name and schema_name for all functions in the set

## Important Concepts
- **Function Overloading**: Multiple implementations with different parameter types
- **Catalog Location**: Functions know their catalog and schema location
- **Aggregate Function Set**: Collection of related aggregate function implementations
