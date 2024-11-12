#include "LSystem.h"

#ifndef __WIND_D0LSYSTEM
#define __WIND_LSYSTEM

#define NDEBUG

#include "LSysParser.hpp"

namespace LSystem {

using namespace std;
using LProduction = unordered_set<string>;

class D0L_System {
    LSysParser::config::LSystem lsystem;
    string axiom;
    string current_state;  // 迭代的当前状态

   public:
    D0L_System(const string& axiom, const string& productions)
        : axiom(axiom), current_state(axiom), lsystem() {
        auto lsys_prods_string = lexy::string_input(productions);
        auto lsys_res = lexy::parse<LSysParser::grammar::LSystem>(lsys_prods_string, lexy_ext::report_error);
        assert(lsys_res.is_success());
        this->lsystem = lsys_res.value();
    }

    D0L_System(const string& axiom, const vector<string>& productions)
        : axiom(axiom), current_state(axiom), lsystem() {
        vector<LSysParser::config::LProduction> prods;
        for (const string& production : productions) {
            auto lsys_prod_string = lexy::string_input(production);
            auto lsys_prod_res = lexy::parse<LSysParser::grammar::LProduction>(lsys_prod_string, lexy_ext::report_error);
            assert(lsys_prod_res.is_success());
            prods.push_back(lsys_prod_res.value());
        }
        this->lsystem = LSysParser::config::LSystem(prods);
    }

    string next(uint32_t n=1) {
        for(uint32_t i=0; i<n; i++)
            this->current_state = this->iterate(this->current_state);
        return this->current_state;
    }

    void reset() { this->current_state = this->axiom; }

    virtual string iterate(const string& p) {
        auto input_string = lexy::string_input(p);
        auto input_res = lexy::parse<LSysParser::grammar::LSysCall>(input_string, lexy_ext::report_error);
        assert(input_res.is_success());
        const LSysParser::config::LSysCall& syscall = input_res.value();
        return syscall.apply(this->lsystem);
    };
};

}  // namespace LSystem

#endif
