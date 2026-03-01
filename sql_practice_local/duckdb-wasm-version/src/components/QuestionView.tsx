/**
 * QuestionView Component
 * Displays the current question with schema, hints, and difficulty badge
 */

import { useMemo } from 'react';
import { Lightbulb, AlertCircle, Sparkles } from 'lucide-react';
import type { Question } from '../types/question.types';
import { SchemaViewer } from './SchemaViewer';
import { hintService } from '../services/hint.service';

interface QuestionViewProps {
  question: Question;
}

export function QuestionView({ question }: QuestionViewProps) {
  // Generate dynamic hints based on schema and question
  const dynamicHints = useMemo(() => {
    const context = {
      schema: question.schema,
      question: question.question,
      expectedColumns: question.expectedOutput.columns
    };
    return hintService.generateHints(context);
  }, [question.schema, question.question, question.expectedOutput.columns]);

  // Combine static hints with dynamic hints
  const allHints = useMemo(() => {
    const staticHints = question.hints || [];
    const enriched = hintService.enrichHints(staticHints, {
      schema: question.schema,
      question: question.question,
      expectedColumns: question.expectedOutput.columns
    });
    return [...enriched, ...dynamicHints.map(h => h.text)];
  }, [question.hints, question.schema, question.question, question.expectedOutput.columns, dynamicHints]);

  const getDifficultyColor = (difficulty: string) => {
    switch (difficulty) {
      case 'easy':
        return 'bg-green-900 text-green-300 border-green-700';
      case 'medium':
        return 'bg-yellow-900 text-yellow-300 border-yellow-700';
      case 'hard':
        return 'bg-red-900 text-red-300 border-red-700';
      default:
        return 'bg-slate-700 text-slate-300 border-slate-600';
    }
  };

  return (
    <div className="space-y-6">
      {/* Question Header */}
      <div className="bg-slate-800 rounded-lg border border-slate-700 p-6">
        <div className="flex items-start justify-between gap-4 mb-4">
          <div className="flex-1">
            <div className="flex items-center gap-3 mb-2">
              <span className="text-xs text-slate-500 font-mono">#{question.order}</span>
              <h2 className="text-2xl font-bold text-white">{question.title}</h2>
            </div>
            {question.category && (
              <span className="inline-block px-2 py-0.5 text-xs font-medium rounded bg-slate-700 text-slate-300 mb-2">
                {question.category}
              </span>
            )}
          </div>
          <span
            className={`px-3 py-1 text-sm font-medium rounded border shrink-0 capitalize ${getDifficultyColor(
              question.difficulty
            )}`}
          >
            {question.difficulty}
          </span>
        </div>

        <p className="text-slate-300 text-lg">{question.question}</p>

        {/* Tags */}
        {question.tags && question.tags.length > 0 && (
          <div className="flex flex-wrap gap-2 mt-3">
            {question.tags.map((tag) => (
              <span
                key={tag}
                className="px-2 py-1 text-xs rounded bg-slate-700 text-slate-400"
              >
                #{tag}
              </span>
            ))}
          </div>
        )}
      </div>

      {/* Schema Viewer (Compact) */}
      <SchemaViewer schema={question.schema} compact />

      {/* Hints */}
      {allHints.length > 0 && (
        <div className="bg-slate-800 rounded-lg border border-slate-700 p-6">
          <div className="flex items-center gap-2 mb-3">
            <Lightbulb className="w-5 h-5 text-yellow-400" />
            <h3 className="font-semibold text-white">Hints</h3>
            <span className="text-xs text-slate-500">(click to reveal)</span>
            {dynamicHints.length > 0 && (
              <span className="ml-auto flex items-center gap-1 text-xs text-blue-400">
                <Sparkles className="w-3 h-3" />
                <span>{dynamicHints.length} AI-generated</span>
              </span>
            )}
          </div>
          <ul className="space-y-2">
            {allHints.map((hint, i) => {
              const isDynamic = i >= (question.hints?.length || 0);
              return (
                <li key={i} className="group">
                  <details className="text-slate-300">
                    <summary className="cursor-pointer list-none flex items-center gap-2 hover:text-white transition-colors">
                      <span className={`flex items-center justify-center w-6 h-6 rounded text-xs font-mono group-hover:bg-slate-600 ${isDynamic ? 'bg-blue-900 text-blue-300' : 'bg-slate-700'}`}>
                        {i + 1}
                      </span>
                      <span className="text-sm">
                        Show hint {i + 1}
                        {isDynamic && <span className="ml-2 text-blue-400">(Smart)</span>}
                      </span>
                    </summary>
                    <p className={`ml-8 mt-2 text-sm pl-3 border-l-2 ${isDynamic ? 'border-blue-600 text-blue-200' : 'border-slate-600 text-slate-400'}`}>
                      {hint}
                    </p>
                  </details>
                </li>
              );
            })}
          </ul>
        </div>
      )}

      {/* Expected Output Info */}
      <div className="bg-slate-800/50 rounded-lg border border-slate-700/50 p-4">
        <div className="flex items-center gap-2 mb-2">
          <AlertCircle className="w-4 h-4 text-blue-400" />
          <span className="text-sm font-medium text-slate-400">Expected Output</span>
        </div>
        <p className="text-xs text-slate-500">
          Your query should return {question.expectedOutput.rows.length} row(s) with{' '}
          {question.expectedOutput.columns.length} column(s):
        </p>
        <code className="text-xs text-slate-400 font-mono mt-1 block">
          {question.expectedOutput.columns.join(', ')}
        </code>
      </div>
    </div>
  );
}
