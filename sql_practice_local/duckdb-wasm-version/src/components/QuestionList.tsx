/**
 * QuestionList Component
 * Displays a list of available SQL practice questions
 */

import { BookOpen, Check } from 'lucide-react';
import type { Question } from '../types/question.types';

interface QuestionListProps {
  questions: Question[];
  currentSlug: string | null;
  onSelectQuestion: (slug: string) => void;
  solvedQuestions?: Set<string>;
}

export function QuestionList({
  questions,
  currentSlug,
  onSelectQuestion,
  solvedQuestions = new Set(),
}: QuestionListProps) {
  const getDifficultyColor = (difficulty: string) => {
    switch (difficulty) {
      case 'easy':
        return 'bg-green-900 text-green-300';
      case 'medium':
        return 'bg-yellow-900 text-yellow-300';
      case 'hard':
        return 'bg-red-900 text-red-300';
      default:
        return 'bg-slate-700 text-slate-300';
    }
  };

  return (
    <div className="bg-slate-800 rounded-lg border border-slate-700 overflow-hidden">
      <div className="px-4 py-3 border-b border-slate-700 flex items-center gap-2">
        <BookOpen className="w-5 h-5 text-slate-400" />
        <h2 className="font-semibold text-white">Questions</h2>
        <span className="ml-auto text-xs text-slate-500">
          {solvedQuestions.size}/{questions.length} solved
        </span>
      </div>
      <div className="max-h-[600px] overflow-y-auto">
        {questions.map((q) => {
          const isSolved = solvedQuestions.has(q.slug);
          return (
            <button
              key={q.slug}
              onClick={() => onSelectQuestion(q.slug)}
              className={`w-full px-4 py-3 text-left hover:bg-slate-700 border-b border-slate-700 last:border-0 transition-colors ${
                currentSlug === q.slug ? 'bg-slate-700' : ''
              }`}
            >
              <div className="flex items-start justify-between gap-2">
                <div className="flex-1 min-w-0">
                  <div className="flex items-center gap-2">
                    {isSolved && (
                      <Check className="w-4 h-4 text-green-400 shrink-0" />
                    )}
                    <span className="text-xs text-slate-500">#{q.order}</span>
                    <h3
                      className={`font-medium truncate ${isSolved ? 'text-green-300' : 'text-white'}`}
                    >
                      {q.title}
                    </h3>
                  </div>
                  <p className="text-sm text-slate-400 truncate mt-0.5">{q.description}</p>
                </div>
                <span
                  className={`px-2 py-1 text-xs font-medium rounded shrink-0 capitalize ${getDifficultyColor(
                    q.difficulty
                  )}`}
                >
                  {q.difficulty}
                </span>
              </div>
            </button>
          );
        })}
      </div>
    </div>
  );
}
