#include "stdafx.h"
#include "c_core.h"

#define COMPILER c_compiler

#include "spu.h"
#include "gencode.h"

namespace gencode_impl {
        const COMPILER::tac* m_last;
}

gencode::gencode(const std::vector<COMPILER::tac*>& v3ac)
  : m_v3ac(v3ac), m_counter(-1)
{
        gencode_impl::m_last = v3ac.back();
}

namespace gencode_impl {
  bool big_aggregate_stuff;
  inline bool cmp_id(const COMPILER::tac* ptr, COMPILER::tac::id_t id)
  {
    return ptr->m_id == id;
  }
  class copy_big_aggregate_param {
    int m_cnt;
  public:
    copy_big_aggregate_param();
    void operator()(const COMPILER::tac*);
  };
  void prepare_big_aggregate_return(const COMPILER::tac*);
  struct tac_table : std::map<COMPILER::tac::id_t, void(*)(const COMPILER::tac*)> {
    tac_table();
  } m_tac_table;
};

void gencode::operator()(const COMPILER::tac* ptr)
{
  using namespace std;
  using namespace COMPILER;
  using namespace gencode_impl;

  ++m_counter;
  if ( debug_flag ){
    out << '\t' << "/* ";
    output3ac(out, ptr);
    out << " */" << '\n';
  }

  if ( cmp_id(ptr, tac::PARAM) && !big_aggregate_stuff ){
          vector<tac*>::const_iterator p =
                  find_if(m_v3ac.begin()+m_counter,m_v3ac.end(),bind2nd(ptr_fun(cmp_id),tac::CALL));
          for_each(m_v3ac.begin()+m_counter,p,copy_big_aggregate_param());
          assert(p != m_v3ac.end());
          prepare_big_aggregate_return(*p);
          big_aggregate_stuff = true;
  }
  else if ( cmp_id(ptr,tac::CALL) )
    big_aggregate_stuff = false;

  m_tac_table[ptr->m_id](ptr);
}

namespace gencode_impl {
        address* getaddr(const COMPILER::var*);
}

gencode_impl::copy_big_aggregate_param::copy_big_aggregate_param()
  : m_cnt(-1) {}

namespace gencode_impl {
        void copy(address* dst, address* src, int size);
}

void gencode_impl::copy_big_aggregate_param::operator()(const COMPILER::tac* ptr)
{
  using namespace COMPILER;
  ++m_cnt;
  assert(ptr->m_id == tac::PARAM);
  var* entry = ptr->y;
  const type* T = entry->m_type;
  int size = T->size();
  if (size <= 1232 - 16 * m_cnt)
    return;
  ::stack* dst = big_aggregate_param[entry];
  address* src = getaddr(entry);
  copy(dst, src, size);
}

void gencode_impl::copy(address* dst, address* src, int size)
{
  if (dst && src) {
    reg r3(3);
    dst->get(r3);
    reg r4(4);
    src->get(r4);
  }
  else if (dst) {
    reg r3(3);
    dst->get(r3);
  }
  else if (src) {
    reg r4(4);
    src->get(r4);
  }
  reg r5(5);
  out << '\t' << "il" << '\t' << r5 << ',' << size << '\n';
  out << '\t' << "ai" << '\t' << "$sp,$sp,-32" << '\n';
  out << '\t' << "brsl" << '\t' << "$lr, memcpy" << '\n';
  out << '\t' << "ai" << '\t' << "$sp,$sp,32" << '\n';
}

namespace gencode_impl {
  void assign(const COMPILER::tac*);
  void add(const COMPILER::tac*);
  void sub(const COMPILER::tac*);
  void mul(const COMPILER::tac*);
  void div(const COMPILER::tac*);
  void mod(const COMPILER::tac*);
  void lsh(const COMPILER::tac*);
  void rsh(const COMPILER::tac*);
  void _and(const COMPILER::tac*);
  void _xor(const COMPILER::tac*);
  void _or(const COMPILER::tac*);
  void uminus(const COMPILER::tac*);
  void tilde(const COMPILER::tac*);
  void tc(const COMPILER::tac*);
  void addr(const COMPILER::tac*);
  void invladdr(const COMPILER::tac*);
  void invraddr(const COMPILER::tac*);
  void loff(const COMPILER::tac*);
  void roff(const COMPILER::tac*);  
  void param(const COMPILER::tac*);
  void call(const COMPILER::tac*);
  void _return(const COMPILER::tac*);
  void ifgoto(const COMPILER::tac*);
  void _goto(const COMPILER::tac*);
  void to(const COMPILER::tac*);
  void alloc(const COMPILER::tac*);
  void dealloc(const COMPILER::tac*);
  void asm_(const COMPILER::tac*);
  void _va_start(const COMPILER::tac*);
  void _va_arg(const COMPILER::tac*);
  void _va_end(const COMPILER::tac*);
}

gencode_impl::tac_table::tac_table()
{
  using namespace COMPILER;
  (*this)[tac::ASSIGN] = assign;
  (*this)[tac::ADD] = add;
  (*this)[tac::SUB] = sub;
  (*this)[tac::MUL] = mul;
  (*this)[tac::DIV] = div;
  (*this)[tac::MOD] = mod;
  (*this)[tac::LSH] = lsh;
  (*this)[tac::RSH] = rsh;
  (*this)[tac::AND] = _and;
  (*this)[tac::XOR] = _xor;
  (*this)[tac::OR] = _or;
  (*this)[tac::UMINUS] = uminus;
  (*this)[tac::TILDE] = tilde;
  (*this)[tac::CAST] = tc;
  (*this)[tac::ADDR] = addr;
  (*this)[tac::INVLADDR] = invladdr;
  (*this)[tac::INVRADDR] = invraddr;
  (*this)[tac::LOFF] = loff;
  (*this)[tac::ROFF] = roff;
  (*this)[tac::PARAM] = param;
  (*this)[tac::CALL] = call;
  (*this)[tac::RETURN] = _return;
  (*this)[tac::GOTO] = _goto;
  (*this)[tac::TO] = to;
  (*this)[tac::ALLOC] = alloc;
  (*this)[tac::DEALLOC] = dealloc;
  (*this)[tac::ASM] = asm_;
  (*this)[tac::VASTART] = _va_start;
  (*this)[tac::VAARG] = _va_arg;
  (*this)[tac::VAEND] = _va_end;
}

namespace gencode_impl {
  void assign_scalar(const COMPILER::tac*);
  void assign_aggregate(const COMPILER::tac*);
}

void gencode_impl::assign(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  const type* T = tac->x->m_type;
  T->scalar() ? assign_scalar(tac) : assign_aggregate(tac);
}

void gencode_impl::assign_scalar(const COMPILER::tac* tac)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  reg r(3);
  y->load(r);
  x->store(r);
}

void gencode_impl::assign_aggregate(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  const type* T = tac->x->m_type;
  int size = T->size();
  copy(x, y, size);
}

namespace gencode_impl {
  void binop_notlonglong(const COMPILER::tac*, std::string);
  void add_longlong(const COMPILER::tac*);
}

void gencode_impl::add(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  const type* T = tac->x->m_type;
  int size = T->size();
  if ( T->real() )
    size < 8 ? binop_notlonglong(tac,"fa") : binop_notlonglong(tac,"dfa");
  else
    size < 8 ? binop_notlonglong(tac,"a") : add_longlong(tac);
}

void gencode_impl::add_longlong(const COMPILER::tac* tac)
{
  using namespace std;
  using namespace COMPILER;

  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  address* z = getaddr(tac->z);

  reg a(3); y->load(a);
  reg b(4); z->load(b);
  reg c(5);
  out << '\t' << "cg" << '\t' << c << ',' << b << ',' << a << '\n';
  if ( !_04050607808080800c0d0e0f80808080.get() ){
    const type* T = tac->x->m_type;
    _04050607808080800c0d0e0f80808080 =
      auto_ptr<mem>(new mem(new_label(),T));
  }
  reg d(2);
  _04050607808080800c0d0e0f80808080->load(d);
  reg e(6);
  out << '\t' << "shufb" << '\t' << e << ',' << c << ',' << c << ',' << d << '\n';
  out << '\t' << "addx" << '\t' << e << ',' << b << ',' << a << '\n';
  out << '\t' << "ori" << '\t' << a << ',' << e << ',' << 0 << '\n';
  x->store(a);
}

namespace gencode_impl {
  void sub_longlong(const COMPILER::tac*);
}

void gencode_impl::sub(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  const type* T = tac->x->m_type;
  int size = T->size();
  if ( T->real() )
    size < 8 ? binop_notlonglong(tac,"fs") : binop_notlonglong(tac,"dfs");
  else
    size < 8 ? binop_notlonglong(tac,"sf") : sub_longlong(tac);
}

void gencode_impl::sub_longlong(const COMPILER::tac* tac)
{
  using namespace std;
  using namespace COMPILER;

  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  address* z = getaddr(tac->z);

  reg a(3); y->load(a);
  reg b(4); z->load(b);
  reg c(8);
  out << '\t' << "clgt" << '\t' << c << ',' << b << ',' << a << '\n';
  reg d(7);
  out << '\t' << "sf" << '\t' << d << ',' << b << ',' << a << '\n';
  reg e(2);
  if ( !_04050607808080800c0d0e0f80808080.get() ){
    const type* T = tac->x->m_type;
    _04050607808080800c0d0e0f80808080 =
      auto_ptr<mem>(new mem(new_label(),T));
  }
  _04050607808080800c0d0e0f80808080->load(e);
  reg f(5);
  out << '\t' << "shufb" << '\t' << f << ',' << c << ',' << c << ',' << e << '\n';
  reg g(6);
  out << '\t' << "a" << '\t' << g << ',' << d << ',' << f << '\n';
  out << '\t' << "ori" << '\t' << a << ',' << g << ',' << 0 << '\n';
  x->store(a);
}

namespace gencode_impl {
  void mul_notlonglong(const COMPILER::tac*);
  void runtime_binary(const COMPILER::tac*, std::string);
}

void gencode_impl::mul(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  const type* T = tac->x->m_type;
  int size = T->size();
  if ( T->real() )
    size < 8 ? binop_notlonglong(tac,"fm") : binop_notlonglong(tac,"dfm");
  else
    size < 8 ? mul_notlonglong(tac) : runtime_binary(tac,"__muldi3");
}

void gencode_impl::mul_notlonglong(const COMPILER::tac* tac)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  address* z = getaddr(tac->z);

  reg a(3); y->load(a);
  reg b(2); z->load(b);
  reg c(5), d(4);

  out << '\t' << "mpyh" << '\t' << c << ',' << a << ',' << b << '\n';
  out << '\t' << "mpyh" << '\t' << d << ',' << b << ',' << a << '\n';
  out << '\t' << "mpyu" << '\t' << a << ',' << a << ',' << b << '\n';
  out << '\t' << "a"    << '\t' << b << ',' << c << ',' << d << '\n';
  out << '\t' << "a"    << '\t' << d << ',' << b << ',' << a << '\n';

  x->store(d);
}

namespace gencode_impl {
  void div_mod_notlonglong(const COMPILER::tac*, bool);
  void div_mod_longlong(const COMPILER::tac*, bool);
  void div_single(const COMPILER::tac*);
  void div_double(const COMPILER::tac*);
}

void gencode_impl::div(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  const type* T = tac->x->m_type;
  int size = T->size();
  if ( T->real() )
    size < 8 ? div_single(tac) : div_double(tac);
  else
    size < 8 ? div_mod_notlonglong(tac,true) : div_mod_longlong(tac,true);
}

