#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cstring>
#include <iomanip>
#include <sstream>

using namespace std;

// Constants
const int MAX_STRING_LEN = 65;
const string ACCOUNT_FILE = "accounts.dat";
const string BOOK_FILE = "books.dat";
const string FINANCE_FILE = "finance.dat";
const string LOG_FILE = "log.dat";

// Utility functions
bool isValidChar(char c, bool allowQuote = true) {
    if (c < 32 || c > 126) return false;
    if (!allowQuote && c == '"') return false;
    return true;
}

bool isValidUserID(const string& s) {
    if (s.empty() || s.length() > 30) return false;
    for (char c : s) {
        if (!isalnum(c) && c != '_') return false;
    }
    return true;
}

bool isValidPassword(const string& s) {
    return isValidUserID(s);
}

bool isValidUsername(const string& s) {
    if (s.empty() || s.length() > 30) return false;
    for (char c : s) {
        if (!isValidChar(c)) return false;
    }
    return true;
}

bool isValidISBN(const string& s) {
    if (s.empty() || s.length() > 20) return false;
    for (char c : s) {
        if (!isValidChar(c)) return false;
    }
    return true;
}

bool isValidBookName(const string& s) {
    if (s.empty() || s.length() > 60) return false;
    for (char c : s) {
        if (!isValidChar(c, false)) return false;
    }
    return true;
}

bool isValidKeyword(const string& s) {
    if (s.empty() || s.length() > 60) return false;
    vector<string> parts;
    string part;
    for (char c : s) {
        if (c == '|') {
            if (part.empty()) return false;
            parts.push_back(part);
            part.clear();
        } else {
            if (!isValidChar(c, false)) return false;
            part += c;
        }
    }
    if (part.empty()) return false;
    parts.push_back(part);
    
    // Check for duplicates
    set<string> unique(parts.begin(), parts.end());
    return unique.size() == parts.size();
}

bool isValidQuantity(const string& s) {
    if (s.empty() || s.length() > 10) return false;
    for (char c : s) {
        if (!isdigit(c)) return false;
    }
    return true;
}

bool isValidPrice(const string& s) {
    if (s.empty() || s.length() > 13) return false;
    bool hasDot = false;
    int beforeDot = 0, afterDot = 0;
    for (char c : s) {
        if (c == '.') {
            if (hasDot) return false;
            hasDot = true;
        } else if (isdigit(c)) {
            if (hasDot) afterDot++;
            else beforeDot++;
        } else {
            return false;
        }
    }
    if (beforeDot == 0) return false;  // Must have at least one digit before dot
    return !hasDot || afterDot <= 2;
}

// Data structures
struct Account {
    char userID[31];
    char password[31];
    char username[31];
    int privilege;
    
    Account() : privilege(0) {
        memset(userID, 0, sizeof(userID));
        memset(password, 0, sizeof(password));
        memset(username, 0, sizeof(username));
    }
};

struct Book {
    char ISBN[21];
    char name[61];
    char author[61];
    char keyword[61];
    double price;
    int quantity;
    
    Book() : price(0), quantity(0) {
        memset(ISBN, 0, sizeof(ISBN));
        memset(name, 0, sizeof(name));
        memset(author, 0, sizeof(author));
        memset(keyword, 0, sizeof(keyword));
    }
};

struct Transaction {
    double amount;
    bool isIncome;
};

// Account Manager
class AccountManager {
private:
    map<string, Account> accounts;
    
    void loadAccounts() {
        ifstream file(ACCOUNT_FILE, ios::binary);
        if (!file) return;
        
        int count;
        file.read((char*)&count, sizeof(count));
        for (int i = 0; i < count; i++) {
            Account acc;
            file.read((char*)&acc, sizeof(acc));
            accounts[acc.userID] = acc;
        }
        file.close();
    }
    
