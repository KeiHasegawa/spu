#include "stdafx.h"
#include "c_core.h"

#define COMPILER c_compiler

#include "spu.h"

extern "C" DLL_EXPORT int generator_seed()
{
#ifdef _MSC_VER
  int r = _MSC_VER;
#ifndef CXX_GENERATOR
  r += 10000000;
#else // CXX_GENERATOR
  r += 20000000;
#endif // CXX_GENERATOR
#ifdef WIN32
  r += 100000;
#endif // WIN32
#endif // _MSC_VER
#ifdef __GNUC__
  int r = (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__);
#ifndef CXX_GENERATOR
  r += 30000000;
#else // CXX_GENERATOR
  r += 40000000;
#endif // CXX_GENERATOR
#endif // __GNUC__
  return r;
}

std::ostream out(std::cout.rdbuf());

std::ofstream* ptr_out;

extern "C" DLL_EXPORT int generator_open_file(const char* fn)
{
  ptr_out = new std::ofstream(fn);
  out.rdbuf(ptr_out->rdbuf());
  return 0;
}

extern "C" DLL_EXPORT void generator_generate(const COMPILER::generator::interface_t* ptr)
{
  genobj(ptr->m_root);
  if ( ptr->m_func )
    genfunc(ptr->m_func,*ptr->m_code);
}

std::string make_pattern(signed char c)
{
  std::ostringstream os;
  os << '\t' << ".byte" << '\t' << int(c);
  return os.str();
}

inline void destroy_address(std::pair<const COMPILER::var*, address*> x)
{
    delete x.second;
}

#ifdef CXX_GENERATOR
inline void call_simply(std::string func)
{
  out << '\t' << "brsl" << '\t' << "$lr," << func << '\n';
}
#endif // CXX_GENERATOR