void gencode_impl::div_single(const COMPILER::tac* tac)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  address* z = getaddr(tac->z);

  reg a(3); y->load(a);
  reg b(4); z->load(b);
  reg c(6), d(2), e(5);

  out << '\t' << "frest" << '\t' << c << ',' << b << '\n';
  out << '\t' << "fi"    << '\t' << d << ',' << b << ',' << c << '\n';
  out << '\t' << "fm"    << '\t' << e << ',' << a << ',' << d << '\n';
  out << '\t' << "fnms"  << '\t' << a << ',' << e << ',' << b << ',' << a << '\n';
  out << '\t' << "fma"   << '\t' << a << ',' << a << ',' << d << ',' << e << '\n';
  x->store(a);
}

void gencode_impl::div_double(const COMPILER::tac* tac)
{
  runtime_binary(tac,"__divdf3");
}

void gencode_impl::mod(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  const type* T = tac->x->m_type;
  int size = T->size();
  size < 8 ? div_mod_notlonglong(tac,false) : div_mod_longlong(tac,false);
}

namespace gencode_impl {
  void sw_div_mod_notlonglong(const reg&, const reg&, const reg&, const reg&,
                              bool);
}

void gencode_impl::div_mod_notlonglong(const COMPILER::tac* tac, bool div)
{
  using namespace COMPILER;
  const type* T = tac->x->m_type;
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  address* z = getaddr(tac->z);

  reg did(3); y->load(did);
  reg dis(2); z->load(dis);
  reg quo(4), rem(5);

  bool b = !T->_signed();
  sw_div_mod_notlonglong(did,dis,quo,rem,b);

  x->store(div ? quo : rem);
}

void gencode_impl::sw_div_mod_notlonglong(const reg& did,
                                          const reg& dis,
                                          const reg& quo,
                                          const reg& rem,
                                          bool is_unsigned)
{
  reg a(10), b(11), c(12), d(13);
  reg e(6), f(9), g(7), h(14), i(8);

  out << '\t' << "heqi"     << '\t' << dis << ',' << 0 << '\n';
  out << '\t' << "hbrr"     << '\t' << "3f,1f" << '\n';
  if (is_unsigned){
  out << '\t' << "clz" << '\t' << f << ',' << dis << '\n';
  out << '\t' << "clz" << '\t' << e << ',' << did << '\n';
  } else {
  out << '\t' << "sfi"      << '\t' << a << ',' << did << ',' << 0 << '\n';
  out << '\t' << "sfi"      << '\t' << b << ',' << dis << ',' << 0 << '\n';
  out << '\t' << "cgti"     << '\t' << c << ',' << did << ',' << -1 << '\n';
  out << '\t' << "cgti"     << '\t' << d << ',' << dis << ',' << -1 << '\n';
  out << '\t' << "selb"     << '\t' << a << ',' << a << ',' << did << ',' << c << '\n';
  out << '\t' << "selb"     << '\t' << b << ',' << b << ',' << dis << ',' << d << '\n';
  out << '\t' << "clz"      << '\t' << e << ',' << a << '\n';
  out << '\t' << "clz"      << '\t' << f << ',' << b << '\n';
  }
  out << '\t' << "il"       << '\t' << g << ',' << 1 << '\n';
  out << '\t' << "fsmbi"    << '\t' << quo << ',' << 0 << '\n';
  out << '\t' << "sf"       << '\t' << f << ',' << e << ',' << f << '\n';
  if (is_unsigned){
  out << '\t' << "ori" << '\t' << rem << ',' << did << ',' << 0 << '\n';
  } else {
  out << '\t' << "shlqbyi"  << '\t' << rem << ',' << a << ',' << 0 << '\n';
  out << '\t' << "xor"             << '\t' << d << ',' << c << ',' << d << '\n';
  }
  out << '\t' << "shl"      << '\t' << g << ',' << g << ',' << f << '\n';
  if (is_unsigned){
  out << '\t' << "shl" << '\t' << e << ',' << dis << ',' << f << '\n';
  out << "1:";
  out << '\t' << "or"       << '\t' << a << ',' << quo << ',' << g << '\n';
  } else {
  out << '\t' << "shl"      << '\t' << e << ',' << b << ',' << f << '\n';
  out << '\t' << "lnop"     << '\n';
  out << "1:";
  out << '\t' << "or"       << '\t' << h << ',' << quo << ',' << g << '\n';
  }
  out << '\t' << "rotqmbii" << '\t' << g << ',' << g << ',' << -1 << '\n';
  out << '\t' << "clgt"     << '\t' << i << ',' << e << ',' << rem << '\n';
  out << '\t' << "lnop"     << '\n';
  out << '\t' << "sf"       << '\t' << f << ',' << e << ',' << rem << '\n';
  out << '\t' << "rotqmbii" << '\t' << e << ',' << e << ',' << -1 << '\n';
  if (is_unsigned){
  out << '\t' << "selb"     << '\t' << quo << ',' << a << ',' << quo << ',' << i << '\n';
  } else {
  out << '\t' << "selb"     << '\t' << quo << ',' << h << ',' << quo << ',' << i << '\n';
  }
  out << '\t' << "lnop"     << '\n';
  out << '\t' << "selb"     << '\t' << rem << ',' << f << ',' << rem << ',' << i << '\n';
  out << "3:";
  out << '\t' << "brnz"     << '\t' << g << ',' << "1b" << '\n';
  out << "2:";
  if (is_unsigned){
  out << '\n';
  } else {
  out << '\t' << "sfi"      << '\t' << a << ',' << rem << ',' << 0 << '\n';
  out << '\t' << "sfi"      << '\t' << b << ',' << quo << ',' << 0 << '\n';
  out << '\t' << "selb"     << '\t' << rem << ',' << a << ',' << rem << ',' << c << '\n';
  out << '\t' << "selb"     << '\t' << quo << ',' << quo << ',' << b << ',' << d << '\n';
  }
}

void gencode_impl::div_mod_longlong(const COMPILER::tac* tac, bool div)
{
  using namespace std;
  using namespace COMPILER;
  const type* T = tac->x->m_type;
  string fun;
  if ( div )
    fun = T->_signed() ? "__divdi3" : "__udivdi3";
  else
    fun = T->_signed() ? "__moddi3" : "__umoddi3";
  runtime_binary(tac,fun);
}

namespace gencode_impl {
  void lsh_longlong(const COMPILER::tac*);
}

void gencode_impl::lsh(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  const type* T = tac->x->m_type;
  int size = T->size();
  size < 8 ? binop_notlonglong(tac,"shl") : lsh_longlong(tac);
}

void gencode_impl::lsh_longlong(const COMPILER::tac* tac)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  address* z = getaddr(tac->z);

  reg a(3); y->load(a);
  reg b(4); z->load(b);

  reg c(6);
  out << '\t' << "rotqbyi" << '\t' << c << ',' << a << ',' << 8 << '\n';
  out << '\t' << "shlqbyi" << '\t' << b << ',' << b << ',' << 4 << '\n';
  reg d(5);
  out << '\t' << "shlqbyi" << '\t' << d << ',' << c << ',' << 8 << '\n';
  out << '\t' << "shlqbybi" << '\t' << a << ',' << d << ',' << b << '\n';
  reg e(2);
  out << '\t' << "shlqbi" << '\t' << e << ',' << a << ',' << b << '\n';
  out << '\t' << "ori" << '\t' << a << ',' << e << ',' << 0 << '\n';
  x->store(a);
}

namespace gencode_impl {
  void rsh_notlonglong(const COMPILER::tac*);
  void rsh_longlong(const COMPILER::tac*);
  void rsh_ulonglong(const COMPILER::tac*);
}

void gencode_impl::rsh(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  const type* T = tac->x->m_type;
  int size = T->size();
  if ( size < 8 )
    rsh_notlonglong(tac);
  else
    !T->_signed() ? rsh_ulonglong(tac) : rsh_longlong(tac);
}

void gencode_impl::rsh_notlonglong(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  const type* T = tac->x->m_type;
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  address* z = getaddr(tac->z);

  reg a(3); y->load(a);
  reg b(4); z->load(b);
  reg c(2);
  std::string op = !T->_signed() ? "rotm" : "rotma";
  out << '\t' << "sfi" << '\t' << c << ',' << b << ',' << 0 << '\n';
  out << '\t' << op << '\t' << c << ',' << a << ',' << c << '\n';
  x->store(c);
}

void gencode_impl::rsh_longlong(const COMPILER::tac* tac)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  address* z = getaddr(tac->z);

  reg a(3); y->load(a);
  reg b(4); z->load(b);
  reg c(9);
  out << '\t' << "rotmai" << '\t' << c << ',' << a << ',' << -31 << '\n';
  out << '\t' << "shlqbyi" << '\t' << b << ',' << b << ',' << 4 << '\n';
  reg d(6);
  out << '\t' << "fsmbi" << '\t' << d << ',' << 65280 << '\n';
  reg e(8);
  out << '\t' << "fsm" << '\t' << e << ',' << c << '\n';
  reg f(5);
  out << '\t' << "sfi" << '\t' << f << ',' << b << ',' << 0 << '\n';
  reg g(7);
  out << '\t' << "selb" << '\t' << g << ',' << e << ',' << a << ',' << d << '\n';
  out << '\t' << "rotqbybi" << '\t' << a << ',' << g << ',' << f << '\n';
  reg h(2);
  out << '\t' << "rotqbi" << '\t' << h << ',' << a << ',' << f << '\n';
  out << '\t' << "ori" << '\t' << a << ',' << h << ',' << 0 << '\n';
  x->store(a);
}

void gencode_impl::rsh_ulonglong(const COMPILER::tac* tac)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  address* z = getaddr(tac->z);

  reg a(3); y->load(a);
  reg b(4); z->load(b);

  out << '\t' << "shlqbyi" << '\t' << b << ',' << b << ',' << 4 << '\n';
  reg c(2);
  out << '\t' << "sfi" << '\t' << c << ',' << b << ',' << 0 << '\n';
  reg d(5);
  out << '\t' << "sfi" << '\t' << d << ',' << b << ',' << 7 << '\n';
  out << '\t' << "rotqmbi" << '\t' << a << ',' << a << ',' << c << '\n';
  out << '\t' << "rotqmbybi" << '\t' << a << ',' << a << ',' << d << '\n';
  x->store(a);
}

void gencode_impl::_and(const COMPILER::tac* tac)
{
  binop_notlonglong(tac,"and");
}

void gencode_impl::_xor(const COMPILER::tac* tac)
{
  binop_notlonglong(tac,"xor");
}

void gencode_impl::_or(const COMPILER::tac* tac)
{
  binop_notlonglong(tac,"or");
}

namespace gencode_impl {
  void uminus_notlonglong(const COMPILER::tac*);
  void uminus_longlong(const COMPILER::tac*);
  void uminus_single(const COMPILER::tac*);
  void uminus_double(const COMPILER::tac*);
}

void gencode_impl::uminus(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  const type* T = tac->x->m_type;
  int size = T->size();
  if ( T->real() )
    size < 8 ? uminus_single(tac) : uminus_double(tac);
  else
    size < 8 ? uminus_notlonglong(tac) : uminus_longlong(tac);
}

void gencode_impl::uminus_notlonglong(const COMPILER::tac* tac)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);

  reg r(3); y->load(r);
  out << '\t' << "sfi" << '\t' << r << ',' << r << ',' << 0 << '\n';
  x->store(r);
}

