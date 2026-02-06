// Global state
let currentQuestion = null;
let sessionToken = localStorage.getItem('sessionToken') || null;
let editor = null;

// Initialize CodeMirror editor
document.addEventListener('DOMContentLoaded', () => {
    editor = CodeMirror.fromTextArea(document.getElementById('sql-editor'), {
        mode: 'text/x-sql',
        theme: 'dracula',
        lineNumbers: true,
        autoCloseBrackets: true,
        matchBrackets: true,
        indentWithTabs: true,
        smartIndent: true,
        lineWrapping: true,
        extraKeys: {
            'Ctrl-Enter': runSQL,
            'Cmd-Enter': runSQL
        }
    });

    loadQuestions();

    // Filter event listeners
    document.getElementById('difficulty-filter').addEventListener('change', loadQuestions);
    document.getElementById('category-filter').addEventListener('change', loadQuestions);
});

// Load questions from API
async function loadQuestions() {
    const difficulty = document.getElementById('difficulty-filter').value;
    const category = document.getElementById('category-filter').value;

    const params = new URLSearchParams();
    if (difficulty) params.append('difficulty', difficulty);
    if (category) params.append('category', category);

    try {
        const response = await fetch(`/api/questions?${params}`);
        const questions = await response.json();

        const questionList = document.getElementById('question-list');
        questionList.innerHTML = '';

        questions.forEach(q => {
            const item = document.createElement('div');
            item.className = 'question-item';
            item.dataset.slug = q.slug;
            item.innerHTML = `
                <div class="question-item-title">${q.title}</div>
                <div class="question-item-meta">
                    <span class="badge ${q.difficulty}">${q.difficulty}</span>
                    <span class="badge ${q.category}">${q.category}</span>
                </div>
            `;
            item.addEventListener('click', () => loadQuestion(q.slug));
            questionList.appendChild(item);
        });
    } catch (error) {
        console.error('Failed to load questions:', error);
        document.getElementById('question-list').innerHTML = '<p class="error">Failed to load questions</p>';
    }
}

// Load specific question
async function loadQuestion(slug) {
    try {
        const response = await fetch(`/api/questions/${slug}`);
        const question = await response.json();

        currentQuestion = question;

        // Update UI
        document.getElementById('question-title').textContent = question.title;
        document.getElementById('question-description').textContent = question.description;

        // Update badges
        const difficultyBadge = document.getElementById('difficulty-badge');
        difficultyBadge.textContent = question.difficulty;
        difficultyBadge.className = `badge ${question.difficulty}`;

        const categoryBadge = document.getElementById('category-badge');
        categoryBadge.textContent = question.category;

        // Update starter code
        if (question.starter_code) {
            editor.setValue(question.starter_code);
        } else {
            editor.setValue('-- Write your SQL query here\nSELECT * FROM employees LIMIT 10;');
        }

        // Show schema if available
        if (question.schema && question.schema.length > 0) {
            document.getElementById('schema-panel').style.display = 'block';
            document.getElementById('schema-content').innerHTML = formatSchema(question.schema, question.sample_data);
        } else {
            document.getElementById('schema-panel').style.display = 'none';
        }

        // Show hints if available
        if (question.hints && question.hints.length > 0) {
            document.getElementById('hints-section').style.display = 'block';
            const hintsList = document.getElementById('hints-list');
            hintsList.innerHTML = question.hints.map(hint => `<li>${hint}</li>`).join('');
        } else {
            document.getElementById('hints-section').style.display = 'none';
        }

        // Hide results
        document.getElementById('results-section').style.display = 'none';

        // Update active question in sidebar
        document.querySelectorAll('.question-item').forEach(item => {
            item.classList.remove('active');
            if (item.dataset.slug === slug) {
                item.classList.add('active');
            }
        });
    } catch (error) {
        console.error('Failed to load question:', error);
    }
}

