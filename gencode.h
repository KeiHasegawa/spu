#ifndef _GENCODDE_H_
#define _GENCODDE_H_

class gencode {
  const std::vector<COMPILER::tac*>& m_v3ac;
  int m_counter;
public:
  gencode(const std::vector<COMPILER::tac*>&);
  void operator()(const COMPILER::tac*);
};

#endif // _GENCODDE_H_
