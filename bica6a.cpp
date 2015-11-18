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

  //float                           input   ;
  //float                           thresh  ;
  string                          symbol  ;
  
 
  map<Node*,  float>               branches;

  map<string, Node*>               branches_string;


  
  void   add_branch(Node*,  float weight );


  Node*  add_sequence_markov    (list<Node*>::iterator,  int depth,  Node*);
  //
  // every node is a copy
  // equivalent to conjunctive 
  // join between symbols
  //  A --()-- B

  void  add_sequence_singleton (list<Node*>::iterator,  int depth);
  //
  // no nodes are a copy
  // equivalent to direct 
  // join between symbols                                   
  //  A --- B		 


  Node*  add_sequence_markov_filter(list<Node*>::iterator,  int depth,  Node*);
     



  void  output       (  int  depth,  int tabs     );

  void  clear_weights();

  Node(){};
  Node(string name){symbol=name;/*input=0;thresh=0;*/}

}; 




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
Node::add_sequence_singleton( list<Node*>::iterator node, int depth){

  if ((*node)->symbol=="-stop") return;


  // search by pointer
  
  map< Node*, float >::iterator b=branches.find( *node );

  if (b==branches.end())
    {
      add_branch( *node, 0);
      
      b=branches.find( *node );
    }

  b->second++;

  if (depth<1) return;
  
  b->first->add_sequence_singleton(++node, depth-1);




}







//----------------------------------------
//
Node*
Node::add_sequence_markov( list<Node*>::iterator node, int depth, Node* head){

  //cout<<symbol<<"   "<<(*node)->symbol<<"   "<<depth<<endl;


  if ((*node)->symbol=="-stop") return this;


  // search by symbol
  
  map< string, Node*>::iterator bs=branches_string.find((*node)->symbol);

  if (bs==branches_string.end())   // not found
    {
      Node*       new_node = new Node((*node)->symbol);

      add_branch( new_node, 0);
      
      bs=branches_string.find(        (*node)->symbol);
    }

  map<Node*,float>::iterator b=branches.find(bs->second);

  if (b==branches.end()){cout<<"node add_sequence_markov error"<<endl;return NULL;}

  b->second++;

  if (depth<1) return this;
  
  node++;

  Node* node_retval = bs->second->add_sequence_markov(node, depth-1, this);

  node--;

  //cout<< "node_retval: "<<symbol<<"  "<<node_retval->symbol<<"  "<<(*node)->symbol<<endl;

  // returns here with node unchanged




  // ------------- execute -fork command
  bs=branches_string.find("-fork");

  if (bs!=branches_string.end()){

    Node* n1= bs->second;                           //-fork node
    Node* n2= n1->branches_string.begin()->second;  // target
 
    list<Node*>::iterator node1=node;
    node1++;

    /*
    cout<<"*fork"<<endl;
    cout<<symbol<<"   "<< n2->symbol<<"  "
	<<(*node)->symbol<<"   "
	<<(*node1)->symbol<<"   "
	<<depth<< endl; 
    */
    //cout<<node_retval->symbol<<endl;
    //cout<<this->symbol<<endl;

    head->add_branch(n2,0);   // manually add fork target node to head node

    node_retval = n2->add_sequence_markov(node, depth-1, head);
    //cout<<node_retval->symbol<<endl;

   }
  // ------------- execute -fork command


  // ------------- execute -join command
  bs=branches_string.find("-join");

  if (bs!=branches_string.end()){

    Node* n1= (bs->second)->branches_string.begin()->second;
    Node* n2= n1->          branches_string.begin()->second;

    list<Node*> sent;

    sent.push_back(n1);
    sent.push_back(n2);
    /*
    cout<<"*join"<<endl;
    cout<<symbol<<"   "<<(*node)->symbol<<
      "   "<<n1->symbol<<"   "<<n2->symbol<<endl; 
    */
    bs=branches_string.find((*node)->symbol);
    //cout<<bs->second->symbol<<endl;


    (bs->second )->add_sequence_singleton(sent.begin(), 1);
  }

  // ------------- execute -join command

  /*
  // ------------- execute -fork_r command
  bs=branches_string.find("-fork_head");

  if (bs!=branches_string.end()){

    Node* n1= (bs->second)->branches_string.begin()->second;

    list<Node*> sent;

    sent.push_back(head);

    n1->add_sequence_markov(sent.begin(),0,this);
  }
  // ------------- execute -fork_r command
  */


  return node_retval;

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
      branches       .insert(pair<Node*, float> (node,         weight));
      branches_string.insert(pair<string,Node*> (node->symbol, node  ));
    }

  else
    { b->second++;}
  
}