void gencode_impl::uminus_longlong(const COMPILER::tac* tac)
{
  using namespace std;
  using namespace COMPILER;

  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);

  reg a(3); y->load(a);
  reg b(4);
  out << '\t' << "il" << '\t' << b << ',' << 0 << '\n';
  if ( !_04050607808080800c0d0e0f80808080.get() ){
    const type* T = tac->x->m_type;
    _04050607808080800c0d0e0f80808080 =
      auto_ptr<mem>(new mem(new_label(),T));
  }
  reg c(2);
  _04050607808080800c0d0e0f80808080->load(c);
  reg d(8);
  out << '\t' << "clgt" << '\t' << d << ',' << a << ',' << b << '\n';
  reg e(7);
  out << '\t' << "sf" << '\t' << e << ',' << a << ',' << b << '\n';
  reg f(5);
  out << '\t' << "shufb" << '\t' << f << ',' << d << ',' << d << ',' << c << '\n';
  reg g(6);
  out << '\t' << "a" << '\t' << g << ',' << e << ',' << f << '\n';
  out << '\t' << "ori" << '\t' << a << ',' << g << ',' << 0 << '\n';
  x->store(a);
}

void gencode_impl::uminus_single(const COMPILER::tac* tac)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);

  reg a(3); y->load(a);
  reg b(4);
  out << '\t' << "ilhu" << '\t' << b << ',' << -32768 << '\n';
  out << '\t' << "xor"  << '\t' << a << ',' << a << ',' << b << '\n';
  x->store(a);
}

void gencode_impl::uminus_double(const COMPILER::tac* tac)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);

  reg a(3); y->load(a);
  reg b(4);
  out << '\t' << "fsmbi" << '\t' << b << ',' << 32896 << '\n';
  out << '\t' << "andbi" << '\t' << b << ',' << b << ',' << 128 << '\n';
  out << '\t' << "xor"   << '\t' << a << ',' << a << ',' << b << '\n';
  x->store(a);
}

void gencode_impl::tilde(const COMPILER::tac* tac)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);

  reg r(3);
  y->load(r);
  out << '\t' << "nor" << '\t' << r << ',' << r << ',' << r << '\n';
  x->store(r);
}

namespace gencode_impl {
  int tc_id(const COMPILER::type*);
  struct tc_table : std::map<std::pair<int,int>, void (*)(const COMPILER::tac*)> {
    tc_table();
  } m_tc_table;
}

void gencode_impl::tc(const COMPILER::tac* tac)
{
  using namespace std;
  int x = tc_id(tac->x->m_type);
  int y = tc_id(tac->y->m_type);
  m_tc_table[make_pair(x,y)](tac);
}

int gencode_impl::tc_id(const COMPILER::type* T)
{
  int n = (T->integer() && !T->_signed()) ? 1 : 0;
  return (T->size() << 2) + (n << 1) + T->real();
}

namespace gencode_impl {
  void sint08_sint08(const COMPILER::tac*);
  void sint08_uint08(const COMPILER::tac*);
  void sint08_sint16(const COMPILER::tac*);
  void sint08_uint16(const COMPILER::tac*);
  void sint08_sint32(const COMPILER::tac*);
  void sint08_uint32(const COMPILER::tac*);
  void sint08_sint64(const COMPILER::tac*);
  void sint08_uint64(const COMPILER::tac*);
  void sint08_single(const COMPILER::tac*);
  void sint08_double(const COMPILER::tac*);

  void uint08_sint08(const COMPILER::tac*);
  void uint08_sint16(const COMPILER::tac*);
  void uint08_uint16(const COMPILER::tac*);
  void uint08_sint32(const COMPILER::tac*);
  void uint08_uint32(const COMPILER::tac*);
  void uint08_sint64(const COMPILER::tac*);
  void uint08_uint64(const COMPILER::tac*);
  void uint08_single(const COMPILER::tac*);
  void uint08_double(const COMPILER::tac*);

  void sint16_sint08(const COMPILER::tac*);
  void sint16_uint08(const COMPILER::tac*);
  void sint16_uint16(const COMPILER::tac*);
  void sint16_sint32(const COMPILER::tac*);
  void sint16_uint32(const COMPILER::tac*);
  void sint16_sint64(const COMPILER::tac*);
  void sint16_uint64(const COMPILER::tac*);
  void sint16_single(const COMPILER::tac*);
  void sint16_double(const COMPILER::tac*);

  void uint16_sint08(const COMPILER::tac*);
  void uint16_uint08(const COMPILER::tac*);
  void uint16_sint16(const COMPILER::tac*);
  void uint16_sint32(const COMPILER::tac*);
  void uint16_uint32(const COMPILER::tac*);
  void uint16_sint64(const COMPILER::tac*);
  void uint16_uint64(const COMPILER::tac*);
  void uint16_single(const COMPILER::tac*);
  void uint16_double(const COMPILER::tac*);

  void sint32_sint08(const COMPILER::tac*);
  void sint32_uint08(const COMPILER::tac*);
  void sint32_sint16(const COMPILER::tac*);
  void sint32_uint16(const COMPILER::tac*);
  void sint32_sint32(const COMPILER::tac*);
  void sint32_uint32(const COMPILER::tac*);
  void sint32_sint64(const COMPILER::tac*);
  void sint32_uint64(const COMPILER::tac*);
  void sint32_single(const COMPILER::tac*);
  void sint32_double(const COMPILER::tac*);

  void uint32_sint08(const COMPILER::tac*);
  void uint32_uint08(const COMPILER::tac*);
  void uint32_sint16(const COMPILER::tac*);
  void uint32_uint16(const COMPILER::tac*);
  void uint32_sint32(const COMPILER::tac*);
  void uint32_uint32(const COMPILER::tac*);
  void uint32_sint64(const COMPILER::tac*);
  void uint32_uint64(const COMPILER::tac*);
  void uint32_single(const COMPILER::tac*);
  void uint32_double(const COMPILER::tac*);

  void sint64_sint08(const COMPILER::tac*);
  void sint64_uint08(const COMPILER::tac*);
  void sint64_sint16(const COMPILER::tac*);
  void sint64_uint16(const COMPILER::tac*);
  void sint64_sint32(const COMPILER::tac*);
  void sint64_uint32(const COMPILER::tac*);
  void sint64_uint64(const COMPILER::tac*);
  void sint64_single(const COMPILER::tac*);
  void sint64_double(const COMPILER::tac*);

  void uint64_sint08(const COMPILER::tac*);
  void uint64_uint08(const COMPILER::tac*);
  void uint64_sint16(const COMPILER::tac*);
  void uint64_uint16(const COMPILER::tac*);
  void uint64_sint32(const COMPILER::tac*);
  void uint64_uint32(const COMPILER::tac*);
  void uint64_sint64(const COMPILER::tac*);
  void uint64_single(const COMPILER::tac*);
  void uint64_double(const COMPILER::tac*);
    
  void single_sint08(const COMPILER::tac*);
  void single_uint08(const COMPILER::tac*);
  void single_sint16(const COMPILER::tac*);
  void single_uint16(const COMPILER::tac*);
  void single_sint32(const COMPILER::tac*);
  void single_uint32(const COMPILER::tac*);
  void single_sint64(const COMPILER::tac*);
  void single_uint64(const COMPILER::tac*);
  void single_double(const COMPILER::tac*);

  void double_sint08(const COMPILER::tac*);
  void double_uint08(const COMPILER::tac*);
  void double_sint16(const COMPILER::tac*);
  void double_uint16(const COMPILER::tac*);
  void double_sint32(const COMPILER::tac*);
  void double_uint32(const COMPILER::tac*);
  void double_sint64(const COMPILER::tac*);
  void double_uint64(const COMPILER::tac*);
  void double_single(const COMPILER::tac*);
  void double_double(const COMPILER::tac*);
}

gencode_impl::tc_table::tc_table()
{
  using namespace std;

  (*this)[make_pair(4,4)] = sint08_sint08;
  (*this)[make_pair(4,6)] = sint08_uint08;
  (*this)[make_pair(4,8)] = sint08_sint16;
  (*this)[make_pair(4,10)] = sint08_uint16;
  (*this)[make_pair(4,16)] = sint08_sint32;
  (*this)[make_pair(4,18)] = sint08_uint32;
  (*this)[make_pair(4,32)] = sint08_sint64;
  (*this)[make_pair(4,34)] = sint08_uint64;
  (*this)[make_pair(4,17)] = sint08_single;
  (*this)[make_pair(4,33)] = sint08_double;

  (*this)[make_pair(6,4)] = uint08_sint08;
  (*this)[make_pair(6,8)] = uint08_sint16;
  (*this)[make_pair(6,10)] = uint08_uint16;
  (*this)[make_pair(6,16)] = uint08_sint32;
  (*this)[make_pair(6,18)] = uint08_uint32;
  (*this)[make_pair(6,32)] = uint08_sint64;
  (*this)[make_pair(6,34)] = uint08_uint64;
  (*this)[make_pair(6,17)] = uint08_single;
  (*this)[make_pair(6,33)] = uint08_double;

  (*this)[make_pair(8,4)] = sint16_sint08;
  (*this)[make_pair(8,6)] = sint16_uint08;
  (*this)[make_pair(8,10)] = sint16_uint16;
  (*this)[make_pair(8,16)] = sint16_sint32;
  (*this)[make_pair(8,18)] = sint16_uint32;
  (*this)[make_pair(8,32)] = sint16_sint64;
  (*this)[make_pair(8,34)] = sint16_uint64;
  (*this)[make_pair(8,17)] = sint16_single;
  (*this)[make_pair(8,33)] = sint16_double;

  (*this)[make_pair(10,4)] = uint16_sint08;
  (*this)[make_pair(10,6)] = uint16_uint08;
  (*this)[make_pair(10,8)] = uint16_sint16;
  (*this)[make_pair(10,16)] = uint16_sint32;
  (*this)[make_pair(10,18)] = uint16_uint32;
  (*this)[make_pair(10,32)] = uint16_sint64;
  (*this)[make_pair(10,34)] = uint16_uint64;
  (*this)[make_pair(10,17)] = uint16_single;
  (*this)[make_pair(10,33)] = uint16_double;

  (*this)[make_pair(16,4)] = sint32_sint08;
  (*this)[make_pair(16,6)] = sint32_uint08;
  (*this)[make_pair(16,8)] = sint32_sint16;
  (*this)[make_pair(16,10)] = sint32_uint16;
  (*this)[make_pair(16,16)] = sint32_sint32;
  (*this)[make_pair(16,18)] = sint32_uint32;
  (*this)[make_pair(16,32)] = sint32_sint64;
  (*this)[make_pair(16,34)] = sint32_uint64;
  (*this)[make_pair(16,17)] = sint32_single;
  (*this)[make_pair(16,33)] = sint32_double;

  (*this)[make_pair(18,4)] = uint32_sint08;
  (*this)[make_pair(18,6)] = uint32_uint08;
  (*this)[make_pair(18,8)] = uint32_sint16;
  (*this)[make_pair(18,10)] = uint32_uint16;
  (*this)[make_pair(18,16)] = uint32_sint32;
  (*this)[make_pair(18,18)] = uint32_uint32;
  (*this)[make_pair(18,32)] = uint32_sint64;
  (*this)[make_pair(18,34)] = uint32_uint64;
  (*this)[make_pair(18,17)] = uint32_single;
  (*this)[make_pair(18,33)] = uint32_double;

  (*this)[make_pair(32,4)] = sint64_sint08;
  (*this)[make_pair(32,6)] = sint64_uint08;
  (*this)[make_pair(32,8)] = sint64_sint16;
  (*this)[make_pair(32,10)] = sint64_uint16;
  (*this)[make_pair(32,16)] = sint64_sint32;
  (*this)[make_pair(32,18)] = sint64_uint32;
  (*this)[make_pair(32,34)] = sint64_uint64;
  (*this)[make_pair(32,17)] = sint64_single;
  (*this)[make_pair(32,33)] = sint64_double;

  (*this)[make_pair(34,4)] = uint64_sint08;
  (*this)[make_pair(34,6)] = uint64_uint08;
  (*this)[make_pair(34,8)] = uint64_sint16;
  (*this)[make_pair(34,10)] = uint64_uint16;
  (*this)[make_pair(34,16)] = uint64_sint32;
  (*this)[make_pair(34,18)] = uint64_uint32;
  (*this)[make_pair(34,32)] = uint64_sint64;
  (*this)[make_pair(34,17)] = uint64_single;
  (*this)[make_pair(34,33)] = uint64_double;
    
  (*this)[make_pair(17,4)] = single_sint08;
  (*this)[make_pair(17,6)] = single_uint08;
  (*this)[make_pair(17,8)] = single_sint16;
  (*this)[make_pair(17,10)] = single_uint16;
  (*this)[make_pair(17,16)] = single_sint32;
  (*this)[make_pair(17,18)] = single_uint32;
  (*this)[make_pair(17,32)] = single_sint64;
  (*this)[make_pair(17,34)] = single_uint64;
  (*this)[make_pair(17,33)] = single_double;

  (*this)[make_pair(33,4)] = double_sint08;
  (*this)[make_pair(33,6)] = double_uint08;
  (*this)[make_pair(33,8)] = double_sint16;
  (*this)[make_pair(33,10)] = double_uint16;
  (*this)[make_pair(33,16)] = double_sint32;
  (*this)[make_pair(33,18)] = double_uint32;
  (*this)[make_pair(33,32)] = double_sint64;
  (*this)[make_pair(33,34)] = double_uint64;
  (*this)[make_pair(33,17)] = double_single;
  (*this)[make_pair(33,33)] = double_double;
}

