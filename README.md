# Mini_Compiler

# Mini Compiler Lab Suite (Case-Based Menu)

A single-file C++ (CodeBlocks-ready) “Mini Compiler Lab Suite” that demonstrates common compiler-design lab tasks through an interactive console menu.

## Features (Cases)

* **Case 01**: Remove single-line (`//`) and multi-line (`/* */`) comments from C/C++ code
* **Case 02**: Identify tokens from C/C++ code (simple lexical analyzer)
* **Case 03**: Left Factoring for a CFG
* **Case 04**: Left Recursion Elimination (direct + indirect)
* **Case 05**: FIRST and FOLLOW sets (supports default grammar or user grammar + optional preprocessing)
* **Case 06**: LL(1) Parsing Table (supports default grammar or user grammar + optional preprocessing)
* **Case 07**: Predictive Parser (expression grammar) with step-by-step parsing trace

> **Epsilon format**: `eps` (also accepts `epsilon`, `@`, `ε` as input)

---

## How to Build & Run

### CodeBlocks

1. Create a new Console project (C++)
2. Paste the code into `main.cpp`
3. Build & Run

### g++ (optional)

```bash
g++ -std=c++17 -O2 main.cpp -o mini_compiler
./mini_compiler
```

---

## General Input Rules

* When asked for code input via **Paste mode**, you must end by typing:

  ```
  ###END###
  ```
* When entering a grammar:

  * Format: `A -> alpha1 | alpha2`
  * Use `eps` for epsilon
  * Example:

    ```
    E -> E + T | T
    T -> T * F | F
    F -> ( E ) | id
    ```

---

# Step-by-Step Examples (All Cases + All Subcases)

## Main Menu (what you see at start)

Sample output:

```text
================ MINI COMPILER LAB SUITE ================
1) Case 01: Remove comments (// and /* */)
2) Case 02: Identify tokens from C code
3) Case 03: Left factoring for a CFG
4) Case 04: Left recursion elimination
5) Case 05: FIRST and FOLLOW
6) Case 06: LL(1) parsing table
7) Case 07: Predictive parser (id+id*id)
0) Exit
Choose:
```

---

## CASE 01 — Remove Comments

### Subcase 01-A: Paste input (recommended for testing)

**Step-by-step input**

```text
Choose: 1

Choose input method:
1) Read from file path
2) Paste text (end with a single line: ###END###)
Enter choice: 2
Paste now. Finish by typing: ###END### on its own line.
int main(){ // hello
  /* multi
     line */
  int a=5;  // ok
  printf("/* not a comment inside string */");
}
###END###
```

**Sample output**

```text
[Lab 01] Comment Removal

--- Code (Comments Removed) ---
int main(){
  
  int a=5;  
  printf("/* not a comment inside string */");
}
```

---

### Subcase 01-B: Read from file path

**Step-by-step input**

```text
Choose: 1

Choose input method:
1) Read from file path
2) Paste text (end with a single line: ###END###)
Enter choice: 1
Enter file path: D:\test\input.c
```

**Sample output (depends on file contents)**

```text
[Lab 01] Comment Removal

--- Code (Comments Removed) ---
<your file content with comments removed prints here>
```

---

## CASE 02 — Token Identification

### Subcase 02-A: Paste input

**Step-by-step input**

```text
Choose: 2

Choose input method:
1) Read from file path
2) Paste text (end with a single line: ###END###)
Enter choice: 2
Paste now. Finish by typing: ###END### on its own line.
#include <stdio.h>
int main(){
  int x = 10;
  if(x>5) x++;
  return 0;
}
###END###
```

**Sample output (token list)**

