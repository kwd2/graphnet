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


  void  output       (  int  depth,  int tabs     );

  void  clear_weights();

  Node(){};
  Node(string name){symbol=name;}

}; 


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
Node::add_seq_markov( list<string>::iterator word, int depth ){

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

      Node* b = add_branch(bs->second->symbol);

      //cout<<"flag3 "<<symbol<<"  "<<*word<<"  "<<depth<<endl;


      return  b->add_seq_markov( word, depth );
    }
  }


  return b;


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


 





 


  cout<<"ken1"<<endl;

  cout<<"---"<<endl;
  IO.output(2,0);cout<<"---"<<endl;
  cout<<"---"<<endl;
  IO.output(3,0);cout<<"---"<<endl;
  cout<<"---"<<endl;
  IO.output(4,0);cout<<"---"<<endl;




  list<string> sequence;


  sequence.push_back("-flag3" );


  Node* f2 = IO.add_seq_markov(sequence.begin(), sequence.size()-1);

  cout<< f2->symbol<<endl;

  f2->symbol="(pro_3)";

  f2->add_branch("he");	  
  f2->add_branch("she");  
  f2->add_branch("they"); 
  f2->add_branch("i");	  
  f2->add_branch("we");	  
  f2->add_branch("it");	  
  f2->add_branch("this"); 
  f2->add_branch("that"); 
  f2->add_branch("there");
  f2->add_branch("these");
  f2->add_branch("you");

  //--                                       loop through corpus

  IO.clear_weights();
  for (c1=corpus1.begin();c1!=corpus1.end();c1++){          

    w=c1->begin();
 
    if (c1->size()>1)
      IO.add_seq_markov( w, 1 );                                   
  }


  //--

  cout<<"ken2"<<endl;

  cout<<"---"<<endl;
  IO.output(2,0);cout<<"---"<<endl;
  cout<<"---"<<endl;
  IO.output(3,0);cout<<"---"<<endl;
  cout<<"---"<<endl;
  IO.output(4,0);cout<<"---"<<endl;


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









  sequence.clear    (          );
  sequence.push_back("(pro_3)" );
  sequence.push_back("-flag2"  );

  f2 = IO.add_seq_markov(sequence.begin(), sequence.size()-1);

  f2->symbol="(pro_2)";

  f2->add_branch("he");
  f2->add_branch("she");  
  f2->add_branch("they"); 
  f2->add_branch("i");	  
  f2->add_branch("we");	  
  f2->add_branch("it");	  
  f2->add_branch("this"); 
  f2->add_branch("that"); 
  f2->add_branch("there");
  f2->add_branch("these");
  f2->add_branch("you");	  






  list<string> sent;

  //-- loop through corpus
  //
  IO.clear_weights();
  for (c1=corpus1.begin();c1!=corpus1.end();c1++){          

    w=c1->begin();          sent.clear();     
    for (;w!=c1->end();w++) sent.push_back(*w);

    if (c1->size()>2)  IO.add_seq_markov( sent.begin(), 2 );   
  }
  //-- loop through corpus



  cout<<"ken3"<<endl;

  cout<<"---"<<endl;
  IO.output(2,0);cout<<"---"<<endl;
  cout<<"---"<<endl;
  IO.output(3,0);cout<<"---"<<endl;
  cout<<"---"<<endl;
  IO.output(4,0);cout<<"---"<<endl;


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



