# Bajtek and Grandmother's Market Shopping

## Problem Statement
Little Bajtek is spending his vacation at his grandmother Bajtula’s house. Every morning, his grandmother goes to the market to buy some products. The boy quickly noticed an interesting pattern: every day, his grandmother spends an amount of money that is an odd integer. Bajtek soon determined that this pattern is a characteristic of all Bajtock grandmothers.  

Each day, grandmother Bajtula buys at most one unit of each of the \( n \) products available at the market. Being cautious, she does not want to take too much money with her. One day, she asked Bajtek for advice on how much money she should bring if she wants to buy exactly \( k \) products that day. Unfortunately, Bajtek does not know which products his grandmother plans to buy, so the amount of money taken must be enough to buy any \( k \) products (in such a way that their total cost is odd).  

This situation repeated several times. Bajtek decided to take a systematic approach and write a program that, given the prices of all available products at the market, would answer his grandmother’s questions.  

## **Input**
- The first line of input contains a single integer \( n \) (\( 1 \leq n \leq 1,000,000 \)) representing the number of products available at the market.  
- The second line contains \( n \) integers in the range \([1, 10^9]\), representing the prices of the products. These numbers are given in non-decreasing order.  
- The third line contains a single integer \( m \) (\( 1 \leq m \leq 1,000,000 \)) representing the number of days Bajtek will still spend with his grandmother.  
- Each of the next \( m \) lines contains a single integer \( k_i \) (\( 1 \leq k_i \leq n \)), representing the number of products that the grandmother plans to buy on that day.  

## **Output**
Your program should print \( m \) lines.  
- In the \( i \)-th line (for \( i = 1, \dots, m \)), print a single integer representing the maximum odd total price for \( k_i \) products.  
- If it is not possible to choose \( k_i \) products whose total price is odd, print `-1` instead.  

## **Example**
### **Input**
4
1 2 3 4
3
2
3
4
### **Output**
7
9
-1