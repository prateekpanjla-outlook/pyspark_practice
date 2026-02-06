import Link from "next/link";

export default function Home() {
  return (
    <main className="min-h-screen bg-gradient-to-br from-slate-900 to-slate-800">
      <div className="container mx-auto px-4 py-16">
        {/* Header */}
        <div className="text-center mb-16">
          <h1 className="text-5xl font-bold text-white mb-4">
            SQL Practice Platform
          </h1>
          <p className="text-xl text-slate-300">
            Practice SQL interview questions in a safe, local environment
          </p>
        </div>

        {/* Features */}
        <div className="grid md:grid-cols-3 gap-8 mb-16">
          <div className="bg-slate-800 rounded-lg p-6 border border-slate-700">
            <div className="text-3xl mb-4">🔒</div>
            <h3 className="text-xl font-semibold text-white mb-2">
              100% Local & Private
            </h3>
            <p className="text-slate-400">
              All queries execute in isolated containers. Your data never leaves your machine.
            </p>
          </div>

          <div className="bg-slate-800 rounded-lg p-6 border border-slate-700">
            <div className="text-3xl mb-4">⚡</div>
            <h3 className="text-xl font-semibold text-white mb-2">
              Real-Time Feedback
            </h3>
            <p className="text-slate-400">
              Get instant results with execution time and comparison to expected output.
            </p>
          </div>

          <div className="bg-slate-800 rounded-lg p-6 border border-slate-700">
            <div className="text-3xl mb-4">📚</div>
            <h3 className="text-xl font-semibold text-white mb-2">
              FAANG Questions
            </h3>
            <p className="text-slate-400">
              Practice with real interview questions from top tech companies.
            </p>
          </div>
        </div>

        {/* CTA */}
        <div className="text-center">
          <Link
            href="/questions"
            className="inline-block bg-blue-600 hover:bg-blue-700 text-white font-semibold px-8 py-4 rounded-lg text-lg transition-colors"
          >
            Start Practicing →
          </Link>
        </div>

        {/* Stack */}
        <div className="mt-16 text-center text-slate-500 text-sm">
          <p>Built with Next.js, FastAPI, PostgreSQL, and Kubernetes</p>
        </div>
      </div>
    </main>
  );
}
