# 💰 Expense Tracker System

A menu-driven **CLI application in C++** that tracks personal expenses using three core data structures: a **Linked List**, a **Stack**, and **Arrays**.

Built as a Data Structures assignment at **Nirma University** (2CS501CC23).

---

## 📁 Project Structure

```
Expense-Tracker/
├── expense_tracker.cpp   # Main source file
└── output/               # Compiled output
```

---

## 🗂️ Data Structures Used

| Data Structure | Purpose |
|---|---|
| **Linked List** | Dynamically store, add, and remove expense records |
| **Stack** | Maintain LIFO action history (last 20 actions) |
| **Array** | Monthly expense aggregation for reports |

---

## ✨ Features

| # | Feature |
|---|---|
| 1 | Add Expense |
| 2 | Remove Expense by ID |
| 3 | View All Expenses |
| 4 | Filter Expenses by Category |
| 5 | Generate Monthly Report (with bar graph) |
| 6 | Set / Update Category Budget |
| 7 | Check Budget Status (with OVER BUDGET alerts) |
| 8 | View Action History |
| 0 | Exit |

---

## 🛡️ Edge Cases Handled

- **Invalid amounts** — rejects partial inputs like `3-913`, negative values, and non-numeric strings with a re-prompt (no crash)
- **Date validation** — per-month day limits enforced (e.g. June has 30 days, not 31); Feb 28/29 with leap-year detection
- **Future date** — detected and prompts user confirmation before accepting
- **Whitespace input** — blank or space-only fields rejected with re-prompt
- **Empty list** — safe delete and view operations on an empty list
- **Non-existent ID** — graceful error message when removing an ID that doesn't exist
- **Budget overflow** — category budget array capped at 10 with a clear message
- **Memory cleanup** — all linked list nodes freed on exit

---

## 🚀 How to Run

### Compile
```bash
g++ -std=c++17 -o expense_tracker expense_tracker.cpp
```

### Run (Linux / Mac)
```bash
./expense_tracker
```

### Run (Windows)
```bash
expense_tracker.exe
```

---

## 🖥️ Sample Output

```
========================================
       EXPENSE TRACKER SYSTEM
========================================
  1. Add Expense
  2. Remove Expense
  3. View All Expenses
  4. Get Expenses by Category
  5. Generate Monthly Report
  6. Set / Update Budget
  7. Check Budget Status
  8. View Action History
  0. Exit
========================================
Enter your choice: 1

Enter Date:
  Day   (1-31)  : 31
  Month (1-12)  : 6
  Year  (>=1900): 2025
  6/2025 only has 30 days. Try again.
  Day   (1-31)  : 10
  Month (1-12)  : 6
  Year  (>=1900): 2025
Enter Category    : Food
Enter Amount (> 0): 3-913
  Invalid input (extra characters like '3-913'). Try again.
Enter Amount (> 0): 250
Enter Description : Lunch
Successfully added expense entry! (ID: 1)
```

---

## 👨‍💻 Authors

- **Satvik Moradiya** — 24BCE265
- **Krish Sonani** — 24BCE266

Nirma University, Institute of Technology, Ahmedabad
