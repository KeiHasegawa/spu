#include "stdafx.h"

#ifdef CXX_GENERATOR
#include "cxx_core.h"
#else // CXX_GENERATOR
#include "c_core.h"
#endif // CXX_GENERATOR

#include "spu.h"
#include "gencode.h"

void enter(const COMPILER::fundef* func, const std::vector<COMPILER::tac*>& vc);
void leave();

std::string func_label;

void genfunc(const COMPILER::fundef* func, const std::vector<COMPILER::tac*>& v3ac)
{
  using namespace std;
  using namespace COMPILER;
  output_section(rom);
  const usr* entry = func->m_usr;
  usr::flag_t flag = entry->m_flag;
  func_label = entry->m_name;
#ifdef CXX_GENERATOR
  func_label = scope_name(entry->m_scope);
  func_label += func_name(entry->m_name);
  if (!(flag & usr::C_SYMBOL))
    func_label += signature(entry->m_type);
#endif // CXX_GENERATOR
  if ( flag & usr::STATIC)
    out << '\t' << ".global" << '\t' << func_label << '\n';
  out << '\t' << ".align" << '\t' << 4 << '\n';
  out << func_label << ":\n";
  enter(func,v3ac);
  function_exit.m_label = new_label();
  function_exit.m_ref = false;
  if ( !v3ac.empty() )
    for_each(v3ac.begin(),v3ac.end(),gencode(v3ac));
  leave();
}

int sched_stack(const COMPILER::fundef* func, const std::vector<COMPILER::tac*>&);

class save_param {
  int m_cnt;
  void common(const COMPILER::usr*, int);
public:
  save_param();
  void operator()(const COMPILER::usr*);
  static void reg_handle(const reg&, int);
};

void save_if_varg(const COMPILER::type*);

void save_sp(int);

void shift_sp(int);

void enter_helper(int n)
{
  out << '\t' << "stqd" << '\t' << "$lr,16($sp)" << '\n';
  save_sp(n);
  shift_sp(-n);
}

void enter(const COMPILER::fundef* func,
           const std::vector<COMPILER::tac*>& v3ac)
{
  using namespace std;
  using namespace COMPILER;
  if ( debug_flag )
    out << '\t' << "/* enter */" << '\n';

  has_allocatable = false;
  int n = sched_stack(func,v3ac);
  if ( has_allocatable )
    out << '\t' << "stqd" << '\t' << "$127,-16($sp)" << '\n';
  enter_helper(n);
  if ( has_allocatable )
    out << '\t' << "ori" << '\t' << "$127,$sp,0" << '\n';

#ifdef CXX_GENERATOR
  typedef scope param_scope;
#endif // CXX_GENERATOR
  param_scope* param = func->m_param;
  const vector<usr*>& vec = param->m_order;
  for_each(vec.begin(),vec.end(),save_param());
  save_if_varg(func->m_usr->m_type);
}

void save_sp(int n)
{
  if ( n <= 8192 )
    out << '\t' << "stqd" << '\t' << "$sp," << -n << "($sp)" << '\n';
  else {
    reg r(2);
    out << '\t' << "il" << '\t' << r << ',' << -n << '\n';
    out << '\t' << "a" << '\t' << r << ',' << r << ',' << "$sp" << '\n';
    out << '\t' << "stqd" << '\t' << "$sp," << 0 << '(' << r << ')' << '\n';
  }
}

void clear_address_descriptor()
{
        using namespace std;
        using namespace COMPILER;
        typedef map<const var*, address*>::iterator IT;
        for (IT p = address_descriptor.begin(); p != address_descriptor.end(); ) {
                const var* entry = p->first;
                if (is_top(entry->m_scope))
                        ++p;
                else {
                        IT q = p++;
                        delete q->second;
                        address_descriptor.erase(q);
                }
        }
}

inline void destroy(const std::pair<const COMPILER::var*, stack*>& x)
{
  delete x.second;
}

void clear_big_aggregate_param()
{
        using namespace std;
        for_each(big_aggregate_param.begin(),big_aggregate_param.end(),destroy);
        big_aggregate_param.clear();
}

