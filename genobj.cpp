#include "stdafx.h"
#include "c_core.h"

#define COMPILER c_compiler

#include "spu.h"

void output_data(const std::pair<std::string, std::vector<COMPILER::usr*> >&);

void genobj(const COMPILER::scope* tree)
{
  using namespace std;
  using namespace COMPILER;
  const map<string, vector<usr*> >& usrs = tree->m_usrs;
  for_each(usrs.begin(), usrs.end(), output_data);
  const vector<scope*>& children = tree->m_children;
  for_each(children.begin(), children.end(), genobj);
}

void output_data2(COMPILER::usr*);

void output_data(const std::pair<std::string, std::vector<COMPILER::usr*> >& x)
{
  using namespace std;
  using namespace COMPILER;
  const vector<usr*>& vec = x.second;
  for_each(vec.begin(), vec.end(), output_data2);
}

void constant_data(COMPILER::usr* entry);

void string_literal_data(COMPILER::usr* entry);

int initial(int offset, std::pair<int,COMPILER::var*>);

int static_counter;

void output_data2(COMPILER::usr* entry)
{
  using namespace std;
  using namespace COMPILER;

  map<const var*, address*>::const_iterator p = address_descriptor.find(entry);
  if (p != address_descriptor.end())
    return;

  usr::flag flag = entry->m_flag;
  usr::flag mask = usr::flag(usr::TYPEDEF|usr::ENUM_MEMBER);
  if (flag & mask)
    return;

  string label = entry->m_name;
  const type* T = entry->m_type;
  usr::flag mask2 = usr::flag(usr::EXTERN|usr::FUNCTION);
  if (flag & mask2) {
    address_descriptor[entry] = new mem(label,T);
    return;
  }

  if (entry->isconstant())
    return constant_data(entry);

  if (string_literal(entry))
    return string_literal_data(entry);

  if (!is_top(entry->m_scope) && !(flag & usr::STATIC))
    return;

  output_section(T->modifiable() ? ram : rom);

  if (flag & usr::STATIC) {
    if (!is_top(entry->m_scope)) {
      ostringstream os;
      os << '.' << ++static_counter;
      label += os.str();
    }
  }
  else
    out << '\t' << ".global" << '\t' << label << '\n';

  out << '\t' << ".align" << '\t' << T->align() << '\n';

  int size = T->size();

  if (flag & usr::WITH_INI) {
    with_initial* wi = static_cast<with_initial*>(entry);
    map<int, var*>& value = wi->m_value;
    if (int n = size - accumulate(value.begin(),value.end(),0,initial))
      out << '\t' << ".space" << n << '\n';
  }
  else {
    int n = size < 16 ? 16 : size + 16;
    out << '\t' << ".comm" << '\t' << label << ", " << n << " # " << size << '\n';
  }

  address_descriptor[entry] = new mem(label, T);
}

void constant_data(COMPILER::usr* entry)
{
  using namespace std;
  using namespace COMPILER;
  const type* T = entry->m_type;
  if ( T->real() || T->size() > 4 ){
    output_section(rom);
    out << '\t' << ".align" << '\t' << T->align() << '\n';
    string label = new_label();
    out << label << ":\n";
        pair<int, var*> tmp(0, entry);
    initial(0,tmp);
    address_descriptor[entry] = new mem(label,T);
  }
  else
    address_descriptor[entry] = new imm(entry);
}

void string_literal_data(COMPILER::usr* entry)
{
  using namespace std;
  output_section(rom);
  string label = new_label();
  out << label << ":\n";
  string name = entry->m_name;
  name.erase(name.size()-1);
  name += "\\0\"";
  out << '\t' << ".ascii" << '\t' << name << '\n';
  address_descriptor[entry] = new mem(label,entry->m_type);
}

int initial_real(int offset, COMPILER::usr* entry);

int initial_notreal(int offset, COMPILER::usr* entry);

int initial(int offset, std::pair<int, COMPILER::var*> x)
{
  using namespace std;
  using namespace COMPILER;
  int specified = x.first;
  if (int n = specified - offset) {
    out << '\n' << ".space" << '\t' << n << '\n';
    offset = specified;
  }
  
  var* v = x.second;
  usr* entry = v->usr_cast();
  assert(entry);
  const type* T = entry->m_type;
  if ( T->real() )
    return initial_real(offset, entry);
  if ( T->scalar() )
    return initial_notreal(offset, entry);
  assert(string_literal(entry));
  string name = entry->m_name;
  name.erase(name.size()-1);
  name += "\\0\"";
  out << '\t' << ".ascii" << '\t' << name << '\n';
  return offset + name.size();
}

