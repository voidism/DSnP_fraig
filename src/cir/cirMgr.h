/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include "myHashSet.h"
#include "cirGate.h"
#include <unordered_map>
using std::unordered_map;

using namespace std;

#include "cirDef.h"

extern CirMgr *cirMgr;

#define CLEAN_GLIST                                                                    \
  for (std::vector<CirGate *>::iterator it = _idMap.begin(); it != _idMap.end(); ++it) \
  {                                                                                    \
    if (*it == 0)                                                                      \
      continue;                                                                        \
    delete *it;                                                                        \
    *it = 0;                                                                           \
  }

// TODO: Define your own data members and member functions
class CirMgr
{
  friend CirGate;

public:
  CirMgr() {}
  ~CirMgr()
  {
    for (std::vector<CirGate *>::iterator it = _idMap.begin(); it != _idMap.end(); ++it)
    {
      if (*it == 0)
        continue;
      delete *it;
      *it = 0;
    }
  }

  // Access functions
  // return '0' if "gid" corresponds to an undefined gate.
  CirGate *getGate(unsigned gid) const
  {
    //std::map<unsigned int, CirGate *>::const_iterator tmp = _idMap.find(gid);
    //return ((tmp != _idMap.end()) ? tmp->second : 0);
    return _idMap[gid];
  }

  // Member functions about circuit construction
  bool readCircuit(const string &);

  // Member functions about circuit optimization
  void sweep();
  void optimize();
  void streplace(CirGate *, CirGate *);

  // Member functions about simulation
  void randomSim();
  void fileSim(ifstream &);
  void setSimLog(ofstream *logFile) { _simLog = logFile; }
  size_t sim(CirGate *);
  void updateLog(size_t mask);
  void sim_pattern(vector<size_t>);
  void sim_random();
  void classify_first_time(CirGate *, bool &,unordered_map<size_t,unsigned> &);
  bool split_fec_groups(vector<unsigned> &);
  void sort_and_pop();
  void idFecMapGen();
  //bool fec_comp(const vector<unsigned> &, const vector<unsigned> &);

  // Member functions about fraig
  void strash();
  void printFEC() const;
  void fraig();
  bool prove(unsigned &, unsigned &, SatSolver &,vector<Var>&);
  void freplace(CirGate *, CirGate *);

  // Member functions about circuit reporting
  string simstring(size_t);
  //vector<unsigned> getFECgroup(const unsigned&) const;
  void printSummary() const;
  void printNetlist() const;
  void printPIs() const;
  void printPOs() const;
  void printFloatGates() const;
  void writeAag(ostream &) const;
  void printFECPairs() const;
  void writeGate(ostream &, CirGate *) const;

  void DFSearch(CirGate *, unsigned &) const;
  void DFSearch_NoPrint(CirGate *, unsigned &, stringstream &ss) const;
  void DFSlistGen(CirGate *);
  bool DFSopt(CirGate *);
  void insertSort(vector<pair<CirGate *, bool>> &);

private:
  ofstream *_simLog;
  //vector<CirGate *> _Glist;
  vector<CirGate *> _PIlist;
  vector<CirGate *> _POlist;
  vector<CirGate *> _DFSlist;
  //map<unsigned, CirGate *> _idMap;
  vector<CirGate *> _idMap;
  vector<size_t> _simValue;
  vector<vector<unsigned>> _FEClist;
  int m, i, l, o, a;
};

#endif // CIR_MGR_H
