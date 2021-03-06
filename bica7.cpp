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




  void  output       (  int  depth,  int tabs     );

  void  clear_weights();

  Node(){};
  Node(string name){symbol=name;}

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


  if ((*node)->symbol=="-stop") return this;

  Node* node_retval=NULL;

  map< string, Node*>::iterator bs;


  if (depth>0) {

    // search by symbol
  
    map< string, Node*>::iterator bs=branches_string.find((*node)->symbol);

    if (bs==branches_string.end())   // not found
      {
	Node*       new_node = new Node((*node)->symbol);

	add_branch( new_node, 0);
      
	bs=branches_string.find(        (*node)->symbol);
      }

    map<Node*,float>::iterator b=branches.find(bs->second);   if (b==branches.end()){cout<<"node add_sequence_markov error"<<endl;return NULL;}

    b->second++;

    node++;
    
    node_retval = bs->second->add_sequence_markov(node, depth-1, this);
    
    node--;
  }


  // returns here with node unchanged




  // ------------- execute -fork command
  bs=branches_string.find("-fork");

  if (bs!=branches_string.end()){

    Node* n1= bs->second;                           //-fork node
    Node* n2= n1->branches_string.begin()->second;  // target
 
    head->add_branch(n2,0);   // manually add fork target node to head node

    node_retval = n2->add_sequence_markov(node, depth+1, head);
   }
  // ------------- execute -fork command



  // ------------- execute -fork_filter command
  bs=branches_string.find("-filter_fork");

  if (bs!=branches_string.end()){

    Node* n1 = bs->second;                         //-filter_fork 

    bs       = n1->branches_string.find("target");
    Node* n2 = bs->second;                         // fork   node

    bs       = n1->branches_string.find("filter");
    Node* n3 = bs->second;                         // filter node 

    int filter=0;
    string not1="-not";

    if ( n3->branches_string.find( (*node)->symbol ) != n3->branches_string.end()   && 
	 n1->branches_string.find( not1 )         == n1->branches_string.end()      ){
	   filter=1;
    }

    if ( n3->branches_string.find( (*node)->symbol ) == n3->branches_string.end()   && 
	 n1->branches_string.find( not1 )         != n1->branches_string.end()      ){
	   filter=1;
    }

    if (filter){

      this->add_branch(n2,0);   // manually add fork target node to head node

      list<Node*>::iterator node1=node; node1++;

      //node_retval = n2->add_sequence_markov(node1, depth+1, head);
      node_retval = n2->add_sequence_markov(node1, depth+1, this);
    }
  }
  // ------------- execute -fork_filter command





  // ------------- execute -assoc_head command
  bs=branches_string.find("-assoc_head");

  if (bs!=branches_string.end()){

    Node* n1 = bs->second;                         //-assoc_head node 

    bs       = n1->branches_string.find("target");
    Node* n2 = bs->second;                         // target   node

    n2->add_branch(head,0);   // manually add fork target node to head node
  }
  // ------------- execute -assoc_head command





  if (depth<1) return this;

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
  //180K  

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





  // --------------------------------------  make an IO parcel using add_seq

  Node IO1("IO1");
  Node* end_node = new Node("-stop");

  list< list<Node*> >::iterator s;                    // sentence
  list<Node*>::iterator w;

  for (s=corpus1.begin();s!=corpus1.end();s++){      // loop through corpus
    s->push_back(end_node);

    IO1.add_sequence_markov(s->begin() ,1,&IO1); 
  }
  
  // --------------------------------------  make an IO parcel using add_seq



  /*
  IO1.output(1,0);cout<<"---"<<endl;
  IO1.output(2,0);cout<<"---"<<endl;
  IO1.output(3,0);cout<<"---"<<endl;


  // only using start of each sentence:

---
IO1
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

  */






   // ---------------------  couple "a", "an", and "the" together

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


  // ---------------------  couple "a" and "the" together


  /*
  IO1.output(1,0);cout<<"1---"<<endl;
  IO1.output(2,0);cout<<"2---"<<endl;
  IO1.output(3,0);cout<<"3---"<<endl;
  IO1.output(4,0);cout<<"4---"<<endl;




3---
IO1
	   6496.the
		      1.-fork
			      3.(a:the)
	   5915.a
		      1.-fork
			      3.(a:the)
	   2950.he
	   2818.The
	   1476.an
		      1.-fork
			      3.(a:the)
	    861.She
	    835.she
	    827.they
	    805.I
	    772.He
	    761.his


  */







  //---------------- reread corpus to execute fork

  IO1.clear_weights();

  for (s=corpus1.begin();s!=corpus1.end();s++){      

    IO1.add_sequence_markov(s->begin() ,1,&IO1); 
  }

  //---------------- reread corpus to execute fork



