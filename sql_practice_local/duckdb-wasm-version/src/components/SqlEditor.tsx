/**
 * SQL Editor Component using Monaco Editor
 * Features:
 * - SQL syntax highlighting
 * - Autocomplete for SQL keywords
 * - Error detection (basic)
 * - Keyboard shortcuts (Ctrl+Enter to run)
 * - Two buttons: Run (experiment) and Submit (check answer)
 */

import { useRef } from 'react';
import Editor from '@monaco-editor/react';
import { Play, RotateCcw, Check } from 'lucide-react';

interface SqlEditorProps {
  value: string;
  onChange: (value: string) => void;
  onRun: () => void;
  onSubmit?: () => void;
  onReset?: () => void;
  readOnly?: boolean;
  height?: string;
  schema?: string[]; // Table/column names for autocomplete
}

// SQL keywords for Monaco
const SQL_KEYWORDS = [
  'SELECT', 'FROM', 'WHERE', 'JOIN', 'INNER', 'LEFT', 'RIGHT', 'OUTER', 'FULL',
  'GROUP', 'BY', 'ORDER', 'HAVING', 'LIMIT', 'OFFSET',
  'INSERT', 'INTO', 'VALUES', 'UPDATE', 'SET', 'DELETE',
  'CREATE', 'TABLE', 'DROP', 'ALTER', 'INDEX',
  'AND', 'OR', 'NOT', 'IN', 'EXISTS', 'BETWEEN', 'LIKE', 'IS', 'NULL',
  'AS', 'DISTINCT', 'ALL', 'UNION', 'INTERSECT', 'EXCEPT',
  'CASE', 'WHEN', 'THEN', 'ELSE', 'END',
  'SUM', 'COUNT', 'AVG', 'MIN', 'MAX',
  'ASC', 'DESC', 'WITH',
];

const SQL_FUNCTIONS = [
  'SUM', 'COUNT', 'AVG', 'MIN', 'MAX',
  'UPPER', 'LOWER', 'TRIM', 'SUBSTRING', 'CONCAT',
  'COALESCE', 'NULLIF', 'CAST',
  'ROUND', 'CEIL', 'FLOOR', 'ABS',
  'DATE', 'YEAR', 'MONTH', 'DAY', 'HOUR', 'MINUTE', 'SECOND',
  'ROW_NUMBER', 'RANK', 'DENSE_RANK', 'LAG', 'LEAD',
];

