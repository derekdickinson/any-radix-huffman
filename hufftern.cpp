/* 
A Turbo C++ program to generate Huffman codes for EE 707 by Derek Dickinson

This program creates a binary Huffman code from a list of probabilities. 
Unlike most "by hand" techniques the codewords are created as the 
probabilities are combined. Thus, once the last two elements are combined 
all the codewords are complete.

***************************************************************************/
#include <conio.h>
#include <fstream.h>
#include <iomanip.h>
#include <iostream.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define SETID()     setw(4) << resetiosflags(ios::left)
#define SETP(width) setw(width) << setiosflags(ios::left)
#define GETCH()	    { pout << flush; getch(); }

enum boolean { FALSE, TRUE };
/**************************************************************************/
typedef unsigned char cval;

// This is an element in the linked list which represents the code for a
// codeword. Each node represents one digit in the code.
struct CODE
{
	cval val;	    // Value of code [0,1,2]
	CODE *next;		// Pointer to the rest of the codeword or NULL
	CODE(cval Ival, CODE *Inext=NULL) {	val=Ival; next=Inext; };
};

// One for each codeword, array of these is used throughout program to store
// input values, results and for display.
struct CODEWORD
{
	int id;    // i.e. which value am i=1,2,....,n
	float prob; 	// The probability of the this word occuring
	CODE *code;		// The linked list containing code for this word
	CODEWORD();		// For array initialization
};

CODEWORD::CODEWORD() { id=0; prob=1.0; code=NULL; };

// The word "item" refers to an element in the list from which the two
// lowest probabilities are selected. A sorted linked list of these items
// is used in the "huffman" function to generate the huffman code.

// Linked list used to identify the codewords which need their code's 
// modified when a new item has been created.
struct WORD
{
	CODEWORD *theword;	// Pointer to a codeword whose code needs modification
	WORD *next;					// Pointer to other codewords which need modification
	WORD(CODEWORD *Iword) { theword=Iword; next=NULL; }
};

// The list of items 
struct ITEM
{
	float prob;		// Sum of probabilities of words in wlist
	WORD *wlist;	// List of pointers to codewords which this node represents
	ITEM *next;		// Pointer to the rest of the item list
	ITEM(CODEWORD *which)
		{	prob=which->prob;	wlist=new WORD(which); next=NULL; }
	ITEM(float Iprob, WORD *Iwlist)
		{	prob=Iprob;	wlist=Iwlist; next=NULL; }
};

/***************************************************************************/
// Function Prototypes

// Input functions
CODEWORD *getinput(void);

// Output Functions
ostream& operator << (ostream& s,CODEWORD *cword);
ostream& operator << (ostream& s,ITEM *ilist);
void displayout(CODEWORD *wordlist);
void dummy(void);
void dispitems0(ITEM *ilist);
void dispitems1(ITEM *ilist);

// Huffman code related functions
ITEM *insert(ITEM *list,ITEM *item);
WORD **updatew(WORD *wlist,cval val);
void huffman(CODEWORD *list);

// Additional goodies
void dispentnbar(CODEWORD *wordlist);

/***************************************************************************/
/* Global Variables */
ofstream pout;        // Output stream variable, used for easy redirection
boolean conout=TRUE;  // True if output is going to screen
int n;								// Number of words in the code

 // Pointer to display function
void (*dispitems)(ITEM *ilist)=(void (*)(ITEM *)) dummy;

/***************************************************************************/
// Input routine
/***************************************************************************/