    void saveAccounts() {
        ofstream file(ACCOUNT_FILE, ios::binary);
        int count = accounts.size();
        file.write((char*)&count, sizeof(count));
        for (auto& p : accounts) {
            file.write((char*)&p.second, sizeof(Account));
        }
        file.close();
    }
    
public:
    AccountManager() {
        loadAccounts();
        if (accounts.find("root") == accounts.end()) {
            Account root;
            strcpy(root.userID, "root");
            strcpy(root.password, "sjtu");
            strcpy(root.username, "root");
            root.privilege = 7;
            accounts["root"] = root;
            saveAccounts();
        }
    }
    
    bool addAccount(const string& userID, const string& password, 
                   int privilege, const string& username) {
        if (accounts.find(userID) != accounts.end()) return false;
        
        Account acc;
        strcpy(acc.userID, userID.c_str());
        strcpy(acc.password, password.c_str());
        strcpy(acc.username, username.c_str());
        acc.privilege = privilege;
        accounts[userID] = acc;
        saveAccounts();
        return true;
    }
    
    bool deleteAccount(const string& userID) {
        if (accounts.find(userID) == accounts.end()) return false;
        accounts.erase(userID);
        saveAccounts();
        return true;
    }
    
    bool checkPassword(const string& userID, const string& password) {
        if (accounts.find(userID) == accounts.end()) return false;
        return accounts[userID].password == password;
    }
    
    bool changePassword(const string& userID, const string& newPassword) {
        if (accounts.find(userID) == accounts.end()) return false;
        strcpy(accounts[userID].password, newPassword.c_str());
        saveAccounts();
        return true;
    }
    
    int getPrivilege(const string& userID) {
        if (accounts.find(userID) == accounts.end()) return -1;
        return accounts[userID].privilege;
    }
    
    bool exists(const string& userID) {
        return accounts.find(userID) != accounts.end();
    }
};

// Book Manager
class BookManager {
private:
    map<string, Book> books;
    
    void loadBooks() {
        ifstream file(BOOK_FILE, ios::binary);
        if (!file) return;
        
        int count;
        file.read((char*)&count, sizeof(count));
        for (int i = 0; i < count; i++) {
            Book book;
            file.read((char*)&book, sizeof(book));
            books[book.ISBN] = book;
        }
        file.close();
    }
    
    void saveBooks() {
        ofstream file(BOOK_FILE, ios::binary);
        int count = books.size();
        file.write((char*)&count, sizeof(count));
        for (auto& p : books) {
            file.write((char*)&p.second, sizeof(Book));
        }
        file.close();
    }

public:
    BookManager() {
        loadBooks();
    }

    bool addBook(const string& ISBN) {
        if (books.find(ISBN) != books.end()) return false;
        Book book;
        strcpy(book.ISBN, ISBN.c_str());
        books[ISBN] = book;
        saveBooks();
        return true;
    }

    bool exists(const string& ISBN) {
        return books.find(ISBN) != books.end();
    }

    Book* getBook(const string& ISBN) {
        if (books.find(ISBN) == books.end()) return nullptr;
        return &books[ISBN];
    }

    void modifyBook(const string& ISBN, const string& newISBN,
                   const string& name, const string& author,
                   const string& keyword, double price) {
        if (books.find(ISBN) == books.end()) return;

        Book book = books[ISBN];
        if (!newISBN.empty() && newISBN != ISBN) {
            books.erase(ISBN);
            strcpy(book.ISBN, newISBN.c_str());
        }
        if (!name.empty()) strcpy(book.name, name.c_str());
        if (!author.empty()) strcpy(book.author, author.c_str());
        if (!keyword.empty()) strcpy(book.keyword, keyword.c_str());
        if (price >= 0) book.price = price;

        books[book.ISBN] = book;
        saveBooks();
    }

    void importBook(const string& ISBN, int quantity) {
        if (books.find(ISBN) == books.end()) return;
        books[ISBN].quantity += quantity;
        saveBooks();
    }