void leave_helper(int n)
{
  shift_sp(n);
  if ( has_allocatable )
    out << '\t' << "lqd" << '\t' << "$127,-16($sp)" << '\n';
  out << '\t' << "lqd" << '\t' << "$lr,16($sp)" << '\n';
  out << '\t' << "bi" << '\t' << "$lr" << '\n';
}

void leave()
{
  if ( debug_flag )
    out << '\t' << "/* leave */" << '\n';
  if ( function_exit.m_ref )
    out << function_exit.m_label << ":\n";
  if ( has_allocatable )
    out << '\t' << "ori" << '\t' << "$sp,$127,0" << '\n';
  leave_helper(stack_layout.m_size);

  clear_address_descriptor();
  clear_big_aggregate_param();
}

int fun_arg(const std::vector<COMPILER::tac*>&);

int local_variable(const COMPILER::fundef* func, int offset);

inline int align(int offset, int size)
{
  int n = size - 1;
  return (offset + n) & ~n;
}

int sched_stack(const COMPILER::fundef* func, const std::vector<COMPILER::tac*>& v3ac)
{
  int n = fun_arg(v3ac);
  stack_layout.m_fun_arg = n;
  n = local_variable(func,n);
  if ( has_allocatable )
    n += 16;
  n = align(n,16);
  stack_layout.m_size = n;
  return n;
}

class arg_count {
  int* m_res;
  int m_curr;
public:
  arg_count(int* res) : m_res(res), m_curr(0) {}
  void operator()(const COMPILER::tac*);
};

class param_space_big_aggregate {
  int m_cnt;
public:
  param_space_big_aggregate();
  int operator()(int, const COMPILER::tac*);
};

int fun_arg(const std::vector<COMPILER::tac*>& v3ac)
{
  using namespace std;
  int n = 0;
  for_each(v3ac.begin(),v3ac.end(),arg_count(&n));
  n = (n < 1232 ? 0 : n - 1232) + 32;
  return accumulate(v3ac.begin(),v3ac.end(),n,param_space_big_aggregate());
}

inline bool cmp_id(const COMPILER::tac* ptr, COMPILER::tac::id_t id)
{
  return ptr->m_id == id;
}

void arg_count::operator()(const COMPILER::tac* ptr)
{
  using namespace std;
  using namespace COMPILER;

  if (cmp_id(ptr, tac::PARAM)) {
    var* y = ptr->y;
    const type* T = y->m_type;
    T = T->promotion();
    int size = T->size();
    size = align(size, 16);
    m_curr += size;
  }
  else if (cmp_id(ptr,tac::CALL)) {
    *m_res = max(*m_res, m_curr);
    m_curr = 0;
  }
}

param_space_big_aggregate::param_space_big_aggregate() : m_cnt(-1) {}

int param_space_big_aggregate::operator()(int offset, const COMPILER::tac* ptr)
{
        using namespace std;
        using namespace COMPILER;
        ++m_cnt;
        if (!cmp_id(ptr, tac::PARAM))
                return offset;
        var* entry = ptr->y;
        const type* T = entry->m_type;
        int size = T->size();
        if (size <= 1232 - 16 * m_cnt)
                return offset;

        offset = align(offset, 16);
        big_aggregate_param[entry] = new ::stack(offset, -1);
        return offset + size;
}

class recursive_locvar {
  int* m_res;
  static int add1(int, const std::pair<std::string, std::vector<COMPILER::usr*> >&);
  static int add2(int, const COMPILER::usr*);
  static int add3(int, const COMPILER::var*);
public:
  recursive_locvar(int* p) : m_res(p) {}
  void operator()(const COMPILER::scope*);
};

int decide_if_varg(int, const COMPILER::type*, const std::vector<COMPILER::usr*>&);

