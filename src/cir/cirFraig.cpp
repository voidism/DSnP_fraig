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

bool
CirMgr::prove(unsigned &g1, unsigned &g2, SatSolver& s,vector<Var>& Varnode)
{
  bool result;
  bool inv = ((_simValue[g1]) != (_simValue[g2]));
  if(g1!=0){
  Var newV = s.newVar();
  s.addXorCNF(newV, Varnode[g1], false, Varnode[g2], inv);
  s.assumeRelease();            // Clear assumptions
  s.assumeProperty(newV, true); // k = 1
  cout << char(13) << "Proving (" << g1 << ", "
       << ((inv) ? "!" : "") << g2 << ")...";
  result = s.assumpSolve();
  cout << ((result) ? "SAT" : "UNSAT") << "!!";
  cout.flush();
  }
  else if(g1==0){
    s.assumeRelease();            // Clear assumptions
    s.assumeProperty(Varnode[g2], !inv); // k = 1
    cout << char(13) << "Proving " << g2 << " = "
         << ((inv) ? "1" : "0") << "...";
    result = s.assumpSolve();
    cout << ((result)? "SAT" : "UNSAT") << "!!";
    cout.flush();
  }

  cout.flush();
  return result;
}

void
CirMgr::fraig()
{
  SatSolver s;
  vector<size_t> goodpatterns(i,0);
  unsigned gpa = 0;
  vector<vector<unsigned>> to_merge;
  vector<Var> Varnode(_idMap.size());
  unsigned tempsize = _FEClist.size();
  while (_FEClist.size() != 0)
  {
    //unordered_map<unsigned, vector<unsigned> *> merge_map;
    s.reset();
    s.initialize();
    for(auto &x:_DFSlist){
      if(x == 0) continue;
      if(x->type == "AIG"){
      Varnode[x->gateID]=(s.newVar());
      //x->setVar(s.newVar());
      }
    }
    for(auto &x:_DFSlist){
      if(x == 0) continue;
      if(x->type == "AIG"){
      s.addAigCNF(Varnode[x->gateID], Varnode[x->_in[0].first->gateID], x->_in[0].second,Varnode[x->_in[1].first->gateID], x->_in[1].second);
      /* s.addAigCNF(x->getVar(),x->_in[0].first->getVar(),x->_in[0].second,
                             ,x->_in[1].first->getVar(),x->_in[1].second);
      } */
    }
    //all Var(sat node) is linked!!!
    //while(_FEClist.size()!=0){
      unsigned idx = 0;
      //if proved -> separate!
      //and record the good pattern!(SAT)
      //while (j1 < _FEClist[idx].size() && j2 < _FEClist[idx].size())
      for (idx = 0; idx < _DFSlist.size();idx++)//---------DFS loop--------------------
      {
        unsigned &AD = _DFSlist[idx]->fecAddr;
        unsigned &ID = _DFSlist[idx]->gateID;
        unsigned mypos;
        if (AD == INT_MAX || _DFSlist[idx]->type != "AIG")
          continue;
        /* if (_FEClist[AD].size() == 1)
        {
          _FEClist[AD] = _FEClist[_FEClist.size() - 1];
          _FEClist.pop_back();
          continue;
        } */
        for (unsigned j2 = 0; j2 < _FEClist[AD].size();j2++){//---in-group loop
          if(_FEClist[AD][j2] == ID){
            mypos = j2;
            continue;
          }
          if (prove(ID, _FEClist[AD][j2], s, Varnode))
          {
            //cout << "   sperate: (" << _FEClist[idx][j1] << ", " << _FEClist[idx][j2] << ")" << endl;
            //_FEClist[idx][j2] = _FEClist[idx][_FEClist[idx].size() - 1];
            //_FEClist[idx].pop_back();
            for (unsigned pi = 0; pi < _PIlist.size(); pi++)
            {
              goodpatterns[pi] = (goodpatterns[pi] << 1) + s.getValue(Varnode[_PIlist[pi]->gateID]);
              gpa++;
            }
          }
          //if unproved -> merge them!(UNSAT)
          else
          {
            //cout << "   merge: (" << _FEClist[idx][j1] << ", " << _FEClist[idx][j2] << ")" << endl;
            vector<unsigned> temp;
            temp.push_back(ID);
            temp.push_back(_FEClist[AD][j2]);
            to_merge.push_back(temp);
          }
        //if FEC grp has only one item
        /* if(_FEClist[idx].size()==1){
          _FEClist[idx] = _FEClist[_FEClist.size() - 1];
          _FEClist.pop_back();
          idFecMapGen();
        } */
        //if patterns%64 =0 ->simulate!
        if(gpa%64==0){
          //cout << "Updating by SAT... ";
          sim_pattern(goodpatterns);
          if(tempsize != _FEClist.size()) { cout << char(13) << "Updating by SAT... Total #FEC Group = " << _FEClist.size() << endl; tempsize = _FEClist.size();}
          cout.flush();
          goodpatterns.resize(i,0);
          idFecMapGen();
          //if(tempsize != _FEClist.size()) { cout << endl; tempsize = _FEClist.size();}
        }
        

          }//-------in-group loop------------
         _FEClist[_DFSlist[idx]->fecAddr][mypos] = _FEClist[_DFSlist[idx]->fecAddr][_FEClist[_DFSlist[idx]->fecAddr].size() - 1];
         _FEClist[_DFSlist[idx]->fecAddr].pop_back();/* 
         if(_FEClist[_DFSlist[idx]->fecAddr].size()==1){
          _FEClist[_DFSlist[idx]->fecAddr] = _FEClist[_FEClist.size() - 1];
          _FEClist.pop_back();
          idFecMapGen();
        } */

        }//---------DFS loop END---------------
      }
      if(to_merge.size()){//>=(_DFSlist.size()/100)){
      for (auto &x : to_merge) {freplace(_idMap[x[1]], _idMap[x[0]]);}
      to_merge.clear();
      
      if(!_DFSlist.empty()) _DFSlist.clear();
      (CirGate::_globalRef)++;
      for (auto &x:_POlist) { DFSlistGen(x); }
      //if((unsigned)idx > _FEClist.size()-1) idx=0;
      //else idx++;
      //cout << "=======================out=========================" << endl;
    }
    if (gpa % 64 != 0)
    {
      //cout << "Updating by SAT... "
      sim_pattern(goodpatterns);
      cout << char(13) << "Updating by SAT... Total #FEC Group = " << _FEClist.size();
      cout.flush();
      cout << endl;
      goodpatterns.resize(i,0);
      idFecMapGen();
    }
/* 
    for (auto &x : to_merge)
    {
      freplace(_idMap[x[1]], _idMap[x[0]]);
    } */
  if(to_merge.size()){
      for (auto &x : to_merge) {freplace(_idMap[x[1]], _idMap[x[0]]);}
      to_merge.clear();
      if(!_DFSlist.empty()) _DFSlist.clear();
      (CirGate::_globalRef)++;
      for (auto &x:_POlist) { DFSlistGen(x); }
  }
  sort_and_pop();
  cout << "Updating by UNSAT... Total #FEC Group = " << _FEClist.size() << endl;
  if(!_DFSlist.empty()) _DFSlist.clear();
  (CirGate::_globalRef)++;
  for (auto &x:_POlist){DFSlistGen(x);}
  }
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
       a--;m--;
      }
       return;
}

void
CirMgr::freplace(CirGate* it,CirGate* alter)
{
    if(it==0||alter==0){
      return;
    }
    //-------Modify his fanout gates' fanin to the alterative gate
    for (auto &xy : it->_out)
    {
      for (auto &jj : xy.first->_in)
      {
        if (jj.first == it)
        {
          jj.first = alter;
          alter->_out.push_back(make_pair(xy.first, jj.second));
        }
      }
      }
       cout << "\rFraig: " << alter->gateID << " merging " << it->gateID << "..." << endl;
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
       return;
}