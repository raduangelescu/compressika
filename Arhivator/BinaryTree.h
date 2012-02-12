#ifndef _BINARYTREE
#define _BINARYTREE
// este un nod tipic de arbore binar huffman/shannon/ arithmetic.. in fine
struct cBinaryTreeNode 
{ 
     unsigned int count; // de cate ori a aparut simbolul
     unsigned int saved_count;// inutil momentan
     int child_0; // copilul de la 0.. sau stanga
     int child_1; // copilul de la 1 .. sau dreapta
	 unsigned int symbol; // simbolul de codificat
	 cBinaryTreeNode():count(0),saved_count(0),child_0(END_OF_STREAM),child_1(END_OF_STREAM),symbol(0){}

	 cBinaryTreeNode(unsigned int symb):count(0),saved_count(0),child_0(END_OF_STREAM),child_1(END_OF_STREAM),symbol(symb){}
	 ~cBinaryTreeNode()
	 {
	 
	 }
} ; 
#endif