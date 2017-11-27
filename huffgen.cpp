/* 
A Turbo C++ program to generate Huffman codes for EE 707 by Derek Dickinson

This program creates a GENERAL Huffman code from a list of probabilities.
The first value entered is "D" the number of possible values for the each
digit of the resulting code (2 for a binary code). 

Unlike most "by hand" techniques the codewords are created as the 
probabilities are combined. Thus, once the last D elements are combined 
all the codewords are complete.

***************************************************************************/
#include <conio.h>
#include <stdio.h>
#include <fstream.h>
#include <iomanip.h>
#include <iostream.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define SETID()     setw(4) << resetiosflags(ios::left)
#define SETP(width) setw(width) << setiosflags(ios::left)

/**************************************************************************/
enum boolean { FALSE, TRUE };
typedef unsigned char cval;
/**************************************************************************/
// Structure Prototypes
struct CODEWORD;
struct WORD;
struct ITEM;

// Function Prototypes
void *checknew(size_t s); // Overflow Error Check Function
CODEWORD *getinput(void); // Input Function

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
float psum(ITEM *l,int i);
void delist(ITEM *start,ITEM *end);
void huffman(CODEWORD *list);

// Additional goodies
void dispentnbar(CODEWORD *wordlist);

/***************************************************************************/
// Structures

// This is an element in the linked list which represents the code for a
// codeword. Each node represents one digit in the code.
struct CODE
{
	cval val;	    // Value of code [0,1,2]
	CODE *next;		// Pointer to the rest of the codeword or NULL
	CODE(cval Ival, CODE *Inext=NULL) {	val=Ival; next=Inext; };
	void* operator new(size_t s) { return (checknew(s)); }
};

// One for each codeword, array of these is used throughout program to store
// input values, results and for display.
struct CODEWORD
{
	int id;    // i.e. which value am i=1,2,....,n
	float prob; 	// The probability of the this word occuring
	CODE *code;		// The linked list containing code for this word
	CODEWORD();		// For array initialization
	void* operator new(size_t s) { return (checknew(s)); }
};

CODEWORD::CODEWORD() { id=0; prob=1.0; code=NULL; };

// The word "ITEM" refers to an element in the list from which the two
// lowest probabilities are selected. A sorted linked list of these items
// is used in the "huffman" function to generate the huffman code.

// Linked list used to identify the codewords which need their code's 
// modified when a new ITEM has been created.
struct WORD
{
	CODEWORD *theword;	// Pointer to a codeword whose code needs modification
	WORD *next;					// Pointer to other codewords which need modification
	WORD(CODEWORD *Iword) { theword=Iword; next=NULL; }
	void* operator new(size_t s) { return (checknew(s)); }
};

// The list of items 
struct ITEM
{
	float prob;		// Sum of probabilities of words in wlist
	WORD *wlist;	// List of pointers to codewords which this node represents
	ITEM *next;		// Pointer to the rest of the ITEM list
	ITEM(CODEWORD *which)
		{	prob=which->prob;	wlist=new WORD(which); next=NULL; }
	ITEM(float Iprob, WORD *Iwlist)
		{	prob=Iprob;	wlist=Iwlist; next=NULL; }
	void* operator new(size_t s) { return (checknew(s)); }
};

/***************************************************************************/
/* Global Variables */
int n,d;							// Number of words (n), radix (d)
char *diskout=NULL;  	// Pointer to output filename

// Pointer to display function
void (*dispitems)(ITEM *ilist)=(void (*)(ITEM *)) dummy;

/***************************************************************************/
// Special Function used to check for Heap overflow
/***************************************************************************/
void *checknew(size_t s)
{
	void *ptr=new char[s];
	if (ptr==NULL)
	{
		cerr << "Not enough memory to generate Huffman code.\n";
		if (diskout) { fcloseall(); unlink(diskout); }
		exit(1);
	}
	return ptr;
}

/***************************************************************************/
// Input routine
/***************************************************************************/

/* 
Input has the form : 

d n prob1 prob2 prob3 ... probn

Either whitespace or seperate lines between numbers can be used.*/
CODEWORD *getinput(char *inpname)
{
	boolean conin;
	char filename[100];
	CODEWORD *list,*lptr;
	float prsum=0;				// Sum of probabilities (normally 1)
	ifstream infile;
	int i;

	// if not passed a filename then prompt user
	if (inpname==NULL)
	{
		cerr << "Input the filename for the probabilities [or CON]: ";
		cin >> filename; 
	} 
	else strcpy(filename,inpname);

	infile.open(filename);
	if (!infile) 
	{
		cerr << "Unable to open input file [" << filename << "].";
		exit(1);
	}

	conin=(boolean) (stricmp(filename,"CON")==0);

	if (conin) cerr << "Input the number of possible values for each digit:";
	infile >> d;

	if (conin) cerr << "Input the number of elements:";
	infile >> n;

	if ( (unsigned long) n*(unsigned long) sizeof(CODEWORD) > 0xFFFFL)
	{
		cerr << "Input n too large to handle.\n";
		if (diskout) { fcloseall();	unlink(diskout); }
		exit(1);
	}

	lptr=list=new CODEWORD [n+1];
	for (i=0; i<n; i++,lptr++)
	{
		if (conin) cerr << "Input probability for word " << (i+1) << " :";
		infile >> lptr->prob;
		prsum+=lptr->prob;
		lptr->id=i+1; 
		lptr->code=NULL;
	}
	infile.close();

	if (abs(prsum-1.0)>0.0000001)
	{
		cout << "\nSum of probabilities is :" << prsum 
				 << "\nValue adjusted to 1\n";
		for (lptr=list, i=0; i<n; i++,lptr++)	lptr->prob/=prsum;
	}

	return (list);
}

