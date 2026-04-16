#include "FunGT/FunGTIRParser.h"
#include "FunGT/FunGTOps.h"
#include "FunGT/FunGTDialect.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Math/IR/Math.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <cctype>
#include <cstdlib>

namespace mlir {
namespace fungt {

enum class TokenKind {
    NAME, NUMBER,
    PLUS, MINUS, STAR, SLASH,
    EQUALS, LESS, GREATER,
    LPAREN, RPAREN,
    COMMA, COLON, NEWLINE,
    TOK_EOF
};

struct Token {
    TokenKind kind;
    std::string text;
    int line;
    int col;
};

class Tokenizer {
    llvm::StringRef source;
    size_t pos;
    int line;
    int col;

public:
    Tokenizer(llvm::StringRef src) : source(src), pos(0), line(1), col(1) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (pos < source.size()) {
            char c = source[pos];

            if (c == '#') {
                while (pos < source.size() && source[pos] != '\n') pos++;
                continue;
            }
            if (c == ' ' || c == '\t') {
                pos++; col++;
                continue;
            }
            if (c == '\n') {
                tokens.push_back({TokenKind::NEWLINE, "\\n", line, col});
                pos++; line++; col = 1;
                continue;
            }
            if (c == '+') { tokens.push_back({TokenKind::PLUS, "+", line, col}); pos++; col++; continue; }
            if (c == '-') {
                if (pos + 1 < source.size() && (std::isdigit(source[pos+1]) || source[pos+1] == '.')) {
                    if (tokens.empty() || tokens.back().kind == TokenKind::EQUALS ||
                        tokens.back().kind == TokenKind::PLUS || tokens.back().kind == TokenKind::MINUS ||
                        tokens.back().kind == TokenKind::STAR || tokens.back().kind == TokenKind::SLASH ||
                        tokens.back().kind == TokenKind::LPAREN || tokens.back().kind == TokenKind::COMMA ||
                        tokens.back().kind == TokenKind::NEWLINE) {
                        tokens.push_back(readNumber());
                        continue;
                    }
                }
                tokens.push_back({TokenKind::MINUS, "-", line, col}); pos++; col++;
                continue;
            }
            if (c == '*') { tokens.push_back({TokenKind::STAR, "*", line, col}); pos++; col++; continue; }
            if (c == '/') { tokens.push_back({TokenKind::SLASH, "/", line, col}); pos++; col++; continue; }
            if (c == '=') { tokens.push_back({TokenKind::EQUALS, "=", line, col}); pos++; col++; continue; }
            if (c == '<') { tokens.push_back({TokenKind::LESS, "<", line, col}); pos++; col++; continue; }
            if (c == '>') { tokens.push_back({TokenKind::GREATER, ">", line, col}); pos++; col++; continue; }
            if (c == '(') { tokens.push_back({TokenKind::LPAREN, "(", line, col}); pos++; col++; continue; }
            if (c == ')') { tokens.push_back({TokenKind::RPAREN, ")", line, col}); pos++; col++; continue; }
            if (c == ',') { tokens.push_back({TokenKind::COMMA, ",", line, col}); pos++; col++; continue; }
            if (c == ':') { tokens.push_back({TokenKind::COLON, ":", line, col}); pos++; col++; continue; }
            if (std::isdigit(c) || c == '.') {
                tokens.push_back(readNumber());
                continue;
            }
            if (std::isalpha(c) || c == '_') {
                tokens.push_back(readName());
                continue;
            }
            std::cerr << "Unknown character '" << c << "' at line " << line << " col " << col << "\n";
            pos++; col++;
        }
        tokens.push_back({TokenKind::TOK_EOF, "", line, col});
        return tokens;
    }

private:
    Token readNumber() {
        int startCol = col;
        std::string num;
        if (pos < source.size() && source[pos] == '-') {
            num += '-'; pos++; col++;
        }
        while (pos < source.size() && (std::isdigit(source[pos]) || source[pos] == '.')) {
            num += source[pos]; pos++; col++;
        }
        return {TokenKind::NUMBER, num, line, startCol};
    }

