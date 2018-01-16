/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include <sstream>
#include <cmath>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include <bitset>
#include <unordered_map>
using std::unordered_map;
using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
string
CirMgr::simstring(size_t sim){
  std::stringstream ss("");
  size_t mask = 1;
   for(int i = 0; i < 64; i++) {
   	if(!(i % 8) && i!=0) ss << "_";
   	ss << (bool)(sim & mask);
   	mask <<= 1;
   }
   return ss.str();
}
void
CirMgr::randomSim()
{
  unsigned limit = log(_DFSlist.size()) / log(2.5) + 0.5;
  unsigned fail_times = 0;
  unsigned orgFECsize = INT_MAX;
  unsigned count = 0;
  while(true){
    sim_random();
    count++;
    if(orgFECsize==_FEClist.size()) fail_times++;
    else fail_times = 0;
    //if(count%(100*limit/_PIlist.size()) == 0)
    orgFECsize = _FEClist.size();
    if((fail_times>limit)) break;
  }
  sort_and_pop();
  cout << char(13) << count*64 << " patterns simulated." << endl;
  cout.flush();
  idFecMapGen();
}
bool 
fec_comp(const vector<unsigned>& a, const vector<unsigned>& b){
  return (a[0] < b[0]);
}

void
CirMgr::sort_and_pop()
{
  //sorting in each group and between groups!
    for (int idx = 0; idx < (int)_FEClist.size(); idx++)
    {
      //pop out the one that have only one item!
      if (_FEClist[idx].size() == 1)
      {
        _FEClist[idx] = _FEClist[_FEClist.size() - 1];
        _FEClist.pop_back();
        idx--;
      }
      else
      {
        sort(_FEClist[idx].begin(), _FEClist[idx].end());
      }
    }
    sort(_FEClist.begin(), _FEClist.end(), fec_comp);
}

bool
CirMgr::split_fec_groups(vector<unsigned> &orgroup)
{
  int flag = 0;
  unordered_map<size_t,unsigned> fecMap;
  vector<vector<unsigned>> _tempFEClist;
  for (auto &it : orgroup)
  {
    CirGate *x = _idMap[it];
    if (x == 0) continue;
    if ((x->type != "AIG") && (x->type != "CONST"))
      continue;
    unordered_map<size_t,unsigned>::iterator got = fecMap.find(_simValue[x->gateID]);
    if (got == fecMap.end())
    {
      got = fecMap.find(~(_simValue[x->gateID]));
      if (got == fecMap.end())
      {
        //cout << "new group" << _simValue[x->gateID] << endl;
        flag ++;
        vector<unsigned> temp;
        temp.push_back(x->gateID);
        fecMap[_simValue[x->gateID]] = _tempFEClist.size();
        _tempFEClist.push_back(temp);
      }
      else{
        _tempFEClist[got->second].push_back(x->gateID);
      }
    }
    else {_tempFEClist[got->second].push_back(x->gateID);}
  }
  for (unsigned z = 0; z < _tempFEClist.size();z++)
    {
      if (_tempFEClist[z].size() == 1)
        {
          _tempFEClist[z] = _tempFEClist[_tempFEClist.size() - 1];
          _tempFEClist.pop_back();
          z--;
        }
    }
  if(flag > 1){
    _FEClist.insert(_FEClist.end(), _tempFEClist.begin(), _tempFEClist.end());
    return 1;
  }
  else
    return 0;
}


void
CirMgr::sim_random()
{  //start sim!!!
  (CirGate::_globalRef)++;
  _simValue[0] = 0;
  _idMap[0]->_ref = (CirGate::_globalRef);
  for (auto &x : _PIlist)
  {
    _simValue[x->gateID] = ( ((size_t)rnGen(INT_MAX)) << 32 ) | (size_t)rnGen(INT_MAX);
    x->_ref = (CirGate::_globalRef);
  }
  for (auto &y : _POlist)
  {
    size_t ans = sim(y->_in[0].first);
    _simValue[y->gateID] = ((y->_in[0].second) ? ~ans : ans);
    y->_ref = (CirGate::_globalRef);
  }

  //Let's deal the FEC pairs!
  //If it is the first time to sim, we need to recognize which gates are family.
  if (_FEClist.size() == 0)
  {
    bool conflag = 0;
    unordered_map<size_t,unsigned> fecMap;
    for (auto &x : _DFSlist)
    {
      classify_first_time(x, conflag, fecMap);
    }
    if(conflag == 0)
      classify_first_time(_idMap[0], conflag, fecMap);
    sort_and_pop();
  }
  //if it is not the first time
  //we need to split the existed FEC groups with the data we get by furthur sim.
  else
  {
    //cout << "split the existed groups!" << endl;
    for (int idx = 0; idx < (int)_FEClist.size(); idx++)
    {
      if (_FEClist[idx].size() == 1)
      {
        _FEClist[idx] = _FEClist[_FEClist.size() - 1];
        _FEClist.pop_back();
        idx--;
      }
      else
      {
        if(split_fec_groups(_FEClist[idx])){
        _FEClist[idx] = _FEClist[_FEClist.size() - 1];
        _FEClist.pop_back();
        idx--;
        }
      }
    }
  }
  cout << char(13) << "Total #FEC Group = " << _FEClist.size();
  cout.flush();
}

