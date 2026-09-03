#include "FunGT/FunGTIRParser.h"
#include "FunGT/FunGTOps.h"
#include "FunGT/FunGTDialect.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Math/IR/Math.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVDialect.h"
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
    LBRACE, RBRACE,
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
        std::vector<Token> m_tokens;
        while (pos < source.size()) {
            char c = source[pos];

            if (c == '/') { //for comments
                if (pos + 1 < source.size() && source[pos + 1] == '/') {
                    while (pos < source.size() && source[pos] != '\n') pos++;
                    continue;
                }
                m_tokens.push_back({TokenKind::SLASH, "/", line, col}); pos++; col++;
                continue;
            }
            if (c == ' ' || c == '\t') {
                pos++; col++;
                continue;
            }
            if (c == '\n') {
                m_tokens.push_back({TokenKind::NEWLINE, "\\n", line, col});
                pos++; line++; col = 1;
                continue;
            }
            if (c == '+') { m_tokens.push_back({TokenKind::PLUS, "+", line, col}); pos++; col++; continue; }
            if (c == '-') {
                if (pos + 1 < source.size() && (std::isdigit(source[pos+1]) || source[pos+1] == '.')) {
                    if (m_tokens.empty() || m_tokens.back().kind == TokenKind::EQUALS ||
                        m_tokens.back().kind == TokenKind::PLUS || m_tokens.back().kind == TokenKind::MINUS ||
                        m_tokens.back().kind == TokenKind::STAR || m_tokens.back().kind == TokenKind::SLASH ||
                        m_tokens.back().kind == TokenKind::LPAREN || m_tokens.back().kind == TokenKind::COMMA ||
                        m_tokens.back().kind == TokenKind::NEWLINE) {
                        m_tokens.push_back(readNumber());
                        continue;
                    }
                }
                m_tokens.push_back({TokenKind::MINUS, "-", line, col}); pos++; col++;
                continue;
            }
            if (c == '*') { m_tokens.push_back({TokenKind::STAR, "*", line, col}); pos++; col++; continue; }
            if (c == '=') { m_tokens.push_back({TokenKind::EQUALS, "=", line, col}); pos++; col++; continue; }
            if (c == '<') { m_tokens.push_back({TokenKind::LESS, "<", line, col}); pos++; col++; continue; }
            if (c == '>') { m_tokens.push_back({TokenKind::GREATER, ">", line, col}); pos++; col++; continue; }
            if (c == '(') { m_tokens.push_back({TokenKind::LPAREN, "(", line, col}); pos++; col++; continue; }
            if (c == ')') { m_tokens.push_back({TokenKind::RPAREN, ")", line, col}); pos++; col++; continue; }
            if (c == '{') { m_tokens.push_back({TokenKind::LBRACE, "{", line, col}); pos++; col++; continue; }
            if (c == '}') { m_tokens.push_back({TokenKind::RBRACE, "}", line, col}); pos++; col++; continue; }
            if (c == ',') { m_tokens.push_back({TokenKind::COMMA, ",", line, col}); pos++; col++; continue; }
            if (c == ':') { m_tokens.push_back({TokenKind::COLON, ":", line, col}); pos++; col++; continue; }
            if (std::isdigit(c) || c == '.') {
                m_tokens.push_back(readNumber());
                continue;
            }
            if (std::isalpha(c) || c == '_') {
                m_tokens.push_back(readName());
                continue;
            }
            std::cerr << "Unknown character '" << c << "' at line " << line << " col " << col << "\n";
            pos++; col++;
        }
        m_tokens.push_back({TokenKind::TOK_EOF, "", line, col});
        return m_tokens;
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
    std::vector<Token> m_tokens;
    size_t current;
    mlir::OpBuilder builder;
    mlir::Location loc;
    std::unordered_map<std::string, mlir::Value> variables;

public:
    Parser(std::vector<Token> toks, mlir::OpBuilder &b, mlir::Location l)
        : m_tokens(std::move(toks)), current(0), builder(b), loc(l) {}

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

    Token &peek() { return m_tokens[current]; }
    void advance() { if (current < m_tokens.size() - 1) current++; }
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
class ShaderParser{

    std::vector<Token> m_tokens;
    size_t current;
    mlir::OpBuilder builder;
    mlir::Location loc;
    std::unordered_map<std::string, mlir::Value> variables;

