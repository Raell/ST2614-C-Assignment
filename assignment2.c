//Name: Cheong Ren Hann
//Admin No: 1230068
//Class: DISM/FT/2A/02
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define MAX_LINKS 100000 //maximum number of links each node can hold
#define MAX_HOPS 30 //maximum hops in traceroute and tracert
#define ADDRESS_MAX 20 //maximum length of ipaddesses

int nodecount = 1; //counter id for the next node to be created

struct node {

  int id;
  char address[ADDRESS_MAX];
  struct node *next[MAX_LINKS]; //array to hold all previous nodes
  struct node *prev[MAX_LINKS]; //array to hold all next nodes
  struct node *ordered; //used to ensure no infinite loop when viewing and also to hold sort order

};

typedef struct node topoNode;

topoNode *lastnode = NULL; //points to last created node

void menu();
void getInput(char *input);
void readFile(char *filename);
void writeFile(char *filename);
void readLines(FILE *file);
void extractAddress(char *buffer, char *addAddress);
topoNode *processAddress(char *address, topoNode *currentnode);
topoNode *addNode(char *ipaddress, topoNode *previous);
topoNode *checkDuplicate(char *ipaddress, topoNode *current);
topoNode *editNode(topoNode *editnode, topoNode *previousnode);
void nodeSort();
unsigned int addressValue(char *address);
void viewNodes(topoNode *lastnode, FILE *outputStream);
void prevHops();
void nextHops();
void nodesNHopsAway();
int * printHops(topoNode *current, int duplicate[], int n, int hopsLeft);
topoNode* searchNode();

int main(int argc, char **argv) {

  if(argc == 3) { //ensures 2 arguments, one for input and one for output files
    
    readFile(argv[1]); //reads input file from 1st argument
    
    nodeSort(); //sorts the nodes by ipaddress
    viewNodes(lastnode, stdout); //prints out nodelist to console

    writeFile(argv[2]); //writes nodelist to output file
  
    char opt[ADDRESS_MAX]; //used to hold user argument as a string
    int option; //used to as argument for switch statement
    do { //start loop
      menu(); //prints user menu

      getInput(opt); //gets input from user and stores it in opt
      option = (int)opt[0]; //uses only first character in argument string as int
       
      switch(option) { //checks for 1-4 from option and runs respective functions
      case 49:	
	prevHops();
	break;
      case 50:
	nextHops();
	break;
      case 51:
	nodesNHopsAway();
	break;
      case 52:
	printf("\nProgram Exit.\n");
	break;
      default:
	printf("\nInvalid selection. Please input from 1 - 4.\n");

      }
       
    }while (option != 52); //exits loop
  
  }

  else { //does nothing if anything but 2 arguments
    printf("Please specify one input and one output file as first and second arguments respectively.\n");
  }

  return 0;

}

void menu() { //print the menu

  printf(" \nPlease select an option from the menu\n");
  printf(" 1. Report ID of previous hops of a network node\n");
  printf(" 2. Identify the next hops of a network node\n");
  printf(" 3. List network nodes at n hops away\n");
  printf(" 4. Quit\n");
  printf("\nYour Command (1 to 4) => ");
	
}

void getInput(char *input) { //function to get the user input
  
  char choice[ADDRESS_MAX];
  fgets(choice, ADDRESS_MAX, stdin);
  
  int i;
  for(i = 0; i < ADDRESS_MAX; i++) {

    //checks for index of null byte
    if(choice[i] == '\0') {
      i--;
      break;
    }

  }

  //removes newline char from fgets if any and clears stdin
  if(choice[i] == '\n') {
    choice[i] = '\0';
  }
  else {
    int c;
    while((c = getchar()) != '\n' && c != EOF);
  }

  //copies string
  strcpy(input, choice);
  
}

void readFile(char *filename) { //function to read input file

  FILE *file;

  //try to open file
  if((file = fopen(filename, "r")) == NULL) {

    printf("Unable to open %s.\n", filename);
    exit(1);

    }

  //pass on file pointer to extraction routine 
  readLines(file);
  fclose(file);
   
}

void writeFile(char *filename) { //function to write output file

  FILE *file;

  //try to open file
  if((file = fopen(filename, "w")) == NULL) {

    printf("Unable to write to %s.\n", filename);
    exit(1);

  }

  //writes nodelist to file
  viewNodes(lastnode, file);
  fclose(file);

}