void
CirMgr::classify_first_time(CirGate* x,bool& flag,unordered_map<size_t,unsigned> &fecMap)
{
  if(x==0) return;
  if(x->gateID == 0) { flag = 1; }
  if ((x->type != "AIG") && (x->type != "CONST"))
    return;
  unordered_map<size_t,unsigned>::iterator got = fecMap.find(_simValue[x->gateID]);
  if (got == fecMap.end())
  {
    unordered_map<size_t,unsigned>::iterator got2 = fecMap.find(~(_simValue[x->gateID]));
    if (got2 != fecMap.end())
    {
      _FEClist[got2->second].push_back(x->gateID);
    }
    else
    {
      vector<unsigned> temp;
      temp.push_back(x->gateID);
      fecMap[_simValue[x->gateID]] = _FEClist.size();
      _FEClist.push_back(temp);
    }
  }
  else
  {
    _FEClist[got->second].push_back(x->gateID);
  }
}

void CirMgr::updateLog(size_t pos)
{
  size_t submask = 1;
  submask <<= 63;
  while (submask > pos)
  {
    for(auto &x:_PIlist){
      *_simLog << (bool)(_simValue[x->gateID] & submask);
    }
    *_simLog << ' ';
    for(auto &x:_POlist){
      *_simLog << (bool)(_simValue[x->gateID] & submask);
    }
    *_simLog << '\n';
    submask >>= 1;
  }
}

void
CirMgr::idFecMapGen()
{
  for (unsigned idx = 0; idx < _FEClist.size();idx++){
    for(auto &x:_FEClist[idx]){
      _idMap[x]->fecAddr = idx;
    }
  }
}

void
CirMgr::sim_pattern(vector<size_t> pat)
{
  //start sim!!!
  (CirGate::_globalRef)++;
  _simValue[0] = 0;
  _idMap[0]->_ref = (CirGate::_globalRef);
  int pidx = 0;
  for (auto &x : _PIlist)
  {
    _simValue[x->gateID] = pat[pidx];
    pidx++;
    x->_ref = (CirGate::_globalRef);
  }
  for (auto &y : _POlist)
  {
    size_t ans = sim(y->_in[0].first);
    _simValue[y->gateID] = ((y->_in[0].second) ? ~ans : ans);
    y->_ref = (CirGate::_globalRef);
  }

  //Let's deal the FEC pairs!
  //If it is the first time to sim, we need to recognize which gates are family.
  if (_FEClist.size() == 0)
  {
    bool conflag = 0;
    unordered_map<size_t,unsigned> fecMap;
    for (auto &x : _DFSlist)
    {
      classify_first_time(x, conflag, fecMap);
    }
    if(conflag == 0)
      classify_first_time(_idMap[0], conflag, fecMap);
    sort_and_pop();
  }
  //if it is not the first time
  //we need to split the existed FEC groups with the data we get by furthur sim.
  else
  {
    //cout << "split the existed groups!" << endl;
    for (int idx = 0; idx < (int)_FEClist.size(); idx++)
    {
      if (_FEClist[idx].size() == 1)
      {
        _FEClist[idx] = _FEClist[_FEClist.size() - 1];
        _FEClist.pop_back();
        idx--;
      }
      else
      {
        if(split_fec_groups(_FEClist[idx])){
        _FEClist[idx] = _FEClist[_FEClist.size() - 1];
        _FEClist.pop_back();
        idx--;
        }
      }
    }
  }
}

void
CirMgr::fileSim(ifstream &patternFile)
{
    //deal the pattern file:
    vector<size_t> patterns(i, 0);
    string line;
    int count64 = 0;
    while (getline(patternFile, line))
    {
      while (line[0] == ' ')
        line = line.substr(1);
      if(line == "" || line == "\r" || line == "\n" || line == "\r\n") continue;
      if (line.size() != (unsigned)i)
      {
        cerr << "Error: Pattern(" << line << ") length(" << line.size()
             << ") does not match the number of inputs(" << i << ") in a circuit!!" << endl;
        cout << char(13) << 0 << " patterns simulated." << endl;
        cout.flush();
        return;
      }
      for (int idx = 0; idx < i; idx++)
      {
        if(line[idx] != '0' && line[idx] != '1')
			  {
				  cerr << "Error: Pattern(" << line << ") contains a non-0/1 characters(\'" << line[idx] << "\')." << endl;
				  cout << char(13) << 0 << " patterns simulated." << endl;
          cout.flush();
          return;
        }
        patterns[idx] = (patterns[idx] << 1) | (size_t)(line[idx] != '0');
      }
      count64++;
      if ((count64 % 64) == 0)
      {
        sim_pattern(patterns);
        if(_simLog!=0) updateLog(0);
        cout << char(13) << "Total #FEC Group = " << _FEClist.size();
        cout.flush();
      }
    }
    if (count64 % 64 != 0)
    {
      for (int idx = 0; idx < i; idx++){
        patterns[idx] = (patterns[idx] << (64 - count64));
      }
      sim_pattern(patterns);
      size_t pos = 1;
      pos <<= (63 - count64);
      if(_simLog!=0) updateLog(pos);
      cout << char(13) << "Total #FEC Group = " << _FEClist.size();
      cout.flush();
    }
    //sort(_FEClist.begin(), _FEClist.end(), fec_comp);
    sort_and_pop();
    cout << char(13) << count64 << " patterns simulated." << endl;
    cout.flush();
    idFecMapGen();
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
size_t
CirMgr::sim(CirGate* it)
{
  if(it->_ref == (CirGate::_globalRef)){
    return _simValue[it->gateID];
  }
  if(it->_in.size()<2){
    cerr << "floating gate!" << endl;
    return 0;
  }
  assert(it->_in.size() == 2);
  size_t ans = (((it->_in[0].second) ? (~(sim(it->_in[0].first))) : (sim(it->_in[0].first))) & ((it->_in[1].second) ? (~(sim(it->_in[1].first))) : (sim(it->_in[1].first))));
  _simValue[it->gateID] = ans;
  it->_ref = (CirGate::_globalRef);
  return ans;
}