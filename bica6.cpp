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

  string                           symbol  ;
  
 
  map<Node*,  float>               branches;

  map<string, Node*>               branches_string;


  
  void   add_branch(Node*,  float weight );
  void   del_branch(string               );


  Node*  add_sequence_markov    (list<Node*>::iterator,  int depth,  Node*);
  //
  // every node is a copy,
  // equivalent to conjunctive 
  // join between symbols
  //  A --()-- B

  void  add_sequence_singleton (list<Node*>::iterator,  int depth);
  //
  // no nodes are a copy,
  // equivalent to direct 
  // join between symbols                                   
  //  A --- B		 


  //Node*  add_sequence_markov_filter(list<Node*>::iterator,  int depth,  Node*);
     



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

  if (bs==branches_string.end())                               // not found
    {
      Node*       new_node = new Node((*node)->symbol);        // make new copy

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





//----------------------------------------
//
void 
Node::del_branch( string symbol){


  map< string, Node*>::iterator bs=branches_string.find(symbol);

  if (bs!=branches_string.end())
    {
      map< Node*, float>::iterator b=branches.find(bs->second);

      if (b!=branches.end()){

	branches.erase(b);
      }
      branches_string.erase(bs);
    }

  
}











void
string_to_nodes(string code, list<Node*>* sentence){

  // obselete

  // take a string of symbols and convert
  // to a list of corresponding nodes in global map
  // "a b c" -> [a,b,c]

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



  /*

  // --------------------------------------  add symbols to IO
  Node IO("I/O");
  
  list< list<Node*> >::iterator c1;
  list<Node*>::iterator w;

  for (c1=corpus1.begin();c1!=corpus1.end();c1++){      // loop through corpus
    for (w=c1->begin();w!=c1->end();w++){                 

      //IO.add_branch(*w,1);   // direct connect, singleton   
    }
  }
  
  // --------------------------------------  add symbols to IO
  */

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

  list< list<Node*> >::iterator c1;
  list<Node*>::iterator w;

  for (c1=corpus1.begin();c1!=corpus1.end();c1++){      // loop through corpus
    c1->push_back(end_node);

    for (w=c1->begin();w!=c1->end();w++){               // loop every word                

      IO1.add_sequence_markov(w,3,&IO1);                // make a copy of every node    
    }                                                   // in IO parcel
  }
  


  // --------------------------------------  make another IO using add_seq


  IO1.output(1,0);cout<<"---"<<endl;
  IO1.output(2,0);cout<<"---"<<endl;
  IO1.output(3,0);cout<<"---"<<endl;
  IO1.output(4,0);cout<<"---"<<endl;


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

  // obsolete?  could be done with add_branch and self contained -fork node


  string sent1 = "the";  string_to_nodes(sent1, &sentence);                
  Node*  the1  = IO1.add_sequence_markov(sentence.begin(),10,&IO1);   // sets the1 = "75044.the" node above

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

the sequence   "the man" will bifurcate into "(a:the) man"


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





 // -----------------   re-read  corpus
  for (c1=corpus1.begin();c1!=corpus1.end();c1++){  

    for (w=c1->begin();w!=c1->end();w++){            // loop every word                

      IO1.add_sequence_markov(w,2,&IO1);             // re-read
    }
  }
 // -----------------   re-read  corpus




  //IO1.output(2,0);cout<<"---"<<endl;
  //IO1.output(3,0);cout<<"---"<<endl;

  /*
IO1
	 432908.is
	 231185.a
	 226243.the
	 213973.of
	 164638.(a:the)
		   1411.act
		   1409.person
		   1342.genus
                   ....
	 129892.to
	 106646.or
	  94584.in
	  81786.and



  //a_the->output(4,0);cout<<"---"<<endl;


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



 // ------------------------------- combine a, the, and an into (a:the) node










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
	      1.-join              
		      1.-fork
		              1.(noun_adj)
	      1.synonymous
 	      1.sinusoidal
	      1.sinuous

  */







 // -----------------   re-read  corpus
  for (c1=corpus1.begin();c1!=corpus1.end();c1++){   

    for (w=c1->begin();w!=c1->end();w++){            

      IO1.add_sequence_markov(w,2,&IO1);                  // re-read
    }
  }
 // -----------------   re-read  corpus


  //cout<<"---"<<endl;     a_the->output(2,0);
  //cout<<"---"<<endl;     a_the->output(3,0);
  //cout<<"---"<<endl;     a_the->output(4,0);
  //cout<<"---"<<endl;     a_the->output(5,0);
  





  /*
puts a fork in every node after (a:the)

IO1
	 432908.is
	 231185.a
	 226243.the
	 213973.of
	 164638.(a:the)
              ...
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

  //--------------- read corpus in reverse
  for (c1=corpus1.begin();c1!=corpus1.end();c1++){

    temp_list.clear();

    w1=c1->rbegin(); w1++;      // skip -end_node

    for (;w1!=c1->rend();w1++)  temp_list.push_back(*w1);

    temp_list.push_back(end_node);

    for (w=temp_list.begin();w!=temp_list.end();w++){

      IO2.add_sequence_markov(w,0,&IO2);     // make a copy of every node    
    }
  }
  //--------------- read corpus in reverse



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


  Node* fork2  = new Node("-fork");        // new fork node
  Node* a_the2 = new Node("(a:the)_rev");

  sentence.clear();
  sentence.push_back(fork2    );
  sentence.push_back(a_the2   );
  sentence.push_back(end_node);

  the1->add_sequence_singleton(sentence.begin(),10);
  a1  ->add_sequence_singleton(sentence.begin(),10);
  an1 ->add_sequence_singleton(sentence.begin(),10);


  //--------------- read corpus in reverse
  for (c1=corpus1.begin();c1!=corpus1.end();c1++){     

    temp_list.clear();

    w1=c1->rbegin(); w1++;      // skip -end_node

    for (;w1!=c1->rend();w1++)  temp_list.push_back(*w1);

    temp_list.push_back(end_node);

    for (w=temp_list.begin();w!=temp_list.end();w++){          // loop every word                

      IO2.add_sequence_markov(w,2,&IO2);     
    }
  }
  //--------------- read corpus in reverse

  /*
  cout<<"---"<<endl;
  IO2.output(1,0);cout<<"---"<<endl;
  IO2.output(2,0);cout<<"---"<<endl;
  IO2.output(3,0);cout<<"---"<<endl;



---
IO2
	 288482.is
	 153725.a
	 152550.(a:the)_rev
	 150089.the
	 141638.of


	 305101.(a:the)_rev
		 104118.is           is the
		  43596.of           of the
		  22612.in           in the 
		  11630.to           ...
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

  // a_the2->add_sequence_singleton(sentence.begin(),10);
  // comment out so the fork below works
  // only the first -fork works in a node

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







  // -------------  mine the verbs from (a:the)_rev.branches

  Node* fork4  = new Node("-fork");
  Node* verb1a = new Node("(verb1a)");


  sentence.clear();
  sentence.push_back(fork4    );
  sentence.push_back(verb1a   );
  sentence.push_back(end_node);


  map< Node*, float >::iterator b= a_the2->branches.begin();

  for (;b!=a_the2->branches.end();b++){

    if (b->second < 100 && b->second > 1 ){        // ad hoc 

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


 // -------------  mine the verbs from (a:the)_rev.branches



  IO2.output(1,0);cout<<"---"<<endl;
  IO2.output(2,0);cout<<"---"<<endl;
  IO2.output(3,0);cout<<"---"<<endl;
  IO2.output(4,0);cout<<"---"<<endl;
  /*

---
IO2
	 305102.(a:the)_rev
		 104118.is
		  43596.of
		  22612.in
		  17223.(verb1a)
			   3385.to           to   eat    the X
			   1612.that         that ate    the X
			   1248.is           is   eating the X
			   1010.who          who  ate    the X
			    643.he           he   ate    the X
			    617.or
			    485.and
			    412.of
			    299.by
			    284.for
			    220.they
			    166.in
			    146.she
			    116.not
  */



  //verb1a->output(1,0);cout<<"---"<<endl;
  //verb1a->output(2,0);cout<<"---"<<endl;

  /*

       (noun1a)   (verb1a)   (a:the)

---
(verb1a)
	   3385.to          // (noun1a)     2500 nouns and particles
	   1612.that        
	   1248.is
	   1010.who
	    643.he
	    617.or       // conjunction
	    485.and
	    412.of       // preposition
	    299.by
	    284.for
	    220.they
	    166.in
	    146.she
	    116.not
  */








  //  ----------- ad hoc construction of nouns 

  Node* noun1a = new Node("(noun1a)");

  map< string, Node* >::iterator bs;

  for (b= a_the->branches.begin();b!=a_the->branches.end();b++){

    bs = verb1a->branches_string.find(b->first->symbol);

      if (bs != verb1a->branches_string.end() ){

	noun1a->add_branch( b->first, b->second);
      }
  }

  //  ----------- ad hoc construction of nouns 

  //noun1a->output(1,0);cout<<"---"<<endl;
  //noun1a->output(2,0);cout<<"---"<<endl;













  // ---------------   ad-hoc construct noun and verb parcels

  //  use (a:the)_rev counts<100 to get first pass of verbs

  Node verb1("verb1");

  //map< Node*, float >::iterator b 

  for (b=a_the2->branches.begin();b!=a_the2->branches.end();b++){

    if (b->second < 100 && b->second > 15 ){

      verb1.add_branch( b->first, b->second);

    }
  }
  // ---------------   ad-hoc construct noun and verb parcels

  /* 

  verb1.output(1,0);cout<<"---"<<endl;
  verb1.output(2,0);cout<<"---"<<endl;



6000 verbs (~>95%)
      (verb1)   (a:the)
---
verb1
	     98.covers
	     98.cut
	     96.holding
	     96.surrounding
	     94.hold
	     92.see
	     88.affecting
	     88.protects
	     88.won
	     86.supplies
	     86.befitting
	     86.protect
	     86.left
	     84.founded
	     84.causes
	     84.created
	     82.causing
	     82.representing
	     82.producing
	     82.cover
	     82.towards

*/








  // ----------------------------


  Node noun1("noun1");
  Node verb2("verb2_rev");
  Node verb_s("verb2_sort");

  //   build   (noun1)  (verb2)  sequence


  //----- read corpus in reverse
  for (c1=corpus1.begin();c1!=corpus1.end();c1++){     

    temp_list.clear();
    w1=c1->rbegin(); w1++;      // skip -end_node
    for (;w1!=c1->rend();w1++)  temp_list.push_back(*w1);
    temp_list.push_back(end_node);
    
    for (w=temp_list.begin();w!=temp_list.end();w++){          // loop every word                

 
      if ( verb1.branches_string.find((*w)->symbol) !=          // filter
	   verb1.branches_string.end()              ){
	
	verb_s.add_branch((*w),1);

	//verb2.add_sequence_markov(w,2,&verb2); 

	list<Node*>::iterator ww = w; ww++;
	noun1.add_sequence_markov(ww,2,&noun1); 

	/*
	  if ((*ww)->symbol=="an")
	  cout<< (*ww)->symbol <<"  "<< (*w)->symbol <<endl;
	*/
    
      }
    }

  }
 //----- read corpus in reverse




  /*

  verb_s.output(1,0);cout<<"---"<<endl;
  verb_s.output(2,0);cout<<"---"<<endl;

  verb2.output(1,0);cout<<"---"<<endl;
  verb2.output(2,0);cout<<"---"<<endl;


  noun1.output(1,0);cout<<"---"<<endl;
  noun1.output(2,0);cout<<"---"<<endl;



should be all verbs



---
verb2_sort
	   4783.used
	   2439.it
	   1839.her
	   1329.time
	   1232.leaves
	   1152.were
	   1075.you
	    893.use
	    862.work
	    851.often
	    835.place
	    782.so
	    727.found
	    713.been
	    699.if
	    677.back
	    634.set
	    633.line
	    572.sound
	    539.sometimes
	    512.move
	    489.cut
	    482.caused
	    479.given
	    478.force
	    469.do
	    454.open
	    446.marked
	    446.formed
	    439.play
	    432.away
	    431.change
	    422.produced

go through and remove top offender !verbs
  */

  verb_s.del_branch("it");
  verb_s.del_branch("her");
  verb_s.del_branch("you");
  verb_s.del_branch("often");
  verb_s.del_branch("so");
  verb_s.del_branch("if");
  verb_s.del_branch("sometimes");
  verb_s.del_branch("away");
  verb_s.del_branch("what");
  verb_s.del_branch("either");
  verb_s.del_branch("both");
  verb_s.del_branch("them");
  verb_s.del_branch("now");
  verb_s.del_branch("while");
  verb_s.del_branch("quantity");
  verb_s.del_branch("just");
  verb_s.del_branch("classifications");
  verb_s.del_branch("always");
  verb_s.del_branch("even");
  verb_s.del_branch("opposite");
  verb_s.del_branch("e.g.");
  verb_s.del_branch("past");
  verb_s.del_branch("how");
  verb_s.del_branch("then");
  verb_s.del_branch("upon");
  verb_s.del_branch("us");
  verb_s.del_branch("leave");
  verb_s.del_branch("almost");
  verb_s.del_branch("also");
  verb_s.del_branch("once");
  verb_s.del_branch("John");
  verb_s.del_branch("(as");
  verb_s.del_branch("(especially");
  verb_s.del_branch("since");
  verb_s.del_branch("whereby");
  verb_s.del_branch("he's");
  verb_s.del_branch("it's");
  verb_s.del_branch("(of");
  verb_s.del_branch("she's");
  verb_s.del_branch("were");








  Node noun2("noun2");

  verb_s.clear_weights();


  //   build   (noun2)  (verb_s)  sequence


  //----- read corpus in reverse
  for (c1=corpus1.begin();c1!=corpus1.end();c1++){     

    temp_list.clear();
    w1=c1->rbegin(); w1++;      // skip -end_node
    for (;w1!=c1->rend();w1++)  temp_list.push_back(*w1);
    temp_list.push_back(end_node);
    
    for (w=temp_list.begin();w!=temp_list.end();w++){          // loop every word                

 
      if ( verb_s.branches_string.find((*w)->symbol) !=          // filter
	   verb_s.branches_string.end()              ){
	
	verb_s.add_branch((*w),1);


	list<Node*>::iterator ww = w; ww++;
	if (ww!=temp_list.end()) {

	  list<Node*> nv;

	  nv.push_back(*ww);
	  nv.push_back(*w);
	  nv.push_back(end_node);

	  noun2.add_sequence_markov(nv.begin(),2,&noun2); 

	  //noun2.add_branch(*ww,1); 
	}
      }
    }

  }
 //----- read corpus in reverse




  /*  

  verb_s.output(1,0);cout<<"---"<<endl;
  verb_s.output(2,0);cout<<"---"<<endl;

  // all verbs

verb2_sort
---
verb2_sort
	   4783.used
	   1329.time
	   1232.leaves
	   1152.were
	    893.use
      
*/







  /*

  noun2.output(1,0);cout<<"---"<<endl;
  noun2.output(2,0);cout<<"---"<<endl;
  noun2.output(3,0);cout<<"---"<<endl;




  list of "nouns" is polluted





---
noun2
	   9571.to
	   5688.is
	   2516.a
	   2392.or
	   2341.the
	   2053.that
	   1467.and
	   1365.of
	   1240.who
	    900.for
	    811.he
	    608.in
	    521.be
	    488.by
	    454.has
	    448.they
	    429.not
	    409.was
	    336.are
	    250.an
	    243.can

  */




  /*


  list of "nouns" is polluted


---
noun2
	   9571.to                  verbs  250
		    385.move
		    276.treat
		    238.do
		    230.provide
		    158.cover
		    151.bring
		    146.hold
		    143.change
		    141.keep
		    129.work
		    126.prevent
		    126.use
		    117.cut

           5688.is                   tense  (mostly mistakes from insertng "is" in corpus)
	   2516.a                    nouns/adj  that are verbs
		    166.place
		    162.line
		    156.given
		    109.set
		     93.time
		     66.change
		     59.sound

	   2392.or                   conj
	   1467.and                  conj


	   2341.the                  noun/adj


	   2053.that                  pro
	   1240.who                   pro
	    811.he                    pro
	    448.they                  pro


	   1365.of                   prepos
	    900.for                  prepos
	    608.in                   prepos
	    488.by                   prep



	    521.be                   tense
	    454.has                  tense

	    429.not                  negative in tense

	    409.was
	    336.are
	    250.an

	    243.can

	    222.she
	    211.I
	    209.have
	    193.his
	    192.often      adv
	    166.having
	    164.usually
	    163.you
	    153.with
	    146.it
	    146.one
	    143.we



	   2392.or
		     46.used
		     42.force
		     41.caused


alternate is serving or used in place of another 




   (noun)     (te)  (verb)  (te)      (prep)     (noun) 
                                                        
   alternate  is    serve   ing  			     
	            or 				     
	            use          d    in         place  

		                      of         another








	   2053.that                        // pronoun
		    115.causes              
		     80.does                // all verbs
		     79.serves              // in 3rd
		     57.carries
		     52.covers
		     52.involves
		     50.resemble
		     48.supplies
		     39.protects
		     37.gives
		     36.supports
		     34.allows
		     33.measures



killing is an   event that causes someone to die                                



       (noun)     (te)  (verb)  (te)      (prep)     (noun) 

       killing    is                                 an                  //hypernym 
                                                     event  

       that             cause   s                    someone     

                        to
                        die





hypernyms create  (noun) (noun)  parcel sequence


could have any combination of main node parcel sequence

(noun)  (noun)
(noun)  (verb)
(noun)  (verb)

(prep)  (noun)



determiners, adj, and nouns stack in parcel.

particle stacks in verb parcel





	   1467.and            conj
		    150.used
		     38.leaves
		     33.served
		     31.forth







he got change for a twenty and used it to pay the taxi driver 




       (noun)     (te)  (verb)  (te)      (prep)     (noun) 

       he               got                          change 

                                          for        a 
                                                     twenty 
                        and 
                        used                         it 

                        to 
                        pay                          the 
                                                     taxi 
                                                     driver           // what taxi driver?





conjunctions stack in any parcel:


        (noun)    (te)  (verb)  (te)      (prep)     (noun) 

        car
        and
        truck


                 was
                 and
                 is



                       eat
                       and
                       drink
                                           for
                                           and
                                           of       the
                                                    people
 
                                                    and
                                                    several 
                                                    corporations

                                                    and 
                                                    other
                                                    interests





  






(det) (noun)   (te)  (verb)  (3rd) (prog)(past)   (prep)  (det:adj:n) 

      he             set     s                                    policy 
 
                     and 

                     leaves                                 all the administrivia 

                                                   to       his assistant 



 he sets policy and leaves all the administrivia to his assistant 




	   1365.of             prepos  

		    155.time             //nouns that are verb s
		     49.work
		     43.changing
		     37.sound
		     35.taking
		     33.giving
		     32.producing
		     32.writing




	   1240.who       pro
	    900.for            prepos
	    811.he        pro
	    608.in             prepos
	    521.be             tense
	    488.by             prep
	    454.has            tense
	    448.they      pro

	    429.not
		     24.used
		     19.been
		     13.given


(det) (noun)   (te)     (verb)  (3rd) (prog)(past)   (prep)  (det:adj:n) 

      Prosimii is   not  used                        in          all classifications 


      Religion is   not  taugh                       in          school







---
IO2
	 305102.(a:the)_rev
	 288482.is
	 153724.a
	 150088.the
	 141638.of
	  85992.to
            ...
	   7104.not
		   4276.is                    is not
		    366.does
		    222.are                 TE1 nodes preceed "not"
		    202.but
		    136.and
		    126.do
		    124.did
		    122.of
		    110.was
 		    108.or
		    108.could
		     66.has
		     52.will
		     50.would
		     40.were
		     34.should
		     28.to
		     26.have
		     24.if
		     24.must
		     24.had
		     18.especially
                   ...
		      4.rather              rather not
		      4.I'm
		      4.still
		      4.guaranteed
		      4.you
		      2.dared
		      2.`We're
		      2.married,
		      2.wouldst
		      2.means



(det) (noun)   (te)     (verb)  (3rd) (prog)(past)   (prep)  (det:adj:n) 

       he               lost                                   his faith 
                        but 
                 not                                           his morality 

 
       he    did not    lose                                  his morality







	    429.not
	    409.was
	    336.are
	    250.an

	    243.can



(det) (noun)   (te)     (verb)  (3rd) (prog)(past)   (prep)  (det:adj:n) 

the   need                                           for     informational flexibility 

                    can lead                         to      adhocracy 





	    243.can
	    222.she
	    211.I
	    209.have
	    193.his
	    192.often      adv
	    166.having
	    164.usually
	    163.you
	    153.with
	    146.it
	    146.one
	    143.we










[–]joake 445 points an hour ago 
Obama also said that the Keystone pipeline would not create as many jobs in the long run. If the senate really is interested in creating jobs - they should start looking at a new bill that would lift infrastructure

[–]ConradSchu 286 points an hour ago 
35 permanent jobs after it's all said and done. 35.

[–]backattack88 [score hidden] 56 minutes ago 
People are quick to scoff at the thousands of construction jobs it would have created for a few years because they are not permanent, but in reality no construction jobs are permanent. The pipeline would have been a fantastic gig for American construction workers.

[–]iarighter [score hidden] 48 minutes ago 
At the same time, we're looking for more permanent solutions to economic well-being, not some one-off stimulus. In the long run, and for the majority of society, things like Keystone XL seem dangerous.

[–]buddybiscuit [score hidden] 41 minutes ago 
So then reddit's hard on for a New Deal type plan for infrastructure is misplaced because it's a one-off stimulus?


bot could answer questions on reddit

"mis"placed   (node)


[–]3tondickpunch [score hidden] 37 minutes ago 
Yes, because infrastructure is only built once and never needs maintenance.


sarcasm bot






(det) (noun)   (te)      (verb)  (3rd) (prog)(past)   (prep)   (det:adj:n) 
                         (adv )


      Obama              also
                         said                                  that 

the Keystone- 
    pipeline  would not  create                       as       many jobs 
                                                      in       the long run. 



If 
the  senate              really 
                is       interested                   in       creating-jobs,

     they      should    start                                 
  
     looking                                          at       a new-bill 

     that      would     lift                                  infrastructure





(det:adj:n)     (te)     (verb)      (te  )          (prep)   (det:adj:n) 
                         (adv )


      People    are      quick 
                      to scoff                         at      the thousands 
                                                       of      construction 
                                                               jobs 
      it        would 
                have    create        ed               for     a few years 

because 

     they       are 
                not                                            permanent, 

but 

                                                       in      reality 
no     construction 
       jobs           are                                      permanent. 



The pipeline      
                  would 
                  have  
                  been                                          
                                                                 a fantastic 
                                                                 gig
 
                                                       for       
                                                                 American 
                                                                 construction 
                                                                 workers.


*/











  //  go back to noun2, take out pronouns and make the sequence:
  //
  //        (noun3)  (verb3)
  //

  /*
---
noun2


	   2053.that

	   1240.who

	    811.he

	    448.they

	    222.she
 
            ...
 
  */



  Node noun3("noun3");     // pronouns

  Node verb3("verb3");     // associated verbs


  Node pn_1("I")   ; noun3.add_branch(&pn_1, 0);   // manually build
  Node pn_2("you") ; noun3.add_branch(&pn_2, 0);   // pronoun Node
  Node pn_3("he")  ; noun3.add_branch(&pn_3, 0);
  Node pn_4("she") ; noun3.add_branch(&pn_4, 0);
  Node pn_5("it")  ; noun3.add_branch(&pn_5, 0);


  // --- join he and she
  Node* fork5  = new Node("-fork");        // new fork node
  Node* he_she = new Node("(he:she)");

  sentence.clear();
  sentence.push_back(fork5    );
  sentence.push_back(he_she   );
  sentence.push_back(end_node );

  pn_3.add_sequence_singleton(sentence.begin(),10);
  pn_4.add_sequence_singleton(sentence.begin(),10);
  // --- join he and she


  //----- read corpus in reverse
  for (c1=corpus1.begin();c1!=corpus1.end();c1++){     

    temp_list.clear();
    w1=c1->rbegin(); w1++;      // skip -end_node
    for (;w1!=c1->rend();w1++)  temp_list.push_back(*w1);
    temp_list.push_back(end_node);
    
    for (w=temp_list.begin();w!=temp_list.end();w++){          // loop every word                

 
      if ( verb_s.branches_string.find((*w)->symbol) !=          // filter
	   verb_s.branches_string.end()              ){

	
	//verb_s.add_branch((*w),1);
	//verb3.add_branch((*w),1);


	list<Node*>::iterator ww = w; ww++;
	if (ww!=temp_list.end()) {

	  if ( noun3.branches_string.find((*ww)->symbol) !=          // filter
	       noun3.branches_string.end()              ){

	    verb3.add_branch((*w),1);

	    list<Node*> nv;

	    nv.push_back(*ww);
	    nv.push_back(*w);
	    nv.push_back(end_node);

	    noun3.add_sequence_markov(nv.begin(),2,&noun3); 
	  }
	}
      }
    }
  }
 //----- read corpus in reverse


  /*
  verb3.output(1,0);cout<<"---"<<endl;
  verb3.output(2,0);cout<<"---"<<endl;

  noun3.output(1,0);cout<<"---"<<endl;
  noun3.output(2,0);cout<<"---"<<endl;
  noun3.output(3,0);cout<<"---"<<endl;



---
noun3
	   1032.(he:she)
	    811.he
	    222.she
	    211.I
	    163.you
	    146.it



 1032.(he:she)
		     55.did          i  //has ~50% irregular verbs 
		     46.felt         i
		     35.used
		     33.heard        i
		     32.wanted
		     29.asked
		     27.wrote        i
		     25.saw          i
		     25.played
	

	    146.it
		     20.takes          // few irregular
		     11.becomes
		      6.does         i
		      5.started
		      4.indicates
		      4.moves
		      3.extends
		      3.causes


  */

























































}  //fin


