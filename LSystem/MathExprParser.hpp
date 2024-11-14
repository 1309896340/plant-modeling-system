#include "LSystem.h"
#include "lexy/dsl/capture.hpp"
#include "lexy/encoding.hpp"
#include "lexy/lexeme.hpp"
#ifndef __WIND_MATHEXPR
#define __WIND_MATHEXPR

namespace MathParser {
using namespace std;

namespace exception {
class Unknown_Variable : public std::exception {
    string message;

   public:
    string varname;
    string where;

    Unknown_Variable(string varname, string where):varname(varname),where(where) {
        this->message = "unknown variable name \"" + varname + "\" from \"" + where + "\"\n";
    }

    virtual const char* what() const noexcept override {
        return this->message.c_str();
    }
};

};  // namespace exception

namespace ast {  // 抽象语法树构建===============================================================================================
using expr_ptr = shared_ptr<struct Expr>;

// struct function{
//     string name;
//     vector<float> params;
// };

struct Environment {
    map<string, float> vars;
    // map<string, function> fns;
};

struct Expr {
    virtual ~Expr() = default;
    virtual float evaluate(Environment& env) const = 0;
};

struct Expr_literal : Expr {
    float val;
    Expr_literal(float val)
        : val(val) {}
    // Expr_literal(const int a, const optional<int>& b)
    //     : val(static_cast<float>(a)) {  // 初始化的时候就计算
    //     if (b.has_value()) {
    //         float tmp = static_cast<float>(b.value());
    //         while (tmp >= 1.0f)
    //             tmp /= 10.0f;
    //         val += tmp;
    //     }
    // }
    Expr_literal(const int a, const optional<string>& b)
        : val(static_cast<float>(a)) {  // 初始化的时候就计算
        // 计算分数
        if(b.has_value()){
            float p = 0.1f;
            string frac = b.value();
            for(uint8_t i=0; i<frac.size(); i++){
                val += p*(frac[i]-'0');
                p /= 10.0f;
            }
        }
    }
    virtual float evaluate(ast::Environment& env) const { return this->val; }
};

struct Expr_var : Expr {
    string varname;
    Expr_var(string varname)
        : varname(LEXY_MOV(varname)) {};
    Expr_var(lexy::string_lexeme<lexy::ascii_encoding> lexeme)
        : varname(lexy::as_string<string>(lexeme)) {}
    virtual float evaluate(Environment& env) const {
        auto iter = env.vars.find(varname);
        if (iter == env.vars.end()) {
            // 找不到的变量直接报错，防止产生运行时不确定行为
            throw exception::Unknown_Variable(varname, "Expr_var");
        }
        return iter->second;
    }
};

struct Expr_unary : Expr {  // 单目运算
    enum op_t { negate } op;
    expr_ptr rhs;

    explicit Expr_unary(op_t op, expr_ptr rhs)
        : op(op), rhs(LEXY_MOV(rhs)) {}

    virtual float evaluate(Environment& env) const {
        float rhs_v = 0.0f;
        try {
            rhs_v = rhs->evaluate(env);
        } catch (MathParser::exception::Unknown_Variable e) {
            throw MathParser::exception::Unknown_Variable(e.varname, "Expr_unary");
        }
        switch (op) {
            case negate:
                return -rhs_v;
        }
        return rhs_v;
    }
};

struct Expr_binary : Expr {  // 双目运算
    enum op_t { plus,
                minus,
                mul,
                div } op;
    expr_ptr lhs, rhs;

    explicit Expr_binary(expr_ptr lhs, op_t op, expr_ptr rhs)
        : lhs(LEXY_MOV(lhs)), op(op), rhs(LEXY_MOV(rhs)) {}