void gencode_impl::sint08_sint08(const COMPILER::tac* tac)
{
  assign(tac);
}

void gencode_impl::sint08_uint08(const COMPILER::tac* tac)
{
  assign(tac);
}

namespace gencode_impl {
  void extend(const COMPILER::tac*, bool, int);
}

void gencode_impl::sint08_sint16(const COMPILER::tac* tac)
{
  extend(tac,true,8);
}

void gencode_impl::sint08_uint16(const COMPILER::tac* tac)
{
  extend(tac,true,8);
}

void gencode_impl::sint08_sint32(const COMPILER::tac* tac)
{
  extend(tac,true,8);
}

void gencode_impl::sint08_uint32(const COMPILER::tac* tac)
{
  extend(tac,false,8);
}

void gencode_impl::sint08_sint64(const COMPILER::tac* tac)
{
  extend(tac,true,8);
}

void gencode_impl::sint08_uint64(const COMPILER::tac* tac)
{
  extend(tac,true,8);
}

namespace gencode_impl {
  void common_real(const COMPILER::tac*, bool, int);
}

void gencode_impl::sint08_single(const COMPILER::tac* tac)
{
  common_real(tac,true,8);
}

void gencode_impl::sint08_double(const COMPILER::tac* tac)
{
  common_real(tac,true,8);
}

void gencode_impl::uint08_sint08(const COMPILER::tac* tac)
{
  assign(tac);
}

void gencode_impl::uint08_sint16(const COMPILER::tac* tac)
{
  extend(tac,false,8);
}

void gencode_impl::uint08_uint16(const COMPILER::tac* tac)
{
  extend(tac,false,8);
}

void gencode_impl::uint08_sint32(const COMPILER::tac* tac)
{
  extend(tac,false,8);
}

void gencode_impl::uint08_uint32(const COMPILER::tac* tac)
{
  extend(tac,false,8);
}

void gencode_impl::uint08_sint64(const COMPILER::tac* tac)
{
  extend(tac,false,8);
}

void gencode_impl::uint08_uint64(const COMPILER::tac* tac)
{
  extend(tac,false,8);
}

void gencode_impl::uint08_single(const COMPILER::tac* tac)
{
  common_real(tac,false,8);
}

namespace gencode_impl {
  void runtime_unary(const COMPILER::tac*, std::string);
}

void gencode_impl::uint08_double(const COMPILER::tac* tac)
{
  common_real(tac,false,8);
}

void gencode_impl::sint16_sint08(const COMPILER::tac* tac)
{
  extend(tac,true,8);
}

void gencode_impl::sint16_uint08(const COMPILER::tac* tac)
{
  extend(tac,false,8);
}

void gencode_impl::sint16_uint16(const COMPILER::tac* tac)
{
  assign(tac);
}

void gencode_impl::sint16_sint32(const COMPILER::tac* tac)
{
  extend(tac,true,16);
}

void gencode_impl::sint16_uint32(const COMPILER::tac* tac)
{
  extend(tac,true,16);
}

void gencode_impl::sint16_sint64(const COMPILER::tac* tac)
{
  extend(tac,true,16);
}

void gencode_impl::sint16_uint64(const COMPILER::tac* tac)
{
  extend(tac,true,16);
}

void gencode_impl::sint16_single(const COMPILER::tac* tac)
{
  common_real(tac,true,16);
}

void gencode_impl::sint16_double(const COMPILER::tac* tac)
{
  common_real(tac,true,16);
}

void gencode_impl::uint16_sint08(const COMPILER::tac* tac)
{
  extend(tac,true,8);
}

void gencode_impl::uint16_uint08(const COMPILER::tac* tac)
{
  extend(tac,false,8);
}

void gencode_impl::uint16_sint16(const COMPILER::tac* tac)
{
  assign(tac);
}

void gencode_impl::uint16_sint32(const COMPILER::tac* tac)
{
  extend(tac,false,16);
}

void gencode_impl::uint16_uint32(const COMPILER::tac* tac)
{
  extend(tac,false,16);
}

void gencode_impl::uint16_sint64(const COMPILER::tac* tac)
{
  extend(tac,false,16);
}

void gencode_impl::uint16_uint64(const COMPILER::tac* tac)
{
  extend(tac,false,16);
}

void gencode_impl::uint16_single(const COMPILER::tac* tac)
{
  common_real(tac,false,16);
}

void gencode_impl::uint16_double(const COMPILER::tac* tac)
{
  common_real(tac,false,16);
}

void gencode_impl::sint32_sint08(const COMPILER::tac* tac)
{
  extend(tac,true,8);
}

void gencode_impl::sint32_uint08(const COMPILER::tac* tac)
{
  extend(tac,false,8);
}

void gencode_impl::sint32_sint16(const COMPILER::tac* tac)
{
  extend(tac,true,16);
}

void gencode_impl::sint32_uint16(const COMPILER::tac* tac)
{
  extend(tac,false,16);
}

void gencode_impl::sint32_sint32(const COMPILER::tac* tac)
{
  assign(tac);
}

void gencode_impl::sint32_uint32(const COMPILER::tac* tac)
{
  assign(tac);
}

namespace gencode_impl {
  void common_longlong(const COMPILER::tac*);
}

void gencode_impl::sint32_sint64(const COMPILER::tac* tac)
{
  common_longlong(tac);
}

void gencode_impl::sint32_uint64(const COMPILER::tac* tac)
{
  common_longlong(tac);
}

void gencode_impl::sint32_single(const COMPILER::tac* tac)
{
  common_real(tac,true,32);
}

void gencode_impl::sint32_double(const COMPILER::tac* tac)
{
  runtime_unary(tac,"__fixdfsi");
}

void gencode_impl::uint32_sint08(const COMPILER::tac* tac)
{
  extend(tac,true,8);
}

void gencode_impl::uint32_uint08(const COMPILER::tac* tac)
{
  extend(tac,false,8);
}

void gencode_impl::uint32_sint16(const COMPILER::tac* tac)
{
  extend(tac,true,16);
}

void gencode_impl::uint32_uint16(const COMPILER::tac* tac)
{
  extend(tac,false,16);
}

void gencode_impl::uint32_sint32(const COMPILER::tac* tac)
{
  assign(tac);
}

void gencode_impl::uint32_uint32(const COMPILER::tac* tac)
{
  assign(tac);
}

void gencode_impl::uint32_sint64(const COMPILER::tac* tac)
{
  common_longlong(tac);
}

void gencode_impl::uint32_uint64(const COMPILER::tac* tac)
{
  common_longlong(tac);
}

void gencode_impl::uint32_single(const COMPILER::tac* tac)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  reg r(3);
  y->load(r);
  out << '\t' << "cfltu" << '\t' << r << ',' << r << ',' << 0 << '\n';
  x->store(r);
}

void gencode_impl::uint32_double(const COMPILER::tac* tac)
{
  runtime_unary(tac,"__fixunsdfsi");
}

void gencode_impl::sint64_sint08(const COMPILER::tac* tac)
{
  using namespace std;
  using namespace COMPILER;

  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  reg r(3);
  y->load(r);
  reg s(4);
  out << '\t' << "xsbh" << '\t' << s << ',' << r << '\n';
  if ( !_12121212121212131212121212121212.get() ){
    const type* T = tac->x->m_type;
    _12121212121212131212121212121212
      = auto_ptr<mem>(new mem(new_label(),T));
  }
  reg t(2);
  _12121212121212131212121212121212->load(t);
  out << '\t' << "shufb" << '\t' << r << ',' << r << ',' << s << ',' << t << '\n';
  x->store(r);
}

void gencode_impl::sint64_uint08(const COMPILER::tac* tac)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  reg r(3);
  y->load(r);
  out << '\t' << "andi" << '\t' << r << ',' << r << ',' << 0x00ff << '\n';
  out << '\t' << "rotqmbyi" << '\t' << r << ',' << r << ',' << -4 << '\n';
  x->store(r);
}

void gencode_impl::sint64_sint16(const COMPILER::tac* tac)
{
  using namespace std;
  using namespace COMPILER;

  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  reg r(3);
  y->load(r);
  reg s(4);
  out << '\t' << "xshw" << '\t' << s << ',' << r << '\n';
  if ( !_10101010101002031010101010101010.get() ){
    const type* T = tac->x->m_type;
    _10101010101002031010101010101010
      = auto_ptr<mem>(new mem(new_label(),T));
  }
  reg t(2);
  _10101010101002031010101010101010->load(t);
  out << '\t' << "shufb" << '\t' << r << ',' << r << ',' << s << ',' << t << '\n';
  x->store(r);
}

void gencode_impl::sint64_uint16(const COMPILER::tac* tac)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  reg r(3);
  y->load(r);
  reg s(2);
  out << '\t' << "ila" << '\t' << s << ',' << 65535 << '\n';
  out << '\t' << "and" << '\t' << r << ',' << r << ',' << s << '\n';
  out << '\t' << "rotqmbyi" << '\t' << r << ',' << r << ',' << -4 << '\n';
  x->store(r);
}

void gencode_impl::sint64_sint32(const COMPILER::tac* tac)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);

  reg r(3); y->load(r);
  out << '\t' << "rotqbyi" << '\t' << r << ',' << r << ',' << -4 << '\n';
  out << '\t' << "xswd" << '\t' << r << ',' << r << '\n';
  x->store(r);
}

void gencode_impl::sint64_uint32(const COMPILER::tac* tac)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);

  reg r(3); y->load(r);
  int rot = 4;
  out << '\t' << "rotqmbyi" << '\t' << r << ',' << r << ',' << -rot << '\n';
  x->store(r);
}

void gencode_impl::sint64_uint64(const COMPILER::tac* tac)
{
  assign(tac);
}

void gencode_impl::sint64_single(const COMPILER::tac* tac)
{
  runtime_unary(tac,"__fixsfdi");
}

void gencode_impl::sint64_double(const COMPILER::tac* tac)
{
  runtime_unary(tac,"__fixdfdi");
}

void gencode_impl::uint64_sint08(const COMPILER::tac* tac)
{
  sint64_sint08(tac);
}

void gencode_impl::uint64_uint08(const COMPILER::tac* tac)
{
  sint64_uint08(tac);
}

void gencode_impl::uint64_sint16(const COMPILER::tac* tac)
{
  sint64_uint16(tac);
}