/* 
Input has the form : 

n prob1 prob2 prob3 ... probn

Whitespace between numbers or seperate lines can be used.*/
CODEWORD *getinput(char *inpname)
{
	boolean conin;
	char filename[100];
	CODEWORD *list,*lptr;
	float prsum=0;
	ifstream infile;
	int i;

	// if not passed a filename then prompt user
	if (*inpname!=0) strcpy(filename,inpname);
	else
	{
		cout << "Input the filename for the probabilities [or CON]: ";
		cin >> filename; 
	}

	infile.open(filename);
	if (!infile) 
	{
		cout << "Unable to open input file [" << filename << "].";
		exit(1);
	}

	conin=(boolean) (stricmp(filename,"CON")==0);

	if (conin) cout << "Input the number of elements:";
	infile >> n;

	lptr=list=new CODEWORD [n+1];
	for (i=0; i<n; i++,lptr++)
	{
		if (conin) cout << "Input probability for word " << (i+1) << " :";
		infile >> lptr->prob;
		prsum+=lptr->prob;
		lptr->id=i+1; 
		lptr->code=NULL;
	}
	infile.close();     

	pout << "The probabilities sum to : " << prsum << "\n";

	return (list);
}

/***************************************************************************/
// Output routines
/***************************************************************************/

// Standard output for CODEWORD structure when used with "<<" operator
ostream& operator << (ostream& s,CODEWORD *cword)
{
	CODE *cptr;
	s << SETID() << cword->id << " ³ " << SETP(11) << cword->prob << " ³ ";
	for (cptr=cword->code; cptr!=NULL; cptr=cptr->next)	s << (int) cptr->val;
	s << '\n';
	return s;
}

// Standard output for ITEM structure when used with "<<" operator
ostream& operator << (ostream& s,ITEM *ilist)
{
	WORD *wptr;
	s << SETP(5) << ilist->prob << " ³ ";
	for(wptr=ilist->wlist; wptr!=NULL; wptr=wptr->next)
		s << wptr->theword->id << ' ';
	s << '\n';
	return s;
}

// Displays codeword list in order of array
void displayout(CODEWORD *wordlist)
{
	int i;
	pout << "\nWord ³ Probability ³ Code\n";
	for (i=1 ;wordlist->id!=0; i++, wordlist++) pout << wordlist;
}

// Dummy function used with function pointer
void dummy(void) {};

// Displays list of items, Used for "-V"
void dispitems0(ITEM *ilist)
{
	int i;

	pout << "ÄÄÄÄÄÅÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ\n"
		<< "Item ³ Prob. ³ Words\n";

	for (i=1;ilist!=NULL; ilist=ilist->next, i++)
		pout << SETID() << i << " ³ " << ilist;
	if (conout) GETCH();
}

// Displays list of items, Used for "-V2"
// Even more verbose then "-V", Normally used when sending results to disk
void dispitems1(ITEM *ilist)
{
	int i;
	WORD *wptr;

	pout << "\nÄÄÄÄÄÅÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ"
		<< "\nItem ³ Prob. ³ Words\n";

	for (i=1;ilist!=NULL; ilist=ilist->next, i++)
	{
		pout << SETID() << i << " ³ " << ilist;
		for(wptr=ilist->wlist; wptr!=NULL; wptr=wptr->next)
			pout << "     word - " << wptr->theword; 	
	}
	if (conout) GETCH();
}

/***************************************************************************/
// Huffman code routines 
/***************************************************************************/

// Inserts "item" into "list" in the order dictated by the probability field.
ITEM *insert(ITEM *list,ITEM *item)
{
	ITEM *its,**bits;
	bits=&list;
	for(its=list; its!=NULL && item->prob>its->prob; its=its->next ) 
		bits=&(its->next); 
	item->next=its;
	*bits=item;
	return (list);
}

// Inserts code digit into front of codes for all nodes in wlist
// Returns last value in list so that the list can have another list appended
// to it

#define CODEW(w)		(w->theword->code)

WORD **updatew(WORD *wlist,cval val)
{
	WORD **retval;

	// Traverse linked list of words modifying all of them
	for (;wlist!=NULL;wlist=wlist->next)
	{
		// Create node containing new bit and insert it in front of list
		CODEW(wlist)=new CODE(val,CODEW(wlist));

		// Keep track of last element in list for return value
		retval=&(wlist->next);
	}
	return (retval);
}

// A couple of defines to make certain lines semi-readable
#define NEWILIST()		(ilist->next->next->next)
#define NEWILIST2()		(ilist->next->next)
#define PROBSUM()			(ilist->prob+ilist->next->prob+ilist->next->next->prob)
#define PROBSUM2()		(ilist->prob+ilist->next->prob)
#define ZEROBRANCH()	(ilist->next->next->wlist)
#define ONEBRANCH()		(ilist->next->wlist)
#define TWOBRANCH()		(ilist->wlist)