```text
[Lab 02] Token Identification

--- Tokens ---
Line  Type              Lexeme
------------------------------------------------------------
1     PREPROCESSOR      #include <stdio.h>
2     KEYWORD           int
2     IDENTIFIER        main
2     SEPARATOR         (
2     SEPARATOR         )
2     SEPARATOR         {
3     KEYWORD           int
3     IDENTIFIER        x
3     OPERATOR          =
3     NUMBER            10
3     SEPARATOR         ;
4     KEYWORD           if
4     SEPARATOR         (
4     IDENTIFIER        x
4     OPERATOR          >
4     NUMBER            5
4     SEPARATOR         )
4     IDENTIFIER        x
4     OPERATOR          ++
4     SEPARATOR         ;
5     KEYWORD           return
5     NUMBER            0
5     SEPARATOR         ;
6     SEPARATOR         }
```

---

### Subcase 02-B: Read from file path

**Step-by-step input**

```text
Choose: 2

Choose input method:
1) Read from file path
2) Paste text (end with a single line: ###END###)
Enter choice: 1
Enter file path: D:\test\code.c
```

**Sample output**

```text
[Lab 02] Token Identification
--- Tokens ---
<token table prints here>
```

---

## CASE 03 — Left Factoring for a CFG

### Subcase 03-A: Use default example (if-then-else)

**Step-by-step input**

```text
Choose: 3
[Lab 03] Left Factoring
1) Use default example (if-then-else)
2) Enter your own grammar
Choice: 1
```

**Sample output (before and after)**

```text
Before Left Factoring:

--- Grammar ---
Start symbol: S
E -> b
S -> i E t S | i E t S e S | a
NonTerminals: E S
Terminals: a b e i t
--------------

After Left Factoring:

--- Grammar ---
Start symbol: S
E -> b
S -> a | i E t S S'
S' -> eps | e S
NonTerminals: E S S'
Terminals: a b e i t
--------------
```

---

### Subcase 03-B: Enter your own grammar

**Step-by-step input**

```text
Choose: 3
[Lab 03] Left Factoring
1) Use default example (if-then-else)
2) Enter your own grammar
Choice: 2

Enter number of production lines: 2
Enter productions in format: A -> alpha1 | alpha2
Use epsilon as: eps (or epsilon/@/ε)
A -> a b c | a b d | x
B -> y
```

**Sample output**

```text
Before Left Factoring:

--- Grammar ---
Start symbol: A
A -> a b c | a b d | x
B -> y
NonTerminals: A B
Terminals: a b c d x y
--------------

After Left Factoring:

--- Grammar ---
Start symbol: A
A -> x | a b A'
A' -> c | d
B -> y
NonTerminals: A A' B
Terminals: a b c d x y
--------------
```

---

## CASE 04 — Left Recursion Elimination

### Subcase 04-A: Default expression grammar

**Step-by-step input**

```text
Choose: 4
[Lab 05] Left Recursion Elimination
1) Use default expression grammar (left-recursive)
2) Enter your own grammar
Choice: 1
```

**Sample output**

```text
Before elimination:

--- Grammar ---
Start symbol: E
E -> E + T | T
F -> ( E ) | id
T -> T * F | F
NonTerminals: E F T
Terminals: ( ) * + id
--------------

After elimination:

--- Grammar ---
Start symbol: E
E -> T E'
E' -> + T E' | eps
F -> ( E ) | id
T -> F T'
T' -> * F T' | eps
NonTerminals: E E' F T T'
Terminals: ( ) * + id
--------------
```

---

### Subcase 04-B: Enter your own grammar (direct left recursion example)

**Step-by-step input**

```text
Choose: 4
[Lab 05] Left Recursion Elimination
1) Use default expression grammar (left-recursive)
2) Enter your own grammar
Choice: 2

Enter number of production lines: 1
Enter productions in format: A -> alpha1 | alpha2
Use epsilon as: eps (or epsilon/@/ε)
A -> A a | b
```

**Sample output**

```text
Before elimination:

--- Grammar ---
Start symbol: A
A -> A a | b
NonTerminals: A
Terminals: a b
--------------

After elimination:

--- Grammar ---
Start symbol: A
A -> b A'
A' -> a A' | eps
NonTerminals: A A'
Terminals: a b
--------------
```

---

## CASE 05 — FIRST and FOLLOW

