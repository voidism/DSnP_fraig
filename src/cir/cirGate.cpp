/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include <algorithm>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

extern CirMgr *cirMgr;

// TODO: Implement memeber functions for class(es) in cirGate.h
unsigned CirGate::_globalRef = 0;
unsigned CirGate::_fraigRef = 0;

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
  cout << "================================================================================" << endl;
  stringstream ss;
  string sb = ((symb=="")? "":"\""+symb+"\"");
  ss << "= " << type << "(" << gateID << ")" << sb << ", line " << ((linenum==0)? 0: (linenum+1) );
  string s = ss.str();
  cout << s << endl;
  cout << "= FECs:";
  vector<unsigned> *fd = 0;
  for(auto &y:cirMgr->_FEClist){
    for(auto &x:y){
      if(x == gateID){
        fd = &y;
      }
    }
  }
  if(fd!=0){
    for(auto &x:*fd){
      if(x!=gateID)
      cout << ((cirMgr->_simValue[x]!=cirMgr->_simValue[(*fd)[0]])?" !":" ") << x;
    }
  }
  else cout << " ";
  cout << endl;
  cout << "= Value: " << cirMgr->simstring(cirMgr->_simValue[gateID]) << endl;
  cout << "================================================================================" << endl;
}

void CirGate::DFSearchByLevel_fanin(const CirGate *it,int dig_level,int total_level,bool inv) const{
  if(dig_level<0)return;
  bool printed = (it->_ref==CirGate::_globalRef);
  string s = "";
  s.resize(2*(total_level - dig_level),' ');
  cout << s << (inv?"!":"") << it->type << " " << it->gateID;
  cout << ((printed&&(it->type!="PI")&&(it->type!="UNDEF")&&(dig_level!=0))?" (*)":"")<<endl;//(symbol name)
  //if(it->type=="PI") return;
  if(printed) return;
  //if(dig_level==0) return;
  if(!it->_in.empty()&&dig_level!=0) it->_ref=CirGate::_globalRef;
  else return;
  // print myself brfore children are printed!
  //vector<bool> answer;
   for (int jdx = 0; jdx < (int)it->_in.size(); jdx++)
   {
     //if(it->_fin.at(jdx)->_ref!=_globalRef)
     DFSearchByLevel_fanin(it->_in.at(jdx).first,dig_level-1,total_level,(it->_in.at(jdx).second));
     //answer.push_back(tmp);
    }
    // bool ret = answer[0];
    // for(int ip=0;i<answer.size();i++){
    //   ret = ret&&answer[ip];
    // }
    // return ret;
  } 
  

  
void CirGate::DFSearchByLevel_fanout(const CirGate *it,int dig_level,int total_level,bool inv) const{
  if(dig_level<0)return;
  bool printed = (it->_ref==CirGate::_globalRef);
  string s = "";
  s.resize(2*(total_level - dig_level),' ');
  cout << s << (inv?"!":"") << it->type << " " << it->gateID;
  cout << ((printed&&(it->type!="PO")&&(dig_level!=0))?" (*)":"")<<endl;//(symbol name)
  if(printed){return;}
  if(!it->_out.empty()&&dig_level!=0) it->_ref=CirGate::_globalRef;
  else return;
  // print myself brfore children are printed!
  //sort(it->_0fout.begin(),it->_0fout.end());
  /*  for (int jdx = 0; jdx < (int)it->_0fout.size(); jdx++)
   {
    //if(it->_0fout.at(jdx)->_ref!=_globalRef)
    DFSearchByLevel_fanout(it->_0fout.at(jdx),dig_level-1,total_level,(it->_idout.at(jdx) % 2));
  } */
  for (vector<pair<CirGate*,bool> >::const_iterator jj = it->_out.begin();jj!=it->_out.end();jj++){
    DFSearchByLevel_fanout(jj->first,dig_level-1,total_level,jj->second);
  }
}

void
CirGate::reportFanin(int level) const
{
   assert (level >= 0);
   _globalRef++;
   DFSearchByLevel_fanin(this,level,level,false);
}

void
CirGate::reportFanout(int level) const
{
   assert (level >= 0);
   _globalRef++;
   DFSearchByLevel_fanout(this,level,level,false);
}

PI::PI(int var,unsigned line) {
    //varID = (var);
    unused = 0;
    gateID = var;
    //litID = (lit);
    linenum = (line);
    type = "PI";
  }
  
PO::PO(int lit,unsigned line,unsigned gid) {
    //varID = (lit/2);
    /* litID = (lit); */
    unused = 0;
    linenum = (line);
    _idin.push_back(lit);
    gateID = gid;
    type = "PO";
  }
  
AIG::AIG(int var,unsigned in1,unsigned in2,unsigned line) {
    //varID = (var);
    /* litID = (lit); */
    unused = 0;
    gateID = var;
    linenum = (line);
    _idin.push_back(in1);
    _idin.push_back(in2);
    type = "AIG";
  }

AIG::AIG(int var,unsigned line) {
    //varID = (var);
    /* litID = (lit); */
    unused = 0;
    gateID = var;
    linenum = (line);
    type = "AIG";
}
  
Undef::Undef(int var) {
    //varID = (var);
    /* litID = (lit); */
    unused = 0;
    gateID = var;
    linenum = 0;
    type = "UNDEF";
  }
  
Const::Const(int var,/* int lit, */unsigned line) {
    //varID = (var);
    /* litID = (lit); */
    unused = 0;
    gateID = var;
    linenum = (line);
    //_idout.push_back(out);
    type = "CONST";
}