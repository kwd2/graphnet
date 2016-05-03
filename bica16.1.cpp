#include <TError.h>
#include "TCanvas.h"
#include "TStyle.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TMath.h"
#include "TGraph.h"


#include <iomanip>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cmath>
#include <ctype.h>

#include <list>
#include <vector>
#include <map>
using namespace std;



//------------------------------------- globals
int                  current_time    = 0;

//------------------------------------- globals




class Node{
public:

  string                           symbol  ;
  
 
  map<Node*,  float>               branches;

  map<string, Node*>               branches_string;


  //void   fire(){};
 
  Node*  add_branch( string );
  void   add_branch( Node*  );


  Node*  add_seq_markov( list<string>::iterator word, int depth );

  Node*  input         ( string sentence, int depth )            ;


  void   output        ( int  depth,  int tabs )                 ;

  void   clear_weights ()                                        ;


  Node(){};
  Node(string name){symbol=name;}

}; 


//----------------------------------------
//
Node*
Node::input( string sentence, int depth ){


  list<string> sequence;

  int            s1=0;
  unsigned int   s2=0;
  string         field;

  while (s2!=string::npos) {
    s2 = sentence.find(" ", s1); field = sentence.substr(s1,s2-s1); s1+=s2-s1+1;

    if (field=="") continue;

    sequence.push_back(field);
  }

  sequence.push_back("-stop");



  return add_seq_markov(sequence.begin(), depth);
}



//----------------------------------------
//

Node*
Node::add_seq_markov( list<string>::iterator word, int depth ){


  if (*word=="-stop") return this;

  map< string, Node*>::iterator bs;


  bs = branches_string.find("-flag2");// moved here,  flag2 = swap symbol
  if (bs!= branches_string.end()){  

    if ( bs->second->branches_string.find(*word) != 
	 bs->second->branches_string.end() ) {

      *word = bs->second->symbol;
 
    }
  }



  //------------------------------------------

  Node* b = add_branch(*word);

  *word = b->symbol;              //may change

  if (depth==0) return b;


  b = b->add_seq_markov( (++word)--, (--depth)++ );


  //------------------------------------------


  bs = branches_string.find("-flag3");     // flag3 = filter insert
  if (bs!= branches_string.end()){  

    if ( bs->second->branches_string.find(*word) != 
	 bs->second->branches_string.end() ) {

      b = add_branch(bs->second->symbol);

      return  b->add_seq_markov( word, depth );
     }
  }


  return b;
}


//----------------------------------------
//
void
Node::add_branch( Node* node){

  map< Node*, float>::iterator b;

  b = branches.find(node);

  if (b==branches.end())
    {

      branches       .insert(pair<Node*, float> (node        , 1    ));
      branches_string.insert(pair<string,Node*> (node->symbol, node ));
    }

}


//----------------------------------------
//
Node*
Node::add_branch( string symbol1){

  map< string, Node*>::iterator bs;
  
  bs = branches_string.find("-flag1");     // flag1 = lower case
  if (bs!= branches_string.end()){  
    char c    = symbol1[0];
    symbol1[0] = tolower(c);
  }
 


  bs = branches_string.find(symbol1);

  if (bs==branches_string.end())
    {
      Node* node = new Node(symbol1);

      branches       .insert(pair<Node*, float> (node  , 1    ));
      branches_string.insert(pair<string,Node*> (symbol1, node ));

      bs = branches_string.find(symbol1);
    }

  else{
    map<Node*,float>::iterator b = branches.find(bs->second);

    b->second++;  

    // b->first->fire();

  }


  return bs->second;

}







//----------------------------------------
//
void 
Node::clear_weights(){

  map< Node*, float >::iterator b=branches.begin();

  for (;b!=branches.end();b++){

    b->second = 0;

    b->first->clear_weights();
  }

}











//----------------------------------------
//
void 
Node::output(int depth, int tabs){


  cout<<symbol<<endl;;

  if (depth-1>0){

    map     <Node*,string>              node_str;

    map<string, Node*>::iterator bs = branches_string.begin();
    for (;bs!=branches_string.end();bs++)
      { node_str.insert(pair<Node*,string>(bs->second,bs->first)); }


    multimap<float,Node*>               node_sort;

    map<Node*, float>::iterator b = branches.begin();
    for (;b!=branches.end();b++)
    { node_sort .insert(pair<float,Node*>(b->second,b->first)); }

    multimap<float,Node*>::reverse_iterator s = node_sort.rbegin();
    for (;s!=node_sort.rend();s++){

      for (int i=0; i!= tabs+1; i++) cout<<"\t";
      for (int i=6;i>0;i--) if( (s->first)<pow(10,i)) cout<<" ";
      //cout<<(int) b->first<<".";

      ostringstream pf;
      pf<<std::setprecision(6)<<s->first;

      cout<< std::setprecision(6)<<s->first;

      //for (int i=0; i<5-(pf.str()).size(); i++) cout<<" ";

      cout<<". ";

      map<Node*,string>::iterator ns=node_str.find(s->second);
      //cout<< ns->second<<"..";
 
      (s->second)->output(depth-1, tabs+1);
    }
  }

}