int local_variable(const COMPILER::fundef* func, int n)
{
  using namespace std;
  using namespace COMPILER;
#ifdef CXX_GENERATOR
  typedef scope param_scope;
#endif // CXX_GENERATOR
  const param_scope* param = func->m_param;
  assert(param->m_children.size() == 1);
  scope* tree = param->m_children[0];

  recursive_locvar recursive_locvar(&n);
  recursive_locvar(tree);

  const vector<usr*>& order = param->m_order;
  n = align(n,16);
  usr* u = func->m_usr;
  const type* T = u->m_type;
  n = accumulate(order.begin(), order.end(),n,parameter(T,&n));
  return decide_if_varg(n,T,order);
}

void recursive_locvar::operator()(const COMPILER::scope* tree)
{
  using namespace std;
  using namespace COMPILER;
  if (tree->m_id != scope::BLOCK)
    return;
  const map<string, vector<usr*> >& usrs = tree->m_usrs;
  *m_res = accumulate(usrs.begin(), usrs.end(), *m_res, add1);
  const block* b = static_cast<const block*>(tree);
  const vector<var*>& vars = b->m_vars;
  *m_res = accumulate(vars.begin(), vars.end(), *m_res, add3);
  const vector<scope*>& children = tree->m_children;
  for_each(children.begin(), children.end(), recursive_locvar(m_res));
}

int recursive_locvar::add1(int n,
                           const std::pair<std::string,
                           std::vector<COMPILER::usr*> >& x)
{
  using namespace std;
  using namespace COMPILER;
  const vector<usr*>& vec = x.second;
  return accumulate(vec.begin(), vec.end(), n, add2);
}

int recursive_locvar::add2(int offset, const COMPILER::usr* entry)
{
  using namespace std;
  using namespace COMPILER;
  usr::flag_t flag = entry->m_flag;
  usr::flag_t mask = usr::flag_t(usr::STATIC|usr::EXTERN|usr::FUNCTION);
  if (flag & mask)
    return offset;

  if (entry->isconstant())
    return offset;

  if (flag & usr::VL) {
    offset = align(offset, 4);
    address_descriptor[entry] = new alloced_addr(offset);
    has_allocatable = true;
    return offset + 4;
  }

  const type* T = entry->m_type;
  int a = T->align();
  if (!T->scalar())
    a = (a + 15) & ~15;
  offset = align(offset, a);
  int size = T->size();
  int s = T->scalar() ? size : -1;
  address_descriptor[entry] = new ::stack(offset,s);
  return offset + size;
}

int recursive_locvar::add3(int offset, const COMPILER::var* entry)
{
  using namespace COMPILER;
  const type* T = entry->m_type;
  int size = T->size();
  address_descriptor[entry] = new stack(offset, size);
  return offset + size;
}

parameter::parameter(const COMPILER::type* T, int* p) : m_cnt(-1)
{
  using namespace COMPILER;
  assert(T->m_id == type::FUNC);
  typedef const func_type FUNC;
  FUNC* func = static_cast<FUNC*>(T);
  T = func->return_type();
  T = T->unqualified();

  if (T->m_id != type::RECORD) {
    m_hidden = -1;
    return;
  }

  int size = T->size();
  if ( size > 1232 ){
    m_hidden = *p;
    *p += 4;
    ++m_cnt;
  }
  else
    m_hidden = -1;
}

int calc_offset(int n, int size)
{
  switch (size) {
  case 1:  return n + 3;
  case 2:  return n + 2;
  default: return n;
  }
}

int parameter::operator()(int n, const COMPILER::usr* entry)
{
  using namespace COMPILER;
  if (++m_cnt > 77)
    return n;
  const type* T = entry->m_type;
  int size = T->size();
  int sz = T->scalar() ? size : -1;
  int offset = calc_offset(n,sz);
  int m = 1232 - 16 * m_cnt;
  address_descriptor[entry] = new stack(offset,sz,size>m);
  if ( T->scalar() || size > m )
    return n + 16;
  return n + 16 * ((size + 15) / 16);
}

int parameter::m_hidden;

bool take_varg(const COMPILER::type* T)
{
  using namespace std;
  using namespace COMPILER;
  assert(T->m_id == type::FUNC);
  typedef const func_type FUNC;
  FUNC* func = static_cast<FUNC*>(T);
  const vector<const type*>& param = func->param();
  if (param.empty())
    return false;
  T = param.back();
  return T->m_id == type::ELLIPSIS;
}

