/**
 * SchemaViewer Component
 * Displays database schema with tables and columns
 */

import { Database, Table } from 'lucide-react';
import type { QuestionSchema } from '../types/question.types';

interface SchemaViewerProps {
  schema: QuestionSchema;
  compact?: boolean;
}

export function SchemaViewer({ schema, compact = false }: SchemaViewerProps) {
  const getTypeColor = (type: string) => {
    const upper = type.toUpperCase();
    if (upper.includes('INT')) return 'text-blue-400';
    if (upper.includes('VARCHAR') || upper.includes('TEXT') || upper.includes('CHAR'))
      return 'text-green-400';
    if (upper.includes('FLOAT') || upper.includes('DOUBLE') || upper.includes('DECIMAL'))
      return 'text-yellow-400';
    if (upper.includes('DATE') || upper.includes('TIME')) return 'text-purple-400';
    if (upper.includes('BOOL')) return 'text-red-400';
    return 'text-slate-400';
  };

  if (compact) {
    return (
      <div className="bg-slate-900 rounded p-3">
        <div className="flex items-center gap-2 mb-2">
          <Database className="w-4 h-4 text-slate-400" />
          <span className="text-sm font-semibold text-slate-400">Schema</span>
        </div>
        {schema.tables.map((table) => (
          <div key={table.name} className="mb-2 last:mb-0">
            <span className="text-blue-400 font-mono text-sm">{table.name}</span>
            <span className="text-slate-500 font-mono text-xs">
              ({table.columns.map((c) => c.name).join(', ')})
            </span>
          </div>
        ))}
      </div>
    );
  }

  return (
    <div className="bg-slate-800 rounded-lg border border-slate-700 overflow-hidden">
      <div className="px-4 py-3 border-b border-slate-700 flex items-center gap-2">
        <Database className="w-5 h-5 text-slate-400" />
        <h3 className="font-semibold text-white">Database Schema</h3>
      </div>
      <div className="p-4 space-y-4">
        {schema.tables.map((table) => (
          <div key={table.name} className="border border-slate-700 rounded overflow-hidden">
            <div className="bg-slate-900 px-3 py-2 flex items-center gap-2">
              <Table className="w-4 h-4 text-blue-400" />
              <span className="font-mono font-semibold text-blue-400">{table.name}</span>
              <span className="text-xs text-slate-500">({table.columns.length} columns)</span>
            </div>
            <div className="overflow-x-auto">
              <table className="w-full text-sm">
                <thead>
                  <tr className="border-b border-slate-700">
                    <th className="px-3 py-2 text-left text-slate-400 font-medium">Column</th>
                    <th className="px-3 py-2 text-left text-slate-400 font-medium">Type</th>
                  </tr>
                </thead>
                <tbody>
                  {table.columns.map((col, i) => (
                    <tr key={col.name} className="border-b border-slate-700/50 last:border-0">
                      <td className="px-3 py-2 font-mono text-white">{col.name}</td>
                      <td className={`px-3 py-2 font-mono ${getTypeColor(col.type)}`}>
                        {col.type}
                      </td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          </div>
        ))}

        {/* Sample Data Preview */}
        {schema.sampleData && Object.keys(schema.sampleData).length > 0 && (
          <div className="mt-4">
            <h4 className="text-sm font-semibold text-slate-400 mb-2">Sample Data</h4>
            {schema.tables.map((table) => {
              const sampleData = schema.sampleData[table.name];
              if (!sampleData || sampleData.length === 0) return null;

              return (
                <div key={table.name} className="mb-3 last:mb-0">
                  <div className="text-xs text-slate-500 mb-1 font-mono">{table.name}</div>
                  <div className="bg-slate-900 rounded overflow-x-auto">
                    <table className="w-full text-xs">
                      <thead>
                        <tr className="border-b border-slate-700">
                          {table.columns.map((col) => (
                            <th key={col.name} className="px-2 py-1 text-left text-slate-400">
                              {col.name}
                            </th>
                          ))}
                        </tr>
                      </thead>
                      <tbody>
                        {sampleData.slice(0, 3).map((row, i) => (
                          <tr key={i} className="border-b border-slate-700/50 last:border-0">
                            {table.columns.map((col) => (
                              <td key={col.name} className="px-2 py-1 text-slate-300">
                                {row[col.name] === null || row[col.name] === undefined ? (
                                  <span className="text-slate-500">NULL</span>
                                ) : (
                                  String(row[col.name])
                                )}
                              </td>
                            ))}
                          </tr>
                        ))}
                      </tbody>
                    </table>
                    {sampleData.length > 3 && (
                      <div className="px-2 py-1 text-xs text-slate-500 italic">
                        +{sampleData.length - 3} more rows
                      </div>
                    )}
                  </div>
                </div>
              );
            })}
          </div>
        )}
      </div>
    </div>
  );
}
