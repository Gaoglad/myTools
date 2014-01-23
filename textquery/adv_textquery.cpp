/*
Program: Text query to find 'word' line and context
History: 2014/01/16 Julie
Test: 2014/01/16 ok
*/
#include <string>
#include <map>
#include <set>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept> //out of range
#include <cstdlib> //EXIT_FAILURE
#include "adv_textquery.h"

int main(int argc,char* argv[]){
 ifstream infile;
 if(argc<2||!open_file(infile,argv[1])){
  cerr<<"No input file"<<endl;
  return EXIT_FAILURE;
 }
 TextQuery tq;
 tq.read_file(infile);
 while(1){
  cout<<"Enter a word to look for, or 'q' to quit"<<endl;
  string line,word,preword;
  getline(cin,line);
  istringstream stream(line);
  Query q("");
  set<line_no> locs;
  while(stream >> word)
  {
   
   if (word=="q") return 0;   
   if (word=="|") {stream >> word; q=q | Query(word);continue;}
   if (word=="&") {stream >> word; q=q & Query(word);continue;}
   if (word=="~") {stream >> word; q=(~Query(word));continue;}
   q=Query(word); 
  }
  cout<<"Excuted Query for: ";
  locs=q.eval(tq);
  print_results(locs,q,tq);
 }
 return 0;
}
