#include <string>
#include <map>
#include <set>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept> //out of rangeusing namespace std;
#include <algorithm>
using namespace std;


class Query;
class TextQuery;
class Query_base;
class OrQuery;
class AndQuery;
class NotQuery;
typedef vector<string>::size_type line_no;

class TextQuery{
 public:
  void read_file(std::ifstream &is){store_file(is);build_map();}
  set<line_no> run_query(const string&) const;
  string text_line(line_no) const;
  line_no size() const { return lines_of_text.size(); }
 private:
  void store_file(std::ifstream&);
  void build_map();
  vector<std::string> lines_of_text;
  map<std::string,set<line_no> > word_map;
};

ifstream& open_file(ifstream &in,const string &file){
 in.close();
 in.clear();
 in.open(file.c_str());
 return in;
}

string make_plural(size_t ctr,const string &word,const string &ending){
 return (ctr==1)?word:word+ending; 
}

void TextQuery::store_file(ifstream &is){
 string textline;
 while(getline(is,textline))
  lines_of_text.push_back(textline);  //input to vector
}

void TextQuery::build_map(){
 for(line_no line_num=0;line_num!=lines_of_text.size();++line_num){
  istringstream line(lines_of_text[line_num]);  //copy from vector(line)
  string word;                                  
  while(line >> word)                           //handle word
   word_map[word].insert(line_num);
 }
}

set<line_no> TextQuery::run_query(const string &query_word) const
{
 map<string,set<line_no> >::const_iterator loc=word_map.find(query_word);
 if (loc==word_map.end())
  return set<line_no>(); //no find
 else return loc->second;
}

string TextQuery::text_line(line_no line) const{
 if(line < lines_of_text.size())
  return lines_of_text[line];
 throw out_of_range("line number out of range");
}

class Query_base{  //abstract base class
 friend class Query;
protected:
 virtual ~Query_base(){}
private:
 virtual std::set<line_no> eval(const TextQuery&) const = 0;        //return line_no
 virtual std::ostream& display(std::ostream& = std::cout) const = 0;  //print result
};


class Query{  //handle class implement: Query q=Query("apple") & Query("fly") | Query("Wind")
 friend Query operator~(const Query &);
 friend Query operator|(const Query &,const Query &);
 friend Query operator&(const Query &,const Query &);
public:
 Query(const string &);  //**
 ~Query(){decr_use();}
 Query(const Query &c):q(c.q),use(c.use){++*use;}
 Query& operator=(const Query&); 
// ostream& operator <<(ostream&, const Query&); 

 set<line_no> eval(const TextQuery &t) const {return q->eval(t);};
 ostream& display(ostream &os) const {return q->display(os);}     
private:
 Query(Query_base *query):q(query),use(new size_t(1)){}
 Query_base *q;
 std::size_t *use;
 void decr_use(){if(--*use==0) {delete q;delete use;}}
};

Query& Query::operator=(const Query &rhs){   
 ++*rhs.use; //prevent assign-self
 decr_use(); //prevent assign-self

 q=rhs.q;
 use=rhs.use;
 return *this;
}

inline ostream& 
operator <<(ostream &os,const Query &q){
 return q.display(os);
}

class WordQuery : public Query_base{
 friend class Query;   //since Query's constructor using it
public:
 WordQuery(const std::string &s):query_word(s){};
 set<line_no> eval(const TextQuery &t) const {return t.run_query(query_word);}
 ostream& display(ostream &os) const {return os<<query_word;}

 string query_word;
};

class NotQuery : public Query_base{
public:
 friend Query operator~(const Query &); 
 NotQuery(Query q):query(q){};

 set<line_no> eval(const TextQuery &) const;
 ostream& display(ostream &os) const {return os<<"~("<<query<<")";}

 const Query query;
};


set<line_no> NotQuery::eval(const TextQuery &t) const{
 
 set<line_no> has_val=query.eval(t);
 set<line_no> ret_lines;
 for(line_no n=0;n!=t.size();n++)
  if(has_val.find(n)==has_val.end())
    ret_lines.insert(n);

 return ret_lines;
}

class BinaryQuery : public Query_base{
protected:
 BinaryQuery(Query left,Query right,string op):lhs(left),rhs(right),oper(op){}
 
 ostream& display(ostream &os) const {return os<<"("<<lhs<<" "<<oper<<" "<<rhs<<")";}
 
 const Query lhs,rhs;
 const string oper;
};

class OrQuery : public BinaryQuery{
 friend Query operator|(const Query&,const Query&);
 OrQuery(const Query left,const Query right):BinaryQuery(left,right,"|"){};

 set<line_no> eval(const TextQuery &) const; 
};

set<line_no> OrQuery::eval(const TextQuery &t) const{

 set<line_no> right=rhs.eval(t);
 set<line_no> ret_lines=lhs.eval(t);
 ret_lines.insert(right.begin(),right.end());
 return ret_lines;
}
class AndQuery : public BinaryQuery{
 friend Query operator&(const Query&,const Query&);
 AndQuery(const Query left,const Query right):BinaryQuery(left,right,"&"){};

 set<line_no> eval(const TextQuery &) const;
};

set<line_no> AndQuery::eval(const TextQuery &t) const{
 
 set<line_no> right=rhs.eval(t);
 set<line_no> left=lhs.eval(t);
 set<line_no> ret_lines;

 set_intersection(left.begin(),left.end(),right.begin(),right.end(),inserter(ret_lines,ret_lines.begin()));  //stl's algo.
 return ret_lines;
}

Query::Query(const string &s){
 q=new WordQuery(s);use=new size_t(1);
}

inline Query
operator~(const Query &oper){

 return new NotQuery(oper);
}
inline Query 
operator|(const Query &lhs,const Query &rhs){
 return new OrQuery(lhs,rhs);
}
inline Query 
operator&(const Query &lhs,const Query &rhs){
 return new AndQuery(lhs,rhs);
}

void print_results(const set<line_no> &locs,const Query &q,const TextQuery &file){
 typedef set<line_no> line_nums;
 line_nums::size_type size=locs.size();
 cout<<q<<" occurs "<<size<<" times "<<endl;
 line_nums::const_iterator it=locs.begin();
 for(;it!=locs.end();++it){
  cout<<"\t(line "<<(*it)+1<<") "<<file.text_line(*it)<<endl;
 }
}