// The main logic of the huffman code. "list" is an array of structures 
// representing codewords
void huffman(CODEWORD *list)
{
	ITEM *ilist=NULL, *nlist;

	// Create linked and sorted item list (Sorted smallest to largest)
	for (;list->id!=0;list++)	ilist=insert(ilist,new ITEM(list));

	dispitems(ilist); // Display the original item list if verbose

	if (!(n & 1)) // See Gallager's Info. Theory and Rel. Comm. Page 55
	{
		*(updatew(ONEBRANCH(),0))=TWOBRANCH();
		updatew(TWOBRANCH(),1);
		nlist=insert(NEWILIST2(),new ITEM(PROBSUM2(),ONEBRANCH()));
		delete ilist->next;	delete ilist;
		ilist=nlist;
		dispitems(ilist);
	}

	while (ilist->next!=NULL)
	{
		// Least probable branch nodes are always first three
		*(updatew(ZEROBRANCH(),0))=ONEBRANCH();
		*(updatew(ONEBRANCH(),1))=TWOBRANCH();
		updatew(TWOBRANCH(),2);

		// Make new node that represents the other three
		nlist=insert(NEWILIST(),new ITEM(PROBSUM(),ZEROBRANCH()));

		// free memory associated with old nodes and update ilist
		delete ilist->next->next; delete ilist->next;	delete ilist;
		ilist=nlist;

		dispitems(ilist);
	}
	// Free uneeded memory 
	{
		WORD *wlist,*wback;
		wlist=ilist->wlist;
		for(wback=NULL; wlist!=NULL; wlist=wlist->next) 
			{ delete wback; wback=wlist; }
		delete ilist;
	}
}

/***************************************************************************/
// Routine to calculate the entropy and weighted average word length
/***************************************************************************/

void dispentnbar(CODEWORD *wordlist)
{
	float entropy=0,nbar=0,K;
	int n;
	CODE *c;

	K=-1/log(3);

	for (; wordlist->id!=0; wordlist++)
	{
		entropy+=(K*wordlist->prob*log(wordlist->prob));
		for(n=0,c=wordlist->code; c!=NULL; n++, c=c->next);
		nbar+=(wordlist->prob*n);
	}

	pout << "\nThe minimum word length [H(x)/log(3)] is : " << entropy 
			 << "\n         The weighted average [n bar] is : " << nbar << '\n';
}

/***************************************************************************/
// The main program
/***************************************************************************
"argc" is the number of paramters passed to the program on the command line.
This program "requires" no parameters although it will accept several. 
These are:
	inputfilename - if a name is put on the command line it is assumed to 
		be the name of a file from which to get the probabilities.
	/outputfilename - if a filename is preceded with a "/" then the output
		is sent to a file by that name, otherwise output goes to the screen
	-V - If "-V" is on the command line then a verbose output is used in 
		which intermediate steps of the process are displayed.
	-V2 - If "-V2" is even more verbose than "-V" 
***************************************************************************/
void main(int argc, char *argv[])
{
	CODEWORD *wordlist;
	char *inpname=NULL;
	int i;

	clrscr();

	for (i=1; i<argc; i++)
	{
		strupr(argv[i]);
		switch (argv[i][0])
		{
			case '/': pout.open(&argv[i][1]); conout=FALSE; break;
			case '-': 
				if (argv[i][1]=='V')
				{
					if (argv[i][2]=='2') dispitems=dispitems1;
					else dispitems=dispitems0;
					break;
				}
			default: inpname=argv[i];
		}
	}
	if (conout) pout.open("CON");

	wordlist=getinput(inpname);	// Get table of probabilities
	huffman(wordlist);					// Generate huffman code
	displayout(wordlist);				// Display the results
	dispentnbar(wordlist); 			// Calculate and display entropy and n bar

	if (conout) GETCH();
	pout.close();
}
