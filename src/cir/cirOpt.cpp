/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{

  (CirGate::_globalRef)++;
  //for (int idx = i + 1; idx < i + o + 1; idx++)
  /* for (std::map<unsigned int, CirGate *>::iterator it=_idMap.begin(); it!=_idMap.end(); ++it){//clean unused gate
        CirGate * x = it->second;
        if(x==0) continue;
        DFSlistGen(x);
  } */
  for (std::map<unsigned int, CirGate *>::iterator it=_idMap.begin(); it!=_idMap.end(); ++it){//clean unused gate
    CirGate * x = it->second;
    //cout << it -> first << endl;
    if(x==0) continue;
    if(x->unused == 1){
      cout << "Sweeping: " << x->type << "(" << x->gateID << ") removed..." << endl;
/*       std::map<unsigned int, CirGate *>::iterator tmp = _idMap.find(x->gateID);
      if(tmp != _idMap.end()) _idMap.erase(tmp); */
      delete x;
      //_idMap.erase(it);
      it->second = 0;
      a--;
    }
  }
}
void
CirMgr::DFSlistGen(CirGate *it)
{
  if(it->_ref == CirGate::_globalRef) return;
  if(it->type == "UNDEF") {it->_ref = CirGate::_globalRef; return;}
  if(!(it->_fin.empty())){
   for (int jdx = 0; jdx < (int)it->_fin.size(); jdx++)
   {
    if(it->_fin.at(jdx)->_ref == CirGate::_globalRef) continue;
    DFSlistGen(it->_fin.at(jdx));
   }
  }
  _DFSlist.push_back(it);
  it->_ref=CirGate::_globalRef;
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{

}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
