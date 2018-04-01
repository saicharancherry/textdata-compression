// mycompressioncode.cpp : Defines the entry point for the console application.
//

// mycompressionalg.cpp : Defines the entry point for the console application.
//
//this stores header as struct objects.the main draw back is it also stores struct node *left,*right null values.....which might increase 
//aroung 2kb ...

#include"stdafx.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define alphabets 256
#define intbits 32
#define maxbit 2147483648//10000000000000000000000000000000
struct node {
	int weight;
	unsigned char letter;
	struct node *left = NULL;
	struct node *right = NULL;
};
int *freq = NULL;
int numactive = 0;
struct node **Nodes = NULL;
int numnodes = 0;
int i = 0;
unsigned int BUFFUR = 0;
unsigned int BUFFUR_BITS_OCCUPIED = 0;

FILE *in = NULL;
FILE *out = NULL;

void frequency() {
	freq = (int*)calloc(alphabets, sizeof(int));
	int *tfreq = freq;
	//string according to ascii values
	char letter;
	while (fread(&letter, sizeof(char), 1, in) && (int)letter != EOF) {
		if (letter >= 0 && letter <= 127)
			freq[(int)letter] += 1;

	}
	freq[213] = 1;//EOF
	for (i = 0; i<alphabets; i++) {
		if (freq[i])
			numactive++;
	}
	printf("numactive %d\n", numactive);
	//getchar();
}

void addnode(int wgt, char alp) {
	/*printf("adding node : %c \t %d\n",alp,wgt);
	getchar();*/
	i = numnodes++;
	struct node *tnode = (struct node*)malloc(sizeof(struct node));
	(*tnode).letter = alp;
	tnode->weight = wgt;
	tnode->left = NULL;
	tnode->right = NULL;
	i--;
	//sorting during inserting struct nodes based on weights in ascending order
	while (i >= 0) {
		if ((*Nodes[i]).weight == (*tnode).weight) {
			while (i >= 0 && ((*Nodes[i]).weight == (*tnode).weight) && ((*Nodes[i]).letter > (*tnode).letter)) {
				Nodes[i + 1] = Nodes[i];
				i--;
			}
			break;

		}
		if ((*Nodes[i]).weight > (*tnode).weight) {
			Nodes[i + 1] = Nodes[i];
		}
		else
			break;
		i--;
	}
	i++;
	Nodes[i] = tnode;
	//.
}

void alloc_leafnodes() {
	//array of structure pointers
	Nodes = (struct node**)malloc(numactive * sizeof(struct node*));


	for (int j = 0; j<alphabets; j++) {
		//
		if (freq[j]) {
			addnode(freq[j], (char)j);
		}
	}

	fwrite(&numnodes, sizeof(int), 1, out);
	//printf("sizeof struct node %d", sizeof(struct node));
	for (int i = 0; i < numnodes; i++) {

		fwrite(Nodes[i], sizeof(struct node), 1, out);
	}
	char a = '|';
	fwrite(&a, sizeof(char), 1, out);
}
struct node *pop() {
	return Nodes[0];
}
void heapify() {
	Nodes[0] = Nodes[numnodes - 1];
	Nodes[numnodes - 1] = NULL;
	numnodes--;
	i = 0;

	while (i < numnodes) {
		int left = 2 * i + 1;
		int right = 2 * i + 2;
		int minindex = i;
		if (left < numnodes && (*Nodes[i]).weight >(*Nodes[left]).weight) {
			minindex = i + 1;
		}
		if (right < numnodes && (*Nodes[minindex]).weight >(*Nodes[right]).weight) {
			minindex = i + 2;
		}
		if (minindex != i) {
			node *temp = Nodes[minindex];
			Nodes[minindex] = Nodes[i];
			Nodes[i] = temp;
			i = minindex;
		}
		else {
			break;
		}
	}
}

void create_tree() {
	while (numnodes != 1) {
		struct node *first = pop();
		//if (numnodes == 1)break;
		heapify(); //Nodes array of pointers is min heap tree.
		struct node *second = pop();
		//if (numnodes == 1)break;
		struct node *leaf2node = (struct node*)malloc(sizeof(struct node));
		leaf2node->weight = first->weight + second->weight;
		leaf2node->left = first;
		leaf2node->right = second;
		leaf2node->letter = 250;
		Nodes[numnodes++] = leaf2node;
		heapify();
	}
}