    bool buyBook(const string& ISBN, int quantity) {
        if (books.find(ISBN) == books.end()) return false;
        if (books[ISBN].quantity < quantity) return false;
        books[ISBN].quantity -= quantity;
        saveBooks();
        return true;
    }

    vector<Book> searchBooks(const string& type, const string& value) {
        vector<Book> result;

        for (auto& p : books) {
            bool match = false;
            if (type.empty()) {
                match = true;
            } else if (type == "ISBN") {
                match = (p.second.ISBN == value);
            } else if (type == "name") {
                match = (p.second.name == value);
            } else if (type == "author") {
                match = (p.second.author == value);
            } else if (type == "keyword") {
                string keywords = p.second.keyword;
                vector<string> parts;
                string part;
                for (char c : keywords) {
                    if (c == '|') {
                        parts.push_back(part);
                        part.clear();
                    } else {
                        part += c;
                    }
                }
                if (!part.empty()) parts.push_back(part);

                for (const string& kw : parts) {
                    if (kw == value) {
                        match = true;
                        break;
                    }
                }
            }

            if (match) result.push_back(p.second);
        }

        sort(result.begin(), result.end(), [](const Book& a, const Book& b) {
            return strcmp(a.ISBN, b.ISBN) < 0;
        });

        return result;
    }
};

// Finance Manager
class FinanceManager {
private:
    vector<Transaction> transactions;

    void loadTransactions() {
        ifstream file(FINANCE_FILE, ios::binary);
        if (!file) return;

        int count;
        file.read((char*)&count, sizeof(count));
        for (int i = 0; i < count; i++) {
            Transaction trans;
            file.read((char*)&trans, sizeof(trans));
            transactions.push_back(trans);
        }
        file.close();
    }

    void saveTransactions() {
        ofstream file(FINANCE_FILE, ios::binary);
        int count = transactions.size();
        file.write((char*)&count, sizeof(count));
        for (auto& trans : transactions) {
            file.write((char*)&trans, sizeof(trans));
        }
        file.close();
    }

public:
    FinanceManager() {
        loadTransactions();
    }

    void addTransaction(double amount, bool isIncome) {
        Transaction trans;
        trans.amount = amount;
        trans.isIncome = isIncome;
        transactions.push_back(trans);
        saveTransactions();
    }

    pair<double, double> getFinance(int count) {
        double income = 0, expenditure = 0;

        if (count == -1) count = transactions.size();

        int start = max(0, (int)transactions.size() - count);
        for (int i = start; i < transactions.size(); i++) {
            if (transactions[i].isIncome) {
                income += transactions[i].amount;
            } else {
                expenditure += transactions[i].amount;
            }
        }

        return {income, expenditure};
    }

    int getTransactionCount() {
        return transactions.size();
    }
};

// Main System
class BookstoreSystem {
private:
    AccountManager accountMgr;
    BookManager bookMgr;
    FinanceManager financeMgr;

    vector<string> loginStack;
    map<string, string> selectedBooks;
    set<string> loggedInUsers;

    int getCurrentPrivilege() {
        if (loginStack.empty()) return 0;
        return accountMgr.getPrivilege(loginStack.back());
    }

    string getCurrentUser() {
        if (loginStack.empty()) return "";
        return loginStack.back();
    }

    string trim(const string& s) {
        size_t start = 0, end = s.length();
        while (start < end && s[start] == ' ') start++;
        while (end > start && s[end-1] == ' ') end--;
        return s.substr(start, end - start);
    }

    vector<string> split(const string& s) {
        vector<string> result;
        string word;
        for (char c : s) {
            if (c == ' ') {
                if (!word.empty()) {
                    result.push_back(word);
                    word.clear();
                }
            } else {
                word += c;
            }
        }
        if (!word.empty()) result.push_back(word);
        return result;
    }

