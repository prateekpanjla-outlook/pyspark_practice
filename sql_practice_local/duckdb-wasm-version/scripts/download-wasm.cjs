/**
 * Copy DuckDB-Wasm files from node_modules to public folder
 * This is more reliable than downloading from CDN
 */

const fs = require('fs');
const path = require('path');

const SOURCE_DIR = path.join(__dirname, '..', 'node_modules', '@duckdb', 'duckdb-wasm', 'dist');
const OUTPUT_DIR = path.join(__dirname, '..', 'public', 'duckdb-wasm');

// Files to copy: source name in node_modules -> destination name
const FILES = [
  { source: 'duckdb-mvp.wasm', dest: 'duckdb-mvp.wasm' },
  { source: 'duckdb-browser-mvp.worker.js', dest: 'duckdb-mvp.worker.js' },
  { source: 'duckdb-eh.wasm', dest: 'duckdb-eh.wasm' },
  { source: 'duckdb-browser-eh.worker.js', dest: 'duckdb-eh.worker.js' },
];

// Create output directory if it doesn't exist
if (!fs.existsSync(OUTPUT_DIR)) {
  fs.mkdirSync(OUTPUT_DIR, { recursive: true });
}

function copyFileSync(source, dest) {
  const sourcePath = path.join(SOURCE_DIR, source);
  const destPath = path.join(OUTPUT_DIR, dest);

  if (!fs.existsSync(sourcePath)) {
    console.error(`❌ Source file not found: ${sourcePath}`);
    return false;
  }

  fs.copyFileSync(sourcePath, destPath);
  const stats = fs.statSync(destPath);
  console.log(`✓ Copied ${dest} (${(stats.size / 1024 / 1024).toFixed(1)} MB)`);
  return true;
}

async function main() {
  console.log('Copying DuckDB-Wasm files from node_modules...\n');

  // Verify source directory exists
  if (!fs.existsSync(SOURCE_DIR)) {
    console.error(`❌ Source directory not found: ${SOURCE_DIR}`);
    console.error('💡 Run "npm install" first');
    process.exit(1);
  }

  // Copy files
  let success = true;
  for (const file of FILES) {
    if (!copyFileSync(file.source, file.dest)) {
      success = false;
    }
  }

  if (success) {
    console.log('\n✅ All files copied successfully!');
    console.log(`📁 Files saved to: ${OUTPUT_DIR}`);
  } else {
    console.error('\n❌ Some files failed to copy');
    process.exit(1);
  }
}

main();