void
string_to_nodes(string code, list<Node*>* sentence){

  // take a string of symbols and convert
  // to a list of corresponding nodes in global map


  extern map<string, Node*>   map_symbols;     // global map

  sentence->clear();

  int     s1=0;
  unsigned int     s2=0;
  string  field;

  while (s2!=string::npos) {
    s2 = code.find(" ", s1); field = code.substr(s1,s2-s1); s1+=s2-s1+1;

    if (field=="") continue;

    map<string, Node*>::iterator ms = map_symbols.find(field);
    if (ms            == map_symbols.end()){
      Node* new_node  = new Node(field);
      map_symbols.    insert(pair<string,Node*>(field,new_node));
      ms =             map_symbols.find(field);
    }
    sentence->push_back(ms->second);
  }

 Node* end_node = new Node("-stop");

 sentence->push_back(end_node);

}



map<string, Node*>   map_symbols;



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





  // --------------------------------------  build symbols in main()
  //map<string, Node*>  map_symbols;

        list<Node*>   sentence;
  list< list<Node*> > corpus1;
  
  int count=0;
  list<string>::iterator c=corpus.begin();
  for (;c!=corpus.end();c++, count++){

    sentence.clear();
    //cout<<*c<<endl;
    
    int            s1=0;
    unsigned int   s2=0;
    string         field;

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





  // --------------------------------------  add symbols to IO
  Node IO("I/O");
  
  list< list<Node*> >::iterator c1;
  list<Node*>::iterator w;

  for (c1=corpus1.begin();c1!=corpus1.end();c1++){      // loop through corpus
    for (w=c1->begin();w!=c1->end();w++){                 

      IO.add_branch(*w,1);   // direct connect, singleton   
    }
  }
  
  // --------------------------------------  add symbols to IO


  //IO.output(2,0);

  /*
  I/O
         144241.is
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


  // --------------------------------------  make another IO using add_seq
  Node IO1("IO1");
  Node* end_node = new Node("-stop");

  //list< list<Node*> >::iterator c1;
  //list<Node*>::iterator w;

  for (c1=corpus1.begin();c1!=corpus1.end();c1++){      // loop through corpus
    c1->push_back(end_node);

    for (w=c1->begin();w!=c1->end();w++){          // loop every word                

      IO1.add_sequence_markov(w,1,&IO1);     // make a copy of every node    
    }
  }
  


  // --------------------------------------  make another IO using add_seq


  //IO1.output(1,0);cout<<"---"<<endl;
  //IO1.output(2,0);cout<<"---"<<endl;
  //IO1.output(3,0);cout<<"---"<<endl;


 /*
markov
IO1
	 144241.is
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
	   3600.at
	   3552.not
	   3424.be
	   3138.any
	   3115.one
	   3114.genus
	   3110.small
	   2984.are
	   2828.The
	   2822.which
	   2751.into


	  75044.the
		   1308.act
		    817.genus
		    635.United
		    601.quality
		    562.family
		    528.first
		    472.same
		    431.body
		    412.state
		    289.branch
		    261.property
		    254.head

	  76862.a
		   1275.person
		    847.small
		    618.large
		    594.member
		    525.genus
		    420.particular
		    416.long
		    414.group
		    387.city
		    343.state
		    312.native
		    309.single
		    269.unit
*/


 // ------------------------------- combine a, the, and an into (a:the) node

  string sent1 = "the";  string_to_nodes(sent1, &sentence);                
  Node*  the1  = IO1.add_sequence_markov(sentence.begin(),10,&IO1);   // sets the1=75044.the node above

  string sent2 = "a";  string_to_nodes(sent2, &sentence);
  Node*  a1    = IO1.add_sequence_markov(sentence.begin(),10,&IO1);

  string sent3 = "an"; string_to_nodes(sent3, &sentence);
  Node*  an1   = IO1.add_sequence_markov(sentence.begin(),10,&IO1);


  Node* fork  = new Node("-fork");
  Node* a_the = new Node("(a:the)");

  sentence.clear();
  sentence.push_back(fork    );
  sentence.push_back(a_the   );
  sentence.push_back(end_node);

  the1->add_sequence_singleton(sentence.begin(),10);
  a1  ->add_sequence_singleton(sentence.begin(),10);
  an1 ->add_sequence_singleton(sentence.begin(),10);



  //the1->output(4,0);
  /*
puts singleton instance of "-fork (a:the)" 
sequence in a, the, and an

      75044.the
              ...
	      2.incidents
	      2.gale
	      2.ditch
	      2.snowy
	      2.trimming
	      1.-fork
		      1.(a:the)
	      1.husbands
	      1.arrangements
	      1.teenager
	      1.conversation'
	      1.convoy
	      1.cast,
  */



  for (c1=corpus1.begin();c1!=corpus1.end();c1++){   // loop through corpus

    //cout<<"-sent_start"<<endl;

    for (w=c1->begin();w!=c1->end();w++){            // loop every word                

      //cout<<"-word_start"<<endl;

      IO1.add_sequence_markov(w,2,&IO1);                  // re-read
      //IO1.add_sequence_markov(w,5,&IO1);                  // re-read
    }
  }

  //IO1.output(2,0);cout<<"---"<<endl;
  //IO1.output(3,0);cout<<"---"<<endl;


  /*
  IO1.output(4,0);cout<<"---"<<endl;

226243.the
...
		      2.SUVs
		      2.USSR's
		      2.Apollo
		      1.-fork
			      1.(a:the)
	 141638.of
		  26080.the
		  14910.a
  */


  //IO1.output(2,0);cout<<"---"<<endl;
  //IO1.output(3,0);cout<<"---"<<endl;

  /*
IO1
	 432908.is
	 231185.a
	 226243.the
	 213973.of
	 164638.(a:the)
	 129892.to
	 106646.or
	  94584.in
	  81786.and


	 164638.(a:the)
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
  */
 // ------------------------------- combine a, the, and an into (a:the) node





  //a_the->output(4,0);cout<<"---"<<endl;

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


mostly nouns and adjectives follow (a:the)

   */






  // -------------  create (noun_adj) node and connect (a:the) branches 
  // -------------  to it with a "-join" command 




  Node* join1    = new Node("-join");
  Node* fork1    = new Node("-fork");
  Node* noun_adj = new Node("(noun_adj)");

  sentence.clear();
  sentence.push_back(join1   );
  sentence.push_back(fork1   );
  sentence.push_back(noun_adj);
  sentence.push_back(end_node);

  a_the->add_sequence_singleton(sentence.begin(),10);



  //a_the->output(4,0);cout<<"---"<<endl;

  /*

(a:the)
              ...
	      2.no-good
	      2.snowy
	      1.-join              // adds -fork  (noun_adj) to every branch
		      1.-fork
		              1.(noun_adj)
	      1.synonymous
 	      1.sinusoidal
	      1.sinuous

  */








  for (c1=corpus1.begin();c1!=corpus1.end();c1++){   // loop through corpus
    for (w=c1->begin();w!=c1->end();w++){            // loop every word                

      IO1.add_sequence_markov(w,2,&IO1);                  // re-read
    }
  }

  //cout<<"---"<<endl;     a_the->output(2,0);
  //cout<<"---"<<endl;     a_the->output(3,0);
  //cout<<"---"<<endl;     a_the->output(4,0);
  //cout<<"---"<<endl;     a_the->output(5,0);
  





  /*
puts a fork in every node after (a:the)


	      2.Apollo
		      1.program
		      1.-fork
	      1.-join
		      1.-fork

	


  */


  //cout<<"---"<<endl; noun_adj->output(2,0);
  //cout<<"---"<<endl; noun_adj->output(3,0);
 
  /*


(noun_adj)
	  25173.of
	   5260.or
	   3859.that
	   3641.in
	   3046.and


   */




  /*
                 ;cout<<"---"<<endl;
  IO1.output(2,0);cout<<"---"<<endl;
  IO1.output(3,0);cout<<"---"<<endl;
  IO1.output(4,0);cout<<"---"<<endl;
  */


  /*
// added head to add_seq() to add fork


IO1
	 432723.is
	 329279.(a:the)
	 230587.a
	 225133.the
	 212457.of
	 128988.to
	 106122.or
	  94017.in
	  80913.and



		      3.environmentally
	 329279.(a:the)
		 120500.(noun_adj)       <--------- main sequence
		   2822.act
		   2818.person
		   2684.genus
		   1804.small
		   1558.family



	 329279.(a:the)
		 120500.(noun_adj)
			  25173.of
			   5260.or
			   3859.that
			   3641.in
			   3046.and
			   2075.to
			   1933.who
			   1485.for

   */


  // -------------  create (noun_adj) node and connect (a:the) branches 
  // -------------  to it with a "-join" command 







  // -------------  repeat above with reversed sentences
  // -------------  to get prepositions and verbs

  Node IO2("IO2");

  list<Node*> temp_list;
  list<Node*>::reverse_iterator w1;

  //--------------------------------------------- read corpus in reverse
  for (c1=corpus1.begin();c1!=corpus1.end();c1++){      // loop through corpus

    temp_list.clear();

    w1=c1->rbegin(); w1++;      // skip -end_node

    for (;w1!=c1->rend();w1++)  temp_list.push_back(*w1);

    temp_list.push_back(end_node);

    for (w=temp_list.begin();w!=temp_list.end();w++){          // loop every word                

      IO2.add_sequence_markov(w,0,&IO2);     // make a copy of every node    
    }
  }
  //--------------------------------------------- read corpus in reverse

  /*
  cout<<"---"<<endl;
  IO2.output(1,0);cout<<"---"<<endl;
  IO2.output(2,0);cout<<"---"<<endl;
  IO2.output(3,0);cout<<"---"<<endl;
  */

  //  120K unique symbols




 // ------------------------------- combine a, the, and an into (a:the) node

  sent1 = "the";  string_to_nodes(sent1, &sentence);                
  the1  = IO2.add_sequence_markov(sentence.begin(),10,&IO1);   // sets the1=75044.the node above

  sent2 = "a";  string_to_nodes(sent2, &sentence);
  a1    = IO2.add_sequence_markov(sentence.begin(),10,&IO1);

  sent3 = "an"; string_to_nodes(sent3, &sentence);
  an1   = IO2.add_sequence_markov(sentence.begin(),10,&IO1);


  Node* fork2  = new Node("-fork");
  Node* a_the2 = new Node("(a:the)_rev");

  sentence.clear();
  sentence.push_back(fork2    );
  sentence.push_back(a_the2   );
  sentence.push_back(end_node);

  the1->add_sequence_singleton(sentence.begin(),10);
  a1  ->add_sequence_singleton(sentence.begin(),10);
  an1 ->add_sequence_singleton(sentence.begin(),10);



  //--------------------------------------------- read corpus in reverse
  for (c1=corpus1.begin();c1!=corpus1.end();c1++){     

    temp_list.clear();

    w1=c1->rbegin(); w1++;      // skip -end_node

    for (;w1!=c1->rend();w1++)  temp_list.push_back(*w1);

    temp_list.push_back(end_node);

    for (w=temp_list.begin();w!=temp_list.end();w++){          // loop every word                

      IO2.add_sequence_markov(w,2,&IO2);     
    }
  }
  //--------------------------------------------- read corpus in reverse

  /*
  cout<<"---"<<endl;
  IO2.output(1,0);cout<<"---"<<endl;
  IO2.output(2,0);cout<<"---"<<endl;
  IO2.output(3,0);cout<<"---"<<endl;
  */

  /*
---
IO2
	 288482.is
	 153725.a
	 152550.(a:the)_rev
	 150089.the
	 141638.of


	 305101.(a:the)_rev
		 104118.is
		  43596.of
		  22612.in
		  11630.to
		   8336.with
		   7500.on
		   6534.by
		   5576.as
		   5174.from
		   4696.for
		   3568.having
		   3522.at
		   3158.and
		   2578.into
		   1996.which
		   1532.or
		   1532.was
		    998.through
		    946.between
		    936.make
		    904.has
	


mostly preposistions at the start


		     68.call
		     66.found
		     66.were
		     66.called
		     66.set
		     66.just
		     66.played
		     64.beneath     p
		     64.invented
		     64.measure
		     64.studied
		     64.prevent
		     64.if


prepositions thin out lower down

		     22.commit
		     22.equals
		     22.emit
		     22.bordering
		     22.force
		     22.possessing
		     22.lead
		     22.guide
		     22.drawing
		     22.begin

becomes almost pure verbs

		  

  */













  Node* join3    = new Node("-join");
  Node* fork3    = new Node("-fork");
  Node* pos_verb = new Node("(prepos_verb)");

  sentence.clear();
  sentence.push_back(join3   );
  sentence.push_back(fork3   );
  sentence.push_back(pos_verb);
  sentence.push_back(end_node);

  //a_the2->add_sequence_singleton(sentence.begin(),10);


  IO2.clear_weights();

  //--------------------------------------------- read corpus in reverse
  for (c1=corpus1.begin();c1!=corpus1.end();c1++){     

    temp_list.clear();

    w1=c1->rbegin(); w1++;      // skip -end_node

    for (;w1!=c1->rend();w1++)  temp_list.push_back(*w1);

    temp_list.push_back(end_node);

    for (w=temp_list.begin();w!=temp_list.end();w++){          // loop every word                

      IO2.add_sequence_markov(w,4,&IO2);     
    }

  }
  //--------------------------------------------- read corpus in reverse


  /*
                 ;cout<<"---"<<endl;
  IO2.output(2,0);cout<<"---"<<endl;
  IO2.output(3,0);cout<<"---"<<endl;
  IO2.output(4,0);cout<<"---"<<endl;
  */


  /*

IO2
	 152551.(a:the)_rev
	 144241.is
	  76862.a
	  75044.the
	  70819.of
	  42996.to

---
IO2
	 152551.(a:the)_rev
		 144747.(pos_verb)
		  52059.is
		  21798.of
		  11306.in
		   5815.to
		   4168.with
		   3750.on


		    139.considered
		    135.below
		    133.especially
		    123.not
		    122.forming
		    115.resembles
		    114.being
		    110.take
		    109.only
		    107.using
		    105.took
		    101.remove
		     89.making
		     87.are
		     84.involving
		     84.outside
		     83.formerly
		     83.put
		     82.onto
		     81.studies
		     81.forms

   */


  // -------------  repeat above with reversed sentences
  // -------------  to get prepositions and verbs





  Node* fork4  = new Node("-fork");
  Node* verb1a = new Node("(verb1a)");


  sentence.clear();
  sentence.push_back(fork4    );
  sentence.push_back(verb1a   );
  sentence.push_back(end_node);



  map< Node*, float >::iterator b= a_the2->branches.begin();

  for (;b!=a_the2->branches.end();b++){

    if (b->second < 100 && b->second > 1 ){

      (b->first)->add_sequence_singleton(sentence.begin(),10);
    } 
  }

  //----- read corpus in reverse
  for (c1=corpus1.begin();c1!=corpus1.end();c1++){     

    temp_list.clear();

    w1=c1->rbegin(); w1++;      // skip -end_node

    for (;w1!=c1->rend();w1++)  temp_list.push_back(*w1);

    temp_list.push_back(end_node);

    for (w=temp_list.begin();w!=temp_list.end();w++){          // loop every word                

      IO2.add_sequence_markov(w,4,&IO2);     
    }
  }
  //----- read corpus in reverse


  IO2.output(1,0);cout<<"---"<<endl;
  IO2.output(2,0);cout<<"---"<<endl;
  IO2.output(3,0);cout<<"---"<<endl;
  IO2.output(4,0);cout<<"---"<<endl;
















  // ---------------   ad-hoc construct noun and verb parcels

  //  use (a:the)_rev counts<100 to get first pass of verbs

  Node verb1("verb1");

  //map< Node*, float >::iterator b 

  for (a_the2->branches.begin();b!=a_the2->branches.end();b++){

    if (b->second < 100 && b->second > 1 ){

      verb1.add_branch( b->first, b->second);

    }
  }
  // ---------------   ad-hoc construct noun and verb parcels

  

  //verb1.output(1,0);cout<<"---"<<endl;
  //verb1.output(2,0);cout<<"---"<<endl;

  /*

2500 verbs (~>95%)
verb1
	     89.making
	     87.are
	     84.involving
	     84.outside
	     83.formerly
	     83.put
	     82.onto
	     81.studies
	     81.forms
	     78.holds
	     78.makes
	     76.lacking
	     74.got
	     73.gave

	     36.felt
	     35.reach
	     35.carry
	     35.used
	     34.controls
	     34.he's
	     34.raise
	     34.create
	     34.heard
	     34.wrote
	     34.takes
	     34.call
	     33.found
	     33.were
	     33.called


	      9.solve
	      9.spread
	      9.supported
	      9.adopt
	      9.passed
	      9.ate
	      9.suffered
	      9.examine
	      9.performed
	      9.wanted
	      9.explain
	      9.fought
	      9.changed
	      9.attends
	      9.ordered



---
*/




  //---------------------------- count 3rd singular, past, progressing
  int tense_ing = 0;
  int tense_ed  = 0;
  int tense_s   = 0;


  map< string, Node* >::iterator bs = verb1.branches_string.begin();

  for (;bs!=verb1.branches_string.end();bs++){

    string sym = bs->first;

    if ( sym.find("ing") == sym.length()-3) tense_ing++;
    if ( sym.find("ed" ) == sym.length()-2) tense_ed++ ;
    if ( sym.find("s"  ) == sym.length()-1) tense_s++  ;
  }

  //cout<<"ing  "<<tense_ing<<endl;
  //cout<<"ed   "<<tense_ed<<endl;
  //cout<<"s    "<<tense_s<<endl;


  /*
2500 total verbs

ing  472
ed   459
s    289
  */

  //---------------------------- count 3rd singular, past, progressing




  // ----------------------------


  Node noun1("noun1");
  Node verb2("verb2");


  for (c1=corpus1.begin();c1!=corpus1.end();c1++){     

    temp_list.clear();

    w1=c1->rbegin(); w1++;      // skip -end_node

    for (;w1!=c1->rend();w1++)  temp_list.push_back(*w1);

    temp_list.push_back(end_node);

    for (w=temp_list.begin();w!=temp_list.end();w++){          // loop every word                

      if ( verb1.branches_string.find((*w)->symbol) !=
	   verb1.branches_string.end()              ){   //filter

	verb2.add_sequence_markov(w,2,&verb2); 

	list<Node*>::iterator ww = w; ww++;
	noun1.add_sequence_markov(ww,2,&noun1); 

	/*
	if ((*ww)->symbol=="is")
	  cout<< (*ww)->symbol <<"  "<< (*w)->symbol <<endl;
	*/
    
      }
    }

  }


  // ----------------------------


  /*
  verb2.output(1,0);cout<<"---"<<endl;
  verb2.output(2,0);cout<<"---"<<endl;
  */

  noun1.output(1,0);cout<<"---"<<endl;
  noun1.output(2,0);cout<<"---"<<endl;













  // --------------------------------------  make pure Markov graph

  Node  markov("Markov");

  //Node* end_node = new Node("-stop");

  count=0;
  list< list<Node*> >::iterator s;                           //sentence
  for (s=corpus1.begin();s!=corpus1.end();s++,count++){      // loop through corpus

    //s->push_back(end_node);
    
    //markov.add_sequence_markov( s->begin(), 2,&IO1); 

    //if (count>3) break;
  }
  // --------------------------------------  make pure Markov graph
  


  //markov.output(1,0)  ;cout<<"---------------------"<<endl;
  //markov.output(2,0)  ;cout<<"---------------------"<<endl;
  //markov.output(3,0)  ;cout<<"---------------------"<<endl;


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





  // ------------------------------- singleton graph

  Node  singleton("singleton");


  //list< list<Node*> >::iterator s;                           //sentence
  for (s=corpus1.begin();s!=corpus1.end();s++,count++){      // loop through corpus

    //s->push_back(end_node);
    
    //singleton.add_sequence_singleton( s->begin(),2); 

  }



  // ------------------------------- singleton graph


  //singleton.output(1,0)  ;cout<<"---"<<endl;
  //singleton.output(2,0)  ;cout<<"---"<<endl;
  //singleton.output(3,0)  ;cout<<"---"<<endl;
  



  /*

singleton
	   6495.the
		     38.first
		     35.new
		     30.children
		     28.car
		     26.old
		     25.most
		     25.child
		     23.house
		     22.ship
		     21.book
		     21.two
		     20.company
		     19.play
		     19.last
		     18.sun
		     17.whole
		     17.government
		     16.plane
		     16.road
		     15.room
		     15.team
		     14.baby
		     14.ball
		     14.police
		     14.doctor
		     14.president
		     13.head
		     13.British
		     12.river
		     12.film



singleton
	   6495.the
		   1298.act
		    588.quality
		    325.state
		    278.branch
		    235.property
		    206.first
		    198.basic
		    185.process
		    171.capital
		    167.part
		    142.most
		    122.largest
		    120.trait
		    118.time
		    113.position
		     96.condition
		     88.second
		     85.use
		     82.person
		     79.body
		     78.right
		     77.same
		     73.activity
		     73.number
		     72.study
		     72.car
		     71.last
		     69.new
		     66.amount
		     63.old
		     62.highest
		     61.point
		     59.quantity

   */










  
}

