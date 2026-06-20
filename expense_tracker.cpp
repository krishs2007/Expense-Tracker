/*
 * expense_tracker.cpp - Expense Tracker System (C++)
 * Nirma University | Data Structures (2CS501CC23)
 * Submitted by: Satvik Moradiya (24BCE265), Krish Sonani (24BCE266)
 *
 * Data Structures Used:
 *   - Linked List : Dynamic expense storage
 *   - Array       : Monthly report summaries
 *   - Stack       : Recent action history (LIFO)
 *
 * Edge Cases Fixed/Added:
 *   [INPUT]
 *   - Amount: "3-913" or "3abc" treated as invalid (not just negative check)
 *   - Amount: entire field must be a clean number, no partial reads accepted
 *   - Date: day/month/year each validated individually with re-prompt on failure
 *   - Date: future date detected and warned (expense still allowed with confirmation)
 *   - Date: per-month day limits enforced (Feb=28/29, Apr/Jun/Sep/Nov=30)
 *   - Date: leap year check for Feb 29
 *   - Category / Description: whitespace-only input rejected
 *   - All fields: re-prompt instead of aborting on bad input
 *
 *   [LIST]
 *   - Delete on empty list
 *   - Delete non-existent ID
 *   - Category search with no matches
 *
 *   [BUDGET]
 *   - Budget check when none set
 *   - Budget array full guard
 *
 *   [OTHER]
 *   - Stack overflow (capped + shift)
 *   - Invalid menu input (non-integer, out-of-range)
 *   - CSV export on empty list
 *   - All memory freed on exit
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <ctime>
#include <cstdlib>

using namespace std;

// ─── Constants ────────────────────────────────────────────────
const int MAX_HISTORY    = 20;
const int MAX_CATEGORIES = 10;

// ─── Linked List Node ─────────────────────────────────────────
struct ExpenseNode
{
    int         id;
    int         day, month, year;
    string      category;
    float       amount;
    string      description;
    ExpenseNode *next;

    ExpenseNode() : id(0), day(0), month(0), year(0), amount(0.0f), next(nullptr) {}
};

// ─── Stack ────────────────────────────────────────────────────
struct Stack
{
    string actions[MAX_HISTORY];
    int    top = -1;

    void push(const string &action)
    {
        if (top < MAX_HISTORY - 1)
            actions[++top] = action;
        else
        {
            for (int i = 0; i < MAX_HISTORY - 1; i++)
                actions[i] = actions[i + 1];
            actions[MAX_HISTORY - 1] = action;
        }
    }

    void show() const
    {
        if (top < 0) { cout << "No actions recorded yet.\n"; return; }
        cout << "\n--- Action History (most recent first) ---\n";
        for (int i = top; i >= 0; i--)
            cout << "  [" << setw(2) << (top - i + 1) << "] " << actions[i] << "\n";
    }
};

// ─── Budget ───────────────────────────────────────────────────
struct Budget
{
    string category;
    float  limit = 0.0f;
};

// ─── Globals ──────────────────────────────────────────────────
static ExpenseNode *head        = nullptr;
static int          nextId      = 1;
static Stack        history;
static Budget       budgets[MAX_CATEGORIES];
static int          budgetCount = 0;

// ══════════════════════════════════════════════════════════════
//                      HELPER FUNCTIONS
// ══════════════════════════════════════════════════════════════

string toLower(const string &s)
{
    string r = s; for (char &c : r) c = tolower((unsigned char)c); return r;
}

bool iequal(const string &a, const string &b) { return toLower(a) == toLower(b); }

void clearInput() { cin.clear(); cin.ignore(1000, '\n'); }

// Returns true if string is blank or whitespace-only
bool isBlank(const string &s)
{
    return s.find_first_not_of(" \t\r\n") == string::npos;
}

// ── Safe integer read: reads entire line, rejects "3-913", "3abc", etc. ──
bool readInt(const string &prompt, int &out)
{
    while (true)
    {
        cout << prompt;
        string line;
        if (!getline(cin, line))  return false;
        if (isBlank(line))        { cout << "  Input cannot be empty. Try again.\n"; continue; }

        istringstream ss(line);
        int val;
        char leftover;
        if (!(ss >> val))         { cout << "  Invalid input (not a number). Try again.\n"; continue; }
        if (ss >> leftover)       { cout << "  Invalid input (extra characters). Try again.\n"; continue; }

        out = val;
        return true;
    }
}

// ── Safe float read: rejects "3-913", "3abc", "-911", etc. ──
bool readFloat(const string &prompt, float &out)
{
    while (true)
    {
        cout << prompt;
        string line;
        if (!getline(cin, line))  return false;
        if (isBlank(line))        { cout << "  Input cannot be empty. Try again.\n"; continue; }

        istringstream ss(line);
        float val;
        char leftover;
        if (!(ss >> val))         { cout << "  Invalid input (not a number). Try again.\n"; continue; }
        if (ss >> leftover)       { cout << "  Invalid input (extra characters like '3-913'). Try again.\n"; continue; }

        out = val;
        return true;
    }
}

// ── Safe string read: rejects blank / whitespace-only ──
bool readString(const string &prompt, string &out)
{
    while (true)
    {
        cout << prompt;
        if (!getline(cin, out)) return false;
        if (!isBlank(out))      return true;
        cout << "  Input cannot be empty or whitespace. Try again.\n";
    }
}

// ── Leap year check ──
bool isLeapYear(int y)
{
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

// ── Max days in a given month/year ──
int daysInMonth(int m, int y)
{
    const int days[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if (m == 2 && isLeapYear(y)) return 29;
    return days[m];
}

// ── Get today's date from system ──
void getToday(int &d, int &mo, int &y)
{
    time_t t = time(nullptr);
    struct tm *now = localtime(&t);
    d  = now->tm_mday;
    mo = now->tm_mon + 1;
    y  = now->tm_year + 1900;
}

// ── Compare two dates: returns -1 (past), 0 (today), 1 (future) ──
int compareDateToToday(int d, int m, int y)
{
    int td, tm, ty;
    getToday(td, tm, ty);
    if (y != ty) return (y < ty) ? -1 : 1;
    if (m != tm) return (m < tm) ? -1 : 1;
    if (d != td) return (d < td) ? -1 : 1;
    return 0;
}

// ── Read and fully validate a date, with future-date warning ──
bool readDate(int &day, int &month, int &year)
{
    while (true)
    {
        // --- Day ---
        if (!readInt("  Day   (1-31)  : ", day)) return false;
        if (day < 1 || day > 31)
        {
            cout << "  Day must be between 1 and 31. Try again.\n";
            continue;
        }

        // --- Month ---
        if (!readInt("  Month (1-12)  : ", month)) return false;
        if (month < 1 || month > 12)
        {
            cout << "  Month must be between 1 and 12. Try again.\n";
            continue;
        }

        // --- Year ---
        if (!readInt("  Year  (>=1900): ", year)) return false;
        if (year < 1900)
        {
            cout << "  Year must be 1900 or later. Try again.\n";
            continue;
        }

        // --- Per-month day validation ---
        int maxDay = daysInMonth(month, year);
        if (day > maxDay)
        {
            cout << "  " << month << "/" << year
                 << " only has " << maxDay << " days. Try again.\n";
            continue;
        }

        // --- Future date warning ---
        int cmp = compareDateToToday(day, month, year);
        if (cmp == 1)
        {
            cout << "  Warning: " << day << "/" << month << "/" << year
                 << " is a FUTURE date.\n";
            cout << "  Are you sure you want to log a future expense? (y/n): ";
            string confirm;
            getline(cin, confirm);
            if (toLower(confirm) != "y" && toLower(confirm) != "yes")
            {
                cout << "  Date rejected. Please enter a different date.\n";
                continue;
            }
        }

        return true;  // Date is valid (and confirmed if future)
    }
}

// ══════════════════════════════════════════════════════════════
//                    LINKED LIST OPERATIONS
// ══════════════════════════════════════════════════════════════

void addExpense()
{
    ExpenseNode *node = new ExpenseNode();
    node->id = nextId;

    // ── Date ──
    cout << "Enter Date:\n";
    if (!readDate(node->day, node->month, node->year))
    {
        delete node; return;
    }

    // ── Category ──
    if (!readString("Enter Category    : ", node->category))
    {
        delete node; return;
    }

    // ── Amount ──
    while (true)
    {
        float amt;
        if (!readFloat("Enter Amount (> 0): ", amt)) { delete node; return; }
        if (amt <= 0)
        {
            cout << "  Amount must be greater than 0. Try again.\n";
            continue;
        }
        node->amount = amt;
        break;
    }

    // ── Description ──
    if (!readString("Enter Description : ", node->description))
    {
        delete node; return;
    }

    // ── Append to linked list ──
    if (!head)
        head = node;
    else
    {
        ExpenseNode *cur = head;
        while (cur->next) cur = cur->next;
        cur->next = node;
    }

    nextId++;
    history.push("Added ID " + to_string(node->id) + ": "
                 + node->category + " - " + to_string(node->amount));
    cout << "Successfully added expense entry! (ID: " << node->id << ")\n";
}

void removeExpense()
{
    if (!head) { cout << "No expenses to remove.\n"; return; }

    int id;
    if (!readInt("Enter Expense ID to remove: ", id) || id <= 0)
    {
        cout << "Invalid ID.\n"; return;
    }

    ExpenseNode *cur = head, *prev = nullptr;
    while (cur && cur->id != id) { prev = cur; cur = cur->next; }

    if (!cur) { cout << "Expense with ID " << id << " not found.\n"; return; }

    if (!prev) head = cur->next;
    else       prev->next = cur->next;

    history.push("Removed ID " + to_string(cur->id) + ": "
                 + cur->category + " - " + to_string(cur->amount));
    cout << "Expense ID " << id << " removed successfully.\n";
    delete cur;
}

void viewAllExpenses()
{
    if (!head) { cout << "No expenses recorded.\n"; return; }

    cout << "\n" << left
         << setw(4)  << "ID"
         << setw(13) << "Date"
         << setw(16) << "Category"
         << setw(10) << "Amount"
         << "Description\n"
         << string(65, '-') << "\n";

    ExpenseNode *cur = head;
    float total = 0;
    while (cur)
    {
        // Zero-pad date display
        ostringstream date;
        date << setfill('0') << setw(2) << cur->day << "/"
             << setw(2) << cur->month << "/" << cur->year;

        cout << left << setfill(' ')
             << setw(4)  << cur->id
             << setw(13) << date.str()
             << setw(16) << cur->category
             << setw(10) << fixed << setprecision(2) << cur->amount
             << cur->description << "\n";
        total += cur->amount;
        cur = cur->next;
    }
    cout << string(65, '-') << "\n";
    cout << "Total: " << fixed << setprecision(2) << total << "\n";
}

void getByCategory()
{
    string cat;
    if (!readString("Enter Category to filter: ", cat)) return;

    int found = 0;
    cout << "\n--- Expenses in category: " << cat << " ---\n";
    ExpenseNode *cur = head;
    while (cur)
    {
        if (iequal(cur->category, cat))
        {
            cout << "ID: " << cur->id
                 << " | " << setfill('0') << setw(2) << cur->day << "/"
                 << setw(2) << cur->month << "/" << setfill(' ') << cur->year
                 << " | " << fixed << setprecision(2) << cur->amount
                 << " | " << cur->description << "\n";
            found++;
        }
        cur = cur->next;
    }
    if (!found) cout << "No expenses found in category '" << cat << "'.\n";
    else        cout << "Total entries: " << found << "\n";
}

// ══════════════════════════════════════════════════════════════
//                   ARRAY: MONTHLY REPORT
// ══════════════════════════════════════════════════════════════

void generateMonthlyReport()
{
    if (!head) { cout << "No expenses to report.\n"; return; }

    const string monthNames[] = {
        "", "Jan","Feb","Mar","Apr","May","Jun",
        "Jul","Aug","Sep","Oct","Nov","Dec"
    };
    float monthly[13] = {0};

    ExpenseNode *cur = head;
    while (cur)
    {
        if (cur->month >= 1 && cur->month <= 12)
            monthly[cur->month] += cur->amount;
        cur = cur->next;
    }

    float maxVal = *max_element(monthly + 1, monthly + 13);

    cout << "\n--- Monthly Expense Report ---\n";
    cout << left << setw(6) << "Month" << setw(10) << "Amount" << "Graph\n";
    cout << string(40, '-') << "\n";

    for (int m = 1; m <= 12; m++)
    {
        if (monthly[m] > 0)
        {
            int bars = (maxVal > 0) ? (int)((monthly[m] / maxVal) * 20) : 0;
            cout << left << setw(6) << monthNames[m]
                 << setw(10) << fixed << setprecision(2) << monthly[m]
                 << string(bars, '#') << "\n";
        }
    }
}

// ══════════════════════════════════════════════════════════════
//                     BUDGET MANAGEMENT
// ══════════════════════════════════════════════════════════════

void setBudget()
{
    string cat;
    if (!readString("Enter Category: ", cat)) return;

    float limit;
    while (true)
    {
        if (!readFloat("Enter Budget Limit (> 0): ", limit)) return;
        if (limit > 0) break;
        cout << "  Limit must be greater than 0. Try again.\n";
    }

    for (int i = 0; i < budgetCount; i++)
    {
        if (iequal(budgets[i].category, cat))
        {
            budgets[i].limit = limit;
            cout << "Budget updated for '" << cat << "': "
                 << fixed << setprecision(2) << limit << "\n";
            history.push("Updated budget for " + cat + ": " + to_string(limit));
            return;
        }
    }

    if (budgetCount >= MAX_CATEGORIES)
    {
        cout << "Maximum category budgets (" << MAX_CATEGORIES << ") reached.\n";
        return;
    }

    budgets[budgetCount].category = cat;
    budgets[budgetCount].limit    = limit;
    budgetCount++;
    history.push("Set budget for " + cat + ": " + to_string(limit));
    cout << "Budget set for '" << cat << "': " << fixed << setprecision(2) << limit << "\n";
}

void checkBudget()
{
    if (budgetCount == 0) { cout << "No budgets have been set yet.\n"; return; }

    cout << "\n--- Budget Status ---\n";
    cout << left << setw(16) << "Category"
         << setw(10) << "Budget"
         << setw(10) << "Spent"
         << setw(12) << "Remaining"
         << "Status\n"
         << string(58, '-') << "\n";

    for (int i = 0; i < budgetCount; i++)
    {
        float spent = 0;
        ExpenseNode *cur = head;
        while (cur)
        {
            if (iequal(cur->category, budgets[i].category))
                spent += cur->amount;
            cur = cur->next;
        }
        float remaining = budgets[i].limit - spent;
        string status   = (spent > budgets[i].limit) ? "!! OVER BUDGET !!" : "OK";

        cout << left  << setw(16) << budgets[i].category
             << setw(10) << fixed << setprecision(2) << budgets[i].limit
             << setw(10) << spent
             << setw(12) << remaining
             << status << "\n";
    }
}

// ══════════════════════════════════════════════════════════════
//                       EXPORT TO CSV
// ══════════════════════════════════════════════════════════════

void exportCSV()
{
    if (!head) { cout << "No expenses to export.\n"; return; }

    const string filename = "expenses.csv";
    ofstream file(filename);
    if (!file.is_open()) { cout << "Failed to create file '" << filename << "'.\n"; return; }

    file << "ID,Date,Category,Amount,Description\n";
    ExpenseNode *cur = head;
    while (cur)
    {
        file << cur->id << ","
             << setfill('0') << setw(2) << cur->day << "/"
             << setw(2) << cur->month << "/" << setfill(' ') << cur->year << ","
             << cur->category << ","
             << fixed << setprecision(2) << cur->amount << ","
             << cur->description << "\n";
        cur = cur->next;
    }
    file.close();
    history.push("Exported expenses to " + filename);
    cout << "Expenses exported to '" << filename << "' successfully.\n";
}

// ══════════════════════════════════════════════════════════════
//                      FREE ALL MEMORY
// ══════════════════════════════════════════════════════════════

void freeAll()
{
    ExpenseNode *cur = head;
    while (cur) { ExpenseNode *tmp = cur; cur = cur->next; delete tmp; }
    head = nullptr;
}

// ══════════════════════════════════════════════════════════════
//                          MAIN MENU
// ══════════════════════════════════════════════════════════════

void printMenu()
{
    cout << "\n========================================\n";
    cout << "       EXPENSE TRACKER SYSTEM\n";
    cout << "========================================\n";
    cout << "  1. Add Expense\n";
    cout << "  2. Remove Expense\n";
    cout << "  3. View All Expenses\n";
    cout << "  4. Get Expenses by Category\n";
    cout << "  5. Generate Monthly Report\n";
    cout << "  6. Set / Update Budget\n";
    cout << "  7. Check Budget Status\n";
    cout << "  8. View Action History\n";
    cout << "  9. Export to CSV\n";
    cout << "  0. Exit\n";
    cout << "========================================\n";
    cout << "Enter your choice: ";
}

int main()
{
    // Switch cin to line-mode so readInt/readFloat/readString work correctly
    while (true)
    {
        printMenu();

        int choice;
        if (!readInt("", choice))
        {
            cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        switch (choice)
        {
            case 1: addExpense();            break;
            case 2: removeExpense();         break;
            case 3: viewAllExpenses();       break;
            case 4: getByCategory();         break;
            case 5: generateMonthlyReport(); break;
            case 6: setBudget();             break;
            case 7: checkBudget();           break;
            case 8: history.show();          break;
            case 9: exportCSV();             break;
            case 0:
                cout << "Freeing memory and exiting. Goodbye!\n";
                freeAll();
                return 0;
            default:
                cout << "Invalid choice. Please enter 0-9.\n";
        }
    }

    return 0;
}
