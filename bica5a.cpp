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

#include <list>
#include <vector>
#include <map>
using namespace std;



//------------------------------------- globals
int                  current_time    = 0;



//------------------------------------- globals






class Node{
public:

  float                           input   ;
  float                           thresh  ;
  string                          symbol  ;
  
 


  map<Node*,  float>               branches;

  map<string, Node*>               branches_string;

  Node*                            code;

  
  void  add_branch   (Node*,  float weight );

  void  add_sequence_markov (list<Node*>::iterator ,
			     int   depth              );// full Markov like graph

		      

  //void  fire         (  int    flag               );
  //void  touch        (float  weight               );

  void  output       (  int  depth,  int tabs     );


  Node(){};
  Node(string name){symbol=name;input=0;thresh=0;}

}; 










//----------------------------------------
//
void 
Node::add_sequence_markov( list<Node*>::iterator node, int depth){

  if ((*node)->symbol=="-stop") return;


  // search by symbol
  
  map< string, Node*>::iterator bs=branches_string.find((*node)->symbol);

  if (bs==branches_string.end())
    {
      Node*       new_node = new Node((*node)->symbol);
 
      add_branch( new_node, 0);
      
      bs=branches_string.find(        (*node)->symbol);
    }

  map<Node*,float>::iterator b=branches.find(bs->second);

  if (b==branches.end()){cout<<"node add_sequence_markov error"<<endl;return;}

  b->second++;


  if (depth<1) return;
  
  bs->second->add_sequence_markov(++node, depth-1);
}









//----------------------------------------
//
void 
Node::output(int depth, int tabs){


  cout<<symbol<<endl;;

  if (depth-1>0){

    multimap<float,Node*> node_sort;

    map<Node*, float>::iterator b = branches.begin();
    for (;b!=branches.end();b++)
    { node_sort.insert(pair<float,Node*>(b->second,b->first)); }

    multimap<float,Node*>::reverse_iterator s = node_sort.rbegin();
    for (;s!=node_sort.rend();s++){

      for (int i=0; i!= tabs+1; i++) cout<<"\t";
      for (int i=6;i>0;i--) if( (s->first)<pow(10,i)) cout<<" ";
      //cout<<(int) b->first<<".";

      ostringstream pf;
      pf<<std::setprecision(6)<<s->first;

      cout<< std::setprecision(6)<<s->first;

      //for (int i=0; i<5-(pf.str()).size(); i++) cout<<" ";
      cout<<".";
 
      (s->second)->output(depth-1, tabs+1);
    }
  }

}
















//----------------------------------------
//
void 
Node::add_branch( Node* node, float weight){


  map< Node*, float>::iterator b=branches.find(node);

  if (b==branches.end())
    {
      branches.       insert(pair<Node*, float> (node,         weight));
      branches_string.insert(pair<string,Node*> (node->symbol, node  ));
    }

  else
    { b->second++;}
  
}











void
string_to_nodes(string code, list<Node*>* sentence){

  // take a string of symbols and convert
  // to a list of corresponding nodes


  extern map<string, Node*>   map_symbols;     // global map



}



map<string, Node*>   map_symbols;



//-------------------------------------
//-------------------------------------



