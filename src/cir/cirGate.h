/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.
typedef pair<CirGate*,bool> Cell;
/* enum Type{
  PI,
  PO,
  CONST,
  UNDEF,
  AIG
}; */
class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate
{
  friend class CirMgr;

public:
  CirGate(){}
  virtual ~CirGate() {}

  //hash function
  size_t operator () () const 
  {
    if (_in.empty()) return 0;
    size_t _i1;
    size_t _i2;
    if ((_in[0].first->gateID)<(_in[1].first->gateID)){
      _i1 = ((_in[0].first->gateID) << 1) | ((size_t)(_in[0].second));
      _i2 = ((_in[1].first->gateID) << 1) | ((size_t)(_in[1].second));
    }
    else{
      _i1 = ((_in[1].first->gateID) << 1) | ((size_t)(_in[1].second));
      _i2 = ((_in[0].first->gateID) << 1) | ((size_t)(_in[0].second));
    }
    size_t k = (_i1 << 32) + _i2;
    return k;
  }

  // Basic access methods
  unsigned getID() { return gateID; }
  string getTypeStr() const { return type; }
  unsigned getLineNo() const { return linenum; }

  // Printing functions
  virtual void printGate() const = 0;
  virtual bool isAig() const { return false; }
  void reportGate() const;
  void reportFanin(int level) const;
  void reportFanout(int level) const;

  void DFSearchByLevel_fanin(const CirGate *it,int dig_level,int total_level,bool inv) const;
  void DFSearchByLevel_fanout(const CirGate *it,int dig_level,int total_level,bool inv) const;
  //Var getVar() const { return _var; }
  //void setVar(const Var& v) { _var = v; }
private:
protected:
  static unsigned _globalRef;
  mutable unsigned _ref; //marked number used in DFS
  static unsigned _fraigRef;
  mutable unsigned fraig = 0;
  bool unused;
  //int varID;
  //int litID;
  unsigned gateID;
  //vector<CirGate *> _fout;
  vector<unsigned> _idin; //fanin literal id list
  //vector<CirGate *> _fin;
  vector<Cell> _in; 
  //vector<unsigned> _idout;
  vector<Cell> _out;

  unsigned linenum;
  string type;
  string symb = "";
  //Var        _var;
  unsigned fecAddr = INT_MAX;
};

/* struct Cell{
  CirGate* first;
  bool second;
}; */


class PI : public CirGate
{
  friend class CirMgr;
public:
  PI(int, unsigned);
  ~ PI(){}
  void printGate() const {};
};

//const string PI::type = "PI";

class PO : public CirGate
{
  friend class CirMgr;
public:
  PO(int, unsigned, unsigned);
  ~ PO(){}
  void printGate() const {};
};
//const string PO::type = "PO";
class Const : public CirGate
{
  friend class CirMgr;
public:
  Const(int, unsigned);
  ~ Const(){}
  void printGate() const {};
};
//const string Const::type = "CONST";

class Undef : public CirGate
{
  friend class CirMgr;
public:
  Undef(int);
  ~ Undef(){}
  void printGate() const {};
};
//const string Undef::type = "UNDEF";
class AIG : public CirGate
{
  friend class CirMgr;
public:
  AIG(int, unsigned, unsigned, unsigned);
  AIG(int, unsigned);
  ~AIG() {}
  void printGate() const {};
};
//const string AIG::type = "AIG";
#endif // CIR_GATE_H
