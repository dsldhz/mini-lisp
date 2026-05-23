#ifndef VALUE_H
#define VALUE_H

#include <memory>
#include <string>
#include <ostream>
#include <vector>
#include <optional>

class Value {
public:
    virtual ~Value() = default;
    virtual std::string toString() const = 0;
    virtual bool isNil() const { return false; }
    virtual bool isSelfEvaluating() const { return false; }
    virtual std::vector<std::shared_ptr<Value>> toVector() const;
    virtual std::optional<std::string> asSymbol() const { return std::nullopt; }
};

using ValuePtr = std::shared_ptr<Value>;

class BooleanValue : public Value {
private:
    bool value;
public:
    BooleanValue(bool value);
    bool getValue() const;
    std::string toString() const override;
    bool isSelfEvaluating() const override { return true; }
};

class NumericValue : public Value {
private:
    double value;
public:
    NumericValue(double value);
    double getValue() const;
    std::string toString() const override;
    bool isSelfEvaluating() const override { return true; }
};

class StringValue : public Value {
private:
    std::string value;
public:
    StringValue(const std::string& value);
    const std::string& getValue() const;
    std::string toString() const override;
    bool isSelfEvaluating() const override { return true; }
};

class NilValue : public Value {
public:
    NilValue() = default;
    std::string toString() const override;
    bool isNil() const override { return true; }
};

class SymbolValue : public Value {
private:
    std::string name;
public:
    SymbolValue(const std::string& name);
    const std::string& getName() const;
    std::string toString() const override;
    std::optional<std::string> asSymbol() const override { return name; }
};

class PairValue : public Value {
private:
    ValuePtr left;
    ValuePtr right;
public:
    PairValue(ValuePtr left, ValuePtr right);
    ValuePtr getLeft() const;
    ValuePtr getRight() const;
    std::string toString() const override;
};

std::ostream& operator<<(std::ostream& os, const Value& value);

#endif
