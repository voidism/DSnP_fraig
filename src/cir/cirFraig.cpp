/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{
  HashSet<CirGate *> strashSet(_DFSlist.size());
  int modified=0;
  for(auto&x:_DFSlist){
    if ((x->type != "AIG")) continue;
    CirGate *backup = x;
    if (strashSet.query(x)){//x is replaced!!
      //cout << "YEAH! "<< x->gateID << "KEY: " << (*x)() << endl;
      streplace(backup, x);
      modified++;
    }
    else{
      //cout << "INSERT: " << x->gateID << "KEY: " << (*x)() << endl;
      strashSet.insert(x);
    }
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

void
CirMgr::fraig()
{
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
void
CirMgr::streplace(CirGate* it,CirGate* alter)
{
     //-------Modify his fanout gates' fanin to the alterative gate
     for(auto &xy:it->_out){
       for(auto & jj:xy.first->_in){
         if(jj.first == it){
           jj.first = alter;
           alter->_out.push_back(make_pair(xy.first,jj.second));
         }
        }
      }
       cout << "Strashing: " << alter->gateID << " merging " << it->gateID << "..." << endl;
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
       a--;
      }
       return;
}