// Format schema for display
function formatSchema(schema, sampleData) {
    if (!schema || schema.length === 0) return '';

    let html = '<div class="schema-container">';

    schema.forEach(table => {
        html += `
            <div class="schema-table">
                <h4 class="schema-table-name">${table.name}</h4>
                <table class="schema-table-structure">
                    <thead>
                        <tr>
                            <th>Column</th>
                            <th>Type</th>
                        </tr>
                    </thead>
                    <tbody>
                        ${table.columns.map(col => `
                            <tr>
                                <td><code>${col.name}</code></td>
                                <td><span class="type-badge">${col.type}</span></td>
                            </tr>
                        `).join('')}
                    </tbody>
                </table>
        `;

        // Add sample data if available
        if (sampleData && sampleData[table.name]) {
            const sampleRows = sampleData[table.name];
            if (sampleRows && sampleRows.length > 0) {
                html += `
                    <div class="sample-data">
                        <h5>Sample Data</h5>
                        <table class="sample-data-table">
                            <thead>
                                <tr>
                                    ${table.columns.map(col => `<th>${col.name}</th>`).join('')}
                                </tr>
                            </thead>
                            <tbody>
                                ${sampleRows.map(row => `
                                    <tr>
                                        ${table.columns.map(col => `<td>${row[col] || 'NULL'}</td>`).join('')}
                                    </tr>
                                `).join('')}
                            </tbody>
                        </table>
                        ${sampleRows.length > 0 ? '<p class="sample-note">Showing first ' + sampleRows.length + ' rows</p>' : ''}
                    </div>
                `;
            }
        }

        html += '</div>';
    });

    html += '</div>';
    return html;
}

// Run SQL query
async function runSQL() {
    const sql = editor.getValue();

    if (!sql.trim()) {
        showResults(null, 'Please enter a SQL query', 'error');
        return;
    }

    try {
        const response = await fetch('/api/execute', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                session_token: sessionToken,
                question_id: currentQuestion?.id || '',
                user_sql: sql
            })
        });

        const result = await response.json();

        if (result.is_correct) {
            showResults(result, null, 'success');
        } else {
            showResults(null, result.error || 'Query execution failed', 'error');
        }
    } catch (error) {
        console.error('Failed to execute SQL:', error);
        showResults(null, 'Failed to execute query: ' + error.message, 'error');
    }
}

// Show results table
function showResults(result, message, type) {
    const resultsSection = document.getElementById('results-section');
    const resultsTable = document.getElementById('results-table');
    const resultsThead = document.getElementById('results-thead');
    const resultsTbody = document.getElementById('results-tbody');
    const resultsMessage = document.getElementById('results-message');
    const resultsStats = document.getElementById('results-stats');

    resultsSection.style.display = 'block';

    if (message) {
        resultsTable.parentElement.style.display = 'none';
        resultsMessage.style.display = 'block';
        resultsMessage.textContent = message;
        resultsMessage.className = `results-message ${type}`;
        resultsStats.textContent = '';
        return;
    }

    resultsTable.parentElement.style.display = 'block';
    resultsMessage.style.display = 'none';

    // Display columns
    if (result.columns && result.columns.length > 0) {
        resultsThead.innerHTML = '<tr>' + result.columns.map(col => `<th>${col}</th>`).join('') + '</tr>';
    }

    // Display rows
    if (result.rows && result.rows.length > 0) {
        resultsTbody.innerHTML = result.rows.map(row => {
            const cells = result.columns.map(col => {
                const val = row[col] !== undefined ? row[col] : 'NULL';
                return `<td>${val}</td>`;
            }).join('');
            return `<tr>${cells}</tr>`;
        }).join('');
    } else {
        resultsTbody.innerHTML = '<tr><td colspan="' + (result.columns?.length || 1) + '">No results</td></tr>';
    }

    // Show stats
    const rowCount = result.rows ? result.rows.length : 0;
    const execTime = result.execution_time_ms || 0;
    resultsStats.textContent = `${rowCount} row${rowCount !== 1 ? 's' : ''} returned in ${execTime}ms`;
}

// Reset editor
function resetEditor() {
    if (currentQuestion && currentQuestion.starter_code) {
        editor.setValue(currentQuestion.starter_code);
    } else {
        editor.setValue('-- Write your SQL query here\nSELECT * FROM employees LIMIT 10;');
    }
    document.getElementById('results-section').style.display = 'none';
}

// Login functions
function showLogin() {
    document.getElementById('login-modal').style.display = 'flex';
}

function closeLogin() {
    document.getElementById('login-modal').style.display = 'none';
}

async function handleLogin(event) {
    event.preventDefault();
    const username = document.getElementById('username').value;

    try {
        const response = await fetch('/api/login', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ user_id: username })
        });

        const result = await response.json();

        if (result.is_correct) {
            sessionToken = result.error;
            localStorage.setItem('sessionToken', sessionToken);
            closeLogin();
            alert('Logged in successfully!');
        } else {
            alert('Login failed: ' + result.error);
        }
    } catch (error) {
        console.error('Login failed:', error);
        alert('Login failed: ' + error.message);
    }
}
