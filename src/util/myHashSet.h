/****************************************************************************
  FileName     [ myHashSet.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashSet ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_SET_H
#define MY_HASH_SET_H

#include <vector>

using namespace std;

//---------------------
// Define HashSet class
//---------------------
// To use HashSet ADT,
// the class "Data" should at least overload the "()" and "==" operators.
//
// "operator ()" is to generate the hash key (size_t)
// that will be % by _numBuckets to get the bucket number.
// ==> See "bucketNum()"
//
// "operator ==" is to check whether there has already been
// an equivalent "Data" object in the HashSet.
// Note that HashSet does not allow equivalent nodes to be inserted
//
template <class Data>
class HashSet
{
public:
   HashSet(size_t b = 0) : _numBuckets(0), _buckets(0) { if (b != 0) init(b); }
   ~HashSet() { reset(); }

   // TODO: implement the HashSet<Data>::iterator
   // o An iterator should be able to go through all the valid Data
   //   in the Hash
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   class iterator
   {
      friend class HashSet<Data>;

    public:
      iterator(vector<Data>* buk,size_t i,size_t j,size_t _numBuckets):_buckets(buk),_numBuckets(_numBuckets),idx(i),jdx(j) {
        if(_buckets[idx].empty()&&idx!=_numBuckets){(*this)++;}
      }
      const Data& operator * () const { return _buckets[idx][jdx]; }
      iterator& operator ++ () { 
        if(idx == _numBuckets){return *this;}
        if(jdx + 1<_buckets[idx].size()){jdx++;return *this;}
        while(true){
          idx++;
          if(_buckets[idx].size()!=0){break;}
          if(idx == _numBuckets) break;
        }
        jdx=0;
        return (*this);
      }
      iterator operator ++ (int) {
        iterator tmp = *this;
        ++(*this);
        return tmp;
      }
      iterator& operator -- () { 
        if(idx == 0 && jdx == 0){return *this;}
        if(jdx>0){jdx--;return *this;}
        while(true){
          idx--;
          if(_buckets[idx].size()!=0){jdx=_buckets[idx].size()-1; break;}
          if(idx == 0) {jdx = 0; break;}
        }
        return (*this);
      }
      iterator operator -- (int) {
        iterator tmp = *this;
        --(*this);
        return tmp;
      }
      bool operator != (const iterator& i) const { return idx!=i.idx||jdx!=i.jdx; }
      bool operator == (const iterator& i) const { return idx==i.idx&&jdx==i.jdx; }
      iterator operator = (iterator it){
        _buckets = (*it)._buckets;
        _numBuckets = (*it).number;
        idx = (*it).idx;
        jdx = (*it).jdx;
      }

    private:
      vector<Data>*     _buckets;
      size_t            _numBuckets;
      size_t            idx;
      size_t            jdx;
   };

   void init(size_t b) { _numBuckets = b; _buckets = new vector<Data>[b]; }
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
   }
   void clear() {
      for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear();
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<Data>& operator [] (size_t i) { return _buckets[i]; }
   const vector<Data>& operator [](size_t i) const { return _buckets[i]; }

   // TODO: implement these functions
   //
   // Point to the first valid data
   iterator begin() const { return iterator(_buckets,0,0,_numBuckets); }
   // Pass the end
   iterator end() const { return iterator(_buckets,_numBuckets,0,_numBuckets); }
   // return true if no valid data
   bool empty() const {
     iterator test = begin();
     if(++test == end()) return true;
     else return false;
    }
   // number of valid data
   size_t size() const { 
     size_t s = 0;
     for (size_t i = 0; i < _buckets.size();i++){
       s += _buckets[i].size();
     }
     return s; 
    }

   // check if d is in the hash...
   // if yes, return true;
   // else return false;
   bool check(const Data& d) const {
     size_t keyNum = bucketNum(d);
     for (size_t i = 0; i < _buckets[keyNum].size();i++){
       if(  (*d)()   ==     (*(_buckets[keyNum].at(i)))()    ) return true;
     }
     return false;
   }

   // query if d is in the hash...
   // if yes, replace d with the data in the hash and return true;
   // else return false;
   bool query(Data& d) const { 
     size_t keyNum = bucketNum(d);
     for (size_t i = 0; i < _buckets[keyNum].size();i++){
       if(  (*d)()   ==     (*(_buckets[keyNum].at(i)))()    ){
         d = _buckets[keyNum].at(i);
         return true;
        }
     }
     return false; 
   }

   // update the entry in hash that is equal to d (i.e. == return true)
   // if found, update that entry with d and return true;
   // else insert d into hash as a new entry and return false;
   bool update(const Data& d) { 
     size_t keyNum = bucketNum(d);
     for (size_t i = 0; i < _buckets[keyNum].size();i++){
       if(  (*d)()   ==     (*(_buckets[keyNum].at(i)))()    ){
         _buckets[keyNum].at(i) = d;
         return true;
        }
     }
     return false; 
    }

   // return true if inserted successfully (i.e. d is not in the hash)
   // return false is d is already in the hash ==> will not insert
   bool insert(const Data& d) { 
     size_t keyNum = bucketNum(d);
     for (size_t i = 0; i < _buckets[keyNum].size();i++){
       if(  (*d)()   ==     (*(_buckets[keyNum].at(i)))()    ) return false;
     }
     _buckets[keyNum].push_back(d);
     return true;
    }

   // return true if removed successfully (i.e. d is in the hash)
   // return false otherwise (i.e. nothing is removed)
   bool remove(const Data& d) { 
     size_t keyNum = bucketNum(d);
     for (size_t i = 0; i < _buckets[keyNum].size();i++){
       if(  (*d)()   ==     (*(_buckets[keyNum].at(i)))()    ){
         _buckets[keyNum].at(i) = _buckets[keyNum].back();
         _buckets[keyNum].pop_back();
         //_buckets[keyNum].erase(_buckets[keyNum].begin()+i);
         return true;
       }
     }
     return false;
    }

private:
   // Do not add any extra data member
   size_t            _numBuckets;
   vector<Data>*     _buckets;

   size_t bucketNum(const Data& d) const {
      return (((*d)()) % _numBuckets); }
};

#endif // MY_HASH_SET_H
