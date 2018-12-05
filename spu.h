#ifndef _SPU_H_
#define _SPU_H_

#if defined(_MSC_VER) || defined(__CYGWIN__)
#define DLL_EXPORT __declspec(dllexport)
#else // defined(_MSC_VER) || defined(__CYGWIN__)
#define DLL_EXPORT
#endif // defined(_MSC_VER) || defined(__CYGWIN__)

extern std::ostream out;

extern void(*output3ac)(std::ostream&, const COMPILER::tac*);

extern void genobj(const COMPILER::scope* tree);

extern std::string func_label;

extern void genfunc(const COMPILER::fundef* func, const std::vector<COMPILER::tac*>& vc);

struct reg {
  int m_no;
  reg(int no) : m_no(no) {}
};

inline std::ostream& operator<<(std::ostream& os, const reg& r)
{
  return os << '$' << r.m_no;
}

class address {
public:
  virtual void load(const reg&, int = 0) const = 0;
  virtual void store(const reg&, int = 0) const = 0;
  virtual void get(const reg&) const = 0;
  virtual ~address(){}
};

extern std::map<const COMPILER::var*, address*> address_descriptor;

extern bool string_literal(const COMPILER::usr*);

class mem : public address {
  std::string m_label;
  int m_size;
public:
  mem(std::string label, const COMPILER::type*);
  mem(std::string label, int size);
  void load(const reg&, int = 0) const;
  void store(const reg&, int = 0) const;
  void get(const reg&) const;
  std::string label() const { return m_label; }
};

enum section { rom = 1, ram, ctor, dtor };

extern void output_section(section);

inline bool is_top(COMPILER::scope* tree){ return !tree->m_parent; }

extern std::string new_label();

class imm : public address {
  std::string m_value;
  std::pair<int,int> m_pair;
  struct bad_op {};
public:
  imm(const COMPILER::usr*);
  void load(const reg&, int = 0) const;
  void store(const reg&, int = 0) const { throw bad_op(); }
  void get(const reg&) const { throw bad_op(); }
};

extern bool integer_suffix(int);

struct function_exit {
  std::string m_label;
  bool m_ref;
};

extern struct function_exit function_exit;

extern bool debug_flag;

struct stack_layout {
  int m_fun_arg;
  int m_size;
};

extern struct stack_layout stack_layout;

class stack : public address {
  int m_offset;
  int m_size;
  bool m_big_aggregate_param;
  void store_scalar(const reg&) const;
  void store_aggregate(const reg&, int) const;
  void load_normal(const reg&, int) const;
  void load_special(const reg&, int) const;
public:
  stack(int offset, int size, bool flag = false)
    : m_offset(offset), m_size(size), m_big_aggregate_param(flag) {}
  void load(const reg&, int = 0) const;
  void store(const reg&, int = 0) const;
  void get(const reg&) const;
  int offset() const { return m_offset; }
};

class alloced_addr : public address {
  stack m_stack;
  struct bad_op {};
public:
  alloced_addr(int offset) : m_stack(offset,4) {}
  void load(const reg&, int = 0) const { throw bad_op(); }
  void store(const reg&, int = 0) const { throw bad_op(); }
  void get(const reg& r) const { m_stack.load(r); }
  void set(const reg& r) const { m_stack.store(r); }
};

extern std::auto_ptr<mem> double_0x80000000;
extern std::auto_ptr<mem> _02031011121380800607141516178080;
extern std::auto_ptr<mem> _04050607808080800c0d0e0f80808080;
extern std::auto_ptr<mem> _80000000000000000000000000000000;
extern std::auto_ptr<mem> _0000043e0000041e0000000000000000;
extern std::auto_ptr<mem> _02031011121380800607141516178080;
extern std::auto_ptr<mem> _43e0000000000000;
extern std::auto_ptr<mem> _12121212121212131212121212121212;
extern std::auto_ptr<mem> _10101010101002031010101010101010;

extern std::string pattern_operation(int);

extern std::map<const COMPILER::var*, stack*> big_aggregate_param;

class parameter {
  int m_cnt;
public:
  parameter(const COMPILER::type*, int*);
  int operator()(int, const COMPILER::usr*);
  static int m_hidden;
};

extern bool has_allocatable;
extern void enter_helper(int);
extern void leave_helper(int);

#endif // _SPU_H_
