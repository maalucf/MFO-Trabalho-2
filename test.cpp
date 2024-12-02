#include <assert.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

#include "bank.hpp"

using json = nlohmann::json;
using namespace std;

enum class Action {
  Init,
  Deposit,
  Withdraw,
  Transfer,
  BuyInvestment,
  SellInvestment,
  Unknown
};

Action stringToAction(const std::string &actionStr) {
  static const std::unordered_map<std::string, Action> actionMap = {
      {"init", Action::Init},
      {"deposit_action", Action::Deposit},
      {"withdraw_action", Action::Withdraw},
      {"transfer_action", Action::Transfer},
      {"buy_investment_action", Action::BuyInvestment},
      {"sell_investment_action", Action::SellInvestment}};

  auto it = actionMap.find(actionStr);
  if (it != actionMap.end()) {
    return it->second;
  } else {
    return Action::Unknown;
  }
}

int int_from_json(json j) {
  string s = j["#bigint"];
  return stoi(s);
}

map<string, int> balances_from_json(json j) {
  map<string, int> m;
  for (auto it : j["#map"]) {
    m[it[0]] = int_from_json(it[1]);
  }
  return m;
}

map<int, Investment> investments_from_json(json j) {
  map<int, Investment> m;
  for (auto it : j["#map"]) {
    m[int_from_json(it[0])] = {.owner = it[1]["owner"],
                               .amount = int_from_json(it[1]["amount"])};
  }
  return m;
}

BankState bank_state_from_json(json state) {
  map<string, int> balances = balances_from_json(state["balances"]);
  map<int, Investment> investments =
      investments_from_json(state["investments"]);
  int next_id = int_from_json(state["next_id"]);
  return {.balances = balances, .investments = investments, .next_id = next_id};
}

int main() {
  //for (int i = 0; i < 10000; i++) {
  for (int i = 0; i < 10; i++) {
    cout << "Trace #" << i << endl;
    std::ifstream f("traces/out" + to_string(i) + ".itf.json");
    json data = json::parse(f);

    // Estado inicial: começamos do mesmo estado incial do trace
    BankState bank_state =
        bank_state_from_json(data["states"][0]["bank_state"]);

    auto states = data["states"];
    for (auto state : states) {
      string action = state["action_taken"];
      json nondet_picks = state["nondet_picks"];

      string error = "";

      // Próxima transição
      switch (stringToAction(action)) {
        case Action::Init: {
          cout << "Initializing" << endl;
          break;
        }
        case Action::Deposit: {
          string depositor = nondet_picks["depositor"]["value"];
          int amount = int_from_json(nondet_picks["amount"]["value"]);
          error = deposit(bank_state, depositor, amount);
          break;
        }
        case Action::Withdraw: {
          string withdrawer = nondet_picks["withdrawer"]["value"]; 
          int amount = int_from_json(nondet_picks["amount"]["value"]); 
          error = withdraw(bank_state, withdrawer, amount); 
          break; 
        }
        case Action::Transfer: {
          string sender = nondet_picks["sender"]["value"];
          string receiver = nondet_picks["receiver"]["value"];
          int amount = int_from_json(nondet_picks["amount"]["value"]);
          error = transfer(bank_state, sender, receiver, amount);
          break;
        }
        case Action::BuyInvestment: {
          string buyer = nondet_picks["buyer"]["value"];
          int amount = int_from_json(nondet_picks["amount"]["value"]);
          error = buy_investment(bank_state, buyer, amount); 
          break;
        }
        case Action::SellInvestment: {
          string seller = nondet_picks["seller"]["value"];
          int investment_id = int_from_json(nondet_picks["id"]["value"]);
          error = sell_investment(bank_state, seller, investment_id);
          break;
        }
        default: {
          cout << "TODO: fazer a conexão para as outras ações. Ação: " << action
              << endl;
          error = "";
          break;
        }
      }

      BankState expected_bank_state = bank_state_from_json(state["bank_state"]);
      if (expected_bank_state.balances != bank_state.balances) {
        cout << "Os balanços atuais não condizem com o balanço esperado" << endl;
        cout << "Mais especificamente: " << endl;
        for (auto [owner, value] : expected_bank_state.balances) {
          bool exists = bank_state.balances.count(owner) > 0; 
          string to_print = exists ? ("é " + to_string(bank_state.balances[owner])) : ("não existe"); 
          cout << "Balanço de " << owner << " deveria ser " << value << ", mas " << to_print << endl;
        }
      }

      if (expected_bank_state.investments != bank_state.investments) {
        cout << "Os investimentos atuais não condizem com os investimentos esperados" << endl;
        cout << "Mais especificamente: " << endl;
        for (auto [expected_investment_id, expected_investment] : expected_bank_state.investments) {
          if (bank_state.investments.count(expected_investment_id) == 0) {
            cout << "Investimento de ID " << expected_investment_id << " não existe" << endl;
          }
          else {
            Investment investment = bank_state.investments[expected_investment_id]; 
            cout << "Para o investimento de ID " << expected_investment_id << endl;
            bool algo_errado = 0; 
            if (expected_investment.owner != investment.owner) {
              cout << "\t O dono deveria ser " << expected_investment.owner << ", mas é " << investment.owner << endl;
              algo_errado = 1; 
            }
            if (expected_investment.amount != investment.amount) {
              cout << "\t O valor deveria ser " << expected_investment.amount << ", mas é " << investment.amount << endl;
              algo_errado = 1; 
            }
            if (algo_errado == 0) cout << "\t Tudo certo na verdade :)" << endl;
          } 
        }

        for (auto [investment_id, investment] : bank_state.investments) {
          if (expected_bank_state.investments.count(investment_id) == 0) {
            cout << "O investimento de ID " << investment_id << " não deveria existir" << endl;
          }
        }
      }

      if (expected_bank_state.next_id != bank_state.next_id) {
        cout << "O next_id não corresponde " <<  expected_bank_state.next_id << "!=" << bank_state.next_id << endl;
      }
      // cout << "TODO: comparar o estado esperado com o estado obtido" << endl;

      string expected_error = string(state["error"]["tag"]).compare("Some") == 0
                                  ? state["error"]["value"]
                                  : "";

      if (expected_error != error) {
        cout << "Erro deveria ser " << expected_error << ", mas é " << error << endl;
      }
    }
  }
  return 0;
}