    void cmdSu(const vector<string>& args) {
        if (args.size() < 1 || args.size() > 2) {
            cout << "Invalid\n";
            return;
        }

        string userID = args[0];
        string password = args.size() == 2 ? args[1] : "";

        if (!isValidUserID(userID) || (!password.empty() && !isValidPassword(password))) {
            cout << "Invalid\n";
            return;
        }

        if (!accountMgr.exists(userID)) {
            cout << "Invalid\n";
            return;
        }

        int targetPrivilege = accountMgr.getPrivilege(userID);
        int currentPrivilege = getCurrentPrivilege();

        if (password.empty()) {
            if (currentPrivilege <= targetPrivilege) {
                cout << "Invalid\n";
                return;
            }
        } else {
            if (!accountMgr.checkPassword(userID, password)) {
                cout << "Invalid\n";
                return;
            }
        }

        loginStack.push_back(userID);
        loggedInUsers.insert(userID);
    }

    void cmdLogout() {
        if (loginStack.empty()) {
            cout << "Invalid\n";
            return;
        }

        string user = loginStack.back();
        loginStack.pop_back();
        selectedBooks.erase(user);

        // Check if user is still logged in
        bool stillLoggedIn = false;
        for (const string& u : loginStack) {
            if (u == user) {
                stillLoggedIn = true;
                break;
            }
        }
        if (!stillLoggedIn) {
            loggedInUsers.erase(user);
        }
    }

    void cmdRegister(const vector<string>& args) {
        if (args.size() != 3) {
            cout << "Invalid\n";
            return;
        }

        string userID = args[0];
        string password = args[1];
        string username = args[2];

        if (!isValidUserID(userID) || !isValidPassword(password) || !isValidUsername(username)) {
            cout << "Invalid\n";
            return;
        }

        if (!accountMgr.addAccount(userID, password, 1, username)) {
            cout << "Invalid\n";
        }
    }

    void cmdPasswd(const vector<string>& args) {
        if (getCurrentPrivilege() < 1) {
            cout << "Invalid\n";
            return;
        }

        if (args.size() < 2 || args.size() > 3) {
            cout << "Invalid\n";
            return;
        }

        string userID = args[0];
        string currentPassword = args.size() == 3 ? args[1] : "";
        string newPassword = args.size() == 3 ? args[2] : args[1];

        if (!isValidUserID(userID) || (!currentPassword.empty() && !isValidPassword(currentPassword))
            || !isValidPassword(newPassword)) {
            cout << "Invalid\n";
            return;
        }

        if (!accountMgr.exists(userID)) {
            cout << "Invalid\n";
            return;
        }

        if (currentPassword.empty()) {
            if (getCurrentPrivilege() != 7) {
                cout << "Invalid\n";
                return;
            }
        } else {
            if (!accountMgr.checkPassword(userID, currentPassword)) {
                cout << "Invalid\n";
                return;
            }
        }

        accountMgr.changePassword(userID, newPassword);
    }

    void cmdUseradd(const vector<string>& args) {
        if (getCurrentPrivilege() < 3) {
            cout << "Invalid\n";
            return;
        }

        if (args.size() != 4) {
            cout << "Invalid\n";
            return;
        }

        string userID = args[0];
        string password = args[1];
        string privilegeStr = args[2];
        string username = args[3];

        if (!isValidUserID(userID) || !isValidPassword(password) ||
            privilegeStr.length() != 1 || !isdigit(privilegeStr[0]) ||
            !isValidUsername(username)) {
            cout << "Invalid\n";
            return;
        }

        int privilege = privilegeStr[0] - '0';
        if (privilege != 1 && privilege != 3 && privilege != 7) {
            cout << "Invalid\n";
            return;
        }

        if (privilege >= getCurrentPrivilege()) {
            cout << "Invalid\n";
            return;
        }

        if (!accountMgr.addAccount(userID, password, privilege, username)) {
            cout << "Invalid\n";
        }
    }

