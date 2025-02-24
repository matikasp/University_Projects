# **Football Training - Finding the Largest Clique**

## **Problem Statement**
A football training session consists of \( n \) players (\( n \) is an even number). In each match, all players participate, with \( n/2 \) players on each team. The coach wants to organize the teams in such a way that every pair of players has a chance to play against each other at least once (i.e., they must be in opposite teams in at least one match).

However, in practice, some players (despite the coach’s efforts) always play in the same team in all matches. The largest subset of players who **always play in the same team** across all matches is called a **clique**.

The coach has already planned team lineups for the next \( m \) matches. Help him determine how far he is from achieving his goal by finding the size of the **largest clique** of players.

## **Input**
- The first line contains two integers \( n \) and \( m \) (\( 4 \leq n \leq 40,000 \), \( 1 \leq m \leq 50 \)), representing the number of players and the number of scheduled matches, respectively.
- Players are numbered from **1** to **n**.
- Each of the next \( m \) lines contains \( n \) distinct integers from the range **1 to n**, describing the team lineups for each match:
  - The **first \( n/2 \) numbers** represent the players in the first team.
  - The **last \( n/2 \) numbers** represent the players in the second team.

## **Output**
- Print a single integer representing the size of the **largest clique** of players who always play on the same team in all matches.