class add_reg {
  int m_cnt;
public:
  add_reg();
  int operator()(int, const COMPILER::usr*);
};

struct current_varg {
  int m_offset;
  int m_number;
} current_varg;

int decide_if_varg(int n, const COMPILER::type* T, const std::vector<COMPILER::usr*>& param)
{
  using namespace std;
  if (!take_varg(T)) {
    current_varg.m_offset = current_varg.m_number = 0;
    return n;
  }

  current_varg.m_offset = n;
  int m = 77;
  m -= accumulate(param.begin(), param.end(), 0, add_reg());
  if ( m < 0 )
    m = 0;
  current_varg.m_number = m;
  return n + 16 * m;
}

add_reg::add_reg() : m_cnt(-1) {}

int add_reg::operator()(int n, const COMPILER::usr* entry)
{
  using namespace COMPILER;
  ++m_cnt;
  const type* T = entry->m_type;
  if (T->scalar())
    return n + 1;
  int size = T->size();
  if (size > 1232 - 16 * m_cnt)
    return n + 1;
  else
    return n + (size + 15) / 16;
}

save_param::save_param() : m_cnt(-1)
{
  if ( parameter::m_hidden < 0 )
    return;
  reg r3(3);
  out << '\t' << "stqd" << '\t' << r3 << ',' << parameter::m_hidden << "($sp)" << '\n';
  ++m_cnt;
}

void save_param::operator()(const COMPILER::usr* entry)
{
  using namespace COMPILER;
  const type* T = entry->m_type;
  int size = T->size();
  if (size > 1232 - 16 * (m_cnt + 1))
    size = 1;
  int n = T->scalar() ? 1 : (size + 15) / 16;
  for ( int i = 0 ; i < n ; ++i )
    common(entry,16*i);
}

void save_param::common(const COMPILER::usr* entry, int delta)
{
  using namespace std;
  using namespace COMPILER;
  if (++m_cnt >= 77) {
    if (delta)
      return;
    int offset = stack_layout.m_size + 32 + 16 * (m_cnt - 77);
    const type* T = entry->m_type;
    int size = T->size();
    size = T->scalar() ? size : -1;
    offset = calc_offset(offset, size);
    address_descriptor[entry] = new ::stack(offset, size);
    return;
  }
  map<const var*, address*>::const_iterator p = address_descriptor.find(entry);
  assert(p != address_descriptor.end());
  address* ad = p->second;
  ::stack* st = static_cast<::stack*>(ad);
  int offset = st->offset();
  offset &= ~15;
  reg r(m_cnt+3);
  reg_handle(r,offset+delta);
}

void save_param::reg_handle(const reg& r, int offset)
{
  if (offset < 8192)
    out << '\t' << "stqd" << '\t' << r << ',' << offset << "($sp)" << '\n';
  else {
    reg t(2);
    out << '\t' << "ilhu" << '\t' << t << ',' << (offset >> 16) << '\n';
    out << '\t' << "iohl" << '\t' << t << ',' << (offset & 0xffff) << '\n';
    out << '\t' << "a" << '\t' << t << ',' << t << ',' << "$sp" << '\n';
    out << '\t' << "stqd" << '\t' << r << ',' << 0 << '(' << t << ')' << '\n';
  }
}

void save_if_varg(const COMPILER::type* T)
{
  if ( !take_varg(T) )
    return;
  int off = current_varg.m_offset;
  int m = current_varg.m_number;
  for ( int i = 80 - m ; i < 80 ; ++i, off += 16 )
    save_param::reg_handle(reg(i),off);
}

void shift_sp(int n)
{
  if ( -512 <= n && n < 512 )
    out << '\t' << "ai" << '\t' << "$sp,$sp," << n << '\n';
  else {
    reg r(80);
    out << '\t' << "il" << '\t' << r << ',' << n << '\n';
    out << '\t' << "a" << '\t' << "$sp,$sp," << r << '\n';
  }
}