    virtual float evaluate(Environment& env) const {
        float lhs_v = 0.0f, rhs_v = 0.0f;
        try {
            lhs_v = lhs->evaluate(env);
            rhs_v = rhs->evaluate(env);
        } catch (MathParser::exception::Unknown_Variable e) {
            throw MathParser::exception::Unknown_Variable(e.varname, "Expr_binary");
        }

        switch (op) {
            case plus:
                return lhs_v + rhs_v;
            case minus:
                return lhs_v - rhs_v;
            case mul:
                return lhs_v * rhs_v;
            case div:
                return lhs_v / rhs_v;
        }
        return 0.0f;
    }
};
};  // namespace ast
namespace grammar {  // 语法解析=============================================================================================
namespace dsl = lexy::dsl;
constexpr auto escaped_newline = dsl::backslash >> dsl::newline;

struct NFraction{
    // 消耗digit序列，生成string
    static constexpr auto rule = dsl::capture(dsl::digits<>);
    static constexpr auto value = lexy::callback<string>(
        [](lexy::string_lexeme<lexy::ascii_encoding> lex){
            return string(lex.begin(), lex.end());
        } 
    );
};

struct Number {
    static constexpr auto rule = [] {
        auto prefix = dsl::peek(dsl::ascii::digit);
        auto number = dsl::integer<int>;
        auto fraction = dsl::opt(dsl::lit_c<'.'> >> dsl::p<NFraction>);
        return prefix >> number + fraction;
    }();
    static constexpr auto value = lexy::construct<ast::Expr_literal>;
};
struct Variable {
    static constexpr auto rule = dsl::identifier(dsl::ascii::alpha_underscore, dsl::ascii::alpha_digit_underscore);
    static constexpr auto value = lexy::construct<ast::Expr_var>;
};

struct NestedExpr : lexy::transparent_production {
    static constexpr auto whitespace = dsl::ascii::space | escaped_newline;
    static constexpr auto rule = dsl::recurse<struct MathExpr>;
    static constexpr auto value = lexy::forward<ast::expr_ptr>;
};
struct MathExpr : lexy::expression_production {
    struct expected_operand {
        static constexpr auto name = "expected operand";
    };

    static constexpr auto whitespace = dsl::ascii::space;
    static constexpr auto atom = [] {
        auto var_name_prefix = dsl::peek(dsl::ascii::alpha / dsl::lit_c<'_'>);
        auto number_prefix = dsl::peek(dsl::ascii::digit);

        auto literal_number = number_prefix >> dsl::p<grammar::Number>;
        auto var_name = var_name_prefix >> dsl::p<grammar::Variable>;

        auto paren_expr = dsl::parenthesized(dsl::p<NestedExpr>);
        return paren_expr | literal_number | var_name |
               dsl::error<expected_operand>;

        // 这个方案似乎不太可行，本质上只是建立了每个SymMap中各参数到Sym的参数组的映射
    }();

    struct ExprItem : dsl::infix_op_left {  // 乘除运算
        static constexpr auto op =
            dsl::op<ast::Expr_binary::mul>(dsl::lit_c<'*'>) /
            dsl::op<ast::Expr_binary::div>(dsl::lit_c<'/'>);
        using operand = dsl::atom;
    };

    struct ExprNeg : dsl::prefix_op {  // 单目运算取相反数
        // 对ExprItem可能的取反
        static constexpr auto op =
            dsl::op<ast::Expr_unary::negate>(dsl::lit_c<'-'>);
        using operand = ExprItem;
    };

    struct ExprRow : dsl::infix_op_left {  // 加减运算
        static constexpr auto op =
            dsl::op<ast::Expr_binary::plus>(dsl::lit_c<'+'>) /
            dsl::op<ast::Expr_binary::minus>(dsl::lit_c<'-'>);
        using operand = ExprNeg;
    };

    using operation = ExprRow;

    static constexpr auto value = lexy::callback<ast::expr_ptr>(
        lexy::forward<ast::expr_ptr>,
        lexy::new_<ast::Expr_unary, ast::expr_ptr>,
        lexy::new_<ast::Expr_binary, ast::expr_ptr>,
        lexy::new_<ast::Expr_literal, ast::expr_ptr>,
        lexy::new_<ast::Expr_var, ast::expr_ptr>
        // ,lexy::new_<ast::Expr_define, ast::expr_ptr>
    );
};
};  // namespace grammar
};  // namespace MathParser

#endif