ken3
---
IO
	   9751. (pro_3)
		   9751. (pro_2)
			   1010. was
			    483. is
			    303. had
			    195. were
			    131. are
			    119. has...

		      0. (pro_2)
		      0. i
		      0. they
		      0. she
		      0. he
		      0. you
		      0. these
		      0. this
		      0. it
		      0. that
		      0. we
		      0. there
	   9313. the




   */



  sequence.clear()              ;
  sequence.push_back("(pro_3)" );
  sequence.push_back("(pro_2)" );
  sequence.push_back("-flag2"  );

  f2 = IO.add_seq_markov(sequence.begin(), sequence.size()-1);

  f2->symbol="(te_3)";


  f2->add_branch("was");
  f2->add_branch("is");
  f2->add_branch("had");
  f2->add_branch("were");
  f2->add_branch("are");
  f2->add_branch("has");
  f2->add_branch("could");
  f2->add_branch("did");
  f2->add_branch("can");
  f2->add_branch("have");
  f2->add_branch("must");
  f2->add_branch("will");
  f2->add_branch("would");
  f2->add_branch("may");
  f2->add_branch("can't");
  f2->add_branch("cannot");
  f2->add_branch("couldn't");
  f2->add_branch("never");
  f2->add_branch("wasn't");
  f2->add_branch("don't");
  f2->add_branch("doesn't");


   //-- loop through corpus
  //
  IO.clear_weights();
  for (c1=corpus1.begin();c1!=corpus1.end();c1++){          

    w=c1->begin();          sent.clear();     
    for (;w!=c1->end();w++) sent.push_back(*w);

    if (c1->size()>2)  IO.add_seq_markov( sent.begin(), 2 );   
  }
  //-- loop through corpus

  /*
IO
	   9751. (pro_3)
		   9751. (pro_2)
			   2966. (te_3)
				   1010. was
				    483. is
				    303. had
				    195. were
				    131. are
				    119. has
				     96. have
				     95. could
				     85. must
				     68. will
				     67. can
				     60. cannot
				     58. did
				     43. don't
				     35. can't
				     29. never
				     29. would
				     20. couldn't
				     19. may
				     12. doesn't
				      9. wasn't
			   1010. was
			   ...


---
IO
	   9751. (pro_3)
		   9751. (pro_2)
			   2966. (te_2)
			    114. took
			    113. gave
			     88. made
			     87. got
			     62. felt
			     61. looked
			     58. tried
			     58. went
			     51. didn't
			     50. put
			     49. always
			     49. spoke
			     46. used
			     45. worked
			     44. played
			     37. wanted	
			     35. bought
			     34. acted
			     33. am
			     32. should
			     32. came
			     30. turned
			     29. heard
			     29. stood
			     28. fell
			     28. lost
			     27. wrote
			     26. walked
			     26. left
			     26. sat
			     26. ran
			     25. asked
			     24. said
			     24. finally

   */

  cout<<"ken4"<<endl;

  cout<<"---"<<endl;
  IO.output(2,0);cout<<"---"<<endl;
  cout<<"---"<<endl;
  IO.output(3,0);cout<<"---"<<endl;
  cout<<"---"<<endl;
  IO.output(4,0);cout<<"---"<<endl;
  cout<<"---"<<endl;
  IO.output(5,0);cout<<"---"<<endl;
  cout<<"---"<<endl;
  IO.output(6,0);cout<<"---"<<endl;
  cout<<"---"<<endl;
  IO.output(7,0);cout<<"---"<<endl;




  Node IO1("IO1");


  sequence.clear    (          );
  sequence.push_back("-flag1"  );

  f2 = IO1.add_seq_markov(sequence.begin(), sequence.size()-1);


  sequence.clear    (          );
  sequence.push_back("-flag2"  );

  f2 = IO1.add_seq_markov(sequence.begin(), sequence.size()-1);

  f2->symbol="(pro_2)";

  f2->add_branch("he");	  
  f2->add_branch("she");  
  f2->add_branch("they"); 
  f2->add_branch("i");	  
  f2->add_branch("we");	  
  f2->add_branch("it");	  
  f2->add_branch("this"); 
  f2->add_branch("that"); 
  f2->add_branch("there");
  f2->add_branch("these");
  f2->add_branch("you");


  sequence.clear    (          );
  sequence.push_back("(pro_2)"  );
  sequence.push_back("-flag2"  );

  f2 = IO1.add_seq_markov(sequence.begin(), sequence.size()-1);

  f2->symbol="(te_2)";

  f2->add_branch("was");
  f2->add_branch("is");
  f2->add_branch("had");
  f2->add_branch("were");
  f2->add_branch("are");
  f2->add_branch("has");
  f2->add_branch("could");
  f2->add_branch("did");
  f2->add_branch("can");
  f2->add_branch("have");
  f2->add_branch("must");
  f2->add_branch("will");
  f2->add_branch("would");
  f2->add_branch("may");
  f2->add_branch("can't");
  f2->add_branch("cannot");
  f2->add_branch("couldn't");
  f2->add_branch("never");
  f2->add_branch("wasn't");
  f2->add_branch("don't");
  f2->add_branch("doesn't");




  /*
   //-- loop through corpus
  //
  IO.clear_weights();
  for (c1=corpus1.begin();c1!=corpus1.end();c1++){          

    w=c1->begin();          sent.clear();     
    for (;w!=c1->end();w++) sent.push_back(*w);

    if (c1->size()>2)  IO1.add_seq_markov( sent.begin(), 2 );   
  }
  //-- loop through corpus
  */


  cout<<"ken5"<<endl;

  cout<<"---"<<endl;
  IO1.output(2,0);cout<<"---"<<endl;
  cout<<"---"<<endl;
  IO1.output(3,0);cout<<"---"<<endl;
  cout<<"---"<<endl;
  IO1.output(4,0);cout<<"---"<<endl;
  cout<<"---"<<endl;
 
  /*

-ken5
---
IO1
	   9752. (pro_2)
	   9313. the
	   5988. a
	   1486. an
	    898. his
	    373. her
	    357. in
	    189. my
	    116. their
	    111. don't
	    109. our
	    106. after
	    103. what
	    101. when
	     93. some
	     92. can
	     85. all
--
IO1
	   9752. (pro_2)
		   2966. (te_2)
		    114. took
		    113. gave
		     88. made
		     87. got
		     62. felt
		     61. looked
		     58. tried
		     58. went
		     51. didn't
		     50. put
		     49. spoke
		     49. always
		     46. used
		     45. worked
		     44. played
		     37. wanted
		     35. bought
		     34. acted
		     33. am
		     32. came
		     32. should
		     30. turned
		     29. heard
		     29. stood
		     28. lost
		     28. fell
		     27. wrote
		     26. sat
		     26. ran
		     26. left
		     26. walked
		     25. asked
		     24. finally

IO1
	   9752. (pro_2)
		   2966. (te_2)
			    383. a
			    112. to
			     99. the
			     88. not
			     61. an
			     53. no
			     37. be
			     37. in
			     22. always
			     22. been
			     20. never
			     18. his
			     17. see
			     17. have
			     16. know
			     16. too
			     15. still
			     14. all
			     12. only
			     11. do
			     11. hear
			     11. get
			     11. so
			     10. two
			     10. on
			     10. just
			      9. take
			      9. at
			      9. well
			      8. many
			      8. very
			      7. one


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
