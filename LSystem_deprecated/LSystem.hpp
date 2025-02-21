#include "LSystem.h"
#include <cstdint>

#ifndef __WIND_D0LSYSTEM
#    define __WIND_D0LSYSTEM

#    include "LSysParser.hpp"

namespace LSystem {

using namespace std;
using LProduction = unordered_set<string>;

class D0L_System {
    LSysParser::config::LSystem lsystem;
    string                      axiom;
    string                      current_state;   // 迭代的当前状态
    uint32_t                    iter_n;

public:
    D0L_System(const string& axiom, const string& productions)
        : axiom(axiom)
        , current_state(axiom)
        , iter_n(0)
        , lsystem() {
        printf("input production: %s\n", productions.c_str());
        auto lsys_prods_string = lexy::string_input(productions);
        auto lsys_res          = lexy::parse<LSysParser::grammar::LSystem>(lsys_prods_string, lexy_ext::report_error);
        if (lsys_res.is_success())
            this->lsystem = lsys_res.value();
    }

    D0L_System(const string& axiom, const vector<string>& productions)
        : axiom(axiom)
        , current_state(axiom)
        , iter_n(0)
        , lsystem() {

        this->updateProduction(productions);
    }

    uint32_t get_iter_n() const { return this->iter_n; }

    bool updateAxiom(const string& axiom) {
        this->axiom = axiom;
        this->reset();
        return true;
    }

    bool updateProduction(const vector<string>& productions) {
        vector<LSysParser::config::LProduction> prods;
        bool                                    is_success = true;
        for (const string& production : productions) {
            auto lsys_prod_string = lexy::string_input(production);
            auto lsys_prod_res    = lexy::parse<LSysParser::grammar::LProduction>(lsys_prod_string, lexy_ext::report_error);
            if (!lsys_prod_res.is_success()) {
                is_success = false;
                break;
            }
            prods.push_back(lsys_prod_res.value());
        }
        if (is_success)
            this->lsystem = LSysParser::config::LSystem(prods);
        return true;
    }

    string next(uint32_t n = 1) {
        for (uint32_t i = 0; i < n; i++)
            this->current_state = this->iterate(this->current_state);
        this->iter_n += n;
        return this->current_state;
    }

    void reset() { this->current_state = this->axiom; }

    virtual string iterate(const string& p) {
        auto input_string = lexy::string_input(p);
        auto input_res    = lexy::parse<LSysParser::grammar::LSysCall>(input_string, lexy_ext::report_error);
        assert(input_res.is_success());
        const LSysParser::config::LSysCall& syscall = input_res.value();
        return syscall.apply(this->lsystem);
    };
};

}   // namespace LSystem

#endif
