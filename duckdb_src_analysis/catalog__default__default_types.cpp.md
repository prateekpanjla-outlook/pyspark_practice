# catalog__default__default_types.cpp.md

## File Location
`duckdb/src/catalog/default/default_types.cpp`

## Purpose
Implements DefaultTypeGenerator - creates default type entries with custom bind functions.

## Key Classes and Their Purpose

### DefaultType (struct)
- **Purpose**: Defines default type
- **Key Members**:
  - `name` - Type name
  - `type` - LogicalTypeId
  - `bind_function` - Optional bind function for type modifiers

### DefaultTypeGenerator
- **Purpose**: Generates default type catalog entries
- Inherits from DefaultGenerator

## Key Methods with Brief Descriptions

**Type Bind Functions:**
- `BindDecimalType()` - Binds DECIMAL(width, scale)
  - Default: DECIMAL(18, 3)
  - Validates width (1-38) and scale (≤ width)

- `BindTimestampType()` - Binds TIMESTAMP(precision)
  - Precision 0 → TIMESTAMP_S
  - Precision 1-3 → TIMESTAMP_MS
  - Precision 4-6 → TIMESTAMP
  - Precision 7-9 → TIMESTAMP_NS

- `BindVarcharType()` - Binds VARCHAR with collation support
  - Supports COLLATE modifier

- `BindEnumType()` - Binds ENUM(value1, value2, ...)
  - Requires at least one value
  - All values must be non-null VARCHAR

- `BindListType()` - Binds LIST(element_type)
- `BindArrayType()` - Binds ARRAY(element_type, size)
  - Size must be 1 to MAX_ARRAY_SIZE

- `BindStructType()` - Binds STRUCT(field1 type1, ...)
  - All fields must have names or all must be anonymous
  - Validates no duplicate field names

- `BindMapType()` - Binds MAP(key_type, value_type)
- `BindUnionType()` - Binds UNION(member1 type1, ...)
  - Maximum 256 members

- `BindVariantType()` - Binds VARIANT type (semi-structured data)
- `BindGeometryType()` - Binds GEOMETRY with coordinate system

**Generator Methods:**
- `GetDefaultType()` - Gets LogicalTypeId for type name
- `TryDefaultBind()` - Attempts to bind type with modifiers
- `CreateDefaultEntry()` - Creates TypeCatalogEntry for default type
- `GetDefaultEntries()` - Lists all default type names

## Important Concepts
- **Type Modifiers**: Parameters like DECIMAL(18,3) or VARCHAR(100) COLLATE
- **Bind Functions**: Called during type binding to validate and process modifiers
- **Default Types**: Built-in types available without explicit CREATE TYPE
- **Type Aliases**: Multiple names for same type (e.g., INT, INTEGER, INT4)

## BUILTIN_TYPES (81 types):
**Numeric:** decimal, numeric, hugeint, bigint, integer, smallint, tinyint, float, double, ubigint, uinteger, usmallint, utinyint, uhugeint
**Date/Time:** time, time_ns, date, timestamp, datetime, timestamptz, timetz, interval
**String:** varchar, char, text, blob, binary, varbinary, bpchar, nvarchar
**Composite:** struct, row, list, array, map, union
**Special:** bit, bitstring, variant, bignum, varint, boolean, bool, uuid, guid, enum, null, type