//-------------------------------------
//-------------------------------------



int main()
{





  //--------------------------------- ROOT histogram stuff
  //gErrorIgnoreLevel = kFatal;
  //TFile* hfile = new TFile("process_data.root","RECREATE","histogram file");
  //   TH1F* h = new TH1F("","", , , );
  //TCanvas* canvas1 = new TCanvas("canvas1","sub data",200,10,700,500);
  //gStyle->SetOptStat(110000);    //ksiourmen  // supress leftmost 0s
  //----------------------------------- ROOT histogram stuff

 


  // ------------------------------------------- read in corpus
  list<string>          corpus;
  string                line;

  ifstream datafile;  datafile.open("data/corpus.txt");  if(!datafile.is_open()){cout<<"Error 1"<<endl;return 0;}

  while (getline(datafile, line)){
    if(datafile.eof()) break;
    corpus.push_back(line);
  }
  //cout<<corpus.size()<<endl;
  // ------------------------------------------- read in corpus



  // ------------------------------------------- tokenize

  list<list<string> >  corpus1;
       list<string>    sentence;

  list<string>::iterator c=corpus.begin();
  for (;c!=corpus.end();c++){

    int            s1=0;
    unsigned int   s2=0;
    string         field;

    while (s2!=string::npos) {
      s2 = c->find(" ", s1); field = c->substr(s1,s2-s1); s1+=s2-s1+1;

      if (field=="") continue;

      sentence.push_back(field);
  
      //cout<<field<<endl;
      //break;
    }
    if (sentence.size()>0) corpus1.push_back(sentence);
    sentence.clear();                      // important 
   }

  cout<<corpus1.size()<<endl;

  // ------------------------------------------- tokenize



  // --------------------------------------  add symbol nodes to IO parcel
  Node IO("IO");
  
  list< list<string> >::iterator c1;
        list<string>  ::iterator  w;

  for (c1=corpus1.begin();c1!=corpus1.end();c1++){          // loop through corpus
    w=c1->begin();
 
    IO.add_branch(*w);                                   

}
  
  // --------------------------------------  add symbol nodes to IO parcel



  //IO.output(2,0);cout<<"---"<<endl;


  /*
 
  IO
	   6495.the
	   5914.a
	   2950.he	p
	   2818.The
	   1475.an
	    861.She	p
	    835.she	p
	    827.they	p
	    805.I	p
	    772.He	p
	    761.his	p
	    450.it	p
	    393.We	p
	    384.This	p
	    328.in
	    311.we	p
	    256.her	p
	    250.there	p


  */


 

 
  IO.add_branch("-flag1");   //fix caps

  IO.clear_weights();


  for (c1=corpus1.begin();c1!=corpus1.end();c1++){          // loop through corpus

    w=c1->begin();
 
    IO.add_branch(*w);                                   
  }




  //IO.output(2,0);cout<<"---"<<endl;

  /*

IO
	   9313.the
	   5990.a
	   3722.he
	   1696.she
	   1487.an
	   1023.they
	    899.his
	    806.i
   */


 







  Node* f2 = IO.input("-flag3",1);

  f2->symbol="(pronoun)";



  IO.input("-flag3 he    ",2);
  IO.input("-flag3 she"   ,2);
  IO.input("-flag3 they"  ,2);
  IO.input("-flag3 i"     ,2);	
  IO.input("-flag3 we"    ,2);	
  IO.input("-flag3 it"    ,2);	
  IO.input("-flag3 this"  ,2);
  IO.input("-flag3 that"  ,2);
  IO.input("-flag3 there" ,2);
  IO.input("-flag3 these" ,2);
  IO.input("-flag3 you"   ,2);





  //--- re-read corpus
  //
  IO.clear_weights();
  for (c=corpus.begin();c!=corpus.end();c++) IO.input(*c,1);




                 cout<<"ken2"<<endl;
                 cout<<"---"<<endl;
  IO.output(2,0);cout<<"---"<<endl;
  IO.output(3,0);cout<<"---"<<endl;
  IO.output(4,0);cout<<"---"<<endl;
  IO.output(5,0);cout<<"---"<<endl;
  IO.output(6,0);cout<<"---"<<endl;





  /*
ken2
---
IO
	   9755. (pro_3)
		   3722. he
		   1696. she
		   1023. they
		    806. i
		    704. we
		    625. this
		    498. it
		    263. there
		    184. you
		    147. these
		     87. that
	   9313. the
	   5990. a
	   3722. he
	   1696. she
	   1487. an
	   1023. they
	    899. his
	    806. i
	    704. we
	    625. this
	    498. it
	    374. her
	    361. in
	    263. there
	    189. my

   */










}  //fin







 /*

 (noun) --> (pro)     --> (person) -------------------- (1st) 
                          (thing)---                    (2nd)
                                    |                   (plural)
                                    |
                                    --- he/she - reg 
                                        it       irreg

                         p
                        ----
            he           i 
            she          you
            they         s/he
            i            we
            we           they
            this
            it
            there        t 
            you         ----
            these        it
            that         those
                         this
                         these
                         there
                         that


   */