int main()
{
  

 

  //--------------------------------- ROOT histogram stuff
  gErrorIgnoreLevel = kFatal;
  //TFile* hfile = new TFile("process_data.root","RECREATE","histogram file");
  //   TH1F* h = new TH1F("","", , , );
  //TCanvas* canvas1 = new TCanvas("canvas1","sub data",200,10,700,500);
  gStyle->SetOptStat(110000);    //ksiourmen  // supress leftmost 0s
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





  // --------------------------------------  build symbols in main()
  //map<string, Node*>  map_symbols;

        list<Node*>   sentence;
  list< list<Node*> > corpus1;
  
  int count=0;
  list<string>::iterator c=corpus.begin();
  for (;c!=corpus.end();c++, count++){

    sentence.clear();
    //cout<<*c<<endl;
    
    int     s1=0;
    unsigned int     s2=0;
    string  field;

    while (s2!=string::npos) {
      s2 = c->find(" ", s1); field = c->substr(s1,s2-s1); s1+=s2-s1+1;

      if (field=="") continue;
      //cout<<field<<endl;

      map<string, Node*>::iterator ms = map_symbols.find(field);
      if (ms            == map_symbols.end()){
	Node* new_node  = new Node(field);
	map_symbols.    insert(pair<string,Node*>(field,new_node));
	ms =             map_symbols.find(field);
      }
      sentence.push_back(ms->second);
    }
    corpus1.push_back(sentence);
    //if (count1>3) break;
    
  }
  // --------------------------------------  build symbols in main()



  // --------------------------------------  make pure Markov graph

  Node  markov("Markov");

  Node* end_node = new Node("-stop");

  count=0;
  list< list<Node*> >::iterator s;                           //sentence
  for (s=corpus1.begin();s!=corpus1.end();s++,count++){      // loop through corpus

    s->push_back(end_node);
    
    markov.add_sequence_markov( s->begin(), 2); 

    //if (count>3) break;
  }

  // --------------------------------------  make pure Markov graph
  


  markov.output(1,0)  ;cout<<"---------------------"<<endl;
  markov.output(2,0)  ;cout<<"---------------------"<<endl;
  markov.output(3,0)  ;cout<<"---------------------"<<endl;


  /*

Markov
	   6495.the
	   5914.a
	   2950.he
	   2818.The
	   1475.an
	    861.She
	    835.she
	    827.they
	    805.I
	    772.He
	    761.his
            450.it
	    393.We
	    384.This
	    328.in
	    311.we
	    256.her
	    250.there
	    241.this
	    196.They
	    138.His
	    118.Her
	    104.you
	     98.My
	     98.their
	     95.These
	     91.my
	     85.cut
	     84.was
	     80.You
	     79.our
	     79.that
	     78.some
	     76.A
	     74.when
	     73.Can
	     72.had
	     71.what

   */


  // -------------------------------use code flag to make (a:the)

  //list<Node*> sentence;


  string code;

  code="the -fork (a:the)"; string_to_nodes(code, &sentence);

  //markov.add_sequence_markov(sentence.begin(),10);




  // -------------------------------use code flag to make (a:the)


  // all functionality below built with single conntections
  // between nodes.   All symbols have one node.
  // 
  // 


  
  // --------------------------------------  add symbols to IO
  Node IO("I/O");
  
  list< list<Node*> >::iterator c1;
  list<Node*>::iterator w;

  for (c1=corpus1.begin();c1!=corpus1.end();c1++){      // loop through corpus
    for (w=c1->begin();w!=c1->end();w++){                 

      IO.add_branch(*w,1);      
    }
  }
  
  // --------------------------------------  add symbols to IO


  //IO.output(2,0);

  /*
  I/O
         144241.is      -added 120K "is" symbols to corpus
          76862.a
          75044.the
          70819.of
          42996.to
          35374.or
          31339.in
          26971.and
          14529.an
          13799.that
          13296.with
          11308.by
          10999.for
           7388.on
           6766.from
           6671.as
           5969.who
           5803.having
           4783.used
           4346.was
           3947.he
           3841.his

[kwd1@dms-lab bica1]$ wc -l scr.txt
120821 scr.txt
  */


  // -------------------------------------------- build (a:the) conjunctive node         

  Node* a_the = new Node("(a:the)");
  
  for (c1=corpus1.begin();c1!=corpus1.end();c1++){   // loop over corpus    sentences
    for (w=c1->begin();w!=c1->end();w++){            // loop over words  in sentences

      if ((*w)->symbol == "a" || (*w)->symbol=="the" || (*w)->symbol=="an"){

	list<Node*>::iterator w1=w;	
	if (++w1!=c1->end()){

	  a_the->add_branch(*w1,1);
	}

      }

    }
  }
  // -------------------------------------------- build (a:the) conjunctive node         

  
  //a_the->output(2,0);

  
  /*
(a:the)
           1411.act
           1409.person
           1342.genus
            902.small
            779.family
            755.state
            705.United
            662.large
            631.quality
            601.member
            589.body
            555.first
            472.same
            451.long
            449.group
            439.city
            435.particular
            414.form
            382.surface
            379.new
            374.branch
            318.native

[kwd1@dms-lab bica1]$ wc -l scr.txt
24377 scr.txt

  */

  
  // -------------------------------------------- seperate nouns and adjectives
  //
  // adjectives never follow (a:the) alone

  // to first order:
  //    (a:the)     noun  !adj:noun   
  //    (a:the) adj noun  !adj:noun   
  

  //    (a:the) becomes (n:adj)

  Node* n_adj = new Node("n:adj");

  map<Node*, float>::iterator b;

  for(b=a_the->branches.begin();b!=a_the->branches.end();b++){

    //n_adj->branches.insert(pair<Node*,float>(b->first, b->second));    
    n_adj->add_branch( b->first, 1);


  }

  
  Node* noun = new Node("noun");
    

  for (c1=corpus1.begin();c1!=corpus1.end();c1++){   // loop over corpus    sentences
    for (w=c1->begin();w!=c1->end();w++){            // loop over words  in sentences

      if ( (*w)->symbol=="a"   ||
	   (*w)->symbol=="the" ||
	   (*w)->symbol=="an"  ){

	list<Node*>::iterator w1=w;	
	if (++w1!=c1->end()){

	  if ( n_adj->branches.find(*w1) !=
	       n_adj->branches.end()                ){


	    list<Node*>::iterator w2=w1;	
	    if (++w2!=c1->end()){

	      if ( n_adj->branches.find(*w2) ==
		   n_adj->branches.end()                ){

		// (a:the)  (n_adj)   X
		//    w       w1      w2

		//   w1 is always a noun
		
		noun->add_branch(*w1, 1);

	      }

		
	    }
	    

	  }
	}

      }

    }
  }

  // -------------------------------------------- seperate nouns and adjectives      


  //noun->output(2,0);


  /*
noun
           1403.act
           1325.genus
           1156.person
            635.United
            635.family
            623.quality
            605.state
            592.member
            396.form
            381.group
            367.branch
            306.native
            292.part
            256.property
            253.body
            243.process
            231.unit
            224.number
            213.use
            203.system
            202.variety
            196.order

[kwd1@dms-lab bica1]$ wc -l scr.txt 
11142 scr.txt
  */


  // -------------------------------------------- noun (verb)
  //

  Node* verb = new Node("verb");

  for (c1=corpus1.begin();c1!=corpus1.end();c1++){   // loop over corpus    sentences
    for (w=c1->begin();w!=c1->end();w++){            // loop over words  in sentence
      
      if ( (*w)->symbol=="a"   ||
	   (*w)->symbol=="the" ||
	   (*w)->symbol=="an"  ){

       list<Node*>::iterator w1=w;w1++;	
       if (w1!=c1->end()){

	 if (noun->branches.find(*w1) != noun->branches.end() ) {

  
	   list<Node*>::iterator w2=w1;w2++;	
	   if (w2!=c1->end()){

	     verb->add_branch(*w2,1);

	   }
	 }
       }

      }
    }
  }

  // -------------------------------------------- noun (verb)

  //verb->output(2,0);

  /*
verb
	  27902.of
	   5697.or
	   4181.that
	   3591.in
	   2738.and
	   2104.who
	   2067.to
	   1706.for
	   1244.with
	    986.is
	    858.on
	    749.was
	    615.from
	    587.States
	    541.by
	    514.at
	    466.between
	    432.where
	    427.manner
	    423.used
	    353.made
	    338.part
	    284.unit
	    280.as
	    268.time
	    250.system
	    243.city
	    233.into
	    225.but
	    215.body
	    213.consisting
	    211.whose
	    197.having
	    182.point
	    180.state
	    179.line
	    178.person
	    170.United
   */

  
  
  // -------------------------------------------- seperate adjectives
  //

  Node* adj = new Node("adj");
  
  for(b =n_adj->branches.begin();
      b!=n_adj->branches.end()  ;b++){

    if (noun->branches.find(b->first) == noun->branches.end()){

      // not a noun, so call it an adj
      
      adj->add_branch( b->first, 1);
      
    }
      
  }

  // -------------------------------------------- seperate adjectives


  //adj->output(2,0);

  /*

adj
              1.wishful
              1.unwarrantable
              1.unchivalrous
              1.unarguable
              1.stereotypical
              1.conversation'
              1.unsentimental
              1.byword
              1.prosy
              1.pacifistic
              1.disaster!
              1.jet-black
              1.north-northeast
              1.mincing
              1.messy,
              1.mawkish
              1.insinuating
              1.harmfully
              1.illustriously
              1.high-minded
              1.gloating
              1.plucky


[kwd1@dms-lab bica1]$ wc -l scr.txt
13236 scr.txt
*/


  // ------------------ count the adjectives in " (a:the)  (adj)"  sequences  

  for (c1=corpus1.begin();c1!=corpus1.end();c1++){   // loop over corpus    sentences
    for (w=c1->begin();w!=c1->end();w++){            // loop over words  in sentence
      
     if ( (*w)->symbol=="a"   ||
	  (*w)->symbol=="the" ||
	  (*w)->symbol=="an"  ){

       list<Node*>::iterator w1=w;	
       if (++w1!=c1->end()){


	 if (adj->branches.find(*w1) != adj->branches.end() ){

	   adj->add_branch( *w1, 1);
	 }
       }
     }
     
     }
  }
 // ------------------ count the adjectives in " (a:the)  (adj)"  sequences
  
  
  //adj->output(2,0);

/*
adj
	    140.musical
	     61.ordinal
	     50.imaginary
	     50.southwestern
	     49.academic
	     48.anterior
	     46.Algonquian
	     46.measuring
	     46.radioactive
	     44.southeastern
	     40.armed
	     39.philosophical
	     37.informal
	     36.surgical
	     35.morbid
	     34.Siouan
	     34.Islamic
	     34.international
	     34.free
	     33.Bantu
	     33.shallow
	     32.Dravidian
	     31.abdominal
	     31.spinal
*/




//  ------------------------  now seperate verbs and prepositions


  Node* prep_verb = new Node("prep_verb");
  
  for (c1=corpus1.begin();c1!=corpus1.end();c1++){   // loop over corpus    sentences
    for (w=c1->begin();w!=c1->end();w++){            // loop over words  in sentence
      
      list<Node*>::iterator w1=w;	
      if (++w1!=c1->end()){

	if ( (*w1)->symbol=="a"   ||
	     (*w1)->symbol=="the" ||
	     (*w1)->symbol=="an"  ){

	  //    (prep_verb)   (a:the)

	  prep_verb->add_branch(*w,1);
	  
	}
      }

    }
  }


//  ------------------------  now seperate verbs and prepositions


  //prep_verb->output(2,0);

  /*
 
prep_verb
	  52059.is
	  21798.of
	  11306.in
	   5815.to
	   4168.with
	   3750.on
	   3267.by
	   2788.as
	   2587.from
	   2348.for
	   1784.having
	   1761.at
	   1579.and
	   1289.into
	    998.which
	    766.or
	    766.was
	    499.through
	    473.between
	    468.make

	     32.playing
	     31.discovered
	     31.since
	     31.serving
	     31.reduce
	     31.pay
	     30.connecting
	     30.denoting
	     30.whereby
	     30.show
	     30.indicates
	     30.upon
	     29.achieve
	     28.via
	     28.join
	     28.concerning
	     28.turn
	     28.leave
	     28.held
	     28.enter
	     28.find
  */

  
}