### Subcases inside Case 05

You will get **two prompts**:

1. Grammar source:

* `1` Default expression grammar (E,T,F)
* `2` Enter your own grammar

2. Preprocess decision:

* `1` Yes (Eliminate Left Recursion + Left Factoring)
* `2` No (use grammar as entered)

---

### Subcase 05-A1: Default grammar + Preprocess YES (recommended)

**Step-by-step input**

```text
Choose: 5

[Lab 06 & 07: FIRST and FOLLOW] Choose grammar source:
1) Use default expression grammar (E,T,F)
2) Enter your own grammar
Choice: 1

Preprocess grammar before calculation?
1) Yes (Eliminate Left Recursion + Left Factoring)  [Recommended]
2) No  (Use grammar as entered)
Choice: 1
```

**Sample output (FIRST/FOLLOW)**

```text
--- Grammar ---
Start symbol: E
E -> T E'
E' -> + T E' | eps
F -> ( E ) | id
T -> F T'
T' -> * F T' | eps
NonTerminals: E E' F T T'
Terminals: ( ) * + id
--------------

--- FIRST sets ---
FIRST(E) = { ( id }
FIRST(E') = { + eps }
FIRST(F) = { ( id }
FIRST(T) = { ( id }
FIRST(T') = { * eps }

--- FOLLOW sets ---
FOLLOW(E) = { ) $ }
FOLLOW(E') = { ) $ }
FOLLOW(F) = { * + ) $ }
FOLLOW(T) = { + ) $ }
FOLLOW(T') = { + ) $ }
```

---

### Subcase 05-A2: Default grammar + Preprocess NO

**Step-by-step input**

```text
Choose: 5
Choice: 1

Preprocess grammar before calculation?
1) Yes (Eliminate Left Recursion + Left Factoring)  [Recommended]
2) No  (Use grammar as entered)
Choice: 2
```

**Sample output note**

* It will still print FIRST/FOLLOW, but results can be confusing if grammar is left-recursive.
* Output will still be produced, but **recommended is preprocess YES**.

---

### Subcase 05-B1: User grammar + Preprocess YES

**Step-by-step input**

```text
Choose: 5

[Lab 06 & 07: FIRST and FOLLOW] Choose grammar source:
1) Use default expression grammar (E,T,F)
2) Enter your own grammar
Choice: 2

Enter number of production lines: 3
Enter productions in format: A -> alpha1 | alpha2
Use epsilon as: eps (or epsilon/@/ε)
S -> A B
A -> a | eps
B -> b

Preprocess grammar before calculation?
1) Yes (Eliminate Left Recursion + Left Factoring)  [Recommended]
2) No  (Use grammar as entered)
Choice: 1
```

**Sample output**

```text
--- Grammar ---
Start symbol: S
A -> a | eps
B -> b
S -> A B
NonTerminals: A B S
Terminals: a b
--------------

--- FIRST sets ---
FIRST(A) = { a eps }
FIRST(B) = { b }
FIRST(S) = { a b }

--- FOLLOW sets ---
FOLLOW(A) = { b }
FOLLOW(B) = { $ }
FOLLOW(S) = { $ }
```

---

### Subcase 05-B2: User grammar + Preprocess NO

Same grammar input as above, but choose preprocess option `2`. Output will reflect the grammar exactly as entered.

---

## CASE 06 — LL(1) Parsing Table

### Subcases inside Case 06

Same two-step choices as Case 05:

1. Grammar source (default / user)
2. Preprocess (YES / NO)

---

### Subcase 06-A1: Default grammar + Preprocess YES (recommended)

**Step-by-step input**

```text
Choose: 6

[Lab 08: LL(1) Parsing Table] Choose grammar source:
1) Use default expression grammar (E,T,F)
2) Enter your own grammar
Choice: 1

Preprocess grammar before calculation?
1) Yes (Eliminate Left Recursion + Left Factoring)  [Recommended]
2) No  (Use grammar as entered)
Choice: 1
```

