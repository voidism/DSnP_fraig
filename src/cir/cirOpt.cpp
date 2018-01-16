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

  //for (int idx = i + 1; idx < i + o + 1; idx++)
  /* for (std::map<unsigned int, CirGate *>::iterator it=_idMap.begin(); it!=_idMap.end(); ++it){//clean unused gate
        CirGate * x = it->second;
        if(x==0) continue;
        DFSlistGen(x);
  } */
  for (std::vector<CirGate *>::iterator it=_idMap.begin(); it!=_idMap.end(); ++it){//clean unused gate
    CirGate * &x = *it;//->second;
    //cout << it -> first << endl;
    if(x==0) continue;
    if(x->unused == 1){
      cout << "Sweeping: " << x->type << "(" << x->gateID << ") removed..." << endl;
    for(vector<Cell>::iterator ii=x->_in.begin(); ii!=x->_in.end();ii++){
      //cout << "(" << x->gateID << ")" << " IN: " <<ii->first->gateID << endl;
       for(vector<Cell>::iterator jj=ii->first->_out.begin(); jj!=ii->first->_out.end();){
        //cout << "(" << x->gateID << ")" << " IN: " <<ii->first->gateID << " OUT: " << jj->first->gateID << endl;
        if(jj->first == x){
          //cout << "ERASE: " << x->gateID << endl;
           ii->first->_out.erase(jj);
           break;
        }
          else jj++;
      }
    }
      delete *it;
      //_idMap.erase(it);
      *it = 0;
      a--;m--;
    }
  }
}
void
CirMgr::DFSlistGen(CirGate *it)
{
  if(it->_ref == CirGate::_globalRef) return;
  //if(it->type == "UNDEF") { _DFSlist.push_back(it); it->_ref = CirGate::_globalRef; return;}
  if(!(it->_in.empty())){
   for (int jdx = 0; jdx < (int)it->_in.size(); jdx++)
   {
    if(it->_in[jdx].first->_ref == CirGate::_globalRef) continue;
    if(it == it->_in[jdx].first) continue;
    DFSlistGen(it->_in[jdx].first);
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
  (CirGate::_globalRef)++;
  int modified=0;
  for(auto &x:_DFSlist){
    if(DFSopt(x)) modified++;
  }

  if(modified!=0){
    if(!_DFSlist.empty()) _DFSlist.clear();
    (CirGate::_globalRef)++;
      for (auto &x:_POlist)
      {
        DFSlistGen(x);
      }
  }
}

bool
CirMgr::DFSopt(CirGate *it)
{
  
  if(it->_ref == CirGate::_globalRef) return 0;
  if((it->_in.empty())){ return 0; }
  if(it->_in.size()<2){ return 0; }
    //CirGate* congate = 0;
    CirGate* other = 0;
    bool inv = 0;
  if (it->_in.at(0).first==_idMap[0]){
    //congate = it->_in.at(0).first;
    other = it->_in.at(1).first;
    inv = it->_in.at(0).second;
  }
  else if (it->_in.at(1).first==_idMap[0]){
    //congate = it->_in.at(1).first;
    other = it->_in.at(0).first;
    inv = it->_in.at(1).second;
  }
  //===================IF THE IN(0) IS CONST=========================
   if (other != 0) {
     //-------------IF IN(0) IS 1: merged with IN(1) else: merged with CONST 0
     CirGate * alter = ((inv)? other:_idMap[0]);
     //-------Modify his fanout gates' fanin to the alterative gate
     for(auto &xy:it->_out){
       for(auto & jj:xy.first->_in){
         if(jj.first == it){
           jj.first = alter;
           jj.second = (bool)(inv^jj.second);
           alter->_out.push_back(make_pair(xy.first,jj.second));
         }
        }
      }
       cout << "Simplifying: " << alter->gateID << " merging " << ((inv)? "!":"") << it->gateID << "..." << endl;
       //-------Modify his fanin gates' fanout (erase itself first)
       for(vector<Cell>::iterator ii=it->_in.begin(); ii!=it->_in.end();ii++){
       for(vector<Cell>::iterator jj=ii->first->_out.begin(); jj!=ii->first->_out.end();){
        if(jj->first == it){
           ii->first->_out.erase(jj);
          }
          else jj++;
        }
      }
      //------------Add his fanout to the alter's fanout
       //alter->_out.insert(alter->_out.end(), it->_out.begin(), it->_out.end());
       it->unused = 1;
       if(it->type!="PI" && it->type!="PO" && it->type!="CONST"){
       delete it;
       _idMap[it->gateID] = 0;
       a--;m--;
      }
       return 1;
      }
      if(it->_in.at(0).first == it->_in.at(1).first){
        CirGate* alter = 0;
        bool inv = 0;
        if(it->_in.at(0).second == it->_in.at(1).second){ alter = it->_in.at(0).first; inv = it->_in.at(0).second;}
        else alter = _idMap[0];

        //-------Modify his fanout gates' fanin to the alterative gate
     for(auto &xy:it->_out){
       for(auto & jj:xy.first->_in){
         if(jj.first == it){
           jj.first = alter;
           jj.second = (bool)(inv^jj.second);
           alter->_out.push_back(make_pair(xy.first,jj.second));
         }
        }
      }
       cout << "Simplifying: " << alter->gateID << " merging " << ((inv)? "!":"") << it->gateID << "..." << endl;
       //-------Modify his fanin gates' fanout (erase itself first)
       for(vector<Cell>::iterator ii=it->_in.begin(); ii!=it->_in.end();ii++){
       for(vector<Cell>::iterator jj=ii->first->_out.begin(); jj!=ii->first->_out.end();){
        if(jj->first == it){
           ii->first->_out.erase(jj);
          }
          else  jj++;
        }
      }
      //------------Add his fanout to the alter's fanout
       //alter->_out.insert(alter->_out.end(), it->_out.begin(), it->_out.end());
       it->unused = 1;
       if(it->type!="PI" && it->type!="PO" && it->type!="CONST"){
       delete it;
       _idMap[it->gateID] = 0;
       a--;m--;
      }
       return 1;
      }
      return 0;
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