void gencode_impl::uint64_uint16(const COMPILER::tac* tac)
{
  sint64_uint16(tac);
}

void gencode_impl::uint64_sint32(const COMPILER::tac* tac)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  reg r(3);
  y->load(r);
  out << '\t' << "rotqbyi" << '\t' << r << ',' << r << ',' << -4 << '\n';
  out << '\t' << "xswd" << '\t' << r << ',' << r << '\n';
  x->store(r);
}

void gencode_impl::uint64_uint32(const COMPILER::tac* tac)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  reg r(3);
  y->load(r);
  out << '\t' << "rotqmbyi" << '\t' << r << ',' << r << ',' << -4 << '\n';
  x->store(r);
}

void gencode_impl::uint64_sint64(const COMPILER::tac* tac)
{
  assign(tac);
}

void gencode_impl::uint64_single(const COMPILER::tac* tac)
{
  runtime_unary(tac,"__fixunssfdi");
}

void gencode_impl::uint64_double(const COMPILER::tac* tac)
{
  runtime_unary(tac,"__fixunsdfdi");
}

namespace gencode_impl {
  void single_common(const COMPILER::tac*, bool, int);
}

void gencode_impl::single_sint08(const COMPILER::tac* tac)
{
  single_common(tac,true,8);
}

void gencode_impl::single_uint08(const COMPILER::tac* tac)
{
  single_common(tac,false,8);
}

void gencode_impl::single_sint16(const COMPILER::tac* tac)
{
  single_common(tac,true,16);
}

void gencode_impl::single_uint16(const COMPILER::tac* tac)
{
  single_common(tac,false,16);
}

void gencode_impl::single_sint32(const COMPILER::tac* tac)
{
  single_common(tac,true,32);
}

void gencode_impl::single_uint32(const COMPILER::tac* tac)
{
  single_common(tac,false,32);
}

void gencode_impl::single_common(const COMPILER::tac* tac, bool signextend, int bit)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  reg r(3);
  y->load(r);
  reg s(2);

  switch ( bit ){
  case 8:
    if ( signextend ){
      out << '\t' << "xsbh" << '\t' << s << ',' << r << '\n';
      out << '\t' << "xshw" << '\t' << r << ',' << s << '\n';
    }
    else
      out << '\t' << "andi" << '\t' << r << ',' << r << ',' << 0x00ff << '\n';
    break;
  case 16:
    if ( signextend )
      out << '\t' << "xshw" << '\t' << r << ',' << r << '\n';
    else {
      out << '\t' << "ila" << '\t' << s << ',' << 65535 << '\n';
      out << '\t' << "and" << '\t' << r << ',' << r << ',' << s << '\n';
    }
    break;
  }
  std::string op = signextend ? "csflt" : "cuflt";
  out << '\t' << op << '\t' << r << ',' << r << ',' << 0 << '\n';
  x->store(r);
}

void gencode_impl::single_sint64(const COMPILER::tac* tac)
{
  runtime_unary(tac,"__floatdisf");
}

void gencode_impl::single_uint64(const COMPILER::tac* tac)
{
  address* y = getaddr(tac->y);
  reg r3(3);
  y->load(r3);
  reg b(5);
  out << '\t' << "ori" << '\t' << b << ',' << r3 << ',' << 0 << '\n';
  reg c(4);
  out << '\t' << "il" << '\t' << c << ',' << 0 << '\n';
  reg d(6);
  out << '\t' << "il" << '\t' << d << ',' << 1 << '\n';
  reg e(7);
  out << '\t' << "rotqmbyi" << '\t' << e << ',' << b << ',' << 0 << '\n';
  reg f(12);
  out << '\t' << "clgt" << '\t' << f << ',' << c << ',' << b << '\n';
  reg g(10);
  out << '\t' << "ceq" << '\t' << g << ',' << c << ',' << b << '\n';
  reg h(8);
  out << '\t' << "cgt" << '\t' << h << ',' << c << ',' << b << '\n';
  reg i(11);
  out << '\t' << "rotqbyi" << '\t' << i << ',' << f << ',' << 4 << '\n';
  reg j(13);
  out << '\t' << "xswd" << '\t' << j << ',' << d << '\n';
  out << '\t' << "rotqmbii" << '\t' << e << ',' << e << ',' << -1 << '\n';
  out << '\t' << "and" << '\t' << d << ',' << r3 << ',' << j << '\n';
  reg k(9);
  out << '\t' << "and" << '\t' << k << ',' << g << ',' << i << '\n';
  reg l(2);
  out << '\t' << "or" << '\t' << l << ',' << k << ',' << h << '\n';
  std::string label = new_label();
  out << '\t' << "brnz" << '\t' << l << ',' << label << '\n';
  out << '\t' << "ai" << '\t' << "$sp,$sp,-32" << '\n';
  out << '\t' << "brsl" << '\t' << "$lr," << "__floatdisf" << '\n';
  out << '\t' << "ai" << '\t' << "$sp,$sp,32" << '\n';
  std::string label2 = new_label();
  out << '\t' << "br" << '\t' << label2 << '\n';
  out << label << ":\n";
  out << '\t' << "or" << '\t' << r3 << ',' << d << ',' << e << '\n';
  out << '\t' << "ai" << '\t' << "$sp,$sp,-32" << '\n';
  out << '\t' << "brsl" << '\t' << "$lr," << "__floatdisf" << '\n';
  out << '\t' << "ai" << '\t' << "$sp,$sp,32" << '\n';
  out << '\t' << "fa" << '\t' << r3 << ',' << r3 << ',' << r3 << '\n';
  out << label2 << ":\n";
  address* x = getaddr(tac->x);
  x->store(r3);
}

void gencode_impl::single_double(const COMPILER::tac* tac)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);

  reg r(3);
  y->load(r);
  out << '\t' << "frds" << '\t' << r << ',' << r << '\n';
  x->store(r);
}

namespace gencode_impl {
  void double_sint(const COMPILER::tac*, int);
  void double_uint(const COMPILER::tac*, int);
}

void gencode_impl::double_sint08(const COMPILER::tac* tac)
{
  double_sint(tac,8);
}

void gencode_impl::double_uint08(const COMPILER::tac* tac)
{
  double_uint(tac,8);
}

void gencode_impl::double_sint16(const COMPILER::tac* tac)
{
  double_sint(tac,16);
}

void gencode_impl::double_uint16(const COMPILER::tac* tac)
{
  double_uint(tac,16);
  /*
.LC0:
        .byte        2
        .byte        3
        .byte        16
        .byte        17
        .byte        18
        .byte        19
        .byte        -128
        .byte        -128
        .byte        6
        .byte        7
        .byte        20
        .byte        21
        .byte        22
        .byte        23
        .byte        -128
        .byte        -128
.text
        .align        3
        .global        f
        .type        f, @function
f:
                                ila        $12,65535
        hbr        .L2,$lr
        il        $10,1054
        lnop
                                and        $11,$3,$12
        lqa        $3,.LC0
        clz        $5,$11
        shl        $9,$11,$5
        ceqi        $7,$5,32
        sf        $8,$5,$10
        a        $6,$9,$9
        andc        $2,$8,$7
        shufb        $4,$2,$6,$3
        shlqbii        $3,$4,4
        nop        $127
.L2:
        bi        $lr
  */
}

void gencode_impl::double_sint32(const COMPILER::tac* tac)
{
  double_sint(tac,32);
}

void gencode_impl::double_uint32(const COMPILER::tac* tac)
{
  double_uint(tac,32);
}

void gencode_impl::double_sint(const COMPILER::tac* tac, int bit)
{
  using namespace std;
  using namespace COMPILER;

  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);

  reg r(3);
  y->load(r);

  switch ( bit ){
  case 8:
    out << '\t' << "xsbh" << '\t' << r << ',' << r << '\n';
  case 16 :
    out << '\t' << "xshw" << '\t' << r << ',' << r << '\n';
    break;
  }

  reg a(14), b(2), c(11), d(13), e(4), f(12), g(10), h(6), i(9), j(5);
  reg k(8), l(7);
  out << '\t' << "ilhu" << '\t' << a << ',' <<         -32768 << '\n';
  if ( !_02031011121380800607141516178080.get() ){
    const type* T = tac->y->m_type->varg();
    _02031011121380800607141516178080 =
      auto_ptr<mem>(new mem(new_label(),T));
  }
  _02031011121380800607141516178080->load(b);
  out << '\t' << "il" << '\t' << c << ',' << 1054 << '\n';
  out << '\t' << "xor" << '\t' << d << ',' << r << ',' << a << '\n';
  if ( !double_0x80000000.get() ){
    const type* T = tac->y->m_type->varg();
    double_0x80000000 = auto_ptr<mem>(new mem(new_label(),T));
  }
  double_0x80000000->load(e);
  out << '\t' << "clz" << '\t' << f << ',' << d << '\n';
  out << '\t' << "shl" << '\t' << g << ',' << d << ',' << f << '\n';
  out << '\t' << "ceqi" << '\t' << h << ',' << f << ',' << 32 << '\n';
  out << '\t' << "sf"  << '\t' << i << ',' << f << ',' << c << '\n';
  out << '\t' << "a"   << '\t' << j << ',' << g << ',' << g << '\n';
  out << '\t' << "andc" << '\t' << k << ',' << i << ',' << h << '\n';
  out << '\t' << "shufb" << '\t' << l << ',' << k << ',' << j << ',' << b << '\n';
  out << '\t' << "shlqbii" << '\t' << r << ',' << l << ',' << 4 << '\n';
  out << '\t' << "dfs" << '\t' << r << ',' << r << ',' << e << '\n';
  x->store(r);
}

void gencode_impl::double_uint(const COMPILER::tac* tac, int bit)
{
  using namespace std;
  using namespace COMPILER;

  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);

  reg a(3);
  y->load(a);

  reg b(4);
  switch ( bit ){
  case 8:
    out << '\t' << "andi" << '\t' << a << ',' << a << ',' << 0x00ff << '\n';
  case 16:
    out << '\t' << "ila" << '\t' << b << ',' << 65535 << '\n';
    out << '\t' << "and" << '\t' << a << ',' << a << ',' << b << '\n';
    break;
  }
  out << '\t' << "clz" << '\t' << b << ',' << a << '\n';
  reg c(10);
  out << '\t' << "il" << '\t' << c << ',' << 1054 << '\n';
  reg d(2);
  if ( !_02031011121380800607141516178080.get() ){
    const type* T = tac->y->m_type->varg();
    _02031011121380800607141516178080 =
      auto_ptr<mem>(new mem(new_label(),T));
  }
  _02031011121380800607141516178080->load(d);
  reg e(9);
  out << '\t' << "shl" << '\t' << e << ',' << a << ',' << b << '\n';
  reg f(6);
  out << '\t' << "ceqi" << '\t' << f << ',' << b << ',' << 32 << '\n';
  reg g(8);
  out << '\t' << "sf" << '\t' << g << ',' << b << ',' << c << '\n';
  reg h(5);
  out << '\t' << "a" << '\t' << h << ',' << e << ',' << e << '\n';
  out << '\t' << "andc" << '\t' << a << ',' << g << ',' << f << '\n';
  reg i(7);
  out << '\t' << "shufb" << '\t' << i << ',' << a << ',' << h << ',' << d << '\n';
  out << '\t' << "shlqbii" << '\t' << a << ',' << i << ',' << 4 << '\n';
  x->store(a);
}