void readLines(FILE *file) { //function to read each line from the input file

  int h = 1;
  topoNode *currentnode = NULL;
  
  while(1) {
    char buffer[500];
    int hop = 0;

    //use fgets to retrieve a single line
    if(fgets(buffer, 500, file) == NULL) {
      break;
    }

    //scans the first digit of the line
    //usually the index of node
    sscanf(buffer, "%d", &hop);
    
    //checks if finished current set of traceroute data
    if(hop < h) {
      h = 1;
      currentnode = NULL; //currentnode points to previous node added/modified for each new traceroute set
    }

    //skips if this line is not node data
    if(hop != h) {
      continue;
    }

    //increments h to check next line if current line is valid
    h++;

    char address[ADDRESS_MAX];
    extractAddress(buffer, address);
    currentnode = processAddress(address, currentnode);
      
  }

}

void extractAddress(char *buffer, char *addAddress) { //function to extract ipaddresses

  //store ipaddress here
  char address[ADDRESS_MAX];

  //finds ipaddresses based on its pattern
  //read every character in the line
  int i, j;
  int asteriskCheck = 0;
  for(i = 0; buffer[i] != 0; i++) {
    
    //find exact position of address in the line where i is the starting index
    //and j is the ending index
    //also confirms that ipaddress is at least 7 characters consisting of numbers or '.' and if address is * * *
    int check = 0;
    for(j = i; buffer[j] != 0; j++) {
      
      if(buffer[j] == '*') {
	
	if(buffer[j+2] == '*' && buffer[j+4] == '*') {
	  asteriskCheck = 1;
	  break;
	}
	
      }

      if(isdigit(buffer[j]) || buffer[j] == '.') {
	  
	check++;
	  
      }
      else if (check < 7) {

	break;

      }
      else {

	j--;
	break;

      }
	    
    }
      
    if(check >= 7) {
      break;	  
    }
      
  }
    
  int a;
  
  //copy over the address
  if(!asteriskCheck) {
    for(a = 0; i <= j; a++, i++) {

      address[a] = buffer[i];

    }

    address[a] = '\0';
  }
  //if address is * * *
  else {
    
    a = 0;
    address[a++] = '*';
    address[a++] = ' ';
    address[a++] = '*';
    address[a++] = ' ';
    address[a++] = '*';
    address[a++] = '\0';
    
  }

  //copy over address
  strcpy(addAddress, address);

}

topoNode * processAddress(char *address, topoNode *currentnode) { //function to handle node data processing

  //checks if address was already in previous nodes
  topoNode *duplicate = checkDuplicate(address, lastnode);
  
  if(duplicate != NULL) { //update node if duplicate is found
    currentnode = editNode(duplicate, currentnode);
  }
  else { //add new node if not
    lastnode = addNode(address, currentnode); //lastnode will always point to the last created node
    currentnode = lastnode;
  }

  return currentnode;    

}


topoNode * addNode(char *ipaddress, topoNode *previous) { //function to add new nodes

  topoNode *newNode = (topoNode *)malloc(sizeof(topoNode));

  //insert new data
  newNode->id = nodecount++;
  strcpy(newNode->address, ipaddress);
  newNode->prev[0] = previous; //update previous node
  newNode->ordered = lastnode; //update current node order
  
  if(previous != NULL) { //checks if there is a previous node
    
    int i;
    for(i = 0; i < MAX_LINKS; i++) {

      if(previous->next[i] == NULL) { //updates previous node's next array
	previous->next[i] = newNode;
	break;
      }
      
    }

  }

  return newNode;
  
}

topoNode * checkDuplicate(char *ipaddress, topoNode *last) { //function to check for duplicate addresses

  topoNode *list = last;

  if(ipaddress[0] != '*') { //ignore duplicate if address is * * *

    while(list != NULL) { //loops through the nodes in order

      if(!strncmp(list->address, ipaddress, ADDRESS_MAX)) { //returns pointer to duplicate entry if found
      
	return list;

      }

      else {

	list = list->ordered;

      }

    }

  }

  return NULL; //returns NULL is no duplicates are found

}