    void cmdDelete(const vector<string>& args) {
        if (getCurrentPrivilege() < 7) {
            cout << "Invalid\n";
            return;
        }

        if (args.size() != 1) {
            cout << "Invalid\n";
            return;
        }

        string userID = args[0];

        if (!isValidUserID(userID)) {
            cout << "Invalid\n";
            return;
        }

        if (!accountMgr.exists(userID)) {
            cout << "Invalid\n";
            return;
        }

        if (loggedInUsers.find(userID) != loggedInUsers.end()) {
            cout << "Invalid\n";
            return;
        }

        accountMgr.deleteAccount(userID);
    }

    void cmdShow(const vector<string>& args) {
        if (getCurrentPrivilege() < 1) {
            cout << "Invalid\n";
            return;
        }

        string type, value;

        if (args.empty()) {
            type = "";
            value = "";
        } else if (args.size() == 1) {
            string arg = args[0];
            if (arg.substr(0, 6) == "-ISBN=") {
                type = "ISBN";
                value = arg.substr(6);
                if (value.empty() || !isValidISBN(value)) {
                    cout << "Invalid\n";
                    return;
                }
            } else if (arg.substr(0, 6) == "-name=") {
                if (arg.length() < 9 || arg[6] != '"' || arg.back() != '"') {
                    cout << "Invalid\n";
                    return;
                }
                type = "name";
                value = arg.substr(7, arg.length() - 8);
                if (value.empty() || !isValidBookName(value)) {
                    cout << "Invalid\n";
                    return;
                }
            } else if (arg.substr(0, 8) == "-author=") {
                if (arg.length() < 11 || arg[8] != '"' || arg.back() != '"') {
                    cout << "Invalid\n";
                    return;
                }
                type = "author";
                value = arg.substr(9, arg.length() - 10);
                if (value.empty() || !isValidBookName(value)) {
                    cout << "Invalid\n";
                    return;
                }
            } else if (arg.substr(0, 9) == "-keyword=") {
                if (arg.length() < 12 || arg[9] != '"' || arg.back() != '"') {
                    cout << "Invalid\n";
                    return;
                }
                type = "keyword";
                value = arg.substr(10, arg.length() - 11);
                if (value.empty() || !isValidBookName(value)) {
                    cout << "Invalid\n";
                    return;
                }
                // Check if keyword contains '|'
                if (value.find('|') != string::npos) {
                    cout << "Invalid\n";
                    return;
                }
            } else {
                cout << "Invalid\n";
                return;
            }
        } else {
            cout << "Invalid\n";
            return;
        }

        vector<Book> books = bookMgr.searchBooks(type, value);

        if (books.empty()) {
            cout << "\n";
        } else {
            for (const Book& book : books) {
                cout << book.ISBN << "\t" << book.name << "\t" << book.author << "\t"
                     << book.keyword << "\t" << fixed << setprecision(2) << book.price << "\t"
                     << book.quantity << "\n";
            }
        }
    }

    void cmdBuy(const vector<string>& args) {
        if (getCurrentPrivilege() < 1) {
            cout << "Invalid\n";
            return;
        }

        if (args.size() != 2) {
            cout << "Invalid\n";
            return;
        }

        string ISBN = args[0];
        string quantityStr = args[1];

        if (!isValidISBN(ISBN) || !isValidQuantity(quantityStr)) {
            cout << "Invalid\n";
            return;
        }

        long long quantity = stoll(quantityStr);
        if (quantity <= 0) {
            cout << "Invalid\n";
            return;
        }

        Book* book = bookMgr.getBook(ISBN);
        if (!book) {
            cout << "Invalid\n";
            return;
        }

        if (book->quantity < quantity) {
            cout << "Invalid\n";
            return;
        }

        double total = book->price * quantity;
        bookMgr.buyBook(ISBN, quantity);
        financeMgr.addTransaction(total, true);

        cout << fixed << setprecision(2) << total << "\n";
    }