unsigned int nbitsmoved = 0;
unsigned int bitpath = 0;
unsigned int arr[256][2];
void initialize_active_ele_details(struct node *node, int n, int i, int nbits) {
	if (node == NULL)
		return;
	n = n << 1;
	n = n ^ i;
	nbits += 1;
	initialize_active_ele_details(node->left, n, 0, nbits);
	initialize_active_ele_details(node->right, n, 1, nbits);
	if (node->letter <= 256 && node->letter >= 0) {

		bitpath = n;			//path to char in bits representation
		nbitsmoved = nbits - 1;
		arr[node->letter][0] = bitpath;//nothing but path length
		arr[node->letter][1] = nbitsmoved;
		return;

	}
}
void FLUSH_BUFFER(FILE *out) {
	fwrite(&BUFFUR, sizeof(int), 1, out);
	BUFFUR = 0;
	BUFFUR_BITS_OCCUPIED = 0;
}
void write_data(FILE *in, FILE *out) {
	unsigned char letter;
	initialize_active_ele_details(Nodes[0], 0, 0, 0);
	while (fread(&letter, sizeof(char), 1, in) && (int)letter != EOF) {
		nbitsmoved = arr[letter][1];
		bitpath = arr[letter][0];

		//search(&letter, Nodes[0], 0, 0, 0);
		if (BUFFUR_BITS_OCCUPIED == 32) {
			FLUSH_BUFFER(out);
		}
		if (32 - BUFFUR_BITS_OCCUPIED >= nbitsmoved) {
			BUFFUR = BUFFUR << nbitsmoved;
			BUFFUR = BUFFUR ^ bitpath;
			BUFFUR_BITS_OCCUPIED += nbitsmoved;
		}
		else {
			unsigned int c = 32 - BUFFUR_BITS_OCCUPIED;
			int temp = bitpath;
			temp = temp >> (nbitsmoved - c);
			BUFFUR = BUFFUR << c;
			BUFFUR = BUFFUR ^ temp;
			bitpath = (temp << (nbitsmoved - c)) ^ bitpath;
			FLUSH_BUFFER(out);
			BUFFUR = BUFFUR ^ bitpath;
			BUFFUR_BITS_OCCUPIED += (nbitsmoved - c);
		}
	}
	unsigned char end = (char)213;//EOF

								  //search(&end, Nodes[0], 0, 0, 0);
	bitpath = arr[213][0];
	nbitsmoved = arr[213][1];
	if (32 - BUFFUR_BITS_OCCUPIED >= nbitsmoved) {
		BUFFUR = BUFFUR << nbitsmoved;
		BUFFUR = BUFFUR ^ bitpath;
		BUFFUR_BITS_OCCUPIED += nbitsmoved;
		BUFFUR = BUFFUR << 32 - BUFFUR_BITS_OCCUPIED;
		FLUSH_BUFFER(out);
	}
	else {
		unsigned int c = 32 - BUFFUR_BITS_OCCUPIED;
		unsigned int temp = bitpath;
		temp = temp >> (nbitsmoved - c);
		BUFFUR = BUFFUR << c;
		BUFFUR = BUFFUR ^ temp;
		bitpath = (temp << (nbitsmoved - c)) ^ bitpath;
		FLUSH_BUFFER(out);
		BUFFUR = BUFFUR ^ bitpath;
		BUFFUR = BUFFUR << (32 - (nbitsmoved - c));
		FLUSH_BUFFER(out);
	}

}

//decode part.........................................................................................................................................
void read_header() {

	i = 0;
	fread(&numnodes, sizeof(int), 1, in);
	Nodes = (struct node**)malloc(numnodes * sizeof(struct node*));
	for (int x = 0; x < numnodes; x++) {
		struct node temp[1];
		fread(temp, sizeof(struct node), 1, in);
		struct node *pnode = (struct node*)malloc(sizeof(struct node));
		(*pnode).letter = temp[0].letter;
		(*pnode).weight = temp[0].weight;
		Nodes[x] = pnode;
		numactive++;
	}
	unsigned char a;
	fread(&a, sizeof(char), 1, in);

}

int LoadBuffur() {
	fread(&BUFFUR, sizeof(int), 1, in);
	nbitsmoved = 0;
	return 1;
}

char searchbin(node *Node) {
	if (nbitsmoved == 32) {
		LoadBuffur();
	}
	if (Node == NULL)
		return '\0';
	if (((*Node).letter >= 0 && (*Node).letter <= 213) || (*Node).letter == 213) {

		return (*Node).letter;
	}
	unsigned int boolean = BUFFUR & maxbit;
	if (!boolean) {
		BUFFUR = BUFFUR << 1;
		nbitsmoved += 1;
		return searchbin(Node->left);
	}
	if (boolean) {
		BUFFUR = BUFFUR << 1;
		nbitsmoved += 1;
		return searchbin(Node->right);
	}

}

void write_bin_data() {
	LoadBuffur();
	while (true) {
		char s = searchbin(Nodes[0]);
		if (s == (char)213)
			break;
		fwrite(&s, sizeof(char), 1, out);
	}
}
void Decode() {
	read_header();

	create_tree();

	write_bin_data();
	fclose(out);

}
//..................................................................................................................................................
//by default compressed bin file is stored in C drive....
int main(int argc, char *arg[])
{
	char filename[200];
	printf("[1]encode\t[2]decode\n");
	int i = 0;
	scanf_s("%d", &i, 1);
	if (i == 1)
	{
		printf("Enter file path:\nhelp:c:\\foldernames....\\filename.txt or bin\n");
		getchar();
		gets_s(filename, 200);
		fopen_s(&in, filename, "r");
		fopen_s(&out, "C:\compressed.bin", "wb");
		if (in == NULL || out == NULL) {
			printf("error in opening file");
			exit(1);
		}
		printf("\n Processing..................\n");
		frequency();
		alloc_leafnodes();

		create_tree();
		fseek(in, 0, SEEK_SET);

		if (out == NULL) {
			printf("error in opening bin file");
			exit(2);
		}
		write_data(in, out);
		fclose(in);

		fclose(out);
		printf("Compression on file is completed\nplease enter any key.............");
		getchar();
	}
	else if (i == 2)
	{
		printf("plese enter you destination path....\nex:c:\\foldername\\.....\n\tsuggestion : don't mention file name:\n");
		char DEST[100];
		getchar();
		gets_s(DEST, 100);
		fopen_s(&in, "C:\compressed.bin", "rb");
		if (in == NULL) {
			printf("error in reading file\n");
			return 0;
		}
		strcat_s(DEST, "\decompressed.txt");
		fopen_s(&out, DEST, "w");
		printf("decompression taking place 1\n processing..........");
		Decode();
		fclose(in);
		fclose(out);
		printf("\nplease , search for decompressed in %s\n", filename);
		getchar();
	}
	else
		printf("terminated unexpectedly");
	return 0;
}

