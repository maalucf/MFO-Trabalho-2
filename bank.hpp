#include <map>
#include <string>
using namespace std;

struct Investment {
  string owner;
  int amount;
  bool operator==(const Investment& ot) const {
    return owner == ot.owner && amount == ot.amount; 
  }
};

struct BankState {
  map<string, int> balances;
  map<int, Investment> investments;
  int next_id;
};

string deposit(BankState &bank_state, string depositor, int amount) {
  if (amount <= 0) {
    return "Amount should be greater than zero";
  } else {
    bank_state.balances[depositor] += amount;
    return "";
  }
}

string withdraw(BankState &bank_state, string withdrawer, int amount) {
  if (amount <= 0) {
    return "Amount should be greater than zero";
  } else if (amount > bank_state.balances[withdrawer]) {
    return "Balance is too low";
  } else {
    bank_state.balances[withdrawer] -= amount;
    return ""; 
  }
}

string transfer(BankState &bank_state, string sender, string receiver,
                int amount) {
  if (amount <= 0) {
    return "Amount should be greater than zero";
  } else if (amount > bank_state.balances[sender]) {
    return "Balance is too low";
  } else {
    bank_state.balances[sender] -= amount;
    bank_state.balances[receiver] += amount;
    return "";
  }
}

string buy_investment(BankState &bank_state, string buyer, int amount) {
  if (amount <= 0) {
    return "Amount should be greater than zero";
  }
  else if (amount > bank_state.balances[buyer]) {
    return "Balance is too low";
  } else {
    bank_state.balances[buyer] -= amount;
    bank_state.investments[bank_state.next_id] = {buyer, amount};
    bank_state.next_id++;
    return "";
  } 
}

string sell_investment(BankState &bank_state, string seller,
                       int investment_id) {
  if (bank_state.investments.count(investment_id) == 0) {
    return "No investment with this id";
  } else {
    if (bank_state.investments[investment_id].owner != seller) {
      return "Seller can't sell an investment they don't own"; 
    }
    else {
      bank_state.balances[seller] += bank_state.investments[investment_id].amount;
      bank_state.investments.erase(investment_id); // erasing investiment
      return "";
    } 
  } 
}