    void cmdSelect(const vector<string>& args) {
        if (getCurrentPrivilege() < 3) {
            cout << "Invalid\n";
            return;
        }

        if (args.size() != 1) {
            cout << "Invalid\n";
            return;
        }

        string ISBN = args[0];

        if (!isValidISBN(ISBN)) {
            cout << "Invalid\n";
            return;
        }

        if (!bookMgr.exists(ISBN)) {
            bookMgr.addBook(ISBN);
        }

        selectedBooks[getCurrentUser()] = ISBN;
    }

    void cmdModify(const vector<string>& args) {
        if (getCurrentPrivilege() < 3) {
            cout << "Invalid\n";
            return;
        }

        string currentUser = getCurrentUser();
        if (selectedBooks.find(currentUser) == selectedBooks.end()) {
            cout << "Invalid\n";
            return;
        }

        string currentISBN = selectedBooks[currentUser];

        if (args.empty()) {
            cout << "Invalid\n";
            return;
        }

        string newISBN, name, author, keyword;
        double price = -1;
        set<string> usedParams;

        for (const string& arg : args) {
            string param;
            if (arg.substr(0, 6) == "-ISBN=") {
                param = "ISBN";
                if (usedParams.count(param)) {
                    cout << "Invalid\n";
                    return;
                }
                usedParams.insert(param);
                newISBN = arg.substr(6);
                if (newISBN.empty() || !isValidISBN(newISBN)) {
                    cout << "Invalid\n";
                    return;
                }
                if (newISBN == currentISBN) {
                    cout << "Invalid\n";
                    return;
                }
                if (bookMgr.exists(newISBN)) {
                    cout << "Invalid\n";
                    return;
                }
            } else if (arg.substr(0, 6) == "-name=") {
                param = "name";
                if (usedParams.count(param)) {
                    cout << "Invalid\n";
                    return;
                }
                usedParams.insert(param);
                if (arg.length() < 9 || arg[6] != '"' || arg.back() != '"') {
                    cout << "Invalid\n";
                    return;
                }
                name = arg.substr(7, arg.length() - 8);
                if (name.empty() || !isValidBookName(name)) {
                    cout << "Invalid\n";
                    return;
                }
            } else if (arg.substr(0, 8) == "-author=") {
                param = "author";
                if (usedParams.count(param)) {
                    cout << "Invalid\n";
                    return;
                }
                usedParams.insert(param);
                if (arg.length() < 11 || arg[8] != '"' || arg.back() != '"') {
                    cout << "Invalid\n";
                    return;
                }
                author = arg.substr(9, arg.length() - 10);
                if (author.empty() || !isValidBookName(author)) {
                    cout << "Invalid\n";
                    return;
                }
            } else if (arg.substr(0, 9) == "-keyword=") {
                param = "keyword";
                if (usedParams.count(param)) {
                    cout << "Invalid\n";
                    return;
                }
                usedParams.insert(param);
                if (arg.length() < 12 || arg[9] != '"' || arg.back() != '"') {
                    cout << "Invalid\n";
                    return;
                }
                keyword = arg.substr(10, arg.length() - 11);
                if (keyword.empty() || !isValidKeyword(keyword)) {
                    cout << "Invalid\n";
                    return;
                }
            } else if (arg.substr(0, 7) == "-price=") {
                param = "price";
                if (usedParams.count(param)) {
                    cout << "Invalid\n";
                    return;
                }
                usedParams.insert(param);
                string priceStr = arg.substr(7);
                if (priceStr.empty() || !isValidPrice(priceStr)) {
                    cout << "Invalid\n";
                    return;
                }
                price = stod(priceStr);
            } else {
                cout << "Invalid\n";
                return;
            }
        }

        bookMgr.modifyBook(currentISBN, newISBN, name, author, keyword, price);

        if (!newISBN.empty()) {
            selectedBooks[currentUser] = newISBN;
        }
    }