    public:
    ShaderParser(std::vector<Token> toks, mlir::OpBuilder &b, mlir::Location l)
        : m_tokens(std::move(toks)), current(0), builder(b), loc(l) 
    {

    }
        bool parseUniform(mlir::Block *block) {
        advance(); // consume 'uniform'
        if (peek().kind != TokenKind::NAME) return error("expected binding name");
        std::string name = peek().text;
        advance();

        if (peek().text != "set") return error("expected 'set'");
        advance();
        if (!expect(TokenKind::LPAREN)) return error("expected '('");
        if (peek().kind != TokenKind::NUMBER) return error("expected set index");
        int set = std::stoi(peek().text);
        advance();
        if (!expect(TokenKind::RPAREN)) return error("expected ')'");

        if (peek().text != "binding") return error("expected 'binding'");
        advance();
        if (!expect(TokenKind::LPAREN)) return error("expected '('");
        if (peek().kind != TokenKind::NUMBER) return error("expected binding index");
        int binding = std::stoi(peek().text);
        advance();
        if (!expect(TokenKind::RPAREN)) return error("expected ')'");

        if (!expect(TokenKind::COLON)) return error("expected ':'");
        mlir::Type type = parseType();
        if (!type) return false;

        {
            mlir::OpBuilder::InsertionGuard guard(builder);
            builder.setInsertionPointToEnd(block);
            builder.create<fungt::ResourceBindingOp>(
                loc,
                builder.getStringAttr(name),
                builder.getStringAttr("Uniform"),
                builder.getI32IntegerAttr(set),
                builder.getI32IntegerAttr(binding),
                mlir::TypeAttr::get(type));
        }
        return true;
    }

    bool parseInput() {
        advance(); // consume 'in'
        if (peek().kind != TokenKind::NAME) return error("expected input name");
        std::string name = peek().text;
        advance();
        if (peek().text != "location") return error("expected 'location'");
        advance();
        if (!expect(TokenKind::LPAREN)) return error("expected '('");
        if (peek().kind != TokenKind::NUMBER) return error("expected location index");
        int location = std::stoi(peek().text);
        advance();
        if (!expect(TokenKind::RPAREN)) return error("expected ')'");
        if (!expect(TokenKind::COLON)) return error("expected ':'");
        mlir::Type type = parseType();
        if (!type) return false;

        auto inputOp = builder.create<fungt::InputOp>(
            loc, type,
            builder.getI32IntegerAttr(location));
        variables[name] = inputOp.getResult();
        return true;
    }

    bool parseOutput() {
        advance(); // consume 'out'
        if (peek().kind != TokenKind::NAME) return error("expected output name");
        std::string name = peek().text;
        advance();
        if (peek().text != "location") return error("expected 'location'");
        advance();
        if (!expect(TokenKind::LPAREN)) return error("expected '('");
        if (peek().kind != TokenKind::NUMBER) return error("expected location index");
        int location = std::stoi(peek().text);
        advance();
        if (!expect(TokenKind::RPAREN)) return error("expected ')'");

        auto it = variables.find(name);
        if (it == variables.end()) return error("undefined variable: " + name);

        builder.create<fungt::OutputOp>(
            loc, it->second,
            builder.getI32IntegerAttr(location));
        return true;
    }

    bool parseAssignment() {
        std::string name = peek().text;
        advance();
        if (!expect(TokenKind::EQUALS)) return error("expected '='");
        mlir::Value val = parseLoadExpr();
        if (!val) return false;
        variables[name] = val;
        return true;
    }

    mlir::Value parseLoadExpr() {
        if (peek().kind == TokenKind::NAME && peek().text == "load") {
            advance();
            if (!expect(TokenKind::LPAREN)) { error("expected '('"); return nullptr; }
            if (peek().kind != TokenKind::NAME) { error("expected resource name"); return nullptr; }
            std::string resName = peek().text;
            advance();
            if (!expect(TokenKind::RPAREN)) { error("expected ')'"); return nullptr; }
            auto vecType = mlir::VectorType::get({4}, builder.getF32Type());
            return builder.create<fungt::LoadResourceOp>(
                loc, vecType,
                mlir::FlatSymbolRefAttr::get(builder.getContext(), resName)).getResult();
        }
        if (peek().kind == TokenKind::NAME) {
            std::string name = peek().text;
            advance();
            auto it = variables.find(name);
            if (it == variables.end()) { error("undefined variable: " + name); return nullptr; }
            return it->second;
        }
        error("unexpected token in expression: " + peek().text);
        return nullptr;
    }
    bool parseFragmentShader(mlir::Block *moduleBlock) {
        advance(); // consume 'fragment_shader'
        if (!expect(TokenKind::LBRACE)) return error("expected '{'");

        auto shaderOp = builder.create<fungt::ShaderEntryOp>(
            loc,
            builder.getStringAttr("main"),
            builder.getStringAttr("Fragment")
        );

        auto *shaderBlock = new mlir::Block();
        shaderOp.getBody().push_back(shaderBlock);
        builder.setInsertionPointToStart(shaderBlock);

        skipNewlines();
        while (peek().kind != TokenKind::RBRACE && peek().kind != TokenKind::TOK_EOF) {
            skipNewlines();
            if (peek().kind == TokenKind::RBRACE) break;
            if (peek().kind == TokenKind::NAME) {
                if (peek().text == "uniform") {
                   if (!parseUniform(moduleBlock)) return false;
                } else if (peek().text == "in") {
                    if (!parseInput()) return false;
                } else if (peek().text == "out") {
                    if (!parseOutput()) return false;
                } else {
                    if (!parseAssignment()) return false;
                }
            }
            skipNewlines();
        }

        builder.create<fungt::ShaderEndOp>(loc);
        if (!expect(TokenKind::RBRACE)) return error("expected '}'");
        return true;
    }