export function SqlEditor({
  value,
  onChange,
  onRun,
  onSubmit,
  onReset,
  readOnly = false,
  height = '300px',
  schema = [],
}: SqlEditorProps) {
  const editorRef = useRef<any>(null);

  const handleEditorDidMount = (editor: any, monaco: any) => {
    editorRef.current = editor;

    // Register SQL language
    monaco.languages.register({ id: 'sql' });

    // Define SQL syntax highlighting
    monaco.languages.setMonarchTokensProvider('sql', {
      keywords: SQL_KEYWORDS,
      functions: SQL_FUNCTIONS,
      operators: [
        '=', '>', '<', '!', '~', '?', ':', '==', '<=', '>=', '!=',
        '&&', '||', '++', '--', '+', '-', '*', '/', '&', '|', '^', '%',
      ],
      tokenizer: {
        root: [
          [/"/, 'string', '@string.double'],
          [/'/, 'string', '@string.single'],
          [/[0-9]+(\.[0-9]+)?/, 'number'],
          [/[a-zA-Z_]\w*/, {
            cases: {
              '@keywords': 'keyword',
              '@functions': 'type.identifier',
              '@default': 'identifier',
            },
          }],
          [/[[\]{}().,;@]/, 'delimiter'],
        ],
        'string.double': [
          [/[^"]+/, 'string'],
          [/"/, { token: 'string', next: '@pop' }],
        ],
        'string.single': [
          [/[^']+/, 'string'],
          [/'/, { token: 'string', next: '@pop' }],
        ],
      },
    });

    // Set up autocomplete
    monaco.languages.registerCompletionItemProvider('sql', {
      provideCompletionItems: (model: any, position: any) => {
        const suggestions = [
          ...SQL_KEYWORDS.map((keyword) => ({
            label: keyword,
            kind: monaco.languages.CompletionItemKind.Keyword,
            insertText: keyword,
            detail: 'SQL keyword',
          })),
          ...SQL_FUNCTIONS.map((func) => ({
            label: func,
            kind: monaco.languages.CompletionItemKind.Function,
            insertText: func + '()',
            detail: 'SQL function',
          })),
          // Add schema tables/columns
          ...schema.map((name) => ({
            label: name,
            kind: monaco.languages.CompletionItemKind.Variable,
            insertText: name,
            detail: 'Table or column',
          })),
        ];

        return { suggestions };
      },
    });

    // Keyboard shortcut: Ctrl+Enter to run query
    editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.Enter, () => {
      onRun();
    });

    // Set SQL as the language
    monaco.editor.setModelLanguage(editor.getModel(), 'sql');
  };

  // Handle keyboard shortcuts
  const handleKeyDown = (e: React.KeyboardEvent) => {
    // Ctrl+Enter to run
    if (e.ctrlKey && e.key === 'Enter') {
      e.preventDefault();
      onRun();
    }
  };

  return (
    <div className="bg-slate-800 rounded-lg border border-slate-700 overflow-hidden" onKeyDown={handleKeyDown}>
      {/* Toolbar */}
      <div className="px-4 py-2 border-b border-slate-700 flex items-center justify-between bg-slate-750">
        <div className="flex items-center gap-2">
          <span className="text-sm font-medium text-slate-300">SQL Editor</span>
          <span className="text-xs text-slate-500 hidden sm:inline">
            Ctrl+Enter to run
          </span>
        </div>
        <div className="flex items-center gap-2">
          {onReset && (
            <button
              onClick={onReset}
              className="p-2 text-slate-400 hover:text-slate-200 hover:bg-slate-700 rounded transition-colors"
              title="Reset code"
            >
              <RotateCcw className="w-4 h-4" />
            </button>
          )}
          {/* Run button - for experimentation */}
          <button
            onClick={onRun}
            className="flex items-center gap-2 px-3 py-1.5 bg-slate-600 hover:bg-slate-500 text-white text-sm font-medium rounded transition-colors"
            title="Run query to see results (not checked against expected output)"
          >
            <Play className="w-4 h-4" />
            <span className="hidden sm:inline">Run</span>
          </button>
          {/* Submit button - to check answer */}
          {onSubmit && (
            <button
              onClick={onSubmit}
              className="flex items-center gap-2 px-3 py-1.5 bg-blue-600 hover:bg-blue-700 text-white text-sm font-medium rounded transition-colors"
              title="Submit to check if your answer is correct"
            >
              <Check className="w-4 h-4" />
              <span className="hidden sm:inline">Submit</span>
            </button>
          )}
        </div>
      </div>

      {/* Monaco Editor */}
      <Editor
        height={height}
        value={value}
        onChange={(val) => onChange(val || '')}
        onMount={handleEditorDidMount}
        theme="vs-dark"
        language="sql"
        options={{
          readOnly,
          minimap: { enabled: false },
          fontSize: 14,
          lineNumbers: 'on',
          scrollBeyondLastLine: false,
          automaticLayout: true,
          tabSize: 2,
          wordWrap: 'on',
          formatOnPaste: true,
          formatOnType: true,
          suggestOnTriggerCharacters: true,
          quickSuggestions: {
            other: true,
            comments: false,
            strings: false,
          },
          parameterHints: { enabled: true },
          folding: true,
          bracketPairColorization: { enabled: true },
        }}
      />
    </div>
  );
}