    void cmdImport(const vector<string>& args) {
        if (getCurrentPrivilege() < 3) {
            cout << "Invalid\n";
            return;
        }

        string currentUser = getCurrentUser();
        if (selectedBooks.find(currentUser) == selectedBooks.end()) {
            cout << "Invalid\n";
            return;
        }

        if (args.size() != 2) {
            cout << "Invalid\n";
            return;
        }

        string quantityStr = args[0];
        string totalCostStr = args[1];

        if (!isValidQuantity(quantityStr) || !isValidPrice(totalCostStr)) {
            cout << "Invalid\n";
            return;
        }

        long long quantity = stoll(quantityStr);
        double totalCost = stod(totalCostStr);

        if (quantity <= 0 || totalCost <= 0) {
            cout << "Invalid\n";
            return;
        }

        string ISBN = selectedBooks[currentUser];
        bookMgr.importBook(ISBN, quantity);
        financeMgr.addTransaction(totalCost, false);
    }

    void cmdShowFinance(const vector<string>& args) {
        if (getCurrentPrivilege() < 7) {
            cout << "Invalid\n";
            return;
        }

        int count = -1;

        if (!args.empty()) {
            if (args.size() != 1) {
                cout << "Invalid\n";
                return;
            }

            string countStr = args[0];
            if (!isValidQuantity(countStr)) {
                cout << "Invalid\n";
                return;
            }

            count = stoi(countStr);

            if (count == 0) {
                cout << "\n";
                return;
            }

            if (count > financeMgr.getTransactionCount()) {
                cout << "Invalid\n";
                return;
            }
        }

        auto [income, expenditure] = financeMgr.getFinance(count);
        cout << "+ " << fixed << setprecision(2) << income << " - " << expenditure << "\n";
    }

    void cmdLog() {
        if (getCurrentPrivilege() < 7) {
            cout << "Invalid\n";
            return;
        }
        // Self-defined format - just output empty line for now
        cout << "\n";
    }

    void cmdReportFinance() {
        if (getCurrentPrivilege() < 7) {
            cout << "Invalid\n";
            return;
        }
        // Self-defined format - just output empty line for now
        cout << "\n";
    }

    void cmdReportEmployee() {
        if (getCurrentPrivilege() < 7) {
            cout << "Invalid\n";
            return;
        }
        // Self-defined format - just output empty line for now
        cout << "\n";
    }

public:
    void processCommand(const string& line) {
        string trimmed = trim(line);
        if (trimmed.empty()) return;

        vector<string> parts = split(trimmed);
        if (parts.empty()) return;

        string cmd = parts[0];
        vector<string> args(parts.begin() + 1, parts.end());

        if (cmd == "quit" || cmd == "exit") {
            exit(0);
        } else if (cmd == "su") {
            cmdSu(args);
        } else if (cmd == "logout") {
            cmdLogout();
        } else if (cmd == "register") {
            cmdRegister(args);
        } else if (cmd == "passwd") {
            cmdPasswd(args);
        } else if (cmd == "useradd") {
            cmdUseradd(args);
        } else if (cmd == "delete") {
            cmdDelete(args);
        } else if (cmd == "show") {
            if (!args.empty() && args[0] == "finance") {
                vector<string> financeArgs(args.begin() + 1, args.end());
                cmdShowFinance(financeArgs);
            } else {
                cmdShow(args);
            }
        } else if (cmd == "buy") {
            cmdBuy(args);
        } else if (cmd == "select") {
            cmdSelect(args);
        } else if (cmd == "modify") {
            cmdModify(args);
        } else if (cmd == "import") {
            cmdImport(args);
        } else if (cmd == "log") {
            cmdLog();
        } else if (cmd == "report") {
            if (args.size() == 1) {
                if (args[0] == "finance") {
                    cmdReportFinance();
                } else if (args[0] == "employee") {
                    cmdReportEmployee();
                } else {
                    cout << "Invalid\n";
                }
            } else {
                cout << "Invalid\n";
            }
        } else {
            cout << "Invalid\n";
        }
    }

    void run() {
        string line;
        while (getline(cin, line)) {
            processCommand(line);
        }
    }
};

int main() {
    BookstoreSystem system;
    system.run();
    return 0;
}