/***************************************************************************/
// Output routines
/***************************************************************************/

// Standard output for CODEWORD structure when used with "<<" operator
ostream& operator << (ostream& s,CODEWORD *cword)
{
	CODE *cptr;
	static char dispchars[]=
		"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	s << SETID() << cword->id << " ³ " << SETP(11) << cword->prob << " ³ ";
	for (cptr=cword->code; cptr!=NULL; cptr=cptr->next)	
		s << dispchars[cptr->val];
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

// Displays codeword list in order of input
void displayout(CODEWORD *wordlist)
{
	int i;
	cout << "\nWord ³ Probability ³ Code\n";
	for (i=1 ;wordlist->id!=0; i++, wordlist++) cout << wordlist;
}

void dummy(void) {};

// Displays list of items, Used for "-V"
void dispitems0(ITEM *ilist)
{
	int i;

	cout << "ÄÄÄÄÄÅÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ\n"
		<< "Item ³ Prob. ³ Words\n";

	for (i=1;ilist!=NULL; ilist=ilist->next, i++)
		cout << SETID() << i << " ³ " << ilist;
	if (!diskout) getch();
}

// Displays list of items, Used for "-V2"
// Even more verbose then "-V", Normally used when sending results to disk
void dispitems1(ITEM *ilist)
{
	int i;
	WORD *wptr;

	cout << "\nÄÄÄÄÄÅÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ"
		<< "\nItem ³ Prob. ³ Words\n";

	for (i=1;ilist!=NULL; ilist=ilist->next, i++)
	{
		cout << SETID() << i << " ³ " << ilist;
		for(wptr=ilist->wlist; wptr!=NULL; wptr=wptr->next)
			cout << "     word - " << wptr->theword; 	
	}
	if (!diskout) getch();
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

// Sums probabilities of first i elements of list l
float psum(ITEM *l,int i)
{
	float sum=0;
	for (;i!=0;i--,l=l->next) sum+=l->prob;
	return sum;
}

// Gives all nodes between and including start and end back to heap
void delist(ITEM *start,ITEM *end)
{
	ITEM *holder;

	while (start->next!=end->next) 
	{ 
		holder=start->next;
		delete start;
		start=holder;
	}
	delete end;
}

// The main logic of the huffman code. "list" is an array of structures 
// representing codewords
void huffman(CODEWORD *list)
{
	ITEM *ilist=NULL, *nlist,*ip,*lip;
	int i,k; // k is number of branches combined on each step

	// Create linked and sorted item list (Sorted smallest to largest)
	for (;list->id!=0;list++)	ilist=insert(ilist,new ITEM(list));

	// Display the original item list (if command line -V or -V2)
	dispitems(ilist);

	// See Gallager's Info. Theory and Reliable Comm. Page 55
	k=2+(n-2) % (d-1); // Initial number of branches combined k <= d 

	// Done when one node is left in ilist
	while (ilist->next!=NULL)
	{
		// least probable branch nodes are first ones, combine first k branches
		for (i=0,ip=ilist; i<k; i++,ip=ip->next)
		{
			*(updatew(ip->wlist,k-i-1))=(i==(k-1)) ? NULL : ip->next->wlist;
			lip=ip;
		}

		// Make new node that represents the other two, and insert it in list
		nlist=insert(ip,new ITEM(psum(ilist,k),ilist->wlist) );

		delist(ilist,lip); 	// free memory associated with old nodes
		ilist=nlist;				// update ilist, nlist is short for new list
		dispitems(ilist); 	// Display intermediate results?
		k=d; 								// After initial iteration, combine d branches 
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

	K=-1/log(d);

	for (; wordlist->id!=0; wordlist++)
	{
		entropy+=(K*wordlist->prob*log(wordlist->prob));
		for(n=0,c=wordlist->code; c!=NULL; n++, c=c->next);
		nbar+=(wordlist->prob*n);
	}

	cout << "\nThe minimum word length [H(x)/log(" << d << ")] is : " << entropy 
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
	ofstream fout;

	// Parse Command Line
	for (i=1; i<argc; i++)
	{
		strupr(argv[i]);
		switch (argv[i][0])
		{
			case '/': fout.open(diskout=&argv[i][1]); cout=fout; break;
			case '-': if (argv[i][1]=='V') {
				dispitems=(argv[i][2]=='2') ? dispitems1 : dispitems0;
				break; }
			default: inpname=argv[i];
		}
	}
	wordlist=getinput(inpname);	// Get table of probabilities
	huffman(wordlist);					// Generate huffman code
	displayout(wordlist);				// Display the results
	dispentnbar(wordlist); 			// Calculate and display entropy and n bar
}
