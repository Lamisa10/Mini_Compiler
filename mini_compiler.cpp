/*
Mini Compiler Lab Suite (CodeBlocks-ready, single file)

Implements:
Lab 01: Single-line & multi-line comment removal from C code
Lab 02: Token identification (simple lexical analyzer for C-like code)
Lab 04: Left Factoring for a CFG
Lab 05: Left Recursion Elimination (direct + indirect)
Lab 06 & 07: FIRST and FOLLOW
Lab 08: LL(1) Parsing Table
Lab 09: Predictive Parser for expression grammar + input string (e.g., id+id*id)

How to use:
- Paste into CodeBlocks -> Build & Run
- Use the menu. For grammar labs, default grammars are included.
- Epsilon is printed as: eps
*/

#include <bits/stdc++.h>
using namespace std;

static const string EPS = "eps";
static const string END_MARK = "$";

// ----------------------------- Utility -----------------------------
static inline string trim(const string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

static inline bool isIdentChar(char c) {
    return (isalnum((unsigned char)c) || c == '_' || c == '\'');
}

static inline bool isTerminalSymbol(const string &sym) {
    // Heuristic: terminals include punctuation tokens and "id"/numbers/keywords from token stream.
    // For grammar usage, we compute terminals precisely, so this is rarely needed.
    return true;
}

static string joinSymbols(const vector<string> &rhs) {
    if (rhs.empty()) return EPS;
    string out;
    for (size_t i = 0; i < rhs.size(); i++) {
        if (i) out += " ";
        out += rhs[i];
    }
    return out;
}

// Tokenize a grammar RHS segment (no '|' or '->' inside).
// Supports: E, E', id, +, *, (, ), etc. Epsilon accepted as: eps / epsilon / ε / @
static vector<string> tokenizeGrammarSegment(const string &segRaw) {
    string seg = segRaw;
    vector<string> tokens;
    string cur;

    auto flush = [&]() {
        if (!cur.empty()) {
            string t = cur;
            cur.clear();
            // normalize epsilon
            if (t == "ε" || t == "epsilon" || t == "eps" || t == "@") t = EPS;
            tokens.push_back(t);
        }
    };

    for (size_t i = 0; i < seg.size(); i++) {
        char c = seg[i];
        if (isspace((unsigned char)c)) {
            flush();
            continue;
        }
        // operators / parentheses as standalone tokens
        if (c=='(' || c==')' || c=='+' || c=='*' || c=='-' || c=='/' || c=='|' ) {
            flush();
            string t(1, c);
            tokens.push_back(t);
            continue;
        }
        // part of identifier/nonterminal (including apostrophe)
        if (isIdentChar(c)) {
            cur.push_back(c);
        } else {
            // any other punctuation becomes token
            flush();
            string t(1, c);
            tokens.push_back(t);
        }
    }
    flush();

    // If segment is exactly epsilon or empty
    if (tokens.size() == 1 && tokens[0] == EPS) return {EPS};
    if (tokens.empty()) return {EPS};
    return tokens;
}

// ----------------------------- Grammar Structure -----------------------------
struct Grammar {
    string start;
    set<string> nonterminals;
    set<string> terminals;
    map<string, vector<vector<string>>> prod; // A -> list of RHS (each RHS is vector of symbols)

    bool isNonTerminal(const string &s) const {
        return nonterminals.count(s) > 0;
    }

    bool isTerminal(const string &s) const {
        return terminals.count(s) > 0;
    }

    void recomputeSymbols() {
        terminals.clear();
        // terminals = all RHS symbols that are not nonterminals and not EPS
        for (auto &kv : prod) {
            for (auto &rhs : kv.second) {
                for (auto &sym : rhs) {
                    if (sym == EPS) continue;
                    if (!isNonTerminal(sym)) terminals.insert(sym);
                }
            }
        }
    }

    void print() const {
        cout << "\n--- Grammar ---\n";
        cout << "Start symbol: " << start << "\n";
        for (auto &A : nonterminals) {
            auto it = prod.find(A);
            if (it == prod.end()) continue;
            cout << A << " -> ";
            const auto &alts = it->second;
            for (size_t i = 0; i < alts.size(); i++) {
                if (i) cout << " | ";
                cout << joinSymbols(alts[i]);
            }
            cout << "\n";
        }
        cout << "NonTerminals: ";
        for (auto &x : nonterminals) cout << x << " ";
        cout << "\nTerminals: ";
        for (auto &x : terminals) cout << x << " ";
        cout << "\n--------------\n";
    }
};

static string makeUniqueNonTerminal(const Grammar &g, const string &base) {
    // Try base', base1, base2 ...
    string cand = base + "'";
    if (!g.nonterminals.count(cand)) return cand;
    for (int k = 1; k <= 999; k++) {
        cand = base + to_string(k);
        if (!g.nonterminals.count(cand)) return cand;
        cand = base + "'" + to_string(k);
        if (!g.nonterminals.count(cand)) return cand;
    }
    return base + "_NEW";
}

// Parse a single production line like:
// E -> E + T | T
static void addRuleLine(Grammar &g, const string &lineRaw) {
    string line = trim(lineRaw);
    if (line.empty()) return;

    // Find "->"
    size_t pos = line.find("->");
    if (pos == string::npos) pos = line.find("→"); // optional unicode arrow
    if (pos == string::npos) {
        cerr << "Invalid rule (missing ->): " << line << "\n";
        return;
    }

    string lhs = trim(line.substr(0, pos));
    string rhsAll = trim(line.substr(pos + 2));

    if (lhs.empty()) {
        cerr << "Invalid rule (empty LHS): " << line << "\n";
        return;
    }

    g.nonterminals.insert(lhs);
    if (g.start.empty()) g.start = lhs;

    // Split RHS by '|'
    vector<string> parts;
    {
        string cur;
        for (char c : rhsAll) {
            if (c == '|') {
                parts.push_back(trim(cur));
                cur.clear();
            } else {
                cur.push_back(c);
            }
        }
        parts.push_back(trim(cur));
    }

    for (auto &p : parts) {
        auto tokens = tokenizeGrammarSegment(p);
        // Normalize epsilon-only production
        if (tokens.size() == 1 && tokens[0] == EPS) {
            g.prod[lhs].push_back({EPS});
        } else {
            g.prod[lhs].push_back(tokens);
        }
    }
}

// Read grammar from user
static Grammar readGrammarFromUser() {
    Grammar g;
    cout << "\nEnter number of production lines: ";
    int n;
    cin >> n;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Enter productions in format: A -> alpha1 | alpha2\n";
    cout << "Use epsilon as: eps (or epsilon/@/ε)\n";
    for (int i = 0; i < n; i++) {
        string line;
        getline(cin, line);
        addRuleLine(g, line);
    }
    g.recomputeSymbols();
    return g;
}

// Default expression grammar (left-recursive)
static Grammar defaultExprGrammarLeftRecursive() {
    Grammar g;
    addRuleLine(g, "E -> E + T | T");
    addRuleLine(g, "T -> T * F | F");
    addRuleLine(g, "F -> ( E ) | id");
    g.recomputeSymbols();
    return g;
}

// Classic left-factoring example (if-then-else)
static Grammar defaultLeftFactoringExample() {
    Grammar g;
    addRuleLine(g, "S -> i E t S | i E t S e S | a");
    addRuleLine(g, "E -> b");
    g.recomputeSymbols();
    return g;
}

// ----------------------------- Lab 04: Left Factoring -----------------------------
static size_t commonPrefixLen(const vector<string> &a, const vector<string> &b) {
    size_t i = 0;
    while (i < a.size() && i < b.size() && a[i] == b[i] && a[i] != EPS) i++;
    return i;
}

static bool leftFactorOnce(Grammar &g) {
    // For each nonterminal, find the longest common prefix among any pair of productions
    for (const string &A : vector<string>(g.nonterminals.begin(), g.nonterminals.end())) {
        auto &alts = g.prod[A];
        if (alts.size() < 2) continue;

        size_t bestLen = 0;
        vector<string> bestPrefix;

        // Find best prefix length
        for (size_t i = 0; i < alts.size(); i++) {
            for (size_t j = i + 1; j < alts.size(); j++) {
                size_t len = commonPrefixLen(alts[i], alts[j]);
                if (len > bestLen) {
                    bestLen = len;
                    bestPrefix.assign(alts[i].begin(), alts[i].begin() + (long long)len);
                }
            }
        }
        if (bestLen == 0) continue;

        // Collect all productions that share this prefix
        vector<vector<string>> group, rest;
        for (auto &rhs : alts) {
            bool ok = (rhs.size() >= bestLen);
            for (size_t k = 0; ok && k < bestLen; k++) {
                if (rhs[k] != bestPrefix[k]) ok = false;
            }
            if (ok) group.push_back(rhs);
            else rest.push_back(rhs);
        }
        if (group.size() < 2) continue;

        // Create new nonterminal A'
        string Aprime = makeUniqueNonTerminal(g, A);
        g.nonterminals.insert(Aprime);

        // New productions for A: prefix Aprime plus the rest productions
        vector<vector<string>> newA = rest;
        vector<string> newRhs = bestPrefix;
        newRhs.push_back(Aprime);
        newA.push_back(newRhs);

        // Productions for Aprime are remainders
        vector<vector<string>> newAprime;
        for (auto &rhs : group) {
            vector<string> rem(rhs.begin() + (long long)bestLen, rhs.end());
            if (rem.empty()) rem = {EPS};
            newAprime.push_back(rem);
        }

        g.prod[A] = newA;
        g.prod[Aprime] = newAprime;

        g.recomputeSymbols();
        return true; // did one factoring step
    }
    return false;
}

static void leftFactor(Grammar &g) {
    while (leftFactorOnce(g)) {
        // repeat until stable
    }
}

// ----------------------------- Lab 05: Left Recursion Elimination -----------------------------
static void substituteAjIntoAi(Grammar &g, const string &Ai, const string &Aj) {
    // Replace productions Ai -> Aj γ with Aj alternatives
    vector<vector<string>> newAlts;
    for (auto &rhs : g.prod[Ai]) {
        if (!rhs.empty() && rhs[0] == Aj) {
            vector<string> gamma(rhs.begin() + 1, rhs.end());
            for (auto &delta : g.prod[Aj]) {
                vector<string> expanded;
                if (!(delta.size() == 1 && delta[0] == EPS)) {
                    expanded.insert(expanded.end(), delta.begin(), delta.end());
                }
                expanded.insert(expanded.end(), gamma.begin(), gamma.end());
                if (expanded.empty()) expanded = {EPS};
                newAlts.push_back(expanded);
            }
        } else {
            newAlts.push_back(rhs);
        }
    }
    g.prod[Ai] = newAlts;
}

static void eliminateImmediateLeftRecursion(Grammar &g, const string &A) {
    vector<vector<string>> alpha; // A -> A alpha
    vector<vector<string>> beta;  // A -> beta

    for (auto &rhs : g.prod[A]) {
        if (!rhs.empty() && rhs[0] == A) {
            vector<string> tail(rhs.begin() + 1, rhs.end());
            if (tail.empty()) tail = {EPS};
            alpha.push_back(tail);
        } else {
            beta.push_back(rhs);
        }
    }
    if (alpha.empty()) return;

    string Aprime = makeUniqueNonTerminal(g, A);
    g.nonterminals.insert(Aprime);

    // A -> beta Aprime
    vector<vector<string>> newA;
    for (auto &b : beta) {
        vector<string> rhs = b;
        if (!(rhs.size() == 1 && rhs[0] == EPS)) {
            rhs.push_back(Aprime);
        } else {
            rhs = {Aprime}; // if beta was epsilon, just Aprime
        }
        newA.push_back(rhs);
    }

    // Aprime -> alpha Aprime | eps
    vector<vector<string>> newAprime;
    for (auto &a : alpha) {
        vector<string> rhs = a;
        if (rhs.size() == 1 && rhs[0] == EPS) {
            // A -> A eps is weird; treat as just Aprime -> Aprime, ignore; but keep safe:
            rhs = {Aprime};
        } else {
            rhs.push_back(Aprime);
        }
        newAprime.push_back(rhs);
    }
    newAprime.push_back({EPS});

    g.prod[A] = newA;
    g.prod[Aprime] = newAprime;
}

static void eliminateLeftRecursion(Grammar &g) {
    // Standard algorithm handles indirect left recursion by ordering nonterminals
    vector<string> nts(g.nonterminals.begin(), g.nonterminals.end());

    for (size_t i = 0; i < nts.size(); i++) {
        string Ai = nts[i];
        for (size_t j = 0; j < i; j++) {
            string Aj = nts[j];
            substituteAjIntoAi(g, Ai, Aj);
        }
        eliminateImmediateLeftRecursion(g, Ai);

        // If new nonterminals were added, update list
        if (g.nonterminals.size() != nts.size()) {
            nts.assign(g.nonterminals.begin(), g.nonterminals.end());
            // Ensure i still points to same Ai (re-find)
            auto it = find(nts.begin(), nts.end(), Ai);
            if (it != nts.end()) i = (size_t)(it - nts.begin());
        }
    }
    g.recomputeSymbols();
}

// ----------------------------- Lab 06 & 07: FIRST and FOLLOW -----------------------------
static set<string> firstOfSequence(const vector<string> &seq,
                                  const map<string, set<string>> &FIRST,
                                  const Grammar &g) {
    set<string> result;
    if (seq.empty()) {
        result.insert(EPS);
        return result;
    }
    bool allEps = true;
    for (auto &X : seq) {
        if (X == EPS) {
            result.insert(EPS);
            allEps = true;
            break;
        }
        if (g.isTerminal(X) || !g.isNonTerminal(X)) {
            result.insert(X);
            allEps = false;
            break;
        }
        auto it = FIRST.find(X);
        if (it != FIRST.end()) {
            for (auto &a : it->second) if (a != EPS) result.insert(a);
            if (it->second.count(EPS)) {
                // continue
            } else {
                allEps = false;
                break;
            }
        } else {
            // unknown symbol: treat as terminal
            result.insert(X);
            allEps = false;
            break;
        }
    }
    if (allEps) result.insert(EPS);
    return result;
}

static void computeFIRST(const Grammar &g, map<string, set<string>> &FIRST) {
    FIRST.clear();

    // Terminals
    for (auto &t : g.terminals) FIRST[t].insert(t);
    FIRST[EPS].insert(EPS);

    // Nonterminals
    for (auto &A : g.nonterminals) FIRST[A]; // ensure exists

    bool changed = true;
    while (changed) {
        changed = false;
        for (auto &A : g.nonterminals) {
            for (auto &rhs : g.prod.at(A)) {
                // Compute FIRST(rhs)
                bool allEps = true;
                for (auto &X : rhs) {
                    if (X == EPS) {
                        if (!FIRST[A].count(EPS)) {
                            FIRST[A].insert(EPS);
                            changed = true;
                        }
                        allEps = true;
                        break;
                    }

                    if (g.isTerminal(X) || !g.isNonTerminal(X)) {
                        if (!FIRST[A].count(X)) {
                            FIRST[A].insert(X);
                            changed = true;
                        }
                        allEps = false;
                        break;
                    }

                    // X is nonterminal
                    for (auto &a : FIRST[X]) {
                        if (a == EPS) continue;
                        if (!FIRST[A].count(a)) {
                            FIRST[A].insert(a);
                            changed = true;
                        }
                    }
                    if (FIRST[X].count(EPS)) {
                        // keep going
                    } else {
                        allEps = false;
                        break;
                    }
                }
                if (allEps) {
                    if (!FIRST[A].count(EPS)) {
                        FIRST[A].insert(EPS);
                        changed = true;
                    }
                }
            }
        }
    }
}

static void computeFOLLOW(const Grammar &g,
                          const map<string, set<string>> &FIRST,
                          map<string, set<string>> &FOLLOW) {
    FOLLOW.clear();
    for (auto &A : g.nonterminals) FOLLOW[A]; // ensure exists
    FOLLOW[g.start].insert(END_MARK);

    bool changed = true;
    while (changed) {
        changed = false;
        for (auto &A : g.nonterminals) {
            for (auto &rhs : g.prod.at(A)) {
                for (size_t i = 0; i < rhs.size(); i++) {
                    const string &B = rhs[i];
                    if (!g.isNonTerminal(B)) continue;

                    vector<string> beta(rhs.begin() + (long long)i + 1, rhs.end());
                    set<string> firstBeta = firstOfSequence(beta, FIRST, g);

                    // FIRST(beta) - eps -> FOLLOW(B)
                    for (auto &x : firstBeta) {
                        if (x == EPS) continue;
                        if (!FOLLOW[B].count(x)) {
                            FOLLOW[B].insert(x);
                            changed = true;
                        }
                    }

                    // if eps in FIRST(beta) or beta empty: FOLLOW(A) -> FOLLOW(B)
                    if (beta.empty() || firstBeta.count(EPS)) {
                        for (auto &x : FOLLOW[A]) {
                            if (!FOLLOW[B].count(x)) {
                                FOLLOW[B].insert(x);
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
    }
}

static void printFIRSTFOLLOW(const Grammar &g,
                            const map<string, set<string>> &FIRST,
                            const map<string, set<string>> &FOLLOW) {
    cout << "\n--- FIRST sets ---\n";
    for (auto &A : g.nonterminals) {
        cout << "FIRST(" << A << ") = { ";
        for (auto &x : FIRST.at(A)) cout << x << " ";
        cout << "}\n";
    }
    cout << "\n--- FOLLOW sets ---\n";
    for (auto &A : g.nonterminals) {
        cout << "FOLLOW(" << A << ") = { ";
        for (auto &x : FOLLOW.at(A)) cout << x << " ";
        cout << "}\n";
    }
}

// ----------------------------- Lab 08: LL(1) Parsing Table -----------------------------
struct ParseTableCell {
    bool filled = false;
    bool conflict = false;
    vector<string> rhs; // production RHS
    string fromA; // nonterminal (optional)
};

static void buildParseTable(const Grammar &g,
                            const map<string, set<string>> &FIRST,
                            const map<string, set<string>> &FOLLOW,
                            map<string, map<string, ParseTableCell>> &table,
                            vector<string> &tableTerminalsSorted) {
    table.clear();

    set<string> cols = g.terminals;
    cols.insert(END_MARK);
    tableTerminalsSorted.assign(cols.begin(), cols.end());

    for (auto &A : g.nonterminals) {
        for (auto &a : cols) table[A][a] = ParseTableCell{};
    }

    for (auto &A : g.nonterminals) {
        for (auto &rhs : g.prod.at(A)) {
            set<string> firstAlpha = firstOfSequence(rhs, FIRST, g);

            // For each terminal in FIRST(alpha) except eps
            for (auto &t : firstAlpha) {
                if (t == EPS) continue;
                auto &cell = table[A][t];
                if (!cell.filled) {
                    cell.filled = true;
                    cell.rhs = rhs;
                    cell.fromA = A;
                } else {
                    cell.conflict = true;
                }
            }

            // If eps in FIRST(alpha), add to FOLLOW(A)
            if (firstAlpha.count(EPS)) {
                for (auto &b : FOLLOW.at(A)) {
                    auto &cell = table[A][b];
                    if (!cell.filled) {
                        cell.filled = true;
                        cell.rhs = rhs;
                        cell.fromA = A;
                    } else {
                        cell.conflict = true;
                    }
                }
            }
        }
    }
}

static void printParseTable(const Grammar &g,
                            const map<string, map<string, ParseTableCell>> &table,
                            const vector<string> &cols) {
    cout << "\n--- LL(1) Parsing Table ---\n";
    cout << setw(10) << "NT\\T";
    for (auto &t : cols) cout << setw(12) << t;
    cout << "\n";

    for (auto &A : g.nonterminals) {
        cout << setw(10) << A;
        for (auto &t : cols) {
            const auto &cell = table.at(A).at(t);
            if (!cell.filled) {
                cout << setw(12) << ".";
            } else if (cell.conflict) {
                cout << setw(12) << "CONFLICT";
            } else {
                string pr = A + "->" + joinSymbols(cell.rhs);
                if (pr.size() > 10) pr = pr.substr(0, 9) + "..";
                cout << setw(12) << pr;
            }
        }
        cout << "\n";
    }

    bool hasConflict = false;
    for (auto &A : g.nonterminals)
        for (auto &t : cols)
            if (table.at(A).at(t).conflict) hasConflict = true;

    if (hasConflict) {
        cout << "\nWARNING: Conflicts detected. Grammar may NOT be LL(1).\n";
    } else {
        cout << "\nNo conflicts detected. Grammar looks LL(1).\n";
    }
}

// ----------------------------- Lab 09: Predictive Parser -----------------------------
static vector<string> tokenizeExpressionInput(const string &s) {
    // Converts input like: id+id*id, a+b*(c) into tokens: id + id * id etc.
    vector<string> out;
    for (size_t i = 0; i < s.size();) {
        char c = s[i];
        if (isspace((unsigned char)c)) {
            i++;
            continue;
        }
        if (isalpha((unsigned char)c) || c=='_') {
            size_t j = i;
            while (j < s.size() && (isalnum((unsigned char)s[j]) || s[j]=='_')) j++;
            string word = s.substr(i, j - i);
            // Treat any identifier as "id" (fits expression grammar)
            out.push_back("id");
            i = j;
            continue;
        }
        if (isdigit((unsigned char)c)) {
            // treat number as id (for this grammar)
            size_t j = i;
            while (j < s.size() && (isdigit((unsigned char)s[j]) || s[j]=='.')) j++;
            out.push_back("id");
            i = j;
            continue;
        }
        // single-char operators
        if (c=='+' || c=='*' || c=='(' || c==')') {
            out.push_back(string(1, c));
            i++;
            continue;
        }
        // unknown char: still push
        out.push_back(string(1, c));
        i++;
    }
    out.push_back(END_MARK);
    return out;
}

static string stackToString(vector<string> st) {
    // Print top on right
    string out;
    for (size_t i = 0; i < st.size(); i++) {
        if (i) out += " ";
        out += st[i];
    }
    return out;
}

static string inputToString(const vector<string> &inp, size_t pos) {
    string out;
    for (size_t i = pos; i < inp.size(); i++) {
        if (i > pos) out += " ";
        out += inp[i];
    }
    return out;
}

static bool predictiveParse(const Grammar &g,
                            const map<string, map<string, ParseTableCell>> &table,
                            const string &inputStr,
                            bool showSteps = true) {
    vector<string> inp = tokenizeExpressionInput(inputStr);
    size_t ip = 0;

    vector<string> st;
    st.push_back(END_MARK);
    st.push_back(g.start);

    if (showSteps) {
        cout << "\n--- Predictive Parsing Steps ---\n";
        cout << left << setw(30) << "STACK" << setw(35) << "INPUT" << "ACTION\n";
        cout << string(80, '-') << "\n";
    }

    while (!st.empty()) {
        string X = st.back();
        string a = (ip < inp.size() ? inp[ip] : END_MARK);

        if (showSteps) {
            cout << left << setw(30) << stackToString(st)
                 << setw(35) << inputToString(inp, ip);
        }

        if (X == END_MARK && a == END_MARK) {
            if (showSteps) cout << "ACCEPT\n";
            return true;
        }

        if (!g.isNonTerminal(X) || X == END_MARK) {
            // terminal
            if (X == a) {
                st.pop_back();
                ip++;
                if (showSteps) cout << "match " << a << "\n";
            } else {
                if (showSteps) cout << "ERROR (expected " << X << ")\n";
                return false;
            }
        } else {
            // nonterminal
            auto rowIt = table.find(X);
            if (rowIt == table.end()) {
                if (showSteps) cout << "ERROR (no table row)\n";
                return false;
            }
            auto colIt = rowIt->second.find(a);
            if (colIt == rowIt->second.end() || !colIt->second.filled || colIt->second.conflict) {
                if (showSteps) cout << "ERROR (no rule for [" << X << "," << a << "])\n";
                return false;
            }

            vector<string> rhs = colIt->second.rhs;
            st.pop_back();

            // push RHS in reverse (skip eps)
            if (!(rhs.size() == 1 && rhs[0] == EPS)) {
                for (auto it = rhs.rbegin(); it != rhs.rend(); ++it) {
                    st.push_back(*it);
                }
            }
            if (showSteps) cout << X << " -> " << joinSymbols(rhs) << "\n";
        }
    }

    return false;
}

// ----------------------------- Lab 01: Comment Removal -----------------------------
static string removeCComments(const string &code) {
    string out;
    bool inString = false, inChar = false;
    bool inSL = false, inML = false;
    bool esc = false;

    for (size_t i = 0; i < code.size(); i++) {
        char c = code[i];
        char n = (i + 1 < code.size() ? code[i + 1] : '\0');

        if (inSL) {
            if (c == '\n') {
                inSL = false;
                out.push_back(c);
            }
            continue;
        }

        if (inML) {
            if (c == '*' && n == '/') {
                inML = false;
                i++;
            }
            continue;
        }

        if (inString) {
            out.push_back(c);
            if (!esc && c == '"') inString = false;
            esc = (!esc && c == '\\');
            continue;
        }

        if (inChar) {
            out.push_back(c);
            if (!esc && c == '\'') inChar = false;
            esc = (!esc && c == '\\');
            continue;
        }

        // not inside string/char/comment
        if (c == '"' ) {
            inString = true;
            esc = false;
            out.push_back(c);
            continue;
        }
        if (c == '\'') {
            inChar = true;
            esc = false;
            out.push_back(c);
            continue;
        }

        // comment starts?
        if (c == '/' && n == '/') {
            inSL = true;
            i++;
            continue;
        }
        if (c == '/' && n == '*') {
            inML = true;
            i++;
            continue;
        }

        out.push_back(c);
    }
    return out;
}

// ----------------------------- Lab 02: Tokenizer (simple C-like) -----------------------------
enum class TokType {
    KEYWORD, IDENTIFIER, NUMBER, STRING_LIT, CHAR_LIT,
    OPERATOR, SEPARATOR, PREPROCESSOR, UNKNOWN
};

static string tokTypeName(TokType t) {
    switch (t) {
        case TokType::KEYWORD: return "KEYWORD";
        case TokType::IDENTIFIER: return "IDENTIFIER";
        case TokType::NUMBER: return "NUMBER";
        case TokType::STRING_LIT: return "STRING_LITERAL";
        case TokType::CHAR_LIT: return "CHAR_LITERAL";
        case TokType::OPERATOR: return "OPERATOR";
        case TokType::SEPARATOR: return "SEPARATOR";
        case TokType::PREPROCESSOR: return "PREPROCESSOR";
        default: return "UNKNOWN";
    }
}

struct Token {
    TokType type;
    string lexeme;
    int line;
};

static bool isKeyword(const string &s) {
    static const unordered_set<string> kw = {
        "auto","break","case","char","const","continue","default","do","double","else","enum",
        "extern","float","for","goto","if","inline","int","long","register","restrict","return",
        "short","signed","sizeof","static","struct","switch","typedef","union","unsigned","void",
        "volatile","while","_Bool","_Complex","_Imaginary",
        // common C++ too
        "class","namespace","public","private","protected","template","typename","using","new","delete",
        "try","catch","throw","this","operator","friend","virtual","override","nullptr","bool"
    };
    return kw.count(s) > 0;
}

static vector<Token> tokenizeC(const string &code) {
    vector<Token> tokens;
    int line = 1;

    auto push = [&](TokType t, const string &lex) {
        tokens.push_back({t, lex, line});
    };

    static const vector<string> ops3 = {"<<=", ">>=", "..."};
    static const vector<string> ops2 = {
        "++","--","==","!=","<=",">=","&&","||","+=","-=","*=","/=","%=",
        "<<",">>","->","::","&=","|=","^=","##"
    };
    static const unordered_set<char> seps = {';',',','(',')','{','}','[',']',':','?','.'};

    for (size_t i = 0; i < code.size();) {
        char c = code[i];

        if (c == '\n') { line++; i++; continue; }
        if (isspace((unsigned char)c)) { i++; continue; }

        // Preprocessor (if # at beginning of line or after spaces)
        if (c == '#') {
            size_t j = i;
            while (j < code.size() && code[j] != '\n') j++;
            push(TokType::PREPROCESSOR, code.substr(i, j - i));
            i = j;
            continue;
        }

        // Identifier/Keyword
        if (isalpha((unsigned char)c) || c == '_') {
            size_t j = i;
            while (j < code.size() && (isalnum((unsigned char)code[j]) || code[j] == '_')) j++;
            string w = code.substr(i, j - i);
            push(isKeyword(w) ? TokType::KEYWORD : TokType::IDENTIFIER, w);
            i = j;
            continue;
        }

        // Number
        if (isdigit((unsigned char)c)) {
            size_t j = i;
            bool dot = false;
            while (j < code.size()) {
                char d = code[j];
                if (isdigit((unsigned char)d)) { j++; continue; }
                if (d == '.' && !dot) { dot = true; j++; continue; }
                if ((d=='e' || d=='E') && j+1 < code.size()) {
                    j++;
                    if (code[j]=='+' || code[j]=='-') j++;
                    continue;
                }
                break;
            }
            push(TokType::NUMBER, code.substr(i, j - i));
            i = j;
            continue;
        }

        // String literal
        if (c == '"') {
            size_t j = i + 1;
            bool esc = false;
            while (j < code.size()) {
                char d = code[j];
                if (d == '\n') line++;
                if (!esc && d == '"') { j++; break; }
                esc = (!esc && d == '\\');
                j++;
            }
            push(TokType::STRING_LIT, code.substr(i, j - i));
            i = j;
            continue;
        }

        // Char literal
        if (c == '\'') {
            size_t j = i + 1;
            bool esc = false;
            while (j < code.size()) {
                char d = code[j];
                if (d == '\n') line++;
                if (!esc && d == '\'') { j++; break; }
                esc = (!esc && d == '\\');
                j++;
            }
            push(TokType::CHAR_LIT, code.substr(i, j - i));
            i = j;
            continue;
        }

        // Operators (3-char then 2-char then 1-char)
        bool matched = false;
        if (i + 2 < code.size()) {
            string t3 = code.substr(i, 3);
            for (auto &op : ops3) {
                if (t3 == op) {
                    push(TokType::OPERATOR, t3);
                    i += 3;
                    matched = true;
                    break;
                }
            }
            if (matched) continue;
        }
        if (i + 1 < code.size()) {
            string t2 = code.substr(i, 2);
            for (auto &op : ops2) {
                if (t2 == op) {
                    push(TokType::OPERATOR, t2);
                    i += 2;
                    matched = true;
                    break;
                }
            }
            if (matched) continue;
        }

        // Separators
        if (seps.count(c)) {
            push(TokType::SEPARATOR, string(1, c));
            i++;
            continue;
        }

        // Single char operators fallback
        if (string("+-*/%<>=!&|^~").find(c) != string::npos) {
            push(TokType::OPERATOR, string(1, c));
            i++;
            continue;
        }

        // Unknown
        push(TokType::UNKNOWN, string(1, c));
        i++;
    }

    return tokens;
}

// ----------------------------- Input Helpers -----------------------------
static string readFromFileOrPaste() {
    cout << "\nChoose input method:\n";
    cout << "1) Read from file path\n";
    cout << "2) Paste text (end with a single line: ###END###)\n";
    cout << "Enter choice: ";
    int ch;
    cin >> ch;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (ch == 1) {
        cout << "Enter file path: ";
        string path;
        getline(cin, path);
        ifstream fin(path);
        if (!fin) {
            cerr << "Could not open file.\n";
            return "";
        }
        stringstream ss;
        ss << fin.rdbuf();
        return ss.str();
    } else {
        cout << "Paste now. Finish by typing: ###END### on its own line.\n";
        string line, all;
        while (true) {
            if (!getline(cin, line)) break;
            if (line == "###END###") break;
            all += line + "\n";
        }
        return all;
    }
}

// ----------------------------- Menu Actions -----------------------------
static void Case01() {
    cout << "\n[Lab 01] Comment Removal\n";
    string code = readFromFileOrPaste();
    if (code.empty()) return;
    string cleaned = removeCComments(code);
    cout << "\n--- Code (Comments Removed) ---\n";
    cout << cleaned << "\n";
}

static void Case02() {
    cout << "\n[Lab 02] Token Identification\n";
    string code = readFromFileOrPaste();
    if (code.empty()) return;
    string cleaned = removeCComments(code);
    auto toks = tokenizeC(cleaned);

    cout << "\n--- Tokens ---\n";
    cout << left << setw(6) << "Line" << setw(18) << "Type" << "Lexeme\n";
    cout << string(60, '-') << "\n";
    for (auto &t : toks) {
        cout << left << setw(6) << t.line << setw(18) << tokTypeName(t.type) << t.lexeme << "\n";
    }
}

static void Case03() {
    cout << "\n[Lab 03] Left Factoring\n";
    cout << "1) Use default example (if-then-else)\n";
    cout << "2) Enter your own grammar\n";
    cout << "Choice: ";
    int ch;
    cin >> ch;

    Grammar g;
    if (ch == 1) g = defaultLeftFactoringExample();
    else g = readGrammarFromUser();

    cout << "\nBefore Left Factoring:\n";
    g.recomputeSymbols();
    g.print();

    leftFactor(g);

    cout << "\nAfter Left Factoring:\n";
    g.recomputeSymbols();
    g.print();
}

static void Case04() {
    cout << "\n[Lab 05] Left Recursion Elimination\n";
    cout << "1) Use default expression grammar (left-recursive)\n";
    cout << "2) Enter your own grammar\n";
    cout << "Choice: ";
    int ch;
    cin >> ch;

    Grammar g;
    if (ch == 1) g = defaultExprGrammarLeftRecursive();
    else g = readGrammarFromUser();

    cout << "\nBefore elimination:\n";
    g.recomputeSymbols();
    g.print();

    eliminateLeftRecursion(g);

    cout << "\nAfter elimination:\n";
    g.recomputeSymbols();
    g.print();
}

static void askAndMaybePreprocess(Grammar &g) {
    cout << "\nPreprocess grammar before calculation?\n";
    cout << "1) Yes (Eliminate Left Recursion + Left Factoring)  [Recommended]\n";
    cout << "2) No  (Use grammar as entered)\n";
    cout << "Choice: ";
    int p;
    cin >> p;

    if (p == 1) {
        eliminateLeftRecursion(g);
        leftFactor(g);
    }
    g.recomputeSymbols();
}

static Grammar chooseGrammarForAnalysis(const string &labName, bool offerDefaultExpr = true) {
    cout << "\n[" << labName << "] Choose grammar source:\n";
    if (offerDefaultExpr) {
        cout << "1) Use default expression grammar (E,T,F)\n";
        cout << "2) Enter your own grammar\n";
        cout << "Choice: ";
        int ch;
        cin >> ch;

        Grammar g;
        if (ch == 1) g = defaultExprGrammarLeftRecursive();
        else g = readGrammarFromUser();

        askAndMaybePreprocess(g);
        return g;
    } else {
        // fallback if someday you want custom-only
        Grammar g = readGrammarFromUser();
        askAndMaybePreprocess(g);
        return g;
    }
}

static void Case05() {
    cout << "\n[Lab 06 & 07] FIRST and FOLLOW\n";

    Grammar g = chooseGrammarForAnalysis("Lab 06 & 07: FIRST and FOLLOW", true);
    g.print();

    map<string, set<string>> FIRST, FOLLOW;
    computeFIRST(g, FIRST);
    computeFOLLOW(g, FIRST, FOLLOW);

    printFIRSTFOLLOW(g, FIRST, FOLLOW);
}

static void Case06() {
    cout << "\n[Lab 08] LL(1) Parsing Table\n";

    Grammar g = chooseGrammarForAnalysis("Lab 08: LL(1) Parsing Table", true);
    g.print();

    map<string, set<string>> FIRST, FOLLOW;
    computeFIRST(g, FIRST);
    computeFOLLOW(g, FIRST, FOLLOW);

    map<string, map<string, ParseTableCell>> table;
    vector<string> cols;
    buildParseTable(g, FIRST, FOLLOW, table, cols);

    printParseTable(g, table, cols);
}

static void Case07() {
    cout << "\n[Lab 09] Predictive Parser (Expression Grammar)\n";
    Grammar g = defaultExprGrammarLeftRecursive();
    eliminateLeftRecursion(g);
    leftFactor(g);
    g.recomputeSymbols();
    g.print();

    map<string, set<string>> FIRST, FOLLOW;
    computeFIRST(g, FIRST);
    computeFOLLOW(g, FIRST, FOLLOW);

    map<string, map<string, ParseTableCell>> table;
    vector<string> cols;
    buildParseTable(g, FIRST, FOLLOW, table, cols);

    // quick conflict check
    bool hasConflict = false;
    for (auto &A : g.nonterminals)
        for (auto &t : cols)
            if (table[A][t].conflict) hasConflict = true;

    if (hasConflict) {
        cout << "\nCannot safely run predictive parser: table has conflicts (not LL(1)).\n";
        return;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "\nEnter input string (default: id+id*id). Just press Enter to use default:\n> ";
    string s;
    getline(cin, s);
    if (trim(s).empty()) s = "id+id*id";

    bool ok = predictiveParse(g, table, s, true);
    cout << "\nRESULT: " << (ok ? "String ACCEPTED" : "String REJECTED") << "\n";
}

// ----------------------------- Main -----------------------------
int main() {
    ios::sync_with_stdio(false);
    cin.tie(&cout);

    while (true) {
        cout << "\n================ MINI COMPILER LAB SUITE ================\n";
        cout << "1) Case 01: Remove comments (// and /* */)\n";
        cout << "2) Case 02: Identify tokens from C code\n";
        cout << "3) Case 03: Left factoring for a CFG\n";
        cout << "4) Case 04: Left recursion elimination\n";
        cout << "5) Case 05: FIRST and FOLLOW\n";
        cout << "6) Case 06: LL(1) parsing table\n";
        cout << "7) Case 07: Predictive parser (id+id*id)\n";
        cout << "0) Exit\n";
        cout << "Choose: ";
        cout.flush();

        int op;
        if (!(cin >> op)) break;

        switch (op) {
            case 1: Case01(); break;
            case 2: Case02(); break;
            case 3: Case03(); break;
            case 4: Case04(); break;
            case 5: Case05(); break;
            case 6: Case06(); break;
            case 7: Case07(); break;
            case 0: cout << "Bye!\n"; return 0;
            default: cout << "Invalid option.\n"; break;
        }
    }
    return 0;
}
