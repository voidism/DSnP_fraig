/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <sstream>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
//static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool less_than (const pair<CirGate*,bool>& struct1, const pair<CirGate*,bool>& struct2)
{
    return (struct1.first->getID() < struct2.first->getID());
  }
  
istream & ReadIntoString (std::istream & istr, std::string & str) 
{ 
    std::istreambuf_iterator<char> it(istr), end; 
    std::copy(it, end, std::inserter(str, str.begin())); 
    return istr; 
} 

bool
CirMgr::readCircuit(const string& fileName)
{
  ifstream file;
  lineNo = 0;
  colNo = 0;
  file.open(fileName.c_str());
  if(!file.is_open()){ cerr << "Cannot open design \"" << fileName << "\"!!" << endl; return false;}
  string Head, M, I, L, O, A;
  //int m, i, l, o, a;
  if (file.peek() != 'a') { return parseError(EXTRA_SPACE); }
  if(!(file >> Head)) {
    cerr << "Error1" << endl;
    return false;
  }
  if(Head!="aag") { return parseError(MISSING_SPACE);}
  if(!(file >> M)) {  return parseError(MISSING_SPACE); }
  if(!myStr2Int(M,m)) {  return parseError(MISSING_SPACE); }
  if(!(file >> I)) {  return parseError(MISSING_SPACE); }
  if(!myStr2Int(I,i)) {  return parseError(MISSING_SPACE); }
  if(!(file >> L)) {  return parseError(MISSING_SPACE); }
  if(!myStr2Int(L,l)) {  return parseError(MISSING_SPACE); }
  if(!(file >> O)) {  return parseError(MISSING_SPACE); }
  if(!myStr2Int(O,o)) {  return parseError(MISSING_SPACE); }
  if(!(file >> A)) {  return parseError(MISSING_SPACE); }
  if(!myStr2Int(A,a)) { return parseError(MISSING_SPACE); }
  //Get Miloa !!!
  if(m<i+l+a){
    errMsg = "Number of varibles";
    errInt = m;
    return parseError(NUM_TOO_SMALL);
  }
  lineNo++;
  vector<string> content;
  string line;
  while(getline(file, line)){
    content.push_back(line);
  }
  file.close();
  //(1)_Glist.push_back(new Const(0, 0));
  _idMap[0] = new Const(0, 0);//_Glist.back();
  //read PI
  if(content.empty()){
    cerr << "Error0" << endl;
    return false;
  }
  for (int it = 0; it < i; it++)
  {
    //cout << "I:";
    //cout << content.at(lineNo) << endl;
    if(lineNo>content.size()-1) { return parseError(EXTRA_SPACE);}
    stringstream ss_line(content.at(lineNo));
    unsigned lit;
    if(!(ss_line >> lit)) {
      errMsg = "Something";
      return parseError(MISSING_NUM);
    }
    if (!(lit % 2 == 0)){
    errMsg = "PI";
    errInt = lit;
    return parseError(CANNOT_INVERTED);
  }
  if (lit == 0){
    errInt = lit;
    return parseError(REDEF_CONST);
    }
    _PIlist.push_back(new PI(lit / 2, lineNo));
    //_idMap.insert(pair<unsigned, CirGate *>(lit / 2, _Glist.back()));
    _idMap[lit / 2] = _PIlist.back();
    lineNo++;
  }
  //read latch
  for (int it = 0; it < l; it++)
  {
    lineNo++;
  }
  unsigned num = m;
  //read PO
  for (int it = 0; it < o; it++)
  {
    //cout << "O:";
    //cout << content.at(lineNo) << endl;
    if(lineNo>content.size()-1) {
      errMsg = "PO definition";
      return parseError(MISSING_NUM);
    }
    stringstream ss_line(content.at(lineNo));
    unsigned lit;
    if(!(ss_line >> lit)) {
      errGate = _idMap[0];
      errInt = lit;
      return parseError(REDEF_GATE);
    }
    _POlist.push_back(new PO(/* lit / 2,  */lit, lineNo, ++num));
    _idMap[num] = _POlist.back();
    lineNo++;
  }
  //read AIG
  for (int it = 0; it < a; it++)
  {
   if(lineNo>content.size()-1) {
      errMsg = "AIG definition";
      return parseError(MISSING_NUM);
    }
    stringstream ss_line(content.at(lineNo));
    unsigned lit,in1,in2;
    if(!(ss_line >> lit)) { return parseError(MISSING_SPACE); }
    if(!(ss_line >> in1)) { return parseError(MISSING_SPACE); }
    if(!(ss_line >> in2)) { return parseError(MISSING_SPACE); }
    //(2)_Glist.push_back(new AIG(lit / 2, /* lit, */in1,in2, lineNo));
    _idMap[lit / 2] = new AIG(lit / 2, in1, in2, lineNo);//_Glist.back();
    lineNo++;
  }

  //for (unsigned i = 0; i < _Glist.size();i++){
  for (std::map<unsigned int, CirGate *>::iterator it=_idMap.begin(); it!=_idMap.end(); ++it){//clean unused gate
    CirGate * x = it->second;
    if(x->_idin.empty()) {}
    else{
      for (unsigned j = 0; j < x->_idin.size();j++){
        std::map<unsigned int, CirGate*>::iterator tmp = _idMap.find(x->_idin.at(j)/2);
        CirGate *cpr = tmp->second;
        if (tmp == _idMap.end())
        {
          if(x->_idin.at(j)==0||x->_idin.at(j)==1){
            cpr = _idMap.at(0);
          }
          else{
          //_Glist.push_back(new Undef((_Glist.at(i)->_idin.at(j) / 2)));
          cpr = _idMap[(x->_idin.at(j) / 2)] = new Undef((x->_idin.at(j) / 2));//_Glist.back();
          }
        }
        if (x->type == "UNDEF") x->type = "AIG";
        x->_fin.push_back(cpr);

        cpr->_out.push_back(make_pair(x,(bool)(x->_idin.at(j) % 2)));

      }
    }
  }
  for (std::map<unsigned int, CirGate *>::iterator it=_idMap.begin(); it!=_idMap.end(); ++it){//clean unused gate
        CirGate * x = it->second;
        if(x==0) continue;
        sort(x->_out.begin(), x->_out.end(), less_than);;
  }



  while(lineNo < content.size())
  { 
    stringstream ss_line(content.at(lineNo));
    char io;
    unsigned id;
    string symbolname;
    if (ss_line.peek() == ' ' || ss_line.peek() == '\t') { return parseError(EXTRA_SPACE); }
    if(!(ss_line >> io)) { return parseError(EXTRA_SPACE);}
    if(io == 'c') {
      if(ss_line.peek()!= EOF){
        return parseError(MISSING_NEWLINE);
      }
     break;}
    if(io!='i' && io!='o') { return parseError(EXTRA_SPACE);}
    if (ss_line.peek() == ' ' || ss_line.peek() == '\t') { return parseError(EXTRA_SPACE); }
    if(!(ss_line >> id)) {
      errMsg = "symbol index(a)";
      return parseError(ILLEGAL_NUM);
    }
    //if(!(ss_line >> symbolname)) { cerr << "Error21" << endl;  return false;}
    ss_line.seekg(1,ios::cur);
    if(ss_line.peek() == EOF){
      errMsg = "symbolic name";
      return parseError(MISSING_IDENTIFIER);
    }
    ReadIntoString(ss_line, symbolname);
    //std::map<unsigned int, CirGate*>::iterator tmp = _idMap.find(id);
    //if (tmp == _idMap.end()) { cerr << "Error22 gate not found!" << endl; return false;}
    // = (io == 'i' ? id + 1 : id + i );
    CirGate *cpr = 0;
    if (io=='i') {
      cpr = _PIlist[id];
    }
    else if (io=='o') {
      cpr = _POlist[id];
    }
    else{
      return false;
    }
    if(cpr->type != "PI" && cpr->type != "PO") 
    { cerr << "Error23 AIG cannot be named!" << endl << cpr->type << " " <<cpr->gateID << " "\
    << "i+o=" << i+o << " " <<"m="<<m<< lineNo \
    << " " << id <<endl;  return false;}
    //cout << "peek!!" << ss_line.peek() << endl;
    if(cpr->symb != "") {
      errMsg = io;
      errInt = id;
      return parseError(REDEF_SYMBOLIC_NAME);
    }
    cpr->symb = symbolname;
    lineNo++;
  }

  (CirGate::_globalRef)++;
  for (auto &x:_POlist)
  {
    DFSlistGen(x);
  }
  //for(auto& x:_Glist){//clean unused gate
  for (std::map<unsigned int, CirGate *>::iterator it=_idMap.begin(); it!=_idMap.end(); ++it){//clean unused gate
        CirGate * x = it->second;
        if(x==0) continue;
        if(x->_ref != CirGate::_globalRef && x->type!="PI" && x->type!="PO" && x->type!="CONST"){
          x->unused = 1;
        }
  }
  
  lineNo = 0;
  return true;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
  cout << "\nCircuit Statistics" << endl;
  cout << "==================" << endl;
  cout << "  " << setw(7) << left << "PI" << setw(7) << right << i << endl;
  cout << "  " << setw(7) << left << "PO" << setw(7) << right << o << endl;
  cout << "  " << setw(7) << left << "AIG" << setw(7) << right << a << endl;
  cout << "------------------" << endl;
  cout << "  Total"<< setw(9) << i+o+a  << endl;
}

