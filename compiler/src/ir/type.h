#ifndef TYPE_H
#define TYPE_H

#include <iosfwd>
#include <memory>
#include <vector>

namespace ir {

class type {
  public:
    virtual bool composite() const noexcept = 0;

  private:
    friend std::ostream & operator<<(std::ostream & lhs, const type & rhs) {
        rhs.print(lhs);
        return lhs;
    }

    virtual void print(std::ostream &) const = 0;
};
using type_ptr = std::shared_ptr<type>;

class unit_type final : public type {
    public:
    bool composite() const noexcept final { return false; }
    static type_ptr instance;

  private:
    void print(std::ostream &) const final;
};

class string_type final : public type {
  public:
    bool composite() const noexcept final { return false; }
    static type_ptr instance;

  private:
    void print(std::ostream &) const final;
};

class integer_type final : public type {
  public:
    bool composite() const noexcept final { return false; }
    static type_ptr instance;

  private:
    void print(std::ostream &) const final;
};

class floating_type final : public type {
  public:
    bool composite() const noexcept final { return false; }
    static type_ptr instance;

  private:
    void print(std::ostream &) const final;
};

class boolean_type final : public type {
  public:
    bool composite() const noexcept final { return false; }
    static type_ptr instance;

  private:
    void print(std::ostream &) const final;
};

class character_type final : public type {
  public:
    bool composite() const noexcept final { return false; }
    static type_ptr instance;

  private:
    void print(std::ostream &) const final;
};

class func_type final : public type {
  public:
    bool composite() const noexcept final { return true; }

    explicit func_type(std::vector<type_ptr> && args = {}, type_ptr ret = nullptr)
        : arg_types{std::move(args)}
        , ret_type{std::move(ret)} {}

  private:
    void print(std::ostream &) const final;

    std::vector<type_ptr> arg_types;
    type_ptr ret_type;
};

type_ptr ast_to_ir_type(const std::string &);

} // namespace ir

#endif