void gencode_impl::double_sint64(const COMPILER::tac* tac)
{
  using namespace std;
  using namespace COMPILER;

  reg a(3);
  address* y = getaddr(tac->y);
  y->load(a);

  if ( !_80000000000000000000000000000000.get() ){
    const type* T = tac->x->m_type;
    _80000000000000000000000000000000 =    
      auto_ptr<mem>(new mem(new_label(),T));
  }
  reg b(16);
  _80000000000000000000000000000000->load(b);
  reg c(14);
  if ( !_0000043e0000041e0000000000000000.get() ){
    const type* T = tac->x->m_type;
    _0000043e0000041e0000000000000000 =
      auto_ptr<mem>(new mem(new_label(),T));
  }
  _0000043e0000041e0000000000000000->load(c);
  if ( !_02031011121380800607141516178080.get() ){
    const type* T = tac->x->m_type;
    _02031011121380800607141516178080 =
      auto_ptr<mem>(new mem(new_label(),T));
  }
  reg d(10);
  _02031011121380800607141516178080->load(d);
  reg e(15);
  out << '\t' << "xor" << '\t' << e << ',' << a << ',' << b << '\n';
  if ( !_43e0000000000000.get() ){
    const type* T = tac->x->m_type;
    _43e0000000000000 =
      auto_ptr<mem>(new mem(new_label(),T));
  }
  _43e0000000000000->load(a);
  reg f(13);
  out << '\t' << "clz" << '\t' << f << ',' << e << '\n';
  reg g(12);
  out << '\t' << "shl" << '\t' << g << ',' << e << ',' << f << '\n';
  reg h(8);
  out << '\t' << "ceqi" << '\t' << h << ',' << f << ',' << 32 << '\n';
  reg i(11);
  out << '\t' << "sf" << '\t' << i << ',' << f << ',' << c << '\n';
  reg j(9);
  out << '\t' << "a" << '\t' << j << ',' << g << ',' << g << '\n';
  reg k(5);
  out << '\t' << "andc"        << '\t' << k << ',' << i << ',' << h << '\n';
  reg l(4);
  out << '\t' << "shufb" << '\t' << l << ',' << k << ',' << j << ',' << d << '\n';
  reg m(2);
  out << '\t' << "shlqbii" << '\t' << m << ',' << l << ',' << 4 << '\n';
  reg n(6);
  out << '\t' << "dfs" << '\t' << n << ',' << m << ',' << a << '\n';
  reg o(7);
  out << '\t' << "shlqbyi" << '\t' << o << ',' << m << ',' << 8 << '\n';
  out << '\t' << "dfa" << '\t' << a << ',' << n << ',' << o << '\n';
  address* x = getaddr(tac->x);
  x->store(a);
}

void gencode_impl::double_uint64(const COMPILER::tac* tac)
{
  using namespace std;
  using namespace COMPILER;

  address* y = getaddr(tac->y);
  reg a(3);
  y->load(a);
  reg b(11);
  out << '\t' << "clz" << '\t' << b << ',' << a << '\n';
  reg c(12);
  if ( !_0000043e0000041e0000000000000000.get() ){
    const type* T = tac->x->m_type;
    _0000043e0000041e0000000000000000 =
      auto_ptr<mem>(new mem(new_label(),T));
  }
  _0000043e0000041e0000000000000000->load(c);
  reg d(2);
  if ( !_02031011121380800607141516178080.get() ){
    const type* T = tac->y->m_type->varg();
    _02031011121380800607141516178080 =
      auto_ptr<mem>(new mem(new_label(),T));
  }
  _02031011121380800607141516178080->load(d);
  reg e(10);
  out << '\t' << "shl" << '\t' << e << ',' << a << ',' << b << '\n';
  reg f(7);
  out << '\t' << "ceqi" << '\t' << f << ',' << b << ',' << 32 << '\n';
  reg g(9);
  out << '\t' << "sf" << '\t' << g << ',' << b << ',' << c << '\n';
  reg h(8);
  out << '\t' << "a" << '\t' << h << ',' << e << ',' << e << '\n';
  reg i(4);
  out << '\t' << "andc" << '\t' << i << ',' << g << ',' << f << '\n';
  out << '\t' << "shufb" << '\t' << a << ',' << i << ',' << h << ',' << d << '\n';
  reg j(5);
  out << '\t' << "shlqbii" << '\t' << j << ',' << a << ',' << 4 << '\n';
  reg k(6);
  out << '\t' << "shlqbyi" << '\t' << k << ',' << j << ',' << 8 << '\n';
  out << '\t' << "dfa" << '\t' << a << ',' << j << ',' << k << '\n';
  address* x = getaddr(tac->x);
  x->store(a);
}

void gencode_impl::double_single(const COMPILER::tac* tac)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);

  reg r(3);
  y->load(r);
  out << '\t' << "fesd" << '\t' << r << ',' << r << '\n';
  x->store(r);
}

void gencode_impl::double_double(const COMPILER::tac* tac)
{
  assign(tac);
}

void gencode_impl::extend(const COMPILER::tac* tac, bool signextend, int bit)
{
  using namespace COMPILER;
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  reg r(3); y->load(r);

  const type* T = tac->y->m_type;
  int size = T->size();
  if ( size == 8 ){
    out << '\t' << "shlqbyi" << '\t' << r << ',' << r << ',' << 4 << '\n';
    size = bit >> 3;
  }

  if ( signextend ){
    if ( bit == 8 )
    out << '\t' << "xsbh" << '\t' << r << ',' << r << '\n';
    out << '\t' << "xshw" << '\t' << r << ',' << r << '\n';
  }
  else {
    int mask = (1 << 8 * size) - 1;
    switch ( size ){
    case 1:
      out << '\t' << "andi" << '\t' << r << ',' << r << ',' << mask << '\n';
      break;
    case 2:
      {
        reg s(2);
        out << '\t' << "ila" << '\t' << s << ',' << mask << '\n';
        out << '\t' << "and" << '\t' << r << ',' << r << ',' << s << '\n';
      }
      break;
    }
  }
  x->store(r);
}

void gencode_impl::common_real(const COMPILER::tac* tac, bool signextend, int bit)
{
  using namespace COMPILER;
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  reg r(3);
  y->load(r);
  const type* T = tac->y->m_type;
  bool single = T->size() == 4;
  if ( single )
    out << '\t' << "cflts" << '\t' << r << ',' << r << ',' << 0 << '\n';
  else {
    out << '\t' << "ai"   << '\t' << "$sp,$sp,-32" << '\n';
    out << '\t' << "brsl" << '\t' << "$lr," << "__fixdfsi" << '\n';
    out << '\t' << "ai"   << '\t' << "$sp,$sp,32" << '\n';
  }

  reg s(2);
  switch ( bit ){
  case 8:
    if ( signextend ){
      out << '\t' << "xsbh" << '\t' << s << ',' << r << '\n';
      out << '\t' << "xshw" << '\t' << r << ',' << s << '\n';
    }
    else
      out << '\t' << "andi" << '\t' << r << ',' << r << ',' << 0x00ff << '\n';
    break;
  case 16:
    if ( signextend )
      out << '\t' << "xshw" << '\t' << r << ',' << r << '\n';
    else {
      out << '\t' << "ila" << '\t' << s << ',' << 65535 << '\n';
      out << '\t' << "and" << '\t' << r << ',' << r << ',' << s << '\n';
    }
    break;
  }
  x->store(r);
}

void gencode_impl::common_longlong(const COMPILER::tac* tac)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  reg r(3); y->load(r);
  out << '\t' << "shlqbyi" << '\t' << r << ',' << r << ',' << 4 << '\n';
  x->store(r);
}

void gencode_impl::addr(const COMPILER::tac* tac)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  reg r(3);
  y->get(r);
  x->store(r);
}

namespace gencode_impl {
  void invladdr_scalar(const COMPILER::tac*);
  void invladdr_aggregate(const COMPILER::tac*);
}

void gencode_impl::invladdr(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  const type* T = tac->z->m_type;
  T->scalar() ? invladdr_scalar(tac) : invladdr_aggregate(tac);
}

void gencode_impl::invladdr_scalar(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  address* y = getaddr(tac->y);
  address* z = getaddr(tac->z);

  reg a(3); y->load(a);
  reg b(4); z->load(b);
  reg c(6);
  out << '\t' << "lqd" << '\t' << c << ',' << 0 << '(' << a << ')' << '\n';
  reg d(5);
  const type* T = tac->z->m_type;
  int size = T->size();
  std::string op = pattern_operation(size);
  out << '\t' << op << '\t' << d << ',' << 0 << '(' << a << ')' << '\n';
  reg e(2);
  out << '\t' << "shufb" << '\t' << e << ',' << b << ',' << c << ',' << d << '\n';
  out << '\t' << "stqd" << '\t' << e << ',' << 0 << '(' << a << ')' << '\n';
}

void gencode_impl::invladdr_aggregate(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  address* y = getaddr(tac->y);
  address* z = getaddr(tac->z);

  reg r3(3); y->load(r3);
  const type* T = tac->z->m_type;
  int size = T->size();
  copy(0, z, size);
}

namespace gencode_impl {
  void invraddr_scalar(const COMPILER::tac*);
  void invraddr_aggregate(const COMPILER::tac*);
}

void gencode_impl::invraddr(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  const type* T = tac->x->m_type;
  T->scalar() ? invraddr_scalar(tac) : invraddr_aggregate(tac);
}

namespace gencode_impl {
  int invraddr_helper(int size)
  {
    switch ( size ){
    case 1: return 13;
    case 2: return 14;
    default: return 0;
    }
  }
}

void gencode_impl::invraddr_scalar(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);

  reg a(3);
  y->load(a);
  reg b(2);
  out << '\t' << "lqd" << '\t' << b << ',' << 0 << '(' << a << ')' << '\n';
  const type* T = tac->x->m_type;
  int size = T->size();
  if ( int rot = invraddr_helper(size) )
    out << '\t' << "ai" << '\t' << a << ',' << a << ',' << rot << '\n';
  out << '\t' << "rotqby" << '\t' << a << ',' << b << ',' << a << '\n';
  x->store(a);
}

void gencode_impl::invraddr_aggregate(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);

  reg r4(4); y->load(r4);
  const type* T = tac->x->m_type;
  int size = T->size();
  copy(x, 0, size);
}

namespace gencode_impl {
  int param_reg_counter;
  int param_stack_counter;
  void param_reg(const COMPILER::tac*);
  void param_stack(const COMPILER::tac*);
  void param_big_aggregate(const COMPILER::tac*);
}

void gencode_impl::prepare_big_aggregate_return(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  const var* dest = tac->x;
  if ( !dest )
    return;
  const type* T = dest->m_type;
  int size = T->size();
  if ( size <= 1232 )
    return;
  ++param_reg_counter;
}

namespace gencode_impl {
        void loff_scalar(const COMPILER::tac*);
        void loff_aggregate(const COMPILER::tac*);
}

void gencode_impl::loff(const COMPILER::tac* tac)
{
        using namespace COMPILER;
        const type* T = tac->z->m_type;
        T->scalar() ? loff_scalar(tac) : loff_aggregate(tac);
}

namespace gencode_impl {
        void loff_scalar(const COMPILER::tac*, int delta);
}

void gencode_impl::loff_scalar(const COMPILER::tac* tac)
{
        using namespace std;
        using namespace COMPILER;
        {
                var* y = tac->y;
                if (y->isconstant())
                        return loff_scalar(tac, y->value());
        }
        address* x = getaddr(tac->x);
        address* y = getaddr(tac->y);
        address* z = getaddr(tac->z);

        reg r(1); x->get(r);
        reg a(3); y->load(a);
        out << '\t' << "a" << '\t' << a << ',' << a << ',' << r << '\n';
        reg b(4); z->load(b);
        reg c(6);
        out << '\t' << "lqd" << '\t' << c << ',' << 0 << '(' << a << ')' << '\n';
        reg d(5);
        const type* T = tac->z->m_type;
        int size = T->size();
        string op = pattern_operation(size);
        out << '\t' << op << '\t' << d << ',' << 0 << '(' << a << ')' << '\n';
        reg e(2);
        out << '\t' << "shufb" << '\t' << e << ',' << b << ',' << c << ',' << d << '\n';
        out << '\t' << "stqd" << '\t' << e << ',' << 0 << '(' << a << ')' << '\n';
}