void
CirMgr::printNetlist() const
{
/*
   cout << endl;
   for (unsigned i = 0, n = _dfsList.size(); i < n; ++i) {
      cout << "[" << i << "] ";
      _dfsList[i]->printGate();
   }
*/
  unsigned prindex = 0;
  (CirGate::_globalRef)++;
  cout << endl; 
  for (auto &x:_POlist){
    DFSearch(x, prindex);
  }
}

void CirMgr::DFSearch(CirGate *it,unsigned &prindex) const{
  if(it->_ref==CirGate::_globalRef) return;
  if(it->type == "UNDEF") {it->_ref=CirGate::_globalRef;return;}
  if(!(it->_fin.empty())){
   for (int jdx = 0; jdx < (int)it->_fin.size(); jdx++)
   {
    if(it->_fin.at(jdx)->_ref==CirGate::_globalRef) continue;
    DFSearch(it->_fin.at(jdx),prindex);
   }
  }
  cout << "[" << prindex << "] " << setw(4) << left << it->type << it->gateID;
  for (unsigned u = 0; u < it->_fin.size();u++){
        cout << " " << ((it->_fin.at(u)->type=="UNDEF")? "*":"") << ((it->_idin.at(u) % 2)? "!":"")<< it->_fin.at(u)->gateID;
      }
  string sb = ((it->symb!="")? (" (" + it->symb + ")") : "");
  cout << sb << endl;//(symbol name)
  it->_ref=CirGate::_globalRef;
  (prindex)++;
}