**Sample output (table)**

```text
--- Grammar ---
Start symbol: E
E -> T E'
E' -> + T E' | eps
F -> ( E ) | id
T -> F T'
T' -> * F T' | eps
NonTerminals: E E' F T T'
Terminals: ( ) * + id
--------------

--- LL(1) Parsing Table ---
     NT\T           $           (           )           *           +          id
         E           .     E->T E'          .           .           .     E->T E'
        E'     E'->eps           .     E'->eps          .   E'->+ T..        .
         F           .    F->( E )         .           .           .      F->id
         T           .     T->F T'         .           .           .     T->F T'
        T'     T'->eps          .     T'->eps   T'->* F..   T'->eps          .

No conflicts detected. Grammar looks LL(1).
```

> The program truncates long productions in the table cells with `..`. That is normal.

---

### Subcase 06-A2: Default grammar + Preprocess NO

Same as above, but choose preprocess `2`. It may produce conflicts or unexpected table entries if grammar is not LL(1).

---

### Subcase 06-B1: User grammar + Preprocess YES (simple LL(1) grammar)

**Step-by-step input**

```text
Choose: 6
Choice: 2

Enter number of production lines: 2
Enter productions in format: A -> alpha1 | alpha2
Use epsilon as: eps (or epsilon/@/ε)
S -> a S | eps
```

Now preprocess choice:

```text
Preprocess grammar before calculation?
1) Yes (Eliminate Left Recursion + Left Factoring)  [Recommended]
2) No  (Use grammar as entered)
Choice: 2
```

**Sample output**

```text
--- LL(1) Parsing Table ---
<prints a table for S with terminals {a, $} and rules accordingly>
```

---

## CASE 07 — Predictive Parser (Expression Grammar)

### Subcases inside Case 07

* Input string:

  * Press **Enter** for default: `id+id*id`
  * Or type your own string (e.g., `id*(id+id)`)

---

### Subcase 07-A: Default input (just press Enter)

**Step-by-step input**

```text
Choose: 7

Enter input string (default: id+id*id). Just press Enter to use default:
>
```

**Sample output (parsing steps)**

```text
--- Predictive Parsing Steps ---
STACK                         INPUT                              ACTION
--------------------------------------------------------------------------------
$ E                           id + id * id $                     E -> T E'
$ E' T                        id + id * id $                     T -> F T'
$ E' T' F                     id + id * id $                     F -> id
$ E' T' id                    id + id * id $                     match id
$ E' T'                       + id * id $                        T' -> eps
$ E'                          + id * id $                        E' -> + T E'
$ E' T +                      + id * id $                        match +
$ E' T                        id * id $                          T -> F T'
$ E' T' F                     id * id $                          F -> id
$ E' T' id                    id * id $                          match id
$ E' T'                       * id $                             T' -> * F T'
$ E' T' F *                   * id $                             match *
$ E' T' F                     id $                               F -> id
$ E' T' id                    id $                               match id
$ E' T'                       $                                  T' -> eps
$ E'                          $                                  E' -> eps
$                             $                                  ACCEPT

RESULT: String ACCEPTED
```

---

### Subcase 07-B: Custom accepted input

**Step-by-step input**

```text
Choose: 7
> id*(id+id)
```

**Sample output**

* You will see a similar step table and end with:

```text
RESULT: String ACCEPTED
```

---

### Subcase 07-C: Custom rejected input

**Step-by-step input**

```text
Choose: 7
> id+*id
```

**Sample output**

* It will fail at some point and end with:

```text
RESULT: String REJECTED
```

---

## Exit (Case 0)

**Input**

```text
Choose: 0
```

**Output**

```text
Bye!
```

---

## Notes / Tips

* For **Case 05 & Case 06**, always choose **Preprocess = YES** unless you are intentionally testing raw grammars.
* If your grammar is not LL(1), **Case 06** may show conflicts and **Case 07** (predictive parsing) is not guaranteed for that grammar (Case 07 uses the expression grammar internally).

---

 
