import { useState, useEffect, useMemo } from 'react';
import { Database, BookOpen, Terminal, Trophy } from 'lucide-react';
import { DuckDBService } from './services/duckdb.service';
import { QuestionService } from './services/question.service';
import { storageService } from './services/storage.service';
import { SqlEditor, QuestionList, QuestionView, ResultsTable } from './components';
import type { Question, QueryResult } from './types/question.types';

function App() {
  const [duckdb] = useState(() => new DuckDBService());
  const [questionService] = useState(() => new QuestionService());
  const [initialized, setInitialized] = useState(false);
  const [questions, setQuestions] = useState<Question[]>([]);
  const [currentQuestion, setCurrentQuestion] = useState<Question | null>(null);
  const [userCode, setUserCode] = useState('');
  const [result, setResult] = useState<QueryResult | null>(null);
  const [isCorrect, setIsCorrect] = useState<boolean | null>(null);
  const [resultVersion, setResultVersion] = useState(0); // Force re-render of results
  const [solvedQuestions, setSolvedQuestions] = useState<Set<string>>(new Set());

  useEffect(() => {
    async function init() {
      try {
        // Initialize DuckDB-Wasm
        await duckdb.initialize();

        // Load questions
        const qs = await questionService.loadQuestions();
        setQuestions(qs);

        // Load solved questions from storage
        const progress = storageService.getProgress();
        const solved = new Set<string>();
        for (const [slug, data] of Object.entries(progress.questions)) {
          if (data.solved) {
            solved.add(slug);
          }
        }
        setSolvedQuestions(solved);

        setInitialized(true);
      } catch (error) {
        console.error('Failed to initialize:', error);
      }
    }

    init();
  }, [duckdb, questionService]);

  // Run query - just show results, don't check correctness
  async function handleRun() {
    if (!currentQuestion || !userCode.trim()) return;

    try {
      // Initialize schema for current question
      await duckdb.initializeSchema(currentQuestion.schema);

      // Execute query
      const queryResult = await duckdb.executeQuery(userCode);
      setResult(queryResult);
      setResultVersion(v => v + 1); // Force re-render

      // Don't show correctness status on Run - only on Submit
      setIsCorrect(null);
    } catch (error) {
      console.error('Failed to execute query:', error);
      setResult({
        success: false,
        columns: [],
        rows: [],
        executionTimeMs: 0,
        rowCount: 0,
        errorMessage: error instanceof Error ? error.message : String(error),
      });
      setResultVersion(v => v + 1); // Force re-render
    }
  }

  // Submit answer - execute and check correctness
  async function handleSubmit() {
    if (!currentQuestion || !userCode.trim()) return;

    try {
      // Initialize schema for current question
      await duckdb.initializeSchema(currentQuestion.schema);

      // Execute query
      const queryResult = await duckdb.executeQuery(userCode);
      setResult(queryResult);
      setResultVersion(v => v + 1); // Force re-render

      // Check if correct
      const expected: QueryResult = {
        success: true,
        columns: currentQuestion.expectedOutput.columns,
        rows: currentQuestion.expectedOutput.rows,
        executionTimeMs: 0,
        rowCount: currentQuestion.expectedOutput.rows.length,
      };
      const correct = duckdb.compareResults(queryResult, expected);
      setIsCorrect(correct);

      // Record attempt in storage
      storageService.recordAttempt(
        currentQuestion.slug,
        correct,
        queryResult.executionTimeMs,
        userCode
      );

      // Update solved questions state
      if (correct && !solvedQuestions.has(currentQuestion.slug)) {
        setSolvedQuestions((prev) => new Set(prev).add(currentQuestion.slug));
      }
    } catch (error) {
      console.error('Failed to execute query:', error);
      setResult({
        success: false,
        columns: [],
        rows: [],
        executionTimeMs: 0,
        rowCount: 0,
        errorMessage: error instanceof Error ? error.message : String(error),
      });
      setIsCorrect(false);
      setResultVersion(v => v + 1); // Force re-render

      // Record failed attempt
      storageService.recordAttempt(currentQuestion.slug, false, undefined, userCode);
    }
  }

  async function handleSelectQuestion(slug: string) {
    const q = await questionService.loadQuestion(slug);
    if (q) {
      setCurrentQuestion(q);
      setUserCode(q.startingCode || '');
      setResult(null);
      setIsCorrect(null);
      setResultVersion(0);
    }
  }

  function handleResetCode() {
    if (currentQuestion) {
      setUserCode(currentQuestion.startingCode || '');
      setResult(null);
      setIsCorrect(null);
      setResultVersion(0);
    }
  }

  // Extract table and column names for autocomplete
  const schemaNames = useMemo(() => {
    if (!currentQuestion) return [];
    const names: string[] = [];
    for (const table of currentQuestion.schema.tables) {
      names.push(table.name);
      for (const col of table.columns) {
        names.push(col.name);
      }
    }
    return names;
  }, [currentQuestion]);

  if (!initialized) {
    return (
      <div className="min-h-screen bg-gradient-to-br from-slate-900 to-slate-800 flex items-center justify-center">
        <div className="text-center">
          <Database className="w-16 h-16 text-blue-400 animate-pulse mx-auto mb-4" />
          <p className="text-white text-xl">Initializing DuckDB-Wasm...</p>
          <p className="text-slate-400 mt-2">This may take a few seconds on first load</p>
        </div>
      </div>
    );
  }

  return (
    <div className="min-h-screen bg-slate-900">
      {/* Header */}
      <header className="bg-slate-800 border-b border-slate-700">
        <div className="max-w-7xl mx-auto px-4 py-4">
          <div className="flex items-center justify-between">
            <div className="flex items-center gap-3">
              <Database className="w-8 h-8 text-blue-400" />
              <div>
                <h1 className="text-2xl font-bold text-white">SQL Practice</h1>
                <p className="text-sm text-slate-400">Powered by DuckDB-Wasm</p>
              </div>
            </div>
            <div className="flex items-center gap-4">
              <div className="flex items-center gap-2 text-slate-400">
                <Terminal className="w-5 h-5" />
                <span className="text-sm">{questions.length} questions</span>
              </div>
              {solvedQuestions.size > 0 && (
                <div className="flex items-center gap-2 bg-slate-700 px-3 py-1.5 rounded-full">
                  <Trophy className="w-4 h-4 text-yellow-400" />
                  <span className="text-sm text-white">
                    {solvedQuestions.size}/{questions.length} solved
                  </span>
                </div>
              )}
            </div>
          </div>
        </div>
      </header>

      <div className="max-w-7xl mx-auto px-4 py-8">
        <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
          {/* Question List */}
          <div className="lg:col-span-1">
            <QuestionList
              questions={questions}
              currentSlug={currentQuestion?.slug ?? null}
              onSelectQuestion={handleSelectQuestion}
              solvedQuestions={solvedQuestions}
            />
          </div>

          {/* Main Content */}
          <div className="lg:col-span-2 space-y-6">
            {currentQuestion ? (
              <>
                {/* Question Info, Schema, Hints */}
                <QuestionView question={currentQuestion} />

                {/* SQL Editor */}
                <SqlEditor
                  value={userCode}
                  onChange={setUserCode}
                  onRun={handleRun}
                  onSubmit={handleSubmit}
                  onReset={handleResetCode}
                  schema={schemaNames}
                  height="300px"
                />

                {/* Results */}
                {result && <ResultsTable key={resultVersion} result={result} isCorrect={isCorrect} />}
              </>
            ) : (
              <div className="bg-slate-800 rounded-lg border border-slate-700 p-12 text-center">
                <BookOpen className="w-16 h-16 text-slate-600 mx-auto mb-4" />
                <h2 className="text-xl font-semibold text-white mb-2">Select a Question</h2>
                <p className="text-slate-400">Choose a question from the list to start practicing SQL</p>
              </div>
            )}
          </div>
        </div>
      </div>
    </div>
  );
}

export default App;
