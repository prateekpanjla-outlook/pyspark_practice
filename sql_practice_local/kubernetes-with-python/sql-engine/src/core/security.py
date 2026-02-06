"""
Security utilities for SQL execution
"""

import re
from typing import Dict, List


def is_safe_query(sql: str) -> Dict:
    """
    Check if SQL query is safe to execute

    Returns:
        {
            "is_safe": bool,
            "errors": List[str],
            "warnings": List[str]
        }
    """

    errors = []
    warnings = []
    upper_sql = sql.upper()

    # Blocked keywords (data modification)
    blocked_keywords = [
        "DROP", "DELETE", "UPDATE", "INSERT",
        "ALTER", "TRUNCATE", "CREATE", "GRANT",
        "REVOKE", "COPY", "EXECUTE"
    ]

    for keyword in blocked_keywords:
        if keyword in upper_sql:
            errors.append(f"Unsafe keyword detected: {keyword}")

    # Check for multiple statements (semicolon separation)
    if sql.count(";") > 1:
        warnings.append("Multiple statements detected. Only the first will be executed.")

    # Check for comments that might hide malicious code
    if "--" in sql or "/*" in sql:
        warnings.append("SQL comments detected. They will be stripped before execution.")

    is_safe = len(errors) == 0

    return {
        "is_safe": is_safe,
        "errors": errors,
        "warnings": warnings
    }


def sanitize_sql(sql: str) -> str:
    """
    Sanitize SQL query by removing comments and extra whitespace

    Args:
        sql: Raw SQL query

    Returns:
        Sanitized SQL query
    """

    # Remove single-line comments
    sql = re.sub(r"--.*?\n", "\n", sql)

    # Remove multi-line comments
    sql = re.sub(r"/\*.*?\*/", "", sql, flags=re.DOTALL)

    # Remove extra whitespace
    sql = " ".join(sql.split())

    return sql.strip()


def extract_query(sql: str) -> str:
    """
    Extract the first SQL query from a multi-statement string

    Args:
        sql: SQL string (potentially with multiple statements)

    Returns:
        First SQL query
    """

    # Split by semicolon
    queries = sql.split(";")

    # Return first non-empty query
    for query in queries:
        sanitized = sanitize_sql(query)
        if sanitized:
            return sanitized

    return ""
