#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

/*
	Sets are stored in a struct that had integer that records the index of each set and
	a 2d array is used to represent all of the lines in a set with 3 columns. 0(valid bit) 1(tag) 2(place or age)
*/
typedef
struct sets
{
	int index;
	int** lines;
}
Sets;

/*
	hits - number of times the address is found in the cache
	misses - number of times the address is not found in the cache
	memReads - the number of times the cache has to read to memory
	memWrites - the number of times the cache has to write to memory
	hitsP - number of times the address is found in the cache when prefetching
	missesP  - number of times the address is not found in the cache when prefetching
	readsP  - number of times when the cache has to read to memory prefetching
	writesP - number of times when the cache has to write to memory prefetching
	cacheSize - the size of the cache
	associativity - what type of associativity it is (direct, assoc, or assoc:n where n >= 2)
	repPolicy - the type pf replacement policy (fifo or lru)
	char op - the operation, either read (R) or write (W)
	blockSize - the size of the block
	E - this is the number of lines per set which is determined by the associativity
	S  - the number of sets the cache has
	b - bits of the block
	s - bits of the set
	t - bits of the tag

*/
int hits=0 ,misses=0 , memReads= 0, memWrites=0 ;
int hitsP= 0,missesP=0, readsP=0, writesP=0;
int cacheSize=0;
char* associativity;
char* repPolicy;
char op;
int blockSize=0;
int E =0;// this is the number of lines per set which is determined by the associativity
int S =0;
int numS =0;
int b =0;
int s =0;
int t =0;

/*
	This function initializes all of the lines in every set
*/
void createSets(int numS, Sets set[])
{
	int i,h;
	for(i=0;i<numS;i++)//repeats x amount of times depending on the number of sets
	{
		set[i].index=i;
		set[i].lines= (int**)malloc(sizeof(int*)*E);
		for(h=0;h<E;h++) //allocates memory for all of the lines in the set
		{
			set[i].lines[h]=(int*)malloc(sizeof(int)*3);
			set[i].lines[h][0]=0;
			set[i].lines[h][1]=0;
			set[i].lines[h][2]=h;
			
		}
	}
}
/*
	Prints each set and all of its lines 
*/
void printSets(Sets set[])
{
	int num = numS;
	int i,h;
	for(i=0;i<num;i++)
	{
		printf("Set %d:\n",set[i].index);
		for(h=0;h<E;h++) //allocates memory for all of the lines in the set
		{
			printf("\t valid: %d, tag: %d, place: %d\n",set[i].lines[h][0],set[i].lines[h][1],set[i].lines[h][2]);
		}
	}
}

/*
	This finds the number of lines each set should have based on the associativity
*/
int findE(char* assoc)
{
	int n;
	char m;
	if(!strcmp(assoc,"direct"))
	{
		return 1;
	}
	else if(!strcmp(assoc,"assoc"))
	{
		return cacheSize/blockSize;// you need to parse to the last char in the string and get the number from there
	}
	else
	{
		m= assoc[6];
		n= m -'0';
		return n;// you need to parse to the last char in the string and get the number from there
	}
}

/*
	This takes all of the data accumulated and outputs it in the format specified in the instructions
*/
void output(int hitsNP, int missesNP, int readsNP, int writesNP, int hits, int misses, int reads, int writes)
{
	printf("no-prefetch\n");
	printf("Cache hits: %d\n",hits);
	printf("Cache misses: %d\n",misses);
	printf("Memory reads: %d\n",memReads );
	printf("Memory writes: %d\n", memWrites );
	printf("with-prefetch\n");
	printf("Cache hits: %d\n",hitsP);
	printf("Cache misses: %d\n",missesP);
	printf("Memory reads: %d\n",readsP);
	printf("Memory writes: %d\n",writesP);
}