void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for (auto &x:_PIlist){
     cout << " " << x->gateID;
   }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for (auto &x:_POlist){
     cout << " " << x->gateID;
   }
   cout << endl;
}
void
CirMgr::printFloatGates() const
{
  vector<unsigned> nu;
  vector<unsigned> wf;
  for (std::map<unsigned int, CirGate *>::const_iterator it=_idMap.begin(); it!=_idMap.end(); ++it){//clean unused gate
    CirGate * x = it->second;
    if(x==0) continue;
    if((x->type==("AIG")||x->type==("PI"))&&x->_out.empty()){
      nu.push_back(x->gateID);
    }
    if(x->type==("AIG")||x->type==("PO")){
      for (unsigned u = 0; u < x->_fin.size();u++){
        if(x->_fin.at(u)->type=="UNDEF"){
          wf.push_back(x->gateID);
          break;
        }
      }
    }
  }
  sort(wf.begin(), wf.end());
  sort(nu.begin(), nu.end());
  
  if (wf.size() > 0)
  {
    cout << "Gates with floating fanin(s):";
    for (vector<unsigned>::iterator it = wf.begin(); it != wf.end(); it++)
    {
      cout << " " << *it;
    }
    cout << endl;
  }
  if(nu.size()>0){
  cout << "Gates defined but not used  :";
  for (vector<unsigned>::iterator it = nu.begin(); it != nu.end();it++){
    cout << " " << *it;
  }
  cout << endl;
  }

}

void
CirMgr::DFSearch_NoPrint(CirGate *it,unsigned &prindex, stringstream& ss) const{
  if(it->_ref==CirGate::_globalRef) return;
  if(it->type == "UNDEF" || it->type == "PI") {it->_ref=CirGate::_globalRef;return;}
  if(!(it->_fin.empty())){
   for (int jdx = 0; jdx < (int)it->_fin.size(); jdx++)
   {
    if(it->_fin.at(jdx)->_ref==CirGate::_globalRef) continue;
    DFSearch_NoPrint(it->_fin.at(jdx),prindex,ss);
   }
  }
  if(it->type=="AIG"){
  ss << it->gateID*2;
  for (unsigned u = 0; u < it->_fin.size();u++){
        ss << " " << (it->_idin.at(u));
      }
  (prindex)++;
  ss << endl;
  }//(symbol name)
  it->_ref=CirGate::_globalRef;
}


void
CirMgr::printFECPairs() const
{
}

void
CirMgr::writeAag(ostream& outfile) const
{
  (CirGate::_globalRef)++;
  unsigned realAIGcount = 0;
  stringstream ss;
  for (auto &x:_POlist){
    DFSearch_NoPrint(x,realAIGcount,ss);
  }
  string s=ss.str();
  //cout <<s;
  outfile << "aag" << " " << m << " " << i << " " << l << " " << o << " " << realAIGcount<< endl;
  for (auto &x:_PIlist){
    outfile << x->gateID*2 << endl;
  }
  for (auto &x:_POlist){
    outfile << x->_idin.at(0) << endl;
  }
  outfile << s;
  for (unsigned idx = 0; idx < _PIlist.size(); idx++)
  {
    if(_PIlist.at(idx)->symb!="")
    outfile << "i" << idx << " " << _PIlist.at(idx)->symb << endl;
  }
  for (unsigned idx = 0; idx < _POlist.size(); idx++)
  {
    if(_POlist.at(idx)->symb!="")
    outfile << "o" << idx << " " << _POlist.at(idx)->symb << endl;
  }
  outfile << "c" << endl;
  outfile << "AAG output by Chung-Yang (Ric) Huang" << endl;
  /* for(vector<string>::const_iterator it = c.begin(); it!= c.end();it++){
    outfile << *it << endl;
  } */
}

void
CirMgr::writeGate(ostream& outfile, CirGate *g) const
{
}

