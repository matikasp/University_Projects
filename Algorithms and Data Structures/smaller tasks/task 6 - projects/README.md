# **Minimizing Programmers Needed for Project Execution**

## **Problem Statement**
Bajtazar has been promoted to the head of the IT department at a Very Important State Institution. His job includes managing IT projects. The institution has prepared a list of potential projects, but some projects depend on the successful completion of others. 

Each project also requires a **minimum number of programmers** to complete it.

Due to budget cuts, not all projects can be completed. The management has decided that only **k** projects will be executed. Bajtazar must determine the **minimum number of programmers** required to complete at least **k** projects. Since projects can be completed sequentially, programmers can be reassigned from one project to another.

Your task is to write a program that helps Bajtazar determine the **minimum number of programmers** required to complete at least **k** projects.

## **Input**
- The first line contains three integers \( n, m, k \) (\( 1 \leq n \leq 100,000 \), \( 0 \leq m \leq 500,000 \), \( 0 \leq k \leq n \)):
  - \( n \) — the number of projects.
  - \( m \) — the number of dependencies between projects.
  - \( k \) — the minimum number of projects that must be completed.

- The next \( n \) lines contain an integer \( p_i \) (\( 1 \leq p_i \leq 100,000,000 \)), where \( p_i \) represents the **number of programmers required** to complete the **i-th** project.

- The next \( m \) lines contain two integers \( a, b \) (\( 1 \leq a, b \leq n \), \( a \neq b \)), meaning that **project \( a \) depends on project \( b \)** and cannot be completed before it.

- The dependencies do **not** form cycles.

## **Output**
- Print a single integer: the **minimum number of programmers** required to complete at least **k** projects.