/*
	This increases the age (third column in line array) of all of the tags
*/
void increaseAge(Sets set[], int setIndex, int tag)
{
	int i, h, j=0;
	for(i=0;i<numS;i++)// finds the corresponding set
	{
		if(set[i].index== setIndex) 
		{
			for(h=0;h<E;h++)// finds the corresponding tag
			{
				if(set[i].lines[h][1]== tag)// && set[i].lines[h][0]== 1)
				{
					set[i].lines[h][2]=0;
					while(j<E)//loop through all of the lines in the set
					{
						if(set[setIndex].lines[j][0]==1)// if the line contains a tag
						{
							set[setIndex].lines[j][2]++; // every lines age is increased by one 
						}
						j++;
					}
				}
			}
			break;
		}
	}
}

/*
	This loads the given address into the cache
	It takes the address and inserts it to the given set based on what the replacement policy is
*/
void load(char* repPolicy,int setIndex, int tag, Sets set[])
{
	int count=0;
	int i,h;
	if(setIndex <= numS)
	{
		for(i=0;i<E;i++) // loop through the lines in the given set
		{
			if(set[setIndex].lines[i][0]== 0) //if the valid bit is 0 the line is empty
			{
				set[setIndex].lines[i][0]=1;
				set[setIndex].lines[i][1]= tag;
				if(!strcmp(repPolicy,"lru"))
				{
					set[setIndex].lines[i][2]=0;
					increaseAge(set,setIndex,tag);
				}
				break;
			}
			else // if not increment count which counts the number of full lines
			{
				count++;
			}
		}
		if(count == E)// if the set is full start replacement protocol
		{
			if(!strcmp(repPolicy,"fifo"))
			{
				for(i=0;i<E;i++)//loop through the lines of the set
				{
					if(set[setIndex].lines[i][2]==0) // look for oldest at place 0
					{
						set[setIndex].lines[i][1]= tag; //set oldest to the new tag
						h=0;
						while(h<E) //loop through all of the lines in the set
						{
							set[setIndex].lines[h][2]--; // the age is reduced by one 
							h++;
						}
						set[setIndex].lines[i][2]=E-1; // make the new tag the oldest place
						break;
					}
				}
			}
			else
			{
				int oldest = set[setIndex].lines[0][2];
				int oIndex=0;
				int i,h;
				for(i=0;i<E;i++)//loop through the lines of the set
				{
					if(set[setIndex].lines[i][2]>oldest)
					{
						oldest=set[setIndex].lines[i][2];
						oIndex =i;
					}
				}
				h=0;
				set[setIndex].lines[oIndex][1]= tag;
				set[setIndex].lines[oIndex][2]=0;
				while(h<E) //loop through all of the lines in the set
				{
					set[setIndex].lines[h][2]++; // every lines age is increased by one 
					h++;
				}
			}
		}
	}
}

/*
	This checks to see if the address is in the cache by looping through the sets 
	and checking if the given setIndex is a set in the cache and then looping through
	the lines in the set and finding the given tag. If the address is in the cache it returns
	a 1 otherwise it turns a 0
*/
int findAddress(Sets set[],int tag, int setNumber)
{
	int i, h;
	for(i=0;i<numS;i++)// finds the corresponding set
	{
		if(set[i].index== setNumber) 
		{
			for(h=0;h<E;h++)// finds the corresponding tag
			{
				if(set[i].lines[h][1]== tag)// && set[i].lines[h][0]== 1)
				{
					return 1;
				}
			}
			break;
		}
		
	}
	return 0;
	
}

/*
	This frees all of the memory allocated for the sets
*/
void freeSets(Sets set[])
{
	int i,h;
	for(i=0;i<numS;i++)//repeats x amount of times depending on the number of sets
	{
		for(h=0;h<E;h++) //allocates memory for all of the lines in the set
		{
			free(set[i].lines[h]);
		}
		free(set[i].lines);
	}
}