void gencode_impl::loff_scalar(const COMPILER::tac* tac, int delta)
{
        using namespace std;
        using namespace COMPILER;
        address* x = getaddr(tac->x);
        address* z = getaddr(tac->z);

        reg a(3); x->get(a);
        reg b(4); z->load(b);
        reg c(6);
        out << '\t' << "lqd" << '\t' << c << ',' << delta << '(' << a << ')' << '\n';
        reg d(5);
        const type* T = tac->z->m_type;
        int size = T->size();
        string op = pattern_operation(size);
        out << '\t' << op << '\t' << d << ',' << delta << '(' << a << ')' << '\n';
        reg e(2);
        out << '\t' << "shufb" << '\t' << e << ',' << b << ',' << c << ',' << d << '\n';
        out << '\t' << "stqd" << '\t' << e << ',' << delta << '(' << a << ')' << '\n';
}

namespace gencode_impl {
        void loff_aggregate(const COMPILER::tac* tac, int delta);
}

void gencode_impl::loff_aggregate(const COMPILER::tac* tac)
{
        using namespace std;
        using namespace COMPILER;
        {
                var* y = tac->y;
                if (y->isconstant())
                        return loff_aggregate(tac, y->value());
        }
        address* x = getaddr(tac->y);
        address* y = getaddr(tac->y);
        address* z = getaddr(tac->z);

        reg xx(6); x->get(xx);
        reg r3(3); y->load(r3);
        out << '\t' << "a" << '\t' << r3 << ',' << r3 << ',' << xx << '\n';
        const type* T = tac->z->m_type;
        int size = T->size();
        copy(0, z, size);
}

void gencode_impl::loff_aggregate(const COMPILER::tac* tac, int delta)
{
        using namespace std;
        using namespace COMPILER;
        address* x = getaddr(tac->y);
        address* z = getaddr(tac->z);

        reg r3(3); x->get(r3);
        out << '\t' << "ai" << '\t' << r3 << ',' << r3 << ',' << delta << '\n';
        const type* T = tac->z->m_type;
        int size = T->size();
        copy(0, z, size);
}

namespace gencode_impl {
        void roff_scalar(const COMPILER::tac* tac);
        void roff_aggregate(const COMPILER::tac* tac);
}

void gencode_impl::roff(const COMPILER::tac* tac)
{
        using namespace COMPILER;
        const type* T = tac->x->m_type;
        T->scalar() ? roff_scalar(tac) : roff_aggregate(tac);
}

namespace gencode_impl {
        void roff_scalar(const COMPILER::tac* tac, int delta);
}

void gencode_impl::roff_scalar(const COMPILER::tac* tac)
{
        using namespace COMPILER;
        {
                var* z = tac->z;
                if (z->isconstant())
                        return roff_scalar(tac, z->value());
        }
        address* x = getaddr(tac->x);
        address* y = getaddr(tac->y);
        address* z = getaddr(tac->z);

        reg a(3);
        y->get(a);
        reg c(4);
        z->load(c);
        out << '\t' << "a" << '\t' << a << ',' << a << ',' << c << '\n';

        reg b(2);
        out << '\t' << "lqd" << '\t' << b << ',' << 0 << '(' << a << ')' << '\n';
        const type* T = tac->x->m_type;
        int size = T->size();
        if (int rot = invraddr_helper(size))
                out << '\t' << "ai" << '\t' << a << ',' << a << ',' << rot << '\n';
        out << '\t' << "rotqby" << '\t' << a << ',' << b << ',' << a << '\n';
        x->store(a);
}

void gencode_impl::roff_scalar(const COMPILER::tac* tac, int delta)
{
        using namespace COMPILER;
        address* x = getaddr(tac->x);
        address* y = getaddr(tac->y);

        reg a(3);
        y->get(a);

        reg b(2);
        out << '\t' << "lqd" << '\t' << b << ',' << delta << '(' << a << ')' << '\n';
        const type* T = tac->x->m_type;
        int size = T->size();
        if (int rot = invraddr_helper(size))
                out << '\t' << "ai" << '\t' << a << ',' << a << ',' << rot << '\n';
        out << '\t' << "rotqby" << '\t' << a << ',' << b << ',' << a << '\n';
        x->store(a);
}

namespace gencode_impl{
        void roff_aggregate(const COMPILER::tac* tac, int delta);
}

void gencode_impl::roff_aggregate(const COMPILER::tac* tac)
{
        using namespace COMPILER;
        {
                var* z = tac->z;
                if (z->isconstant())
                        return roff_aggregate(tac, z->value());
        }
        address* x = getaddr(tac->x);
        address* y = getaddr(tac->y);
        address* z = getaddr(tac->z);

        reg r4(4); y->get(r4);
        reg tmp(6); z->load(tmp);
        out << '\t' << "a" << '\t' << r4 << ',' << r4 << ',' << tmp << '\n';
        const type* T = tac->x->m_type;
        int size = T->size();
        copy(x, 0, size);
}

void gencode_impl::roff_aggregate(const COMPILER::tac* tac, int delta)
{
        using namespace COMPILER;
        address* x = getaddr(tac->x);
        address* y = getaddr(tac->y);

        reg r4(4); y->get(r4);
        out << '\t' << "ai" << '\t' << r4 << ',' << r4 << ',' << delta << '\n';
        const type* T = tac->x->m_type;
        int size = T->size();
        copy(x, 0, size);
}

void gencode_impl::param(const COMPILER::tac* tac)
{
  param_reg_counter < 77 ? param_reg(tac) : param_stack(tac);
}

void gencode_impl::param_reg(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  const type* T = tac->y->m_type;
  int size = T->size();
  if ( size > 1232 - 16 * param_reg_counter ){
    param_big_aggregate(tac);
    return;
  }
  address* y = getaddr(tac->y);
  for ( int i = 0 ; size > 0 ; i += 16, size -= 16, ++param_reg_counter ){
    assert(param_reg_counter < 77);
    reg r(param_reg_counter + 3);
    y->load(r,i);
  }
}

void gencode_impl::param_stack(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  const type* T = tac->y->m_type;
  int size = T->size();
  address* y = getaddr(tac->y);
  int& counter = param_stack_counter;
  for ( int i = 0 ; size > 0 ; i += 16, size -= 16, counter += 16 ){
    reg r(80);
    y->load(r,i);
    int offset = counter + 32;
    out << '\t' << "stqd" << '\t' << r << ',' << offset << "($sp)" << '\n';
  }
}

void gencode_impl::param_big_aggregate(const COMPILER::tac* tac)
{
  using namespace std;
  using namespace COMPILER;
  map<const var*, ::stack*>::const_iterator p =
    big_aggregate_param.find(tac->y);
  assert(p != big_aggregate_param.end());
  address* y = p->second;
  assert(param_reg_counter < 77);
  reg r(param_reg_counter + 3);
  y->get(r);
  ++param_reg_counter;
}

namespace gencode_impl {
  void call_void(const COMPILER::tac*);
  void save_return_value(const COMPILER::tac*, int);
}

void gencode_impl::call(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  param_reg_counter = param_stack_counter = 0;

  const var* entry = tac->x;
  if ( entry ){
    const type* T = entry->m_type;
    int size = T->size();
    if ( size > 1232 ){
      address* x = getaddr(tac->x);
      reg r3(3);
      x->get(r3);
    }
  }

  call_void(tac);

  if ( entry ){
    const type* T = entry->m_type;
    int size = T->size();
    if ( size <= 1232 ){
      for ( int i = 0 ; size > 0 ; ++i, size -= 16 )
        save_return_value(tac,i);
    }
  }
}

void gencode_impl::call_void(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  address* y = getaddr(tac->y);
  const type* T = tac->y->m_type;
  if ( T->scalar() ){
    reg r(11);
    y->load(r);
    out << '\t' << "bisl" << '\t' << "$lr, " << r << '\n';
  }
  else {
    mem* m = dynamic_cast<mem*>(y);
    std::string label = m->label();
    out << '\t' << "brsl" << '\t' << "$lr, " << label << '\n';
  }
}

void gencode_impl::save_return_value(const COMPILER::tac* tac, int i)
{
  address* x = getaddr(tac->x);
  x->store(reg(i+3),16*i);
}

namespace gencode_impl {
  void set_return_value(const COMPILER::tac*, int);
  void set_return_value(const COMPILER::tac*);
  void _return_void(const COMPILER::tac*);
}

void gencode_impl::_return(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  const var* entry = tac->y;
  if ( entry ){
    const type* T = entry->m_type;
    int size = T->size();
    if ( size <= 1232 ){
      for ( int i = 0 ; size > 0 ; ++i , size -= 16 )
        set_return_value(tac,i);
    }
    else
      set_return_value(tac);
  }
  _return_void(tac);
}

void gencode_impl::set_return_value(const COMPILER::tac* tac, int i)
{
  address* y = getaddr(tac->y);
  y->load(reg(i+3),16*i);
}

void gencode_impl::set_return_value(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  ::stack ret(parameter::m_hidden,4);
  reg r3(3);
  ret.load(r3);
  address* y = getaddr(tac->y);
  const type* T = tac->y->m_type;
  int size = T->size();
  copy(0, y, size);
}

void gencode_impl::_return_void(const COMPILER::tac* tac)
{
  using namespace std;
  if ( tac != m_last ){
    string label = function_exit.m_label;
    function_exit.m_ref = true;
    out << '\t' << "br" << '\t' << label << '\n';
  }
}

namespace gencode_impl {
  void ifgoto_notlonglong(const COMPILER::tac*);
  void ifgoto_longlong(const COMPILER::tac*);
  void ifgoto_real(const COMPILER::tac*);
}

void gencode_impl::ifgoto(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  const type* T = tac->y->m_type;
  if ( T->real() )
    ifgoto_real(tac);
  else if ( T->size() > 4 )
    ifgoto_longlong(tac);
  else
    ifgoto_notlonglong(tac);
}

namespace gencode_impl {
  using namespace std;
  using namespace COMPILER;
  struct ifgoto_notlonglong_table_entry {
    goto3ac::op m_cmp;
    bool m_swap;
    string m_jmp;
    ifgoto_notlonglong_table_entry(goto3ac::op cmp, bool b, string jmp)
      : m_cmp(cmp), m_swap(b), m_jmp(jmp) {}
    ifgoto_notlonglong_table_entry(){}
  };
  typedef ifgoto_notlonglong_table_entry INTE;
  struct ifgoto_notlonglong_table : map<goto3ac::op,INTE> {
    ifgoto_notlonglong_table()
    {
      (*this)[goto3ac::EQ] = INTE(goto3ac::EQ,false,"brnz");
      (*this)[goto3ac::NE] = INTE(goto3ac::EQ,false,"brz");
      (*this)[goto3ac::LT]  = INTE(goto3ac::GT,true, "brnz");
      (*this)[goto3ac::GT]  = INTE(goto3ac::GT,false,"brnz");
      (*this)[goto3ac::LE] = INTE(goto3ac::GT,false,"brz");
      (*this)[goto3ac::GE] = INTE(goto3ac::GT,true, "brz");
    }
  } ifgoto_notlonglong_table;
}

namespace gencode_impl {
  void ifgoto_common(const COMPILER::tac*, bool);
}

void gencode_impl::ifgoto_notlonglong(const COMPILER::tac* tac)
{
  ifgoto_common(tac,false);
}