    Token readName() {
        int startCol = col;
        std::string name;
        while (pos < source.size() && (std::isalnum(source[pos]) || source[pos] == '_')) {
            name += source[pos]; pos++; col++;
        }
        return {TokenKind::NAME, name, line, startCol};
    }
};

class Parser {
    std::vector<Token> tokens;
    size_t current;
    mlir::OpBuilder builder;
    mlir::Location loc;
    std::unordered_map<std::string, mlir::Value> variables;

public:
    Parser(std::vector<Token> toks, mlir::OpBuilder &b, mlir::Location l)
        : tokens(std::move(toks)), current(0), builder(b), loc(l) {}

    bool parse(mlir::Block *block, mlir::ValueRange blockArgs) {
        variables["pos_x"] = blockArgs[0];
        variables["pos_y"] = blockArgs[1];
        variables["pos_z"] = blockArgs[2];
        variables["vel_x"] = blockArgs[3];
        variables["vel_y"] = blockArgs[4];
        variables["vel_z"] = blockArgs[5];
        variables["mass"]  = blockArgs[6];
        variables["dt"]    = blockArgs[7];
        variables["age"]   = blockArgs[8];

        skipNewlines();
        if (!expectName("update")) return error("expected 'update'");
        advance();
        if (peek().kind != TokenKind::NAME) return error("expected kernel name");
        advance();
        if (!expect(TokenKind::COLON)) return error("expected ':'");
        skipNewlines();

        while (peek().kind != TokenKind::TOK_EOF) {
            skipNewlines();
            if (peek().kind == TokenKind::TOK_EOF) break;
            if (peek().kind == TokenKind::NAME && peek().text == "yield") {
                return parseYield();
            }
            if (!parseStatement()) return false;
            skipNewlines();
        }
        return error("expected 'yield'");
    }

private:
    bool parseStatement() {
        std::string name = peek().text;
        advance();
        if (!expect(TokenKind::EQUALS)) return error("expected '='");
        mlir::Value val = parseExpression();
        if (!val) return false;
        variables[name] = val;
        return true;
    }

    bool parseYield() {
        advance(); // consume 'yield'
        std::vector<mlir::Value> results;
        for (int i = 0; i < 7; i++) {
            mlir::Value val = parseExpression();
            if (!val) return false;
            results.push_back(val);
            if (i < 6) {
                if (!expect(TokenKind::COMMA)) return error("expected ',' in yield");
            }
        }
        builder.create<fungt::YieldOp>(loc,
            results[0], results[1], results[2],
            results[3], results[4], results[5], results[6]);
        return true;
    }

    mlir::Value parseExpression() {
        return parseComparison();
    }

    mlir::Value parseComparison() {
        mlir::Value left = parseAdditive();
        if (!left) return nullptr;
        if (peek().kind == TokenKind::LESS) {
            advance();
            mlir::Value right = parseAdditive();
            if (!right) return nullptr;
            return builder.create<arith::CmpFOp>(loc,
                arith::CmpFPredicate::OLT, left, right).getResult();
        }
        if (peek().kind == TokenKind::GREATER) {
            advance();
            mlir::Value right = parseAdditive();
            if (!right) return nullptr;
            return builder.create<arith::CmpFOp>(loc,
                arith::CmpFPredicate::OGT, left, right).getResult();
        }
        return left;
    }

    mlir::Value parseAdditive() {
        mlir::Value left = parseMultiplicative();
        if (!left) return nullptr;
        while (peek().kind == TokenKind::PLUS || peek().kind == TokenKind::MINUS) {
            TokenKind op = peek().kind;
            advance();
            mlir::Value right = parseMultiplicative();
            if (!right) return nullptr;
            if (op == TokenKind::PLUS)
                left = builder.create<arith::AddFOp>(loc, left, right).getResult();
            else
                left = builder.create<arith::SubFOp>(loc, left, right).getResult();
        }
        return left;
    }

