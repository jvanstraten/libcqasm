#ifndef _CQASM_AST_HPP_INCLUDED_
#define _CQASM_AST_HPP_INCLUDED_

#include <cstdint>
#include <cstdlib>
#include <string>
#include <sstream>
#include <memory>
#include <vector>

namespace cqasm {
namespace ast {

using Str = std::string;
using Int = std::int64_t;
using Real = double;

template <class T>
class Single {
public:
  std::shared_ptr<T> val;
  void set_raw(T *ptr) { this->val = std::shared_ptr<T>(ptr); }
};

template <class T>
class Multiple {
public:
  std::vector<std::shared_ptr<T>> vec;
  void add_raw(T *ptr) { this->vec.emplace_back(ptr); }
  void extend(Multiple<T> &other) {
    this->vec.insert(this->vec.end(), other.vec.begin(), other.vec.end());
  }
};

class Node {
public:
  virtual ~Node() {};
};

class Operand : public Node {
public:
  virtual ~Operand() {};
};

class Expression : public Operand {
public:
  virtual ~Expression() {};
};

class IndexEntry : public Node {
public:
  virtual ~IndexEntry() {};
};

class IndexItem : public IndexEntry {
public:
  Single<Expression> index;
};

class IndexRange : public IndexEntry {
public:
  Single<Expression> start;
  Single<Expression> end;
};

class IndexList : public Node {
public:
  Multiple<IndexEntry> items;
};

class IntegerLiteral : public Expression {
public:
  Int value;
};

class FloatLiteral : public Expression {
public:
  Real value;
};

class Identifier : public Expression {
public:
  Str name;
};

class Index : public Expression {
public:
  Single<Expression> expr;
  Single<IndexList> indices;
};

class UnaryOp : public Expression {
public:
  Single<Expression> expr;
  virtual ~UnaryOp() {};
};

class Negate : public UnaryOp {
};

class BinaryOp : public Expression {
public:
  Single<Expression> lhs;
  Single<Expression> rhs;
  virtual ~BinaryOp() {};
};

class Multiply : public BinaryOp {
};

class Add : public BinaryOp {
};

class Subtract : public BinaryOp {
};

class ErroneousExpression : public Expression {
};

class ExpressionList : public Node {
public:
  Multiple<Expression> items;
};

class MatrixLiteral : public Operand {
public:
  Multiple<ExpressionList> items;
  void set_raw_square_pairs(ExpressionList *square_pairs);
};

class StringBuilder : public Node {
public:
  std::ostringstream stream;
  void push_string(const std::string &str);
  void push_escape(const std::string &escape);
};

class StringLiteral : public Operand {
public:
  Str value;
};

class JsonLiteral : public Operand {
public:
  Str value;
};

class OperandList : public Node {
public:
  Multiple<Operand> items;
};

class AnnotationData : public Node {
public:
  Single<Identifier> interface;
  Single<Identifier> operation;
  Single<OperandList> operands;
};

class Annotated : public Node {
public:
  Multiple<AnnotationData> annotations;
  virtual ~Annotated() {};
};

class Instruction : public Annotated {
public:
  Single<Identifier> name;
  Single<Expression> condition;
  Single<OperandList> operands;
};

class Statement : public Annotated {
public:
  virtual ~Statement() {};
};

class Bundle : public Statement {
public:
  Multiple<Instruction> items;
};

class Mapping : public Statement {
public:
  Single<Identifier> alias;
  Single<Expression> expr;
};

class Subcircuit : public Statement {
public:
  Single<Identifier> name;
  Single<Expression> iterations;
};

class ErroneousStatement : public Statement {
};

class StatementList : public Node {
public:
  Multiple<Statement> items;
};

class Version : public Node {
public:
  std::vector<Int> items;
};

class Root : public Node {
public:
  virtual ~Root() {};
};

class Program : public Root {
public:
  Single<Version> version;
  Single<Expression> num_qubits;
  Single<StatementList> statements;
};

class ErroneousProgram : public Root {
};

} // namespace ast
} // namespace cqasm

#endif