/*              
 
  IO1.output(1,0);cout<<"1---"<<endl;
  IO1.output(2,0);cout<<"2---"<<endl;
  IO1.output(3,0);cout<<"3---"<<endl;
  IO1.output(4,0);cout<<"4---"<<endl;


// IO1 contains most of the pronouns

IO1
	  13883.(a:the)
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

// ------ hand mine pronouns out of IO1




// ------ hand mine pronouns out of IO1



// ~7K entries in (a:the),  mostly nouns and adjectives

2---
IO1
	  13883.(a:the)
		     48.new
		     44.man
		     41.good
		     40.first
		     34.house
		     33.car
		     32.old
		     31.child
		     30.book
		     30.children
		     29.most
		     28.very
		     26.ship
		     23.great
		     22.whole
		     22.play
		     21.large
		     21.company
		     21.two
		     20.hot
		     20.deep
		     20.long
		     20.government
		     20.last
		     19.cold
		     19.team
		     19.sun
		     18.small
  */


  
  // -------------------------  introduce -filter_fork command
  
  // make a copy of (a:the)
  //
  Node* a_the1       = new Node("(a:the)1");
  map< Node*, float>::iterator b= a_the->branches.begin();
  for(;b!=a_the->branches.end();b++){
    a_the1->add_branch(b->first,b->second);
  }


  Node* filter_fork  = new Node("-filter_fork");    // conditional branch (fork) on
  Node* n_adj1       = new Node("(n_adj1)");        // the filter result

  filter_fork->branches_string.insert(pair<string,Node*> ("target",n_adj1));
  filter_fork->branches_string.insert(pair<string,Node*> ("filter",a_the1));

  a_the->add_branch(filter_fork,0);


  //---------------- reread corpus to execute new code flag
  IO1.clear_weights();

  for (s=corpus1.begin();s!=corpus1.end();s++){      

    IO1.add_sequence_markov(s->begin() ,1,&IO1); 
  }
  //---------------- reread corpus to execute new code flag


  // -------------------------  introduce -filter_fork command


  /*
  IO1.output(1,0);cout<<"1---"<<endl;
  IO1.output(2,0);cout<<"2---"<<endl;
  IO1.output(3,0);cout<<"3---"<<endl;
  IO1.output(4,0);cout<<"4---"<<endl;


// all branches of (a.the) pass filter 

2---
IO1
	  13884.(a:the)
		  13883.(n_adj1)
		     48.new
		     44.man
		     41.good
		     40.first
		     34.house
		     33.car
		     32.old
		     31.child


IO1
	  13884.(a:the)
		  13883.(n_adj1)
			   1586.of
			    550.was
			    329.is
			    169.and
			    128.were
			    109.had
			     83.in
			     74.with

  */




  //----------------------  filter a second time 

  Node* filter_fork1  = new Node("-filter_fork");
  Node* n_adj2        = new Node("!(n_adj2)");

  Node* not1          = new Node("-not");

  filter_fork1->branches_string.insert(pair<string,Node*> ("target",n_adj2));
  filter_fork1->branches_string.insert(pair<string,Node*> ("filter",a_the1));

  filter_fork1->branches_string.insert(pair<string,Node*> ("-not"  ,not1));

  n_adj1->add_branch(filter_fork1,0);


  //---------------- reread corpus to execute new code flag
  IO1.clear_weights();

  for (s=corpus1.begin();s!=corpus1.end();s++){      

    IO1.add_sequence_markov(s->begin() ,1,&IO1); 
  }
  //---------------- reread corpus to execute new code flag


  //----------------------  filter a second time 



  /*

  IO1.output(1,0);cout<<"1---"<<endl;
  IO1.output(2,0);cout<<"2---"<<endl;
  IO1.output(3,0);cout<<"3---"<<endl;
  IO1.output(4,0);cout<<"4---"<<endl;
  IO1.output(5,0);cout<<"5---"<<endl;



4---
IO1
	  13884.(a:the)
		  13884.(n_adj1)
			   6051.(n_adj2)
				    729.of
				    138.in
				    118.to
				    101.was
				     89.is
				     78.the


  // added negation flag


3---
IO1
	  13884.(a:the)
		  13884.(n_adj1)<---------------- these should be mostly nouns             
			   7828.!(n_adj2)
			   1586.of
			    550.was
			    329.is
			    169.and
			    128.were
			    109.had

  */




  //----------------------  new -function to harvest head nodes


  Node* assoc_head1   = new Node("-assoc_head");
  Node* nouns1        = new Node("(nouns1)");

  assoc_head1->branches_string.insert(pair<string,Node*> ("target",nouns1));
 
  n_adj2->add_branch(assoc_head1, 0);


  //---------------- reread corpus to execute new code flag
  IO1.clear_weights();

  for (s=corpus1.begin();s!=corpus1.end();s++){      

    IO1.add_sequence_markov(s->begin() ,1,&IO1); 
  }
  //---------------- reread corpus to execute new code flag

  //----------------------  new -function to harvest head nodes


  nouns1->output(1,0);cout<<"1---"<<endl;
  nouns1->output(2,0);cout<<"2---"<<endl;
  nouns1->output(3,0);cout<<"3---"<<endl;





 
}

