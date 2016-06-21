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





class Node{
public:

  string                           symbol  ;
  
 
  map<Node*,  float>               branches;

  map<string, Node*>               branches_string;


 
  Node*  add_branch( string );
  void   add_branch( Node*  );


  Node*  add_seq_markov( list<string>::iterator word, int depth );

  Node*  input         ( string sentence, int depth )            ;
  Node*  input         ( string sentence, Node*     )            ;


  void   output        ( int  depth,  int tabs )                 ;

  void   clear_weights ()                                        ;
  float  count_weights ()                                        ;//kludge


  Node(){};
  Node(string name){symbol=name;}

}; 



//----------------------------------------
//
float
Node::count_weights(){

  map< Node*, float >::iterator b=branches.begin();

  float sum=0;

  for (;b!=branches.end();b++){

    sum=0;

    map< Node*, float >::iterator b1 = b->first->branches.begin();

    for (;b1!= b->first->branches.end();b1++) sum   +=   b1->second;

    b->second=sum;
  }

  return sum;
}



//----------------------------------------
//
Node*
Node::input( string sentence, Node* node ){

  int depth=0;

  int            s1=0;
  unsigned int   s2=0;
  string         field;

  while (s2!=string::npos) {
    s2 = sentence.find(" ", s1); field = sentence.substr(s1,s2-s1); s1+=s2-s1+1;

    if (field=="") continue;

    depth++;
  }


  Node* n = input(sentence, depth);

  n->add_branch(node);

  return n;


}


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


  bs = branches_string.find("-swap");           //  swap symbol
  if (bs!= branches_string.end()){  

    map< Node*, float>::iterator b1 = bs->second->branches.begin();

    b1->second++;

    return  b1->first->add_seq_markov(word, depth);

  }



  bs = branches_string.find("-insert");         //  insert symbol,
  if (bs!= branches_string.end()){  

    map< Node*, float>::iterator b1 = bs->second->branches.begin();

    list<string> temp;

    temp.push_back(symbol);

    list<string>::iterator t = word;

    for (int i = 0; i!=depth; i++)  temp.push_back(*(t++) );

    return  (b1->first)->add_seq_markov( temp.begin(), depth);
 

  }



  //------------------------------------------

  Node* b = add_branch(*word);

  *word = b->symbol;              //may change

  if (depth==0) return b;


  b = b->add_seq_markov( (++word)--, (--depth)++ );


  //------------------------------------------


                                           // this is insert symbol with fork  
  bs = branches_string.find("-flag3");     // flag3 = filter insert
  if (bs!= branches_string.end()){  

    if ( bs->second->branches_string.find(*word) != 
	 bs->second->branches_string.end() ) {

      b = add_branch(bs->second->symbol);

      //return  b->add_seq_markov( word, depth );
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
  
  bs = branches_string.find("-caps");   // make everything lower case
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


 

 
  IO.add_branch("-caps");   //fix caps

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


 

 

  Node* pronoun = IO.input("(pronoun)",1);

  IO.input( "he    -insert", pronoun );
  IO.input( "she   -insert", pronoun );
  IO.input( "they  -insert", pronoun );
  IO.input( "i     -insert", pronoun );
  IO.input( "we    -insert", pronoun );
  IO.input( "it    -insert", pronoun );
  IO.input( "you   -insert", pronoun );
  IO.input( "there -insert", pronoun );
  IO.input( "this  -insert", pronoun );
  IO.input( "that  -insert", pronoun );
  IO.input( "these -insert", pronoun );


  //--- re-read corpus, corpus=list<string>
  //
  IO.clear_weights();
  for (c=corpus.begin();c!=corpus.end();c++) IO.input(*c,3);


  IO.count_weights();


                 cout<<"ken2"<<endl;
                 cout<<"---"<<endl;
  IO.output(2,0);cout<<"---"<<endl;
  IO.output(3,0);cout<<"---"<<endl;
  IO.output(4,0);cout<<"---"<<endl;
  IO.output(5,0);cout<<"---"<<endl;
  IO.output(6,0);cout<<"---"<<endl;


  /*
IO
	   9755. (pronoun)
	   9313. the
	   5990. a
	   1487. an
	    899. his
	    374. her
	    361. in

   */







  Node* person = IO.input("(person)",1);

  IO.input( "(pronoun)", person );

  IO.input( "(pronoun)  he     -swap", person );
  IO.input( "(pronoun)  she    -swap", person );
  IO.input( "(pronoun)  they   -swap", person );
  IO.input( "(pronoun)  i      -swap", person );
  IO.input( "(pronoun)  we     -swap", person );
  IO.input( "(pronoun)  you    -swap", person );




  Node* thing = IO.input("(thing)",1);

  IO.input( "(pronoun)", thing );

  IO.input( "(pronoun)  it     -swap", thing );

  /*
  IO.input( "(pronoun)  there  -swap", thing );
  IO.input( "(pronoun)  this   -swap", thing );
  IO.input( "(pronoun)  that   -swap", thing );
  IO.input( "(pronoun)  these  -swap", thing );
  */



  //--- re-read corpus, corpus=list<string>
  //
  IO.clear_weights();
  for (c=corpus.begin();c!=corpus.end();c++) IO.input(*c,3);


  IO.     count_weights();
  pronoun->count_weights();


                 cout<<"ken3"<<endl;
                 cout<<"---"<<endl;
  IO.output(2,0);cout<<"---"<<endl;
  IO.output(3,0);cout<<"---"<<endl;
  IO.output(4,0);cout<<"---"<<endl;
  IO.output(5,0);cout<<"---"<<endl;



  /*
	   9755. (pronoun)
		   8135. (person)
		   1620. (thing)


	   9755. (pronoun)
		   8135. (person)
			    668. was
			    298. had
			    273. is
			    155. were
			    111. gave
			    110. has


		   1620. (thing)
			    342. was
			    210. is
			     40. were
			     29. are
			     19. will
   */



  //-------------------------------


  Node* te  = person->input("(te)",1);
  Node* te1 = thing ->input("(te)",1);


  IO.input("(pronoun)  (person)  was   -swap", te);
  IO.input("(pronoun)  (person)  is    -swap", te);
  IO.input("(pronoun)  (person)  had   -swap", te);
  IO.input("(pronoun)  (person)  were  -swap", te);
  IO.input("(pronoun)  (person)  are   -swap", te);
  IO.input("(pronoun)  (person)  has   -swap", te);
  IO.input("(pronoun)  (person)  will  -swap", te);
  IO.input("(pronoun)  (person)  could -swap", te);
  IO.input("(pronoun)  (person)  can   -swap", te);
  IO.input("(pronoun)  (person)  have   -swap", te);
  IO.input("(pronoun)  (person)  cannot -swap", te);
  IO.input("(pronoun)  (person)  did    -swap", te);
  IO.input("(pronoun)  (person)  didn't  -swap", te);
  IO.input("(pronoun)  (person)  don't   -swap", te);
  IO.input("(pronoun)  (person)  can't   -swap", te);
  IO.input("(pronoun)  (person)  am      -swap", te);
  IO.input("(pronoun)  (person)  should  -swap", te);
  IO.input("(pronoun)  (person)  never   -swap", te);
  IO.input("(pronoun)  (person)  couldn't  -swap", te);
  IO.input("(pronoun)  (person)  may    -swap", te);
  IO.input("(pronoun)  (person)  won't  -swap", te);
  IO.input("(pronoun)  (person)  wasn't -swap", te);
  IO.input("(pronoun)  (person)  would -swap", te1);
  IO.input("(pronoun)  (person)  isn't -swap", te1);
  IO.input("(pronoun)  (person)  must   -swap", te);


  IO.input("(pronoun)  (thing)  was   -swap", te1);
  IO.input("(pronoun)  (thing)  is    -swap", te1);
  IO.input("(pronoun)  (thing)  had   -swap", te1);
  IO.input("(pronoun)  (thing)  were  -swap", te1);
  IO.input("(pronoun)  (thing)  are   -swap", te1);
  IO.input("(pronoun)  (thing)  has   -swap", te1);
  IO.input("(pronoun)  (thing)  will  -swap", te1);
  IO.input("(pronoun)  (thing)  could -swap", te1);
  IO.input("(pronoun)  (thing)  can   -swap", te1);
  IO.input("(pronoun)  (thing)  have   -swap", te1);
  IO.input("(pronoun)  (thing)  cannot -swap", te1);
  IO.input("(pronoun)  (thing)  did    -swap", te1);
  IO.input("(pronoun)  (thing)  didn't  -swap", te1);
  IO.input("(pronoun)  (thing)  don't   -swap", te1);
  IO.input("(pronoun)  (thing)  can't   -swap", te1);
  IO.input("(pronoun)  (thing)  am      -swap", te1);
  IO.input("(pronoun)  (thing)  should  -swap", te1);
  IO.input("(pronoun)  (thing)  never   -swap", te1);
  IO.input("(pronoun)  (thing)  couldn't  -swap", te1);
  IO.input("(pronoun)  (thing)  may    -swap", te1);
  IO.input("(pronoun)  (thing)  won't  -swap", te1);
  IO.input("(pronoun)  (thing)  wasn't -swap", te1);
  IO.input("(pronoun)  (thing)  would -swap", te1);
  IO.input("(pronoun)  (thing)  isn't -swap", te1);
  IO.input("(pronoun)  (thing)  must   -swap", te);




  //--- re-read corpus, corpus=list<string>
  //
  IO.clear_weights();
  for (c=corpus.begin();c!=corpus.end();c++) IO.input(*c,4);


  IO.     count_weights();
  pronoun->count_weights();




  (IO.input("(pronoun)  (person)",2) )     ->count_weights();
  (IO.input("(pronoun)  (thing) ",2) )     ->count_weights();


                 cout<<"ken4"<<endl;
                 cout<<"---"<<endl;
  IO.output(2,0);cout<<"---"<<endl;
  IO.output(3,0);cout<<"---"<<endl;
  IO.output(4,0);cout<<"---"<<endl;
  IO.output(5,0);cout<<"---"<<endl;




  /*







	   9757. (pronoun)
		   8137. (person)                           1620. (thing)	      
			   2281. (te)			   	    375. (te)	      
			    111. gave      i		   	      6. takes	      
			    108. took      i		   	      6. took     i   
			     87. got       i		   	      5. came     i   
			     86. made    i		   	      4. seems	      
			     62. felt     i		   	      4. grew    i    
			     59. looked			   	      3. brought     i
			     58. went     i		   	      3. seemed	      
			     58. tried			   	      2. broke     i  
			     50. put     i		   	      2. worked	      
			     49. spoke     i		   	      2. vanished     
			     45. used			   	      2. looked	      
			     44. played			   	      2. set     i    
			     43. worked			   	      2. aroused      
			     37. wanted			   	      2. made     i   
			     35. bought     i		   	      2. happens      
			     34. acted			   	      2. cost     i   
			     29. turned			   	      2. does	      
			     29. heard     i		   	      2. gave     i   
			     29. stood     i		   	      2. produced     
			     28. lost     i		   	      2. sounds	      
			     27. fell     i		   	      1. aided	      
			     27. wrote    i		   	      1. all	      
			     26. ran    i		   	      1. rained	      
			     26. came    i		   	      1. occurred     
			     26. sat    i		   	      1. dawned	      
			     26. walked			   	      1. started      
			     25. asked			   	      1. pays	      
			     24. finally		   	      1. follows      
			     24. found    i		   	      1. behooves     
			     24. lived			   	      1. discharged   
			     24. left    i		   	      1. required     
			     24. kept    i		   	      1. signalled    
			     24. said    i






                111. gave                       108. took	                 87. got		   
	       	     27. the		     	     34. a	      	       	     32. a	   
	       	     14. him		     	     17. the	      	       	      7. the	   
	       	     11. me		     	      4. up	      	       	      6. an	   
	       	     11. it		     	      3. her	      	       	      6. his	   
	       	      9. his		     	      3. advantage    	       	      2. into	   
	       	      6. her		     	      3. his	      	       	      2. in	   
	       	      6. a		     	      3. an	      	       	      2. out	   
	       	      5. us		     	      2. it	      	       	      2. back	   
	       	      3. an		     	      2. me	      	       	      2. through   
	       	      2. them		     	      2. off	      	       	      1. down	   
	       	      1. out		     	      2. lessons      	       	      1. life	   
	       	      1. liberally	     	      2. to	      	       	      1. several   
	       	      1. free		     	      2. their	      	       	      1. change	   
	       	      1. only		     	      2. him	      	       	      1. theater   
	       	      1. notice		     	      1. three	      	       	      1. credit	   
	       	      1. directions	     	      1. mind-altering	       	      1. four	   
	       	      1. credence	     	      1. part	      	       	      1. two	   
	       	      1. vent		     	      1. measures     	       	      1. some	   
	       	      1. evidence	     	      1. what	      	       	      1. by	   
	       	      1. up		     	      1. offence      	       	      1. quizzed   
	       	      1. away		     	      1. along	      	       	      1. nothing   
	       	      1. herself	     	      1. time	      	       	      1. funny	   
	       	      1. full		     	      1. driving      	       	      1. it	   
	       	      1. little		     	      1. aim	      	       	      1. promoted  
	       	      1. two		     	      1. first	      	       	      1. hold	   
	       	      1. at		     	      1. our	      	       	      1. annoyed   
	       	      1. voice		     	      1. care	      	       	      1. carried   
	       								       	      1. along	   
	       								       	      1. rather	   
	       								       	      1. up        




                         (adv)<               
          (n-v)  ------  (n)  <  
                         (te) <  
			 (v) 
			 (p) 
			 (n1)  <                 
			 (x)  <                 

                                                    (v)          (n1)
       IO            (adv)     (n)       (te)       (p)          (n2)             AA / LTM / hypernym       
   ------------      ----     ------     ----       -----        ----             ----------                


                               I                     give        him

 

                               I                     give  

                                                    -to          him




                               (person)             (verb)       (person)



                               (person)             (verb)

                                                   -(prep)       (person)




                only        I                give                  him     
                            I      only      give                  him     
                            I                give        only      him     
                            I                give                  him      only



                only        I                give                  to                   him     
                            I      only      give                  to                   him     
                            I                give        only      to                   him         
                            I                give                  to       only        him     
                            I                give                  to                   him      only





first adverb, verb,  and preposistion



                          I           give                        him

                          I           give          to            him



                      (person)       (verb)       (prep)         (person)






                         (adv)<               
          (n-v)  ------  (n)  <  
                         (te) <  
			 (v) 
			 (p) 
			 (n2)  <                 
			 (x)  <                 



                                                    
       IO            (adv)     (n)       (te)       (v)       (p)         (n2)          AA / LTM / hypernym       
   ------------      ----     ------     ----       -----     ----        -----                


                              I                     give                  him
                              I                     give      to          him



                              (person)              (v)                   (person)
                              (person)              (v)       (prep)      (person)             // (prep) grows in


                              (person)              (v)                   (thing)              // (thing) grows in from
                              (person)              (v)       (prep)      (thing)              // (person) in n2




                                he                  got       in         touch 

                                                              with       his 
                                                                         colleagues 


                                he                  got       in 
                                                              on         the 
                                                                         ground 
                                                                         floor 











                         (adv)<               
          (n-v)  ------  (n)  <  
                         (te) <  
			 (v) 
			 (p) 
			 (n1)  <                 
			 (x)  <                 



                                                    
       IO            (adv)     (n)       (te)       (v)       (p)         (n2)          AA / LTM / hypernym       
   ------------      ----     ------     ----       -----     ----        -----                


                              I                     get                   thing

 

                               I                    get

                                                    -to
                                                     keep                 thing




                               (person)             (verb)       (person)



                               (person)             (verb)

                                                   -(prep)       (person)











                         (adv)<               
          (n-v)  ------  (n)  <  
                         (te) <  
			 (v) 
			 (p) 
			 (n1)  <                 
			 (x)  <                 



                                                    
       IO            (adv)     (n)       (te)       (v)       (p)         (n2)          AA / LTM / hypernym       
   ------------      ----     ------     ----       -----     ----        -----                


         give                   i                   give                  it
                
                                                               -to        him



	 take 
	 get  
	 make 
	 want 
	 buy  
	 put  
	 lose 
	 find 
	 keep 
	 study






    i  give  it
    i  give  it  him

    i  give  it  to  him










    
                                                                      I   (verb)     (prep)    him
                                                                      ----------------------------

9757. (pronoun)			get                                    to                            on
 8137. (person)                 give      feel    act     move         from      of                  in
  2281. (te)	    	      					       ----      -----               -----



   111. gave         give	 -1         	                        1        I give of him	     I give in him		
   108. took         take         1		           					
    87. got          get          1          1          					
    86. made         make         1                                                	
    37. wanted	     want         1	        1	           		   	
    35. bought       buy          1                                                	
    50. put          put         -1		           			   	
    28. lost         lose        -1		                        1	   	
    24. found	     find         1	           				   	
    24. kept	     keep         1	                                           	
    24. studied      study        1	           				   	
  






                                                                        it                             it
                                                                  I (v) him           I   (v)  (prep)  him
                                                                  ---------           --------------------
							                      
9757. (pronoun)			get                                                    to                  on       with 
 8137. (person)                 give      feel    act     move                         from      of        in       for     
  2281. (te)	    	      				      	       	               ----      ----      -----    ----    
							                        
							                        
   111. gave         give	 -1         	                  i  give  him          1                            
   108. took         take         1		                  i  take  it  		1			     
    87. got          get          1          1                    i  get   it  		1			    1  
    86. made         make         1                               i  make  it                                 	    1       
    37. wanted	     want         1	     1	                           		   	                       
    35. bought       buy          1		                  i buy   from  him     1	   	            1       
    50. put          put         -1
    28. lost         lose        -1		                  i put         it                      	   	
    24. found	     find         1	           	      	  i find        it	        	    		      	
    24. kept	     keep         1	                          i keep        it                                   	
    24. studied      study        1	                          i study       it	      		        	    		   	
  		  				              		        	    			   	
  		  				              		        	    			   	
    62. felt         feel                   1                     i feel it                                   	
    59. looked	     look                   1                                            1	   	
    44. played	     play                   1                 		        	    			   	
    49. spoke        speak                  1                                            1	 1  	           1        
    43. worked	     work         1         1                                                        	           1        
    29. heard        hear                   1                 	  i hear     it       	 1   	 1                 1        
    27. wrote        write                  1                     i write on it          1	 1         1       1        
    26. walked	     walk                   1                1	  i walk  to it          1                 1       1        
    24. lived	     live                   1                     i live                                   1       1        
    24. said	     say                    1                                            1			
    26. sat         sit                     1    	 	      	  			
    26. ran          run  		    1                1				
    29. stood        stand		    1             						   
  	          										
  		  				           					
    58. went         go                                       1					
    26. came         come                                     1					
    24. left	     leave                                    1					
    21. moved	     move                                     1 				
    26. ran          run			                 1				
    29. turned	     turn	 		                 1				
    27. fell         fall			                 1				
                                                                                                
                                                                                                
    58. tried	     try	                       1					
    45. used	     use			       1    					
    34. acted	     act			       1    					
    25. asked	     ask          	1	 1     1     					
    22. called	     call			 1     1                 1			
    22. talked	     talk			 1          					
    21. enjoyed     enjoy    	         1  	                				
    21. need	     need	      	1        1 						
    20. held	     hold	      	1        1     1					
    20. threw	     threw	      	1        1     1          				
    20. answered     answered     	1              1					
    19. drew	     draw	      	         1						
    19. told	     told	      	         1     1					
    19. works	     works	      								
    18. built	     built	      	    							
    18. served	     served	      	    							
    18. thought      thought      	    							
    18. broke	     broke	      	    							
    18. wore	     wore	      	    							
    18. saw	     saw	      	    							
    18. behaved      behaved      	    							
    17. received     received     	    							
    16. passed	     passed	      	    							
    16. watched      watched      	    							
    16. caught	     caught	      	    							
    16. followed     followed     	    							
    16. sent	     sent	      	    							
    15. spent	     spent	      	    							
    15. won	     won	      	    							
    15. missed	     missed	      	    							
    15. dropped      dropped      	    							
    15. accepted     accepted     	    							
    14. hit	     hit	      	    							
    14. read	     read	      	    							
    14. managed      managed      	    							
    14. want	     want	      	    							
    13. started      started      	    							
    13. smiled	     smiled	      	    							
    13. pulled	     pulled	      	    							
    13. carried      carried      	    							
    13. paid	     paid	      	    							
    13. lives	     lives	      	    							
    13. live	     live	      	    							
    13. know	     know	      	    							
    12. set	     set	      	    							
    12. plays	     plays	      	    							
                                                                                                
    12. changed      changed      	    							
    12. treated      treated      	    							
    12. knew	     knew	      	    							
    12. performed    performed    	    							
    12. joined	     joined	      	    							
    12. listened     listened     	    							
    12. traveled     traveled     	    							
    11. doesn't      doesn't      	    							
    11. admired      admired      	    							
    11. showed	     showed	      	    							
    11. loved	     loved	      	    							
    11. met	     met	      	    							
    11. began	     began	      	    							
    11. think	     think	      	    							
    10. decided      decided      	    							
    10. signed	     signed	      	    							
    10. applied      applied      	    							
    10. claimed      claimed      	    							
    10. suffered     suffered     	    							
    10. became	     became	      	    							
    10. still	     still	      	    							
    10. expressed    expressed    	    							
    10. arrived      arrived      	    							
    10. needed	     needed	      	    							
    10. learned      learned      	    							
    10. dealt	     dealt	      	    							
    10. picked	     picked	      	    							
    10. often	     often	      	    							
    10. keeps	     keeps	      	    							
     9. takes	     takes	      	    							
     9. faced	     faced	      	    							
     9. collected    collected    	    							
     9. fought	     fought	      	    							
     9. hoped	     hoped	      	    							
     9. checked      checked      	    							
     9. writes	     writes	      	    							
     9. refused      refused      	    							
     9. stuck	     stuck	      	    							
     9. stared	     stared	      	    							
     9. argued	     argued	      	    							
     9. does	     does	      	    							
     9. developed    developed    	    							
     9. rode	     rode	      	    							
     9. offered      offered      	    							
     8. hired	     hired	      	    							
     8. proposed     proposed     	    							
     8. stopped      stopped      	    							
     8. waited	     waited	      	    							
     8. shot	     shot	      	    							
     8. stayed	     stayed	      	    							
     8. liked	     liked	      	    							
     8. feared	     feared	      	    							
     8. added	     added	      	    							
     8. stepped      stepped      	    							
     8. complained   complained   	    							
     8. believed     believed     	    							
     8. all	     all	      	    							
     8. only	     only	      	    							
     8. beat	     beat	      	    							
     8. feel	     feel	      	    							
     8. popped	     popped	      	    							
     8. like	     like	      	    							
     8. searched     searched     	    							
     8. drove	     drove	      	    							
     8. agreed	     agreed	      	    							
     8. sang	     sang	      	    							
     7. blew	     blew	      	    							
     7. questioned   questioned   	    							
     7. finished     finished     	    							
     7. goes	     goes	      	    							
     7. behaves      behaves      	    							
     7. visited      visited      	    							
     7. holds	     holds	      	    							
     7. died	     died	      	    							
     7. disliked     disliked     	    							
     7. remained     remained     	    							
     7. led	     led	      	    							
     7. drank	     drank	      	    							
     7. hated	     hated	      	    							
     7. gets	     gets	      	    							
     7. slipped      slipped      	    							
     7. likes	     likes	      	    							
     7. closed	     closed	      	    							
     7. really	     really	      	    							
     7. cut	     cut	      	    							
     7. walks	     walks	      	    							
     7. brought      brought      	    							
     7. painted      painted      	    							
     6. laid	     laid	      	    							
     6. denied	     denied	      	    							
     6. graduated    graduated    	    							
     6. expected     expected     	    							
     6. thinks	     thinks	      	    							
     6. requested    requested    	    							
     6. opened	     opened	      	    							
     6. awoke	     awoke	      	    							
     6. acquired     acquired     	    							
     6. lay	     lay	      	    							
     6. rose	     rose	      	    							
     6. needs	     needs	      	    							
     6. charged      charged      	    							
     6. ate	     ate	      	    							
     6. longed	     longed	      	    							
     6. ordered      ordered      	    							
     6. taught	     taught	      	    							
     6. ignored      ignored      	    							
     6. look	     look	      	    							
     6. grew	     grew	      	    							
     6. hold	     hold	      	    							
     6. invited      invited      	    							
     6. wouldn't     wouldn't     	    							
     6. say	     say	      	    							
     6. makes	     makes	      	    							
     6. struck	     struck	      	    							
     6. tested	     tested	      	    							
     6. hope	     hope	      	    							
     6. wish	     wish	      	    							
     6. just	     just	      	    							
     6. knows	     knows	      	    							
     6. continued    continued    	    							
     6. acts	     acts	      	    							
     6. measured     measured     	    							
     6. waved	     waved	      	    							
     6. shall	     shall	      	    							
     6. trained      trained      	    							
     6. rolled	     rolled	      	    							
     5. supported    supported    	    							
     5. warned	     warned	      	    							
     5. criticized   criticized   	    							
     5. recorded     recorded     	    							
     5. laughed      laughed      	    							
     5. submitted    submitted    	    							
     5. entered      entered      	    							
     5. hasn't	     hasn't	      	    							
     5. paused	     paused	      	    							
     5. remembered   remembered   	    							
     5. published    published    	    							
     5. prepared     prepared     	    							
     5. climbed      climbed      	    							
     5. prefers      prefers      	    							
     5. quickly      quickly      	    							
     5. rejected     rejected     	    							
     5. practiced    practiced    	    							
     5. grabbed      grabbed      	    							
     5. treats	     treats	      	    							
     5. sank	     sank	      	    							
     5. shared	     shared	      	    							
     5. swallowed    swallowed    	    							
     5. discussed    discussed    	    							
     5. wants	     wants	      	    							
     5. considered   considered   	    							
     5. dashed	     dashed	      	    							
     5. cried	     cried	      	    							
     5. rarely	     rarely	      	    							
     5. dipped	     dipped	      	    							
     5. haven't      haven't      	    							
     5. appeared     appeared     	    							
     5. sailed	     sailed	      	    							
     5. adopted      adopted      	    							
     5. assumed      assumed      	    							
     5. let	     let	      	    							
     5. attacked     attacked     	    							
     5. voted	     voted	      	    							
     5. love	     love	      	    							
     5. try	     try	      	    							
     5. settled      settled      	    							
     5. squeezed     squeezed     	    							
     5. do	     do	      	    								
     5. bowed	     bowed	      	    							
     5. demanded     demanded     	    							
     5. nodded	     nodded	      	    							
     5. killed	     killed	      	    							
     5. pushed	     pushed	      	    							
     5. work	     work	      	    							
     4. edited	     edited	      	    							
     4. helped	     helped	      	    							
     4. nailed	     nailed	      	    							
     4. returned     returned     	    							
     4. apologized   apologized   	    							
     4. retired      retired      	    							
     4. uses	     uses	      	    							
     4. resented     resented     	    							
     4. observed     observed     	    							
     4. placed	     placed	      	    							
     4. stated	     stated	      	    							
     4. believes     believes     	    							
     4. comes	     comes	      	    							
     4. handled      handled      	    							
     4. knocked      knocked      	    							
     4. pointed      pointed      	    							
     4. examined     examined     	    							
     4. hadn't	     hadn't	      	    							
     4. insisted     insisted     	    							
     4. objected     objected     	    							
     4. described    described    	    							
     4. buried	     buried	      	    							
     4. pitched      pitched      	    							
     4. entertaine   entertained  	    							
     4. jumped	     jumped	      	    							
     4. topped	     topped	      	    							
     4. dug	     dug	      	    							
     4. reviewed     reviewed     	    							
     4. envied	     envied	      	    							
     4. proceeded    proceeded    	    							
     4. provided     provided     	    							
     4. debated      debated      	    							
     4. reacted      reacted      	    							
     4. kissed	     kissed	      	    							
     4. sold	     sold	      	    							
     4. earned	     earned	      	    							
     4. eked	     eked	      	    							
     4. reads	     reads	      	    							
     4. scored	     scored	      	    							
     4. seems	     seems	      	    							
     4. drives	     drives	      	    							
     4. seemed	     seemed	      	    							
     4. explained    explained    	    							
     4. escaped      escaped      	    							
     4. detected     detected     	    							
     4. adjusted     adjusted     	    							
     4. believe      believe      	    							
     4. shook	     shook	      	    							
     4. dresses      dresses      	    							
     4. fear	     fear	      	    							
     4. carry	     carry	      	    							
     4. screamed     screamed     	    							
     4. noted	     noted	      	
     4. backed      backed              




IO2
	 305102.(a:the)_rev

		  43596.of

		  11630.to
		   5174.from

		   8336.with
		   7500.on
		   6534.by
		   5576.as

		   4696.for

		   3522.at

		  22612.in
		   2578.into
		    998.through
		    946.between
		    872.over
		    760.during
		    756.after
		    652.around
		    572.under
		    540.up
		    474.near
		    456.about
		    420.off
		    416.along
		    382.toward
		    382.against
		    376.within






  


As we will see on this page, verbs are classified in many ways. 

First, some verbs require an object to complete their meaning: "She gave _____ ?" Gave what? She gave money to the church. 

These verbs are called transitive. 

Verbs that are intransitive do not require objects: "The building collapsed." 

In English, you cannot tell the difference between a transitive and intransitive verb by its form; 
you have to see how the verb is functioning within the sentence. 
In fact, a verb can be both transitive and intransitive: "The monster collapsed the building by sitting on it."





That horrid music gave me a headache.



Verbs are also classified as either finite or non-finite. 
A finite verb makes an assertion or expresses a state of being and can stand by itself as the main verb of a sentence.

The truck demolished the restaurant.

The leaves were yellow and sickly.


Non-finite verbs (think "unfinished") cannot, by themselves, be main verbs:

The broken window . . .

The wheezing gentleman . . .







                         (adv)<               
          (n-v)  ------  (n)  <  
                         (te) <  
			 (v) 
			 (p) 
			 (n1)  <                 
			 (x)  <                 



                                                    
       IO            (adv)     (n)       (te)       (v)       (p)         (n2)          AA / LTM / hypernym       
   ------------      ----     ------     ----       -----     ----        -----                


                              the
                              broken
                              window


                              the
                              window                broke



                              the
                              hunted
                              eagle


                              the   
                              eagle      was         hunt ed

                              
                              I                      hunt   ed          eagle
                              I                      hunt               eagle












Those people      are   all 
                        professors.

Those professors  are   brilliant.

This  
room                    smells     bad.

I feel great.
A victory today seems unlikely.

A handful of verbs that reflect a change in state of being are sometimes called resulting copulas. 
They, too, link a subject to a predicate adjective:

His face turned purple.
She became older.
The dogs ran wild.
The milk has gone sour.
The crowd grew ugly.










Major thematic relations[edit]
Here is a list of the major thematic relations.[2]

Agent: deliberately performs the action (e.g., Bill ate his soup quietly.).
Experiencer: the entity that receives sensory or emotional input (e.g. Susan heard the song. I cried.).
Stimulus: Entity that prompts sensory or emotional feeling - not deliberately (e.g. Kim detests sprouts ).
Theme: undergoes the action but does not change its state (e.g., We believe in one God. I have two children. I put the book on the table. He gave the gun to the police officer.) (Sometimes used interchangeably with patient.)
Patient: undergoes the action and changes its state (e.g., The falling rocks crushed the car.). (Sometimes used interchangeably with theme.)
Instrument: used to carry out the action (e.g., Jamie cut the ribbon with a pair of scissors.).
Force or Natural Cause: mindlessly performs the action (e.g., An avalanche destroyed the ancient temple.).
Location: where the action occurs (e.g., Johnny and Linda played carelessly in the park. I'll be at Julie's house studying for my test.).
Direction or Goal: where the action is directed towards (e.g., The caravan continued on toward the distant oasis. He walked to school.).
Recipient: a special kind of goal associated with verbs expressing a change in ownership, possession. (E.g., I sent John the letter. He gave the book to her.)
The Source or The Origin: where the action originated (e.g., The rocket was launched from Central Command. She walked away from him.).
Time: the time at which the action occurs (e.g., The rocket was launched yesterday.).
Beneficiary: the entity for whose benefit the action occurs (e.g.. I baked Reggie a cake. He built a car for me. I fight for the king.).
Manner: the way in which an action is carried out (e.g., With great urgency, Tabitha phoned 911.).
Purpose: the reason for which an action is performed (e.g., Tabitha phoned 911 right away in order to get some help.).
Cause: what caused the action to occur in the first place; not for what, rather because of what (e.g., Because Clyde was hungry, he ate the cake.).






greed envy wrath sloth glutonly vanity lust hubris




































































   */












  /*

localize the trigger node
    
timestamps on flag nodes





  Node* pronoun = IO.input("(pronoun)");



  IO.input( "he  -swap", pronoun );


       he    ran a mile   -->    (pronoun)  ran a mile
				 



  IO.input( "he  -insert", pronoun );     
  IO.input( "she -insert", pronoun );



       he    ran a mile   -->    (pronoun)  he  ran a mile
				 















IO
               
                he   
		  -insert  
		         (pronoun)


	   9313. the
	   6100. (pronoun)
		   2950. he
		    835. she
		    827. they
		    450. it
		    311. we
		    250. there
		    241. this
		    104. you
		     79. that
		     52. these
		      1. i
	   5990. a
	   1487. an
	    899. his
	    861. she
	    805. i
	    772. he
	    393. we



Node pro1(-filter);
     pro1.symbol=(pro)

     pro1.input(he)
     pro1.input(she)


    IO.input("-code", pro1 );


  */






















  /*

  f2 = IO.input("(pronoun) -flag3",2);

  f2->symbol="(person)";



  IO.input("(pronoun) (person) i"   ,2);
  IO.input("(pronoun) (person) you" ,2);
  IO.input("(pronoun) (person) he"  ,2);
  IO.input("(pronoun) (person) she" ,2);
  IO.input("(pronoun) (person) we"  ,2);
  IO.input("(pronoun) (person) they",2);
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



  Biologically Inspired Parcel-Graph Model of Human Thought



   */
