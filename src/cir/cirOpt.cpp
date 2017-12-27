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
  if(!(it->_in.empty())){
   for (int jdx = 0; jdx < (int)it->_in.size(); jdx++)
   {
    if(it->_in.at(jdx).first->_ref == CirGate::_globalRef) continue;
    DFSlistGen(it->_in.at(jdx).first);
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
  for(auto &x:_DFSlist){
    DFSopt(x);
  }
}

void
CirMgr::DFSopt(CirGate *it)
{
  if(it->_ref == CirGate::_globalRef) return;
  //if(it->type == "UNDEF") {it->_ref = CirGate::_globalRef; return;}
  if((it->_in.empty())){ return; }
  if(it->_in.size()>=2){
   if (it->_in.at(0).first==_idMap[0]) {
     //cout << it->type << it->gateID <<" -- _in1:CONST!" << ((it->_in.at(0).second)? "(*)":"") << endl;
     for(auto &xy:it->_out){
       for(auto & jj:xy.first->_in){
         if(jj.first == it){
           jj.first = ((it->_in.at(0).second)? it->_in.at(1).first:_idMap[0]);
         }
        }
      }
    if(it->_in.at(0).second){

       cout << "Simplifying: " << it->_in.at(1).first->gateID << " merging " << it->gateID << "..." << endl;


       for(vector<Cell>::iterator jj=it->_in.at(1).first->_out.begin(); jj!=it->_in.at(1).first->_out.end(); jj++){
        //cout << "first step\n"; 
        if(jj->first == it){
           //cout << "erase!\n";
           it->_in.at(1).first->_out.erase(jj);
           break;
          }
        }
       it->_in.at(1).first->_out.insert(it->_in.at(1).first->_out.end(), it->_out.begin(), it->_out.end());
      }
    else if(!(it->_in.at(0).second)){
       cout << "Simplifying: " << 0 << " merging " << it->gateID << "..." << endl;
       //cout << _idMap[0]->_out[0].first->gateID << endl;
       //for(auto &x:_idMap[0]->_out) cout << x.first->gateID << endl;
       for(vector<Cell>::iterator jj=_idMap[0]->_out.begin(); jj!=_idMap[0]->_out.end();){

          //cout << jj->first->gateID << endl;
        /* }
        cout << "good\n"; 
       for(vector<Cell>::iterator jj=_idMap[0]->_out.begin(); jj!=_idMap[0]->_out.end(); ++jj){
         cout << "ggg"; */
        if(jj->first == it){
           //cout << "erase" << endl;
           _idMap[0]->_out.erase(jj);
           //cout << "fuckyou" << endl;
           break;
          }
          else {
            //cout << "pass" << endl;
            jj++;
        }
      }
      //cout << "insert" << endl;
       _idMap[0]->_out.insert(_idMap[0]->_out.end(), it->_out.begin(), it->_out.end());
      //cout << "afterinsert" << endl;
      }
     
         
  }//_in.at(0) is CONST ... END
  if (it->_in.at(1).first==_idMap.at(0)) cout << it->type << it->gateID <<" -- _in2:CONST!" << (it->_in.at(1).second? "(*)":"") << endl;
  
  }//if have _in>1

}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