extern "C" DLL_EXPORT int generator_close_file()
{
  using namespace std;

  if ( double_0x80000000.get() ){
    output_section(rom);
    out << '\t' << ".align" << '\t' << 4 << '\n';
    string label = double_0x80000000->label();
    out << label << ":\n";
    out << '\t' << ".long" << '\t' << "1105199104\n";
    out << '\t' << ".long" << '\t' << "0\n";
  }
  if ( _02031011121380800607141516178080.get() ){
    output_section(rom);
    out << '\t' << ".align" << '\t' << 4 << '\n';
    string label = _02031011121380800607141516178080->label();
    out << label << ":\n";
    int pattern[] = { 0x02, 0x03, 0x10, 0x11, 0x12, 0x13, 0x80, 0x80,
                      0x06, 0x07, 0x14, 0x15, 0x16, 0x17, 0x80, 0x80 };
    transform(&pattern[0],&pattern[16],ostream_iterator<string>(out,"\n"),
              make_pattern);
  }
  if ( _04050607808080800c0d0e0f80808080.get() ){
    output_section(rom);
    out << '\t' << ".align" << '\t' << 4 << '\n';
    string label = _04050607808080800c0d0e0f80808080->label();
    out << label << ":\n";
    int pattern[] = { 0x04, 0x05, 0x06, 0x07, 0x80, 0x80, 0x80, 0x80,
                      0x0c, 0x0d, 0x0e, 0x0f, 0x80, 0x80, 0x80, 0x80 };
    transform(&pattern[0],&pattern[16],ostream_iterator<string>(out,"\n"),
              make_pattern);
  }
  if ( _80000000000000000000000000000000.get() ){
    output_section(rom);
    out << '\t' << ".align" << '\t' << 4 << '\n';
    string label = _80000000000000000000000000000000->label();
    out << label << ":\n";
    int pattern[] = { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    transform(&pattern[0],&pattern[16],ostream_iterator<string>(out,"\n"),
              make_pattern);
  }
  if ( _0000043e0000041e0000000000000000.get() ){
    output_section(rom);
    out << '\t' << ".align" << '\t' << 4 << '\n';
    string label = _0000043e0000041e0000000000000000->label();
    out << label << ":\n";
    out << '\t' << ".long" << '\t' << 1086 << '\n';
    out << '\t' << ".long" << '\t' << 1054 << '\n';
    out << '\t' << ".long" << '\t' << 0 << '\n';
    out << '\t' << ".long" << '\t' << 0 << '\n';
  }
  if ( _43e0000000000000.get() ){
    output_section(rom);
    out << '\t' << ".align" << '\t' << 4 << '\n';
    string label = _43e0000000000000->label();
    out << label << ":\n";
    out << '\t' << ".long" << '\t' << 1138753536 << '\n';
    out << '\t' << ".long" << '\t' << 0 << '\n';
  }
  if ( _12121212121212131212121212121212.get() ){
    output_section(rom);
    out << '\t' << ".align" << '\t' << 4 << '\n';
    string label = _12121212121212131212121212121212->label();
    out << label << ":\n";
    int pattern[] = { 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x13,
                      0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12 };
    transform(&pattern[0],&pattern[16],ostream_iterator<string>(out,"\n"),
              make_pattern);
  }
  if ( _10101010101002031010101010101010.get() ){
    output_section(rom);
    out << '\t' << ".align" << '\t' << 4 << '\n';
    string label = _10101010101002031010101010101010->label();
    out << label << ":\n";
    int pattern[] = { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x02, 0x03,
                      0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10 };
    transform(&pattern[0],&pattern[16],ostream_iterator<string>(out,"\n"),
              make_pattern);
  }
#ifdef CXX_GENERATOR
  if ( !ctors.empty() ){
    output_section(rom);
    out << '\t' << ".align" << '\t' << 4 << '\n';
    string label = new_label();
    out << label << ":\n";
    enter_helper(32);
    for_each(ctors.begin(),ctors.end(),call_simply);
    leave_helper(32);
    output_section(ctor);
    out << '\t' << ".long" << '\t' << label << '\n';
  }
  if ( !dtor_name.empty() ){
    output_section(rom);
    out << '\t' << ".align" << '\t' << 4 << '\n';
    string label = new_label();
    out << label << ":\n";
    enter_helper(32);
    call_simply(dtor_name);
    leave_helper(32);
    output_section(dtor);
    out << '\t' << ".long" << '\t' << label << '\n';
  }
#endif // CXX_GENERATOR
#ifdef _DEBUG
  for_each(address_descriptor.begin(), address_descriptor.end(),destroy_address);
#endif // _DEBUG
  delete ptr_out;
  return 0;
}

bool debug_flag;

class generator_option_table : public std::map<std::string, int (*)()> {
  static int set_debug_flag()
  {
    debug_flag = true;
    return 0;
  }
public:
  generator_option_table()
  {
    (*this)["--debug"] = set_debug_flag;
  }
} generator_option_table;

int option_handler(const char* option)
{
  std::map<std::string, int (*)()>::const_iterator p =
    generator_option_table.find(option);
  return p != generator_option_table.end() ? (p->second)() : 1;
}

extern "C" DLL_EXPORT void generator_option(int argc, const char** argv, int* error)
{
        using namespace std;
        ++argv;
        --argc;
#ifdef _MSC_VER
        const char** p; int* q;
        for (p = &argv[0], q = &error[0]; p != &argv[argc]; ++p, ++q)
                *q = option_handler(*p);
#else // _MSC_VER
        transform(&argv[0],&argv[argc],&error[0],option_handler);
#endif // _MSC_VER
}

void(*output3ac)(std::ostream&, const COMPILER::tac*);

extern "C" DLL_EXPORT void generator_spell(void* arg)
{
        using namespace std;
        using namespace COMPILER;
        void* magic[] = {
                ((char **)arg)[0],
        };
        int index = 0;
        memcpy(&output3ac, &magic[index++], sizeof magic[0]);
}

std::map<const COMPILER::var*, address*> address_descriptor;

bool string_literal(const COMPILER::usr* entry)
{
  int c = entry->m_name[0];
  if ( c == 'L' )
    c = entry->m_name[1];
  return c == '"';
}

mem::mem(std::string label, const COMPILER::type* T) : m_label(label)
{
        m_size = T->scalar() ? T->size() : -1;
}

mem::mem(std::string label, int size) : m_label(label), m_size(size) {}

int calc_rot(int, int);

void mem::load(const reg& r, int delta) const
{
  out << '\t' << "lqa" << '\t' << r << ',' << m_label;
  if ( delta )
    out << '+' << delta;
  out << '\n';
  if ( int rot = calc_rot(0,m_size) )
    out << '\t' << "rotqbyi" << '\t' << r << ',' << r << ',' << rot << '\n';
}

int calc_rot(int offset, int size)
{
  offset &= 15;

  switch ( size ){
  case 1  : return offset - 3;
  case 2  : return offset - 2;
  default : return offset;
  }
}

void mem::store(const reg& r, int delta) const
{
  if ( int rot = calc_rot(0,m_size) )
    out << '\t' << "shlqbyi" << '\t' << r << ',' << r << ',' << -rot << '\n';
  out << '\t' << "stqa" << '\t' << r << ',' << m_label;
  if ( delta )
    out << '+' << delta;
  out << '\n';
}

void mem::get(const reg& r) const
{
  out << '\t' << "ila" << '\t' << r << ',' << m_label << '\n';
}

void output_section(section kind)
{
  static section current;
  if ( current != kind ){
    current = kind;
    switch ( kind ){
    case rom: out << '\t' << ".text" << '\n'; break;
    case ram: out << '\t' << ".data" << '\n'; break;
    case ctor:
    case dtor:
      out << '\t' << ".section" << '\t';
      out << (kind == ctor ? ".ctors" : ".dtors");
      out << ',' << '"' << "aw" << '"' << '\n';
      break;
    }
  }
}

std::string new_label()
{
  static int cnt;
  std::ostringstream os;
  os << ".L" << ++cnt;
  return os.str();
}

bool character_constant(const COMPILER::usr* entry)
{
  int c = entry->m_name[0];
  if ( c == '\'' )
    return true;

  if ( c == 'L' ){
    c = entry->m_name[1];
    if ( c == '\'' )
      return true;
  }
  return false;
}

bool integer_suffix(int c)
{
  return c == 'U' || c == 'L' || c == 'u' || c == 'l';
}

imm::imm(const COMPILER::usr* entry) : m_pair(std::make_pair(0,0))
{
  using namespace std;

  if ( entry->m_name == "'\\0'" )
    m_value = "0";
  else if ( entry->m_name == "'\\a'" ){
    ostringstream os;
    os << int('\a');
    m_value = os.str();
  }
  else if ( entry->m_name == "'\\b'" ){
    ostringstream os;
    os << int('\b');
    m_value = os.str();
  }
  else if ( entry->m_name == "'\\t'" ){
    ostringstream os;
    os << int('\t');
    m_value = os.str();
  }
  else if ( entry->m_name == "'\\n'" ){
    ostringstream os;
    os << int('\n');
    m_value = os.str();
  }
  else if ( entry->m_name == "'\\v'" ){
    ostringstream os;
    os << int('\v');
    m_value = os.str();
  }
  else if ( entry->m_name == "'\\r'" ){
    ostringstream os;
    os << int('\r');
    m_value = os.str();
  }
  else if ( character_constant(entry) )
    m_value = entry->m_name;
#ifdef CXX_GENERATOR
  else if ( entry->m_name == "true" )
    m_value = "1";
  else if ( entry->m_name == "false" )
    m_value = "0";
#endif // CXX_GENERATOR
  else {
    m_value = entry->m_name;
    while ( integer_suffix(m_value[m_value.size()-1]) )
      m_value.erase(m_value.size()-1);
    int sign;
    if ( m_value[0] == '-' ){
      sign = -1;
      m_value.erase(0,1);
    }
    else
      sign = 1;
    istringstream is(m_value.c_str());
    unsigned int n;
    if ( m_value[0] == '0' && m_value.size() > 1 ){
      if ( m_value[1] == 'x' || m_value[1] == 'X' )
        is >> hex >> n;
      else
        is >> oct >> n;
    }
    else
      is >> n;
    n *= sign;
    int m = int(n) << 16 >> 16;
    if ( n != m ){
      m_value.erase();
      m_pair.first = n >> 16;
      m_pair.second = n & 0xffff;
    }
    else if ( sign == -1 )
      m_value = '-' + m_value;
  }
}

void imm::load(const reg& r, int delta) const
{
  assert(delta == 0);
  if ( !m_value.empty() )
    out << '\t' << "il" << '\t' << r << ',' << m_value << '\n';
  else {
    out << '\t' << "ilhu" << '\t' << r << ',' << m_pair.first << '\n';
    out << '\t' << "iohl" << '\t' << r << ',' << m_pair.second << '\n';
  }
}

struct function_exit function_exit;

#ifdef CXX_GENERATOR
std::string scope_name(symbol_tree* scope)
{
  if ( tag* T = dynamic_cast<tag*>(scope) )
    return scope_name(scope->m_parent) + func_name(T->name());
  if ( _namespace* nmsp = dynamic_cast<_namespace*>(scope) )
    return nmsp->name();
  return "";
}

class conv_fname {
  std::string* m_res;
public:
  conv_fname(std::string* res) : m_res(res) {}
  void operator()(int c)
  {
    switch ( c ){
    case ' ':   *m_res += "sp"; break;
    case '~':   *m_res += "ti"; break;
    case '[':   *m_res += "lb"; break;
    case ']':   *m_res += "rb"; break;
    case '(':   *m_res += "lp"; break;
    case ')':   *m_res += "rp"; break;
    case '&':   *m_res += "an"; break;
    case '<':   *m_res += "lt"; break;
    case '>':   *m_res += "gt"; break;
    case '=':   *m_res += "eq"; break;
    case '!':   *m_res += "ne"; break;
    case '+':   *m_res += "pl"; break;
    case '-':   *m_res += "mi"; break;
    case '*':   *m_res += "mu"; break;
    case '/':   *m_res += "di"; break;
    case '%':   *m_res += "mo"; break;
    case '^':   *m_res += "xo"; break;
    case '|':   *m_res += "or"; break;
    case ',':   *m_res += "cm"; break;
    case '\'':  *m_res += "da"; break;
    default:    *m_res += char(c); break;
    }
  }
};

std::string func_name(std::string name)
{
  std::string res;
  std::for_each(name.begin(),name.end(),conv_fname(&res));
  return res;
}

std::string signature(const type_expr* type)
{
#ifdef _MSC_VER
  use_ostream_magic obj;
#endif // _MSC_VER
  std::ostringstream os;
  type->encode(os);
  return func_name(os.str());
}
#endif // CXX_GENERATOR

struct stack_layout stack_layout;

void stack::load(const reg& r, int delta) const
{
  m_big_aggregate_param ? load_special(r,delta) : load_normal(r,delta);
}

void stack::load_special(const reg& r, int delta) const
{
  get(r);
  assert(!(delta & 15));
  out << '\t' << "lqd" << '\t' << r << ',' << delta << '(' << r << ')' << '\n';
}

void stack::load_normal(const reg& r, int delta) const
{
  int offset = (m_offset + delta) & ~15;
  reg sp(has_allocatable ? 127 : 1);
  if ( offset < 8192 )
    out << '\t' << "lqd" << '\t' << r << ',' << offset << '(' << sp << ')' << '\n';
  else {
    out << '\t' << "il" << '\t' << r << ',' << offset << '\n';
    out << '\t' << "a" << '\t' << r << ',' << r << ',' << sp << '\n';
    out << '\t' << "lqd" << '\t' << r << ',' << 0 << '(' << r << ')' << '\n';
  }
  if ( int rot = calc_rot(m_offset,m_size) )
    out << '\t' << "rotqbyi" << '\t' << r << ',' << r << ',' << rot << '\n';
}

std::string pattern_operation(int);

void stack::store(const reg& r, int delta) const
{
  assert(!m_big_aggregate_param);
  m_size > 0 ? store_scalar(r) : store_aggregate(r,delta);
}

void stack::store_scalar(const reg& r) const
{
  reg x(11), y(12), z(13);
  int offset = m_offset & ~15;
  reg sp(has_allocatable ? 127 : 1);
  if ( offset < 8192 )
    out << '\t' << "lqd" << '\t' << x << ',' << offset << '(' << sp << ')' << '\n';
  else {
    out << '\t' << "il" << '\t' << x << ',' << offset << '\n';
    out << '\t' << "a" << '\t' << x << ',' << x << ',' << sp << '\n';
    out << '\t' << "lqd" << '\t' << x << ',' << 0 << '(' << x << ')' << '\n';
  }
  std::string op = pattern_operation(m_size);
  out << '\t' << op << '\t' << y << ',' << m_offset << '(' << sp << ')' << '\n';
  out << '\t' << "shufb" << '\t' << z << ',' << r << ',' << x << ',' << y << '\n';
  if ( offset < 8192 )
    out << '\t' << "stqd" << '\t' << z << ',' << offset << '(' << sp << ')' << '\n';
  else {
    out << '\t' << "il" << '\t' << x << ',' << offset << '\n';
    out << '\t' << "a" << '\t' << x << ',' << x << ',' << sp << '\n';
    out << '\t' << "stqd" << '\t' << z << ',' << 0 << '(' << x << ')' << '\n';
  }
}

std::string pattern_operation(int size)
{
  switch ( size ){
  case 1:  return "cbd";
  case 2:  return "chd";
  case 4:  return "cwd";
  default: return "cdd";
  }
}

void stack::store_aggregate(const reg& r, int delta) const
{
  int offset = m_offset + delta;
  assert((offset & 15) == 0);
  reg sp(has_allocatable ? 127 : 1);
  out << '\t' << "stqd" << '\t' << r << ',' << offset << '(' << sp << ')' << '\n';
}

void stack::get(const reg& r) const
{
  reg sp(has_allocatable ? 127 : 1);
  if ( m_offset < 512 )
    out << '\t' << "ai" << '\t' << r << ',' << sp << ',' << m_offset << '\n';
  else {
    out << '\t' << "il" << '\t' << r << ',' << m_offset << '\n';
    out << '\t' << "a" << '\t' << r << ',' << r << ',' << sp << '\n';
  }

  if ( m_big_aggregate_param )
    out << '\t' << "lqd" << '\t' << r << ',' << '0' << '(' << r << ')' << '\n';
}

std::auto_ptr<mem> double_0x80000000;
std::auto_ptr<mem> _02031011121380800607141516178080;
std::auto_ptr<mem> _04050607808080800c0d0e0f80808080;
std::auto_ptr<mem> _80000000000000000000000000000000;
std::auto_ptr<mem> _0000043e0000041e0000000000000000;
std::auto_ptr<mem> _43e0000000000000;
std::auto_ptr<mem> _12121212121212131212121212121212;
std::auto_ptr<mem> _10101010101002031010101010101010;

std::map<const COMPILER::var*, stack*> big_aggregate_param;

bool has_allocatable;
