# Creating and Uploading Question Notebooks to Jupyter Server

## Problem Overview
When creating PySpark notebooks for the employees database questions, we faced two main challenges:
1. **Backslash escaping issues** in Jupyter notebook JSON format
2. **File upload issues** from Windows host to Linux VM via Vagrant

---

## 1. Backslash Escaping Issues

### The Problem
In Jupyter notebooks, Python line continuation uses a single backslash `\` at the end of a line:
```python
dev = e.join(de, e["id"] == de["employee_id"]) \
        .join(d, de["department_id"] == d["id"]) \
```

When stored in JSON format:
- JSON requires backslashes to be escaped as `\\`
- So a line ending with `\` becomes `"line \\n"` in JSON
- When Python reads this JSON, `\\` becomes `\` (single backslash)

### Approaches Tried (and Why They Failed)

| Approach | Issue |
|----------|-------|
| **Heredoc with shell variable expansion** | Shell interprets backslashes before they reach Python |
| **Inline Python script via `vagrant ssh -c`** | Complex nested quoting caused syntax errors |
| **Using `\\\\` in Python strings** | JSON encoding added extra escapes, resulting in literal `\n` in code |
| **`json.dump()` with string containing `\\\n`** | Incorrect escaping produced `\n` visible in Jupyter instead of line continuation |

### What Worked
**Create the notebook JSON file locally on Windows** (using Write tool or VSCode), then transfer it.

Key insight: When writing the JSON file directly (not via Python string manipulation), the backslashes are preserved correctly:
```json
"source": [
    "dev = e.join(de, e[\"id\"] == de[\"employee_id\"]) \\\n",
    "        .join(d, de[\"department_id\"] == d[\"id\"]) \\\n",
]
```

The `\\\n` in the JSON file represents:
- `\\` → single backslash (escaped for JSON)
- `\n` → newline character

---

## 2. File Upload Issues

### The Problem
`vagrant upload` command on Windows was translating paths incorrectly:
```
Uploading to C:/Program Files/Git/home/vagrant/Question52.ipynb
```
The file didn't actually appear on the VM.

### Approaches Tried

| Approach | Result |
|----------|--------|
| **`vagrant upload file /home/vagrant/file`** | Failed - incorrect path translation on Windows |
| **Vagrant `scp`-like commands** | Not readily available |
| **For loops in bash** | Failed - `cd` with newlines caused "too many arguments" error |
| **Multiline bash with `&&` chains** | Failed - each `vagrant ssh` starts a new shell, losing `cd` context |
| **Bash scripts with heredocs** | Failed - complex nested quoting issues |

### What Worked
**Pipe the file content through SSH, with commands chained by semicolons:**
```bash
# Single file upload
cd /path/to/vagrant-hadoop-hive-spark
cat /path/to/notebooks/Question_53.ipynb | vagrant ssh -c "cat > /home/vagrant/Question_53.ipynb"

# Batch uploads (chained with semicolons, not newlines)
cd /path/to/vagrant-hadoop-hive-spark; \
cat file1.ipynb | vagrant ssh -c "cat > /home/vagrant/file1.ipynb"; \
cat file2.ipynb | vagrant ssh -c "cat > /home/vagrant/file2.ipynb"
```

**Key lessons:**
1. `cd` and `vagrant ssh` must be on the same line separated by `;`
2. Each `vagrant ssh` starts a fresh shell - can't use multiline scripts inside
3. Upload files in batches of 3-6 to balance speed and reliability
4. Use absolute paths for source files (Windows: `/c/Users/...`)

This bypasses Vagrant's path translation and writes directly via stdin.

---

## 3. Notebook Code Requirements for Server

To ensure notebooks run correctly on the Spark cluster:

### Database Connection Details
```python
url = "jdbc:postgresql://localhost:5432/employees"
properties = {
    "user": "vagrant",
    "password": "vagrant",
    "driver": "org.postgresql.Driver"
}
```

### Table Names (with schema prefix)
- `employees.employee`
- `employees.department`
- `employees.department_employee`
- `employees.salary`
- `employees.title`
- `employees.department_manager`

### Spark Configuration
```python
from pyspark.sql import SparkSession
from pyspark.sql import Window
from pyspark.sql.functions import *

spark = SparkSession.builder \
    .master("local[*]") \
    .appName("PostgresConnector") \
    .config("spark.jars","/usr/lib/spark/jars/postgresql-42.7.4.jar")\
    .getOrCreate()
```

### Avoiding Ambiguous Column References

**Problem**: When joining multiple tables with same column names (like `id`), `.select("id")` fails with:
```
AnalysisException: Reference 'id' is ambiguous, could be: id, id.
```

**Solution**: Use DataFrame aliases and qualified column references:
```python
# Create aliases
e = employee_df.alias("e")
d = department_df.alias("d")
de = department_employee_df.alias("de")

# Use aliases in joins
dev = e.join(de, e["id"] == de["employee_id"]) \
        .join(d, de["department_id"] == d["id"]) \
        .filter(col("d.dept_name") == "Development") \
        .select(col("e.id")).distinct()  # Qualified reference!
```

---

## Summary Workflow

### Single Notebook
1. **Create notebook locally** (Windows) using Write tool or text editor
2. **Transfer via pipe**: `cat file.ipynb | vagrant ssh -c "cat > /home/vagrant/file.ipynb"`
3. **Verify in Jupyter**: Refresh browser to see the new notebook
4. **Run and test**: Execute cells to verify SQL query results

### Bulk Upload (Multiple Notebooks)

**Step 1: Generate all notebooks locally**
```python
# Use generate_notebooks.py script
python generate_notebooks.py
# Creates 96 notebooks (48 questions × 2 versions) in ./notebooks/
```

**Step 2: Upload in batches**
```bash
cd /c/Users/prateek/Desktop/spark/vagrant-hadoop-hive-spark

# Batch upload 6 files at a time (chained with semicolons)
cat /c/Users/prateek/Desktop/spark/pyspark_practice/notebooks/Question_53.ipynb | \
  vagrant ssh -c "cat > /home/vagrant/Question_53.ipynb"; \
cat /c/Users/prateek/Desktop/spark/pyspark_practice/notebooks/Question_53_solution.ipynb | \
  vagrant ssh -c "cat > /home/vagrant/Question_53_solution.ipynb"; \
# ... repeat for 4 more files
```

**Step 3: Verify upload count**
```bash
vagrant ssh -c "ls /home/vagrant/Question_*.ipynb | wc -l"
# Should show 96 for questions 53-100
```

**Tips for bulk upload:**
- Upload 6-12 files per batch command
- Use semicolons `;` to chain commands on one line
- Don't use newlines - breaks the `cd` context
- Verify count periodically to catch failures