int main( int argc, char **argv)
{
	//read text file (should be the last argument)
	FILE* fp;
	fp = fopen(argv[5], "r");
	/*make needed variables*/
	cacheSize= atoi(argv[1]);
	associativity= argv[2];
	repPolicy= argv[3];
	blockSize= atoi(argv[4]);
	E = findE(associativity);// this is the number of lines per set which is determined by the associativity
	S = cacheSize/(blockSize*E);
	numS= S;
	if(numS<=1)
	{
		numS=1;
	}
	s = (int)(log((double)S)/ log(2));
	b = (int)(log((double)blockSize)/ log(2));
	t = 48-b-s;
	int setIndex, tag;
	int setIndexPrefetch, tagPrefetch;
	long unsigned int address;
	char pc[50];
	Sets set[numS];
	Sets setP[numS];
	createSets(numS,set);
	createSets(numS,setP);
	
	while(!feof(fp))
	{	/*
			without pre-fetching
		*/
		fscanf(fp,"%s",pc);
		if(strcmp(pc,"#eof"))
		{
			fscanf(fp,"%*[ \n\t]%c",&op);
			fscanf(fp,"%lx",&address);
			long unsigned int addressP = address >> b;
			if(s==0)
			{
				setIndex=0;
			}
			else
			{
				setIndex= addressP % (int) pow(2,s); //bit shift address left the size of the block offset and then use a mask to get middle bits to get the set
			}
			tag= address >> (s+b); //bit shift address left the size of the set and block offset to get the tag
			if(findAddress(set,tag,setIndex) == 1) // if the address is in the cache 
			{	
				hits++; //increase hits
				if(op == 'W') memWrites++;
				if(!strcmp(repPolicy,"lru"))
				{
					increaseAge(set,setIndex,tag);
				}
			}
			else // if not found in cache
			{
				misses++; // increase misses
				if(op == 'R')
				{
					memReads++;
					load(repPolicy,setIndex,tag,set);
				}					
				else if(op == 'W')
				{
					memReads++;
					load(repPolicy,setIndex,tag,set);
					memWrites++;
				}
			}
		}
		
		/*
			With Pre-fetching
		*/
		if(strcmp(pc,"#eof"))
		{
			long unsigned int addressNP = address>>b;
			if(s==0)
			{
				setIndexPrefetch=0;
			}
			else
			{
				setIndexPrefetch= addressNP % (int) pow(2,s); //bit shift address left the size of the block offset and then use a mask to get middle bits to get the set
			}	
			tagPrefetch= (address) >> (s+b); //bit shift address left the size of the set and block offset to get the tag
			if(findAddress(setP,tagPrefetch,setIndexPrefetch) == 1) // if the address is in the cache 
			{	
				hitsP++; //increase hits
				if(op == 'W') writesP++; 
				if(!strcmp(repPolicy,"lru"))
				{
					increaseAge(setP,setIndexPrefetch,tagPrefetch);
				}
			}
			else // if not found in cache
			{
				missesP++; // increase misses
				if(op == 'R')
				{
					readsP++;
					load(repPolicy,setIndexPrefetch,tagPrefetch,setP);
				}					
				else if(op == 'W')
				{
					readsP++;
					load(repPolicy,setIndexPrefetch,tagPrefetch,setP);
					writesP++;
				}
				addressNP = (address+blockSize);
				long unsigned int addressPrefetch = addressNP >> b;
				setIndexPrefetch= addressPrefetch % (int) pow(2,s); //bit shift address left the size of the block offset and then use a mask to get middle bits to get the set
				tagPrefetch= addressNP >> (s+b);
				if(findAddress(setP,tagPrefetch,setIndexPrefetch) == 0) // if the address is in the cache 
				{
					if(op == 'R')
					{
						readsP++;
						load(repPolicy,setIndexPrefetch,tagPrefetch,setP);
						
					}					
					else if(op == 'W')
					{
						readsP++;
						load(repPolicy,setIndexPrefetch,tagPrefetch,setP);
						
					}
				}
			}
		} 
	}
	fclose(fp);
	output(hitsP, missesP, readsP, writesP, hits, misses,memReads, memWrites);
	freeSets(set);
	return 0;
}