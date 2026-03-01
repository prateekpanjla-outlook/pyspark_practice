# catalog_entry__sequence_catalog_entry.cpp.md

## File Location
`duckdb/src/catalog/catalog_entry/sequence_catalog_entry.cpp`

## Purpose
Implements SequenceCatalogEntry for managing database sequences (auto-incrementing values).

## Key Classes and Their Purpose

### SequenceCatalogEntry
- **Purpose**: Catalog entry for SEQUENCE objects
- **Key Members**:
  - `data` - SequenceData containing counter, increment, min/max values, cycle option
  - `lock` - Mutex protecting sequence state
  - Inherits from StandardEntry

### SequenceData
- **Purpose**: Stores sequence state and configuration
- **Key Members**:
  - `usage_count` - Number of times NextValue was called
  - `counter` - Current/next value to return
  - `last_value` - Most recent value returned
  - `increment` - Step value (can be negative)
  - `min_value` / `max_value` - Value bounds
  - `cycle` - Whether to wrap around on overflow

## Key Methods with Brief Descriptions

- `CurrentValue()` - Returns last value from NextValue()
  - Throws if sequence not yet used in session

- `NextValue()` - Advances sequence and returns current value
  - Thread-safe with mutex lock
  - Handles overflow detection
  - Supports CYCLE option for wrapping
  - Tracks usage in transaction for persistence

- `ReplayValue()` - Updates sequence state during transaction replay
  - Only updates if new usage_count is higher

- `GetInfo()` - Returns CreateSequenceInfo for serialization
- `ToSQL()` - Generates CREATE SEQUENCE SQL statement
- `GetData()` - Thread-safe copy of sequence data

## Important Concepts
- **Sequence Generation**: Thread-safe auto-incrementing values
- **Cycle Option**: Sequences can wrap around when reaching min/max
- **Transaction Tracking**: Sequence usage persisted to storage
- **Overflow Detection**: Checks for min/max violations with clear error messages