int initial_notreal(int offset, COMPILER::usr* entry)
{
  using namespace std;
  using namespace COMPILER;
  const type* T = entry->m_type;
  int size = T->size();

  switch (size) {
  case 1:
    {
      typedef char X;
      typedef constant<X> C;
      C* c = static_cast<C*>(entry);
      out << '\t' << ".byte" << '\t' << int(c->m_value) << '\n';
      return offset + size;
    }
  case 2:
    {
      typedef short X;
      typedef constant<X> C;
      C* c = static_cast<C*>(entry);
      out << '\t' << ".short" << '\t' << int(c->m_value) << '\n';
      return offset + size;
    }
  case 4:
    {
      if (addrof* ad = entry->addrof_cast()) {
	var* v = ad->m_ref;
	usr* u = v->usr_cast();
	string name = u->m_name;
	int offset = ad->m_offset;
	out << '\t' << ".long" << '\t' << name << "+" << offset << '\n';	
      }
      else {
	typedef int X;
	typedef constant<X> C;
	C* c = static_cast<C*>(entry);
	out << '\t' << ".long" << '\t' << int(c->m_value) << '\n';
      }
      return offset + size;
    }
  default:
    {
      assert(size == 8);
      typedef __int64 X;
      typedef constant<X> C;
      C* c = static_cast<C*>(entry);
      union {
	__int64 ll;
	int i[2];
      } tmp = { c->m_value };
      int i = 1;
      if ( *(char*)&i )
	swap(tmp.i[0],tmp.i[1]);
      out << '\t' << ".long" << '\t' << tmp.i[0] << '\n';
      out << '\t' << ".long" << '\t' << tmp.i[1] << '\n';
      return offset + size;
    }
  }
}

int initial_real(int offset, COMPILER::usr* entry)
{
        using namespace std;
        using namespace COMPILER;
        const type* T = entry->m_type;
        int size = T->size();

        if ( size == 4 ){
                union {
                        float f;
                        int i;
                } tmp = { (float)atof(entry->m_name.c_str()) };
                out << '\t' << ".long" << '\t' << tmp.i << '\n';
        }
        else {
                union {
                        double d;
                        int i[2];
                } tmp = { atof(entry->m_name.c_str()) };
                int i = 1;
                if (*(char*)&i)
                        swap(tmp.i[0], tmp.i[1]);
                out << '\t' << ".long" << '\t' << tmp.i[0] << '\n';
                out << '\t' << ".long" << '\t' << tmp.i[1] << '\n';
        }
        return offset + size;
}

#ifdef CXX_GENERATOR
struct cxxbuiltin_table : std::map<std::string, std::string> {
  cxxbuiltin_table()
  {
    (*this)["new"] = "_Znwm";
    (*this)["delete"] = "_ZdlPv";

    (*this)["new.a"] = "_Znam";
    (*this)["delete.a"] = "_ZdaPv";
  }
} cxxbuiltin_table;

bool cxxbuiltin(const symbol* entry, std::string* res)
{
  std::string name = entry->m_name;
  std::map<std::string, std::string>::const_iterator p =
    cxxbuiltin_table.find(name);
  if ( p != cxxbuiltin_table.end() ){
    *res = p->second;
    return true;
  }
  else
    return false;
}

void entry_func_sub(const symbol* func)
{
  if ( func->m_templ_func )
    return;
  if ( address_descriptor.find(func) == address_descriptor.end() ){
    std::string label;
    if ( !cxxbuiltin(func,&label) ){
      label = scope_name(func->m_scope);
      label += func_name(func->m_name);
      if ( !func->m_csymbol )
        label += signature(func->m_type);
    }
    address_descriptor[func] = new mem(label,func->m_type);
  }
}

void entry_func(const std::pair<std::string,std::vector<symbol*> >& pair)
{
  const std::vector<symbol*>& vec = pair.second;
  std::for_each(vec.begin(),vec.end(),entry_func_sub);
}
#endif // CXX_GENERATOR