topoNode * editNode(topoNode *editnode, topoNode *previousnode) { //function to edit existing nodes

  int i;
  
  for(i = 0; i < MAX_LINKS; i++) {
    
    if(editnode->prev[i] == previousnode) {
      break;
    }
    
    if(editnode->prev[i] == NULL) {
      
      editnode->prev[i] = previousnode;
      break;
      
    }
    
  }

  return editnode;

}

void viewNodes(topoNode *last, FILE *outputStream) { //function to print out nodelist

  if(last != NULL) { //if node exists
    char* address = last->address;
    
    if(address[0] != '*') { //extract each number in address for formatting if address is not * * *
      int a, b, c ,d;
      sscanf(address, "%d %*[.] %d %*[.] %d %*[.] %d", &a, &b, &c, &d);
      fprintf(outputStream, "%2d: %3d.%3d.%3d.%3d <--", last->id, a, b, c, d);
    }
    else { //if address is * * *
      fprintf(outputStream, "%2d: %15s <--", last->id, address);
    }

    int i;
    for(i = 0; i < MAX_LINKS; i++) { //print out IDs of all previous nodes

      if(last->prev[i] != NULL) { //prints if next node exists
	
	topoNode *prev = last->prev[i];
	fprintf(outputStream," %2d", prev->id);
	
	if((i + 1) != MAX_LINKS) {
	  
	  if(last->prev[i+1] != NULL) {
	    fprintf(outputStream, ",");
	  }
	
	}
	
      }

      else { //break loop if NULL value is found
	break;
      }

    }

    fprintf(outputStream, "\n");
    viewNodes(last->ordered, outputStream); //recursion to print out the rest of the nodes
    return;
  
  }
  else { //return if node does not exist
    fprintf(outputStream, "\n");
    return;
  }

}

void nodeSort() { //function to sort nodes by address

  int lastIndex = lastnode->id;
  topoNode *current = lastnode;
  topoNode *sortOrder[lastIndex];
  
  int i;
  for(i = 0; i < lastIndex; i++) { //adds all nodes to an array based on current(ID) order

    sortOrder[i] = current;
    current = current->ordered;

  }
  
  for(i = 0; i < lastIndex; i++) { //selection sort

    unsigned int maxValue = addressValue(sortOrder[i]->address); //gets address value as an int and assign it as maximum value
    int highestIndex = i;
    int j;

    for(j = i; j < lastIndex; j++) { //loops through array to find the largest address value

      unsigned int testValue = addressValue(sortOrder[j]->address);
      
      if(testValue > maxValue) {

	maxValue = testValue;
	highestIndex = j;

      }
      
    }

    //swap highest value to the front
    //highest address value will point to NULL
    topoNode *tmp = sortOrder[highestIndex];
    sortOrder[highestIndex] = sortOrder[i];
    sortOrder[i] = tmp;

  }

  topoNode *nextOrder = NULL;
  
  for(i = 0; i < lastIndex; i++) { //reassigns ordered for each node to point to the next highest node

    sortOrder[i]->ordered = nextOrder;
    nextOrder = sortOrder[i];


  }

  lastnode = sortOrder[i - 1]; //reassign lastnode to point to smallest node

}
  
unsigned int addressValue(char *address) { //function to get int value of ipaddresses

  if(address[0] == '*') { //if address is * * *
    return 0;
  }
  
  unsigned int a, b, c, d;
  unsigned int total;
  
  sscanf(address, "%d %*[.] %d %*[.] %d %*[.] %d", &a, &b, &c, &d); //extract each individual number

  total = (a * 256 * 256 * 256) + (b * 256 * 256) + (c * 256) + d; //formula to count value

  return total;

}

topoNode* searchNode() { //function to get user input for queries(Option 1, 2)

  char query[ADDRESS_MAX];
  topoNode *searchnode;
  
  printf("Enter either Node ID or IP Address => ");
  getInput(query);
  
  int i;
  int ipcheck = 0; //check if user is querying ipaddress
  for(i = 0; query[i] != '\0'; i++) {

    if(query[i] == '.') {
      
      ipcheck = 1;
      break;
      
    }
    else if(!isdigit(query[i])) { //check if there is invalid characters

      printf("\nQuery contains invalid characters.\n");
      return NULL;

    }
    
  }

  for(searchnode = lastnode; searchnode != NULL; searchnode = searchnode->ordered) { //try to find node based on user query

    if(ipcheck) { //find by ipaddress

      if(!strncmp(searchnode->address, query, ADDRESS_MAX)) {
	break;
      }

    }
    else { //find by node ID

      int id = atoi(query);

      if(id == searchnode->id) {
	break;
      }

    }

  }
  
  if(searchnode == NULL) {
    printf("\nNode not found.\n");
  }
  
  return searchnode;

}