    bool parseVertexShader(mlir::Block *moduleBlock) {
        advance(); // consume 'vertex_shader'
        if (!expect(TokenKind::LBRACE)) return error("expected '{'");

        auto shaderOp = builder.create<fungt::ShaderEntryOp>(
            loc,
            builder.getStringAttr("main"),
            builder.getStringAttr("Vertex"));

        auto *shaderBlock = new mlir::Block();
        shaderOp.getBody().push_back(shaderBlock);
        builder.setInsertionPointToStart(shaderBlock);

        skipNewlines();
        while (peek().kind != TokenKind::RBRACE && peek().kind != TokenKind::TOK_EOF) {
            skipNewlines();
            if (peek().kind == TokenKind::RBRACE) break;
            if (peek().kind == TokenKind::NAME) {
                if (peek().text == "uniform") {
                    if (!parseUniform(moduleBlock)) return false;
                } else if (peek().text == "in") {
                    if (!parseInput()) return false;
                } else if (peek().text == "out") {
                    if (!parseOutput()) return false;
                } else {
                    if (!parseAssignment()) return false;
                }
            }
            skipNewlines();
        }

        builder.create<fungt::ShaderEndOp>(loc);
        if (!expect(TokenKind::RBRACE)) return error("expected '}'");
        return true;
    }
    bool parse(mlir::Block *moduleBlock) {
        skipNewlines();
        if (peek().kind == TokenKind::NAME && peek().text == "fragment_shader") {
            return parseFragmentShader(moduleBlock);
        }
        if (peek().kind == TokenKind::NAME && peek().text == "vertex_shader") {
            return parseVertexShader(moduleBlock);
        }
        return error("expected 'fragment_shader' or 'vertex_shader'");
    }
    private:

     mlir::Type parseType() {
        if (peek().kind == TokenKind::NAME && peek().text == "vec4") {
            advance();
            return mlir::VectorType::get({4}, builder.getF32Type());
        }
        error("unknown type: " + peek().text);
        return nullptr;
    }

    Token &peek() { return m_tokens[current]; }
    void advance() { if (current < m_tokens.size() - 1) current++; }
    void skipNewlines() { while (peek().kind == TokenKind::NEWLINE) advance(); }

    bool expect(TokenKind kind) {
        if (peek().kind == kind) { advance(); return true; }
        return false;
    }

    bool error(const std::string &msg) {
        std::cerr << "Shader parse error at line " << peek().line
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
    auto m_tokens = tokenizer.tokenize();
    Parser parser(std::move(m_tokens), builder, loc);

    if (!parser.parse(updateBlock, updateBlock->getArguments())) {
        return nullptr;
    }

    builder.setInsertionPointToEnd(entryBlock);
    builder.create<mlir::func::ReturnOp>(loc, updateOp.getResults());

    return moduleOp;
}

mlir::OwningOpRef<mlir::ModuleOp> parseShaderIR(mlir::MLIRContext &ctx, llvm::StringRef source)
{
    ctx.loadDialect<fungt::FunGTDialect>();
    ctx.loadDialect<arith::ArithDialect>();
    ctx.loadDialect<spirv::SPIRVDialect>();

    auto loc = mlir::UnknownLoc::get(&ctx);
    mlir::OpBuilder builder(&ctx);

    auto moduleOp = mlir::ModuleOp::create(loc);
    builder.setInsertionPointToEnd(moduleOp.getBody());

    Tokenizer tokenizer(source);
    auto m_tokens = tokenizer.tokenize();
    ShaderParser parser(std::move(m_tokens), builder, loc);

    if (!parser.parse(moduleOp.getBody())) return nullptr;
    return moduleOp;
}

} // namespace fungt
} // namespace mlir