    mlir::Value parseMultiplicative() {
        mlir::Value left = parseUnary();
        if (!left) return nullptr;
        while (peek().kind == TokenKind::STAR || peek().kind == TokenKind::SLASH) {
            TokenKind op = peek().kind;
            advance();
            mlir::Value right = parseUnary();
            if (!right) return nullptr;
            if (op == TokenKind::STAR)
                left = builder.create<arith::MulFOp>(loc, left, right).getResult();
            else
                left = builder.create<arith::DivFOp>(loc, left, right).getResult();
        }
        return left;
    }

    mlir::Value parseUnary() {
        if (peek().kind == TokenKind::MINUS) {
            advance();
            mlir::Value val = parseUnary();
            if (!val) return nullptr;
            auto zero = builder.create<arith::ConstantOp>(loc,
                builder.getF32FloatAttr(0.0f));
            return builder.create<arith::SubFOp>(loc, zero, val).getResult();
        }
        return parsePrimary();
    }

    mlir::Value parsePrimary() {
        Token tok = peek();

        if (tok.kind == TokenKind::NUMBER) {
            advance();
            float val = std::stof(tok.text);
            return builder.create<arith::ConstantOp>(loc,
                builder.getF32FloatAttr(val)).getResult();
        }

        if (tok.kind == TokenKind::NAME) {
            if (tok.text == "sqrt" || tok.text == "sin" || tok.text == "cos") {
                return parseMathFunc(tok.text);
            }
            if (tok.text == "distance") {
                return parseDistance();
            }
            if (tok.text == "select") {
                return parseSelect();
            }
            advance();
            auto it = variables.find(tok.text);
            if (it == variables.end()) {
                error("undefined variable: " + tok.text);
                return nullptr;
            }
            return it->second;
        }

        if (tok.kind == TokenKind::LPAREN) {
            advance();
            mlir::Value val = parseExpression();
            if (!val) return nullptr;
            if (!expect(TokenKind::RPAREN)) { error("expected ')'"); return nullptr; }
            return val;
        }

        error("unexpected token: " + tok.text);
        return nullptr;
    }

    mlir::Value parseMathFunc(const std::string &name) {
        advance(); // consume function name
        if (!expect(TokenKind::LPAREN)) { error("expected '('"); return nullptr; }
        mlir::Value arg = parseExpression();
        if (!arg) return nullptr;
        if (!expect(TokenKind::RPAREN)) { error("expected ')'"); return nullptr; }
        if (name == "sqrt") return builder.create<math::SqrtOp>(loc, arg).getResult();
        if (name == "sin")  return builder.create<math::SinOp>(loc, arg).getResult();
        if (name == "cos")  return builder.create<math::CosOp>(loc, arg).getResult();
        return nullptr;
    }

    mlir::Value parseDistance() {
        advance();
        if (!expect(TokenKind::LPAREN)) { error("expected '('"); return nullptr; }
        mlir::Value x0 = parseExpression(); if (!x0) return nullptr;
        if (!expect(TokenKind::COMMA)) { error("expected ','"); return nullptr; }
        mlir::Value y0 = parseExpression(); if (!y0) return nullptr;
        if (!expect(TokenKind::COMMA)) { error("expected ','"); return nullptr; }
        mlir::Value z0 = parseExpression(); if (!z0) return nullptr;
        if (!expect(TokenKind::COMMA)) { error("expected ','"); return nullptr; }
        mlir::Value x1 = parseExpression(); if (!x1) return nullptr;
        if (!expect(TokenKind::COMMA)) { error("expected ','"); return nullptr; }
        mlir::Value y1 = parseExpression(); if (!y1) return nullptr;
        if (!expect(TokenKind::COMMA)) { error("expected ','"); return nullptr; }
        mlir::Value z1 = parseExpression(); if (!z1) return nullptr;
        if (!expect(TokenKind::RPAREN)) { error("expected ')'"); return nullptr; }
        return builder.create<fungt::DistanceOp>(loc, x0, y0, z0, x1, y1, z1).getResult();
    }