void gencode_impl::ifgoto_common(const COMPILER::tac* tac, bool single)
{
  using namespace COMPILER;
  address* x = getaddr(tac->y);
  address* y = getaddr(tac->z);
  reg rx(3); x->load(rx);
  reg ry(4); y->load(ry);

  const goto3ac* ptr = static_cast<const goto3ac*>(tac);
  goto3ac::op op = ptr->m_op;
  const INTE& entry = ifgoto_notlonglong_table[op];

  reg tmp(5);
  out << '\t';
  if ( single )
    out << 'f';
  out << 'c';
  const type* T = tac->y->m_type;
  if (!T->_signed() && op != goto3ac::EQ && op != goto3ac::NE)
    out << 'l';
  out << entry.m_cmp << '\t' << tmp << ',';
  if ( entry.m_swap )
    out << ry << ',' << rx << '\n';
  else
    out << rx << ',' << ry << '\n';
  out << '\t' << entry.m_jmp << '\t' << tmp << ", " << '.' << func_label << ptr->m_to << '\n';
}

namespace gencode_impl {
  void ifgoto_single(const COMPILER::tac*);
  void ifgoto_double(const COMPILER::tac*);
}

void gencode_impl::ifgoto_real(const COMPILER::tac* tac)
{
  const type* T = tac->y->m_type;
  int size = T->size();
  size < 8 ? ifgoto_single(tac) : ifgoto_double(tac);
}

void gencode_impl::ifgoto_single(const COMPILER::tac* tac)
{
  ifgoto_common(tac,true);
}

namespace gencode_impl {
  using namespace std;
  using namespace COMPILER;
  struct ifgoto_double_table_entry {
    string m_func;
    string m_cgti;
    string m_jump;
    ifgoto_double_table_entry(string func = "", string cgti = "", string jump = "")
      : m_func(func), m_cgti(cgti), m_jump(jump) {}
  };
  typedef ifgoto_double_table_entry IDTE;
  struct ifgoto_double_table : map<goto3ac::op,IDTE> {
    ifgoto_double_table()
    {
      (*this)[goto3ac::EQ] = IDTE("__nedf2","","brz");
      (*this)[goto3ac::NE] = IDTE("__eqdf2","","brnz");
      (*this)[goto3ac::LT] = IDTE("__gedf2","-1","brz");
      (*this)[goto3ac::GT] = IDTE("__ledf2","0","brnz");
      (*this)[goto3ac::LE] = IDTE("__gtdf2","0","brz");
      (*this)[goto3ac::GE] = IDTE("__ltdf2","-1","brnz");
    }
  } ifgoto_double_table;
}

void gencode_impl::ifgoto_double(const COMPILER::tac* tac)
{
  address* x = getaddr(tac->y);
  address* y = getaddr(tac->z);
  reg r3(3); x->load(r3);
  reg r4(4); y->load(r4);

  const goto3ac* ptr = static_cast<const goto3ac*>(tac);
  const IDTE& entry = ifgoto_double_table[ptr->m_op];

  out << '\t' << "ai" << '\t' << "$sp,$sp,-32" << '\n';
  out << '\t' << "brsl" << '\t' << "$lr, " << entry.m_func << '\n';
  out << '\t' << "ai" << '\t' << "$sp,$sp,32" << '\n';
  if ( !entry.m_cgti.empty() )
    out << '\t' << "cgti" << '\t' << r3 << ',' << r3 << ',' << entry.m_cgti << '\n';
  out << '\t' << entry.m_jump << '\t' << r3 << ", " << '.' << func_label << ptr->m_to << '\n';
}

namespace gencode_impl {
  void ifgoto_longlong_equality(const COMPILER::tac* tac, bool);
  void ifgoto_longlong_relational(const COMPILER::tac* tac);
}

void gencode_impl::ifgoto_longlong(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  const goto3ac* ptr = static_cast<const goto3ac*>(tac);
  goto3ac::op op = ptr->m_op;
  (op == goto3ac::EQ || op == goto3ac::NE) ? ifgoto_longlong_equality(tac,op == goto3ac::EQ)
    : ifgoto_longlong_relational(tac);
}

void gencode_impl::ifgoto_longlong_equality(const COMPILER::tac* tac, bool equal)
{
  address* x = getaddr(tac->y);
  address* y = getaddr(tac->z);
  reg a(3);
  x->load(a);
  reg b(4);
  y->load(b);
  reg c(5);
  out << '\t' << "ceq" << '\t' << c << ',' << a << ',' << b << '\n';
  reg d(2);
  out << '\t' << "gb" << '\t' << d << ',' << c << '\n';
  out << '\t' << "cgti" << '\t' << a << ',' << d << ',' << 11 << '\n';
  std::string op = equal ? "brnz" : "brz";
  const goto3ac* ptr = static_cast<const goto3ac*>(tac);
  out << '\t' << op << '\t' << a << ", " << '.' << func_label << ptr->m_to << '\n';
}

namespace gencode_impl {
  using namespace std;
  using namespace COMPILER;
  struct ifgoto_longlong_table_entry {
    bool m_swap;
    string m_jmp;
    ifgoto_longlong_table_entry(bool b = false, string jmp = "")
      : m_swap(b), m_jmp(jmp) {}
  };
  typedef ifgoto_longlong_table_entry ILTE;
  struct ifgoto_longlong_table : map<goto3ac::op,ILTE> {
    ifgoto_longlong_table()
    {
      (*this)[goto3ac::LT] = ILTE(true,"brnz");
      (*this)[goto3ac::GT] = ILTE(false,"brnz");
      (*this)[goto3ac::LE] = ILTE(false,"brz");
      (*this)[goto3ac::GE] = ILTE(true,"brz");
    }
  } ifgoto_longlong_table;
}

void gencode_impl::ifgoto_longlong_relational(const COMPILER::tac* tac)
{
  address* x = getaddr(tac->y);
  address* y = getaddr(tac->z);
  reg a(3);
  x->load(a);
  reg b(4);
  y->load(b);

  const goto3ac* ptr = static_cast<const goto3ac*>(tac);
  const ILTE& entry = ifgoto_longlong_table[ptr->m_op];

  reg c(9);
  out << '\t' << "clgt" << '\t' << c << ',';
  if ( entry.m_swap )
    out << b << ',' << a << '\n';
  else
    out << a << ',' << b << '\n';
  reg d(7);
  out << '\t' << "ceq" << '\t' << d << ',' << a << ',' << b << '\n';
  reg e(5);
  const type* T = tac->y->m_type;
  std::string cmp = !T->_signed() ? "clgt" : "cgt";
  out << '\t' << cmp << '\t' << e << ',';
  if ( entry.m_swap )
    out << b << ',' << a << '\n';
  else
    out << a << ',' << b << '\n';
  reg f(8);
  out << '\t' << "rotqbyi" << '\t' << f << ',' << c << ',' << 4 << '\n';
  reg g(6);
  out << '\t' << "and" << '\t' << g << ',' << d << ',' << f << '\n';
  reg h(2);
  out << '\t' << "or" << '\t' << h << ',' << g << ',' << e << '\n';
  out << '\t' << entry.m_jmp << '\t' << h << ", " << '.' << func_label << ptr->m_to << '\n';
}

void gencode_impl::_goto(const COMPILER::tac* tac)
{
  const goto3ac* ptr = static_cast<const goto3ac*>(tac);
  if (ptr->m_op == goto3ac::NONE)
          out << '\t' << "br" << '\t' << '.' << func_label << ptr->m_to << '\n';
  else
          ifgoto(tac);
}

void gencode_impl::to(const COMPILER::tac* tac)
{
        out << '.' << func_label << tac << ":\n";
}

void gencode_impl::alloc(const COMPILER::tac* tac)
{
  reg a(3);
  out << '\t' << "lqd" << '\t' << a << ',' << "0($sp)" << '\n';

  reg b(5);
  address* y = getaddr(tac->y);
  y->load(b);
  out << '\t' << "ai" << '\t' << b << ',' << b << ',' << 15 << '\n';
  out << '\t' << "andi" << '\t' << b << ',' << b << ',' << -16 << '\n';
  out << '\t' << "sf" << '\t' << "$sp" << ',' << b << ',' << "$sp" << '\n';
  out << '\t' << "stqd" << '\t' << a << ',' << 0 << "($sp)" << '\n';

  int c = stack_layout.m_fun_arg;
  out << '\t' << "ai" << '\t' << a << ',' << "$sp" << ',' << c << '\n';

  address* x = getaddr(tac->x);
  alloced_addr* p = dynamic_cast<alloced_addr*>(x);
  p->set(a);
}

void gencode_impl::dealloc(const COMPILER::tac* tac)
{
        // Nothing to be done
}

void gencode_impl::asm_(const COMPILER::tac* tac)
{
  const asm3ac* p = static_cast<const asm3ac*>(tac);
  out << '\t' << p->m_inst << '\n';
}

void gencode_impl::_va_start(const COMPILER::tac* tac)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  reg r(3);
  y->get(r);
  out << '\t' << "ai" << '\t' << r << ',' << r << ',' << 16 << '\n';
  x->store(r);
}

void gencode_impl::_va_arg(const COMPILER::tac* tac)
{
  using namespace COMPILER;
  const va_arg3ac* ptr = static_cast<const va_arg3ac*>(tac);
  const type* T = ptr->m_type;
  int size = T->size();

  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  reg r(3);
  y->load(r);
  out << '\t' << "lqd" <<'\t' << r << ',' << 0 << '(' << r << ')' << '\n';
  x->store(r);

  y->load(r);
  out << '\t' << "ai" << '\t' << r << ',' << r << ',' << 16 << '\n';
  reg s(4);
  out << '\t' << "lqd" << '\t' << s << ',' << 0 << "($sp)" << '\n';
  out << '\t' << "ceq" << '\t' << s << ',' << s << ',' << r << '\n';
  std::string label = new_label();
  out << '\t' << "brz" << '\t' << s << ',' << label << '\n';
  out << '\t' << "ai" << '\t' << r << ',' << r << ',' << 32 << '\n';
  out << label << ":\n";
  y->store(r);
}

void gencode_impl::_va_end(const COMPILER::tac*)
{
        // Nothing to be done
}

address* gencode_impl::getaddr(const COMPILER::var* entry)
{
  using namespace std;
  map<const var*, address*>::const_iterator p
    = address_descriptor.find(entry);
  assert(p != address_descriptor.end());
  return p->second;
}

void gencode_impl::binop_notlonglong(const COMPILER::tac* tac, std::string op)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  address* z = getaddr(tac->z);

  reg a(3); y->load(a);
  reg b(4); z->load(b);

  if ( op == "sf" )
    std::swap(a,b);

  out << '\t' << op << '\t' << a << ',' << a << ',' << b << '\n';
  x->store(a);
}

void gencode_impl::runtime_unary(const COMPILER::tac* tac, std::string func)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);

  reg r3(3); y->load(r3);

  out << '\t' << "ai"   << '\t' << "$sp,$sp,-32" << '\n';
  out << '\t' << "brsl" << '\t' << "$lr," << func << '\n';
  out << '\t' << "ai"   << '\t' << "$sp,$sp,32" << '\n';

  x->store(r3);
}

void gencode_impl::runtime_binary(const COMPILER::tac* tac, std::string func)
{
  address* x = getaddr(tac->x);
  address* y = getaddr(tac->y);
  address* z = getaddr(tac->z);

  reg r3(3); y->load(r3);
  reg r4(4); z->load(r4);

  out << '\t' << "ai"   << '\t' << "$sp,$sp,-32" << '\n';
  out << '\t' << "brsl" << '\t' << "$lr," << func << '\n';
  out << '\t' << "ai"   << '\t' << "$sp,$sp,32" << '\n';

  x->store(r3);
}
