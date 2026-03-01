/**
 * ResultsTable Component
 * Displays SQL query results with correctness indicator
 */

import { Download, Check, X, AlertCircle } from 'lucide-react';
import type { QueryResult } from '../types/question.types';

interface ResultsTableProps {
  result: QueryResult;
  isCorrect: boolean | null;
}

export function ResultsTable({ result, isCorrect }: ResultsTableProps) {
  const getStatusBorder = () => {
    if (isCorrect === true) return 'border-green-500';
    if (isCorrect === false) return 'border-orange-500';
    if (!result.success) return 'border-red-600';
    return 'border-slate-700';
  };

  const getStatusHeader = () => {
    if (isCorrect === true) {
      return {
        bg: 'bg-green-900/30',
        icon: <Check className="w-5 h-5 text-green-400" />,
        text: 'text-green-400',
        label: 'Correct! 🎉',
        desc: 'Your query matches the expected output',
      };
    }
    if (isCorrect === false) {
      return {
        bg: 'bg-orange-900/30',
        icon: <X className="w-5 h-5 text-orange-400" />,
        text: 'text-orange-400',
        label: 'Incorrect',
        desc: 'Your output does not match the expected result',
      };
    }
    if (!result.success) {
      return {
        bg: 'bg-red-900/30',
        icon: <AlertCircle className="w-5 h-5 text-red-400" />,
        text: 'text-red-400',
        label: 'Error',
        desc: result.errorMessage || 'Query execution failed',
      };
    }
    return {
      bg: 'bg-slate-700/50',
      icon: null,
      text: 'text-slate-300',
      label: 'Results',
      desc: 'Run your query to see results (use Submit to check answer)',
    };
  };

  const handleExportCSV = () => {
    if (!result.success) return;

    const headers = result.columns.join(',');
    const rows = result.rows.map((row) =>
      result.columns.map((col) => {
        const val = row[col];
        // Escape quotes and wrap strings in quotes
        if (val === null || val === undefined) return '';
        const strVal = String(val);
        if (strVal.includes(',') || strVal.includes('"') || strVal.includes('\n')) {
          return `"${strVal.replace(/"/g, '""')}"`;
        }
        return strVal;
      }).join(',')
    );

    const csv = [headers, ...rows].join('\n');
    const blob = new Blob([csv], { type: 'text/csv' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `query-results-${Date.now()}.csv`;
    a.click();
    URL.revokeObjectURL(url);
  };

  const status = getStatusHeader();

  return (
    <div className={`bg-slate-800 rounded-lg border-2 ${getStatusBorder()} overflow-hidden`}>
      {/* Status Header */}
      <div className={`px-4 py-3 border-b border-slate-700 ${status.bg}`}>
        <div className="flex items-center justify-between mb-1">
          <div className="flex items-center gap-2">
            {status.icon}
            <span className={`font-bold ${status.text} text-base`}>{status.label}</span>
          </div>
          <div className="flex items-center gap-3">
            <span className="text-sm text-slate-400">
              {result.executionTimeMs.toFixed(2)} ms • {result.rowCount} {result.rowCount === 1 ? 'row' : 'rows'}
            </span>
            {result.success && (
              <button
                onClick={handleExportCSV}
                className="p-1.5 text-slate-400 hover:text-slate-200 hover:bg-slate-700 rounded transition-colors"
                title="Export as CSV"
              >
                <Download className="w-4 h-4" />
              </button>
            )}
          </div>
        </div>
        {status.desc && (
          <p className="text-sm text-slate-400">{status.desc}</p>
        )}
      </div>

      {/* Results Table */}
      {result.success ? (
        <div className="overflow-x-auto max-h-96 overflow-y-auto">
          <table className="w-full">
            <thead className="bg-slate-900 sticky top-0">
              <tr>
                {result.columns.map((col) => (
                  <th
                    key={col}
                    className="px-4 py-2 text-left text-sm font-semibold text-slate-300 whitespace-nowrap border-b border-slate-700"
                  >
                    {col}
                  </th>
                ))}
              </tr>
            </thead>
            <tbody>
              {result.rows.map((row, i) => (
                <tr key={i} className="border-t border-slate-700 hover:bg-slate-700/50">
                  {result.columns.map((col) => (
                    <td key={col} className="px-4 py-2 text-sm text-slate-300 whitespace-nowrap">
                      {row[col] === null || row[col] === undefined ? (
                        <span className="text-slate-500 italic">NULL</span>
                      ) : (
                        String(row[col])
                      )}
                    </td>
                  ))}
                </tr>
              ))}
            </tbody>
          </table>
          {result.rowCount === 0 && (
            <div className="p-8 text-center text-slate-500">
              No results returned
            </div>
          )}
        </div>
      ) : (
        <div className="p-4">
          <p className="text-red-400 font-mono text-sm break-words">{result.errorMessage}</p>
        </div>
      )}
    </div>
  );
}
