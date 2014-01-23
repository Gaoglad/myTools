/*
Program: Text query to find 'word' line and context
History: 2014/01/15 Julie
Test:
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
#include "textquery.h"

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
  string s;
  cin >> s;
  if(!cin || s=="q") break;
  set<TextQuery::line_no> locs=tq.run_query(s);
  print_results(locs,s,tq);
 }
}