void prevHops() { //function to print previous hops of a node

  topoNode *searchnode = searchNode(); //gets node which was queried

  if(searchnode == NULL) { //return if no node
    return;
  }

  if(searchnode->prev[0] == NULL) { //return if no prev nodes
    printf("\nNo previous hops for Node %d.\n", searchnode->id);
    return;
  }
  
  printf("\nPrevious hops for Node %d:\n%-3s\n\n", searchnode->id, "ID");

  int i;

  for(i = 0; searchnode->prev[i] != NULL; i++) { //prints ID of all previous nodes
    
    printf("%d\n", searchnode->prev[i]->id);
  

  }

  printf("\n");

}

void nextHops() { //function to print next hops of a node

  topoNode *searchnode = searchNode(); //gets node which was queried

  if(searchnode == NULL) { //return if no node
    return;
  }

  if(searchnode->next[0] == NULL) { //return if no next nodes
    printf("\nNo next hops for Node %d.\n", searchnode->id);
    return;
  }
  
  printf("\nNext hops for Node %d:\n%-3s %-15s\n\n", searchnode->id, "ID", "Address");

  int i;

  for(i = 0; searchnode->next[i] != NULL; i++) { //prints ID and addresses of next nodes

    printf("%-3d %-15s\n", searchnode->next[i]->id, searchnode->next[i]->address);
   
  }

  printf("\n");
  

}

void nodesNHopsAway() { //function to print nodes n hops away

  //query user for n value
  char query[ADDRESS_MAX];
  printf("Enter value of n => ");
  getInput(query);

  int i;
  for(i = 0; query[i] != '\0'; i++) {
    if(!isdigit(query[i])) { //return if invalid value
      printf("\nInvalid value of n.\n");
      return;
    }
    
  }

  int n = atoi(query);

  int lastIndex = lastnode->id;

  //find all possible 1st hops and initialize to array
  topoNode *startNodes[lastIndex];
  topoNode *initialization = lastnode;

  for(i = 0; i < lastIndex;) {

    if(initialization == NULL) {
      startNodes[i++] = NULL;
    }
    else if(initialization->prev[0] == NULL) {
      startNodes[i++] = initialization;
    }

    if(!(initialization == NULL)) {
      initialization = initialization->ordered;
    }

  }

  //used for duplicate checking
  //IDs of nodes that are printed will be added into the array later
  int duplicate[nodecount];
  
  for(i = 0; i < lastIndex; i++) {
    duplicate[i] = 0;
  }

  //last value of array is for minor printing formatting
  duplicate[nodecount] = 1;

  int check = 0;

  int *count = duplicate;

  //call function to print 
  for(i = 0; startNodes[i] != NULL; i++) {
    
    count = printHops(startNodes[i], count, n, n);
    
    if(count[0] == 0) {
      check++;
    }
    
  }

  if(check) { //if no nodes were printed
    printf("\nNo nodes at %d hop(s) away.\n", n);
  }

}

int * printHops(topoNode* current, int duplicate[], int n, int hopsLeft) { //function to run through and print nodes after n hops

  int i;
  hopsLeft--;
  
    
  for(i = 0; duplicate[i] != 0; i++) { //duplicate check
      
    if(current->id == duplicate[i]) {

      break;
	
    }
	
  }
      
  if(current->id != duplicate[i] && hopsLeft == 0) { //prints nodes if n hops reached and non-duplicated node

    if(duplicate[nodecount] == 1) { //for first time node print
	
      printf("\nNodes at %d hop(s) away:\n%-3s %-15s\n\n", n, "ID", "Address");
      duplicate[nodecount]--;
	
    }
      
    duplicate[i] = current->id;
    printf("%-3d %-15s\n", current->id, current->address);
      
  }

  for(i = 0; current->next[i] != NULL; i++) { //recursive search through all next nodes until n hops reached

    duplicate = printHops(current->next[i], duplicate, n, hopsLeft);

  }


  return duplicate; //returns duplicate ID array
  
}
