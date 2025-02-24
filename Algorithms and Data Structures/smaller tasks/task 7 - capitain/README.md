# **Minimizing Captain's Steering Time in the Bajtockie Sea**

## **Problem Statement**
Captain Bajtazar is sailing across the Bajtockie Sea with his first officer, Bajtek. There are **n** islands numbered from **1 to n**. The captain's ship is currently docked at **island 1**, and he plans to sail to **island n**.

The ship can move in one of **four cardinal directions**: **north, south, east, or west**. At any moment, either the **captain** or the **first officer** is steering the ship. However, every time the ship makes a **90-degree turn**, control of the ship switches between them.

During the voyage, the ship can stop at other islands. After each stop, the captain can decide who will **start steering**. However, once the ship begins moving in a straight line, one sailor is in control for that entire segment:
- **One sailor steers when sailing north or south.**
- **The other steers when sailing east or west.**

The captain wants to **minimize the total time he spends steering the ship**. The length of the chosen route does not matter to him. The ship always sails at a constant speed of **one unit per hour**.

## **Input**
- The first line contains an integer \( n \) (\( 2 \leq n \leq 200,000 \)): the number of islands.
- The next **n** lines each contain two integers \( x_i, y_i \) (\( 0 \leq x_i, y_i \leq 1,000,000,000 \)), representing the coordinates of the **i-th** island.
- Each island has unique coordinates.

## **Output**
- Print a **single integer**: the **minimum number of hours the captain must spend steering** on the journey from **island 1 to island n**.