    mlir::Value parseSelect() {
        advance();
        if (!expect(TokenKind::LPAREN)) { error("expected '('"); return nullptr; }
        mlir::Value cond = parseExpression(); if (!cond) return nullptr;
        if (!expect(TokenKind::COMMA)) { error("expected ','"); return nullptr; }
        mlir::Value trueVal = parseExpression(); if (!trueVal) return nullptr;
        if (!expect(TokenKind::COMMA)) { error("expected ','"); return nullptr; }
        mlir::Value falseVal = parseExpression(); if (!falseVal) return nullptr;
        if (!expect(TokenKind::RPAREN)) { error("expected ')'"); return nullptr; }
        return builder.create<fungt::SelectOp>(loc, cond, trueVal, falseVal).getResult();
    }

    Token &peek() { return tokens[current]; }
    void advance() { if (current < tokens.size() - 1) current++; }
    void skipNewlines() { while (peek().kind == TokenKind::NEWLINE) advance(); }

    bool expect(TokenKind kind) {
        if (peek().kind == kind) { advance(); return true; }
        return false;
    }

    bool expectName(const std::string &name) {
        return peek().kind == TokenKind::NAME && peek().text == name;
    }

    bool error(const std::string &msg) {
        std::cerr << "Parse error at line " << peek().line
                  << " col " << peek().col << ": " << msg << "\n";
        return false;
    }
};

mlir::OwningOpRef<mlir::ModuleOp>
parseFunGTIR(mlir::MLIRContext &ctx, llvm::StringRef source) {
    ctx.loadDialect<fungt::FunGTDialect>();
    ctx.loadDialect<arith::ArithDialect>();
    ctx.loadDialect<math::MathDialect>();
    ctx.loadDialect<func::FuncDialect>();

    auto loc = mlir::UnknownLoc::get(&ctx);
    mlir::OpBuilder builder(&ctx);

    auto moduleOp = mlir::ModuleOp::create(loc);
    builder.setInsertionPointToEnd(moduleOp.getBody());

    auto f32 = builder.getF32Type();
    llvm::SmallVector<mlir::Type, 9> inputTypes(9, f32);
    llvm::SmallVector<mlir::Type, 7> resultTypes(7, f32);

    auto funcType = builder.getFunctionType(inputTypes, resultTypes);
    auto funcOp = builder.create<mlir::func::FuncOp>(loc, "particle_update", funcType);
    auto *entryBlock = funcOp.addEntryBlock();
    builder.setInsertionPointToStart(entryBlock);

    auto updateOp = builder.create<fungt::Update>(loc,
        resultTypes,
        entryBlock->getArguments()[0], entryBlock->getArguments()[1],
        entryBlock->getArguments()[2], entryBlock->getArguments()[3],
        entryBlock->getArguments()[4], entryBlock->getArguments()[5],
        entryBlock->getArguments()[6], entryBlock->getArguments()[7],
        entryBlock->getArguments()[8]);

    auto &region = updateOp.getBody();
    auto *updateBlock = new mlir::Block();
    region.push_back(updateBlock);
    for (int i = 0; i < 9; i++)
        updateBlock->addArgument(f32, loc);

    builder.setInsertionPointToStart(updateBlock);

    Tokenizer tokenizer(source);
    auto tokens = tokenizer.tokenize();
    Parser parser(std::move(tokens), builder, loc);

    if (!parser.parse(updateBlock, updateBlock->getArguments())) {
        return nullptr;
    }

    builder.setInsertionPointToEnd(entryBlock);
    builder.create<mlir::func::ReturnOp>(loc, updateOp.getResults());

    return moduleOp;
}

} // namespace fungt
} // namespace mlir