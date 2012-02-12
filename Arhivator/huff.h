//
 // This is huffman.c based on huff.c from Mark Nelson and Jean-Loup Gailly's
 // "The Data Compression Book" and relies on the bitio library from the same.
 // To use this code, make a struct called BIT_FILE and link against another
 // object file with two routines:
 //
 //   int InputBit (BIT_FILE *)
 //   void OutputBits (BIT_FILE *, int bitv, int length)
 //
 // Entrypoints to this module are CompressFile and ExpandFile.
 //
 // Or buy the book and use theirs.  One day I will get around to commenting,
 // modifying and converting bitio.c but until then I do not want to put some-
 // one elses' copyrighted code up on the web.  If you can't do this yourself
 // then bother me and I may do it for you.  If you did it yourself, send me
 // the file and I'll put it up here so others can use it.
 //

/************************** Start of BITIO.C *************************
 *
 * This utility file contains all of the routines needed to impement
 * bit oriented routines under either ANSI or K&R C.  It needs to be
 * linked with every program used in the entire book.
 *
 */
#include <stdio.h>
#include <stdlib.h>

#define PACIFIER_COUNT 2047
typedef struct bit_file {
FILE *file;
unsigned char mask;
int rack;
int pacifier_counter;
} BIT_FILE;
BIT_FILE *OpenOutputBitFile( char *name )
{
  BIT_FILE *bit_file;

    bit_file = (BIT_FILE *) calloc( 1, sizeof( BIT_FILE ) );
    if ( bit_file == NULL )
        return( bit_file );
    bit_file->file = fopen( name, "wb" );
    bit_file->rack = 0;
    bit_file->mask = 0x80;
    bit_file->pacifier_counter = 0;
    return( bit_file );
}

BIT_FILE *OpenInputBitFile( char *name )
{
    BIT_FILE *bit_file;

    bit_file = (BIT_FILE *) calloc( 1, sizeof( BIT_FILE ) );
    if ( bit_file == NULL )
	return( bit_file );
    bit_file->file = fopen( name, "rb" );
    bit_file->rack = 0;
    bit_file->mask = 0x80;
    bit_file->pacifier_counter = 0;
    return( bit_file );
}

void CloseOutputBitFile(BIT_FILE *bit_file )
{
    if ( bit_file->mask != 0x80 )
        putc( bit_file->rack, bit_file->file ) != bit_file->rack;
      
    fclose( bit_file->file );
    free( (char *) bit_file );
}

void CloseInputBitFile(BIT_FILE *bit_file )
{
    fclose( bit_file->file );
    free( (char *) bit_file );
}

void OutputBit( BIT_FILE *bit_file, int bit )
{
  if ( bit )
    bit_file->rack |= bit_file->mask;
  bit_file->mask >>= 1;
  if ( bit_file->mask == 0 ) {
    putc( bit_file->rack, bit_file->file ) != bit_file->rack;
  
    /*else
      if ( ( bit_file->pacifier_counter++ & PACIFIER_COUNT ) == 0 )
      putc( '.', stdout );*/
    bit_file->rack = 0;
    bit_file->mask = 0x80;
  }
}

void OutputBits(BIT_FILE *bit_file,unsigned long code, int count )
{
    unsigned long mask;
    
    mask = 1L << ( count - 1 );
    while ( mask != 0) {
      if ( mask & code )
	bit_file->rack |= bit_file->mask;
      bit_file->mask >>= 1;
      if ( bit_file->mask == 0 ) {
	 putc( bit_file->rack, bit_file->file ) != bit_file->rack ;
 
        /*else if ( ( bit_file->pacifier_counter++ & PACIFIER_COUNT ) == 0 )
	  putc( '.', stdout );*/
	bit_file->rack = 0;
	bit_file->mask = 0x80;
      }
      mask >>= 1;
    }
}

int InputBit( BIT_FILE *bit_file )
{
  int value;
  
  if ( bit_file->mask == 0x80 ) {
    bit_file->rack = getc( bit_file->file );

    /*if ( ( bit_file->pacifier_counter++ & PACIFIER_COUNT ) == 0 )
      putc( '.', stdout );*/
  }
  value = bit_file->rack & bit_file->mask;
  bit_file->mask >>= 1;
  if ( bit_file->mask == 0 )
    bit_file->mask = 0x80;
  return( value ? 1 : 0 );
}

unsigned long InputBits( BIT_FILE *bit_file, int bit_count )
{
    unsigned long mask;
    unsigned long return_value;

    mask = 1L << ( bit_count - 1 );
    return_value = 0;
    while ( mask != 0) {
      if ( bit_file->mask == 0x80 ) {
	bit_file->rack = getc( bit_file->file );
        /*if ( ( bit_file->pacifier_counter++ & PACIFIER_COUNT ) == 0 )
	  putc( '.', stdout );*/
      }
      if ( bit_file->rack & bit_file->mask )
	return_value |= mask;
      mask >>= 1;
      bit_file->mask >>= 1;
      if ( bit_file->mask == 0 )
	bit_file->mask = 0x80;
    }
    return( return_value );
}

void FilePrintBinary( FILE *file, unsigned int code, int bits )
{
    unsigned int mask;

    mask = 1 << ( bits - 1 );
    while ( mask != 0 ) {
        if ( code & mask )
            fputc( '1', file );
        else
            fputc( '0', file );
        mask >>= 1;
    }
}


/*************************** End of BITIO.C **************************/
 
 //
 // The NODE structure is a node in the Huffman decoding tree.  It has
 // a count, which is its weight in the tree, and the node numbers of
 // its two children.  
 //
 typedef struct _NODE 
 {
   unsigned int uWeight;        // scaled byte count
   int iLeftChild;              // child zero
   int iRightChild;             // child one
 } NODE;
 
 //
 // Since walking the Huffman tree is a slow process we only want to do it
 // once for each symbol (not every time we see the symbol).  When encoding,
 // first walk the tree for each symbol and save the codes in a code table.
 // This is a code table entry.
 //
 typedef struct _CODE 
 {
   unsigned int uCode;
   int iCodeBits;
 } CODE;
 
 //
 // The special EOS symbol is 256, the first available symbol after all
 // of the possible bytes.  When decoding, reading this symbols
 // indicates that all of the data has been read in.
 //
 #define END_OF_STREAM  256
 
 //
 // Local function prototypes, defined with ANSI rules only.
 //
 void compress_data(FILE *pfInput, BIT_FILE *pbfOutput, CODE *pcCodes);
 
 BOOL output_counts(FILE *pfOutputFile, NODE *pnNodes);
 BOOL input_counts(FILE *pfInputFile, NODE *pnNodes);
 void count_bytes(FILE *pfInput, unsigned long *puCounts);
 void scale_counts(unsigned long *puCounts, NODE *pnNodes);
 int build_tree(NODE *pnNodes);
 void convert_tree_to_code(NODE *pnNodes, CODE *pcCodes, 
 			  unsigned int iCodeSoFar, int iBits, int iNode);
 
 
 
 
 //
 // This routine works in a fairly straightforward manner.  First, it
 // has to allocate storage for three different arrays of data.  Next,
 // it counts all the bytes in the input file.  The counts are all
 // stored in long int, so the next step is scale them down to single
 // byte counts in the NODE array.  After the counts are scaled, the
 // Huffman decoding tree is built on top of the NODE array.  Another
 // routine walks through the tree to build a table of codes, one per
 // symbol.  Finally, when the codes are all ready, compressing the
 // file is a simple matter.  After the file is compressed, the storage
 // is freed up, and the routine returns.
 //
 
 BOOL CompressFile(FILE *pfInputFile, BIT_FILE *pbfOutputFile)
 {
   unsigned long *puCounts = NULL;
   NODE *pnNodes = NULL;
   CODE *pcCodes = NULL;
   int iRootNode;
   BOOL fRetVal = FALSE;
 
   //
   // Allocate memory:
   //
 
   //
   // This will keep track of the count for each ASCII byte in the input
   // file.
   //
   puCounts = (unsigned long *) malloc( 256 * sizeof(unsigned long));
   if (NULL == puCounts)
   {
     fprintf(stderr, "Out of memory.\n");
     goto CompressReturn;
   }
   memset(puCounts, 0, 256 * sizeof(unsigned long));
 
   //
   // These will be the nodes on the Huffman tree; 514 is the largest number
   // we need because letter data will sit at leaf nodes.  Since in a tree
   // with n leaf nodes there will be (n-1) internal nodes and there are
   // 257 possible characters (256 byte values and 1 made up EOS character)
   // they will need 256 internal nodes.  That's 513 total nodes.
   //
   pnNodes = (NODE *) malloc (514 * sizeof(NODE));
   if (NULL == pnNodes)
   {
     fprintf(stderr, "Out of memory.\n");
     goto CompressReturn;
   }
 
   //
   // These will make up the byte -> code mapping table.  We need 257 because
   // we need one for every byte value plus one for the EOS character we made
   // up.
   //
   pcCodes = (CODE *) malloc(257 * sizeof(CODE));
   if (NULL == pcCodes) 
   {
     fprintf(stderr, "Out of memory.\n");
     goto CompressReturn;
   }
 
   //
   // Run through the input stream and count the frequency of each byte.
   // Place the data in the counts table.
   //
   count_bytes(pfInputFile, puCounts);
 
   //
   // This scales the weights of characters counted in the count_bytes
   // routine down so as to limit the size of the Huffman codes (yet to
   // be generated) to at most 16 bits each.
   //
   scale_counts(puCounts, pnNodes);
 
   //
   // Write the counts to the output file header
   //
   if (!output_counts(pbfOutputFile->file, pnNodes ))
   {
     goto CompressReturn;
   }
 
   //
   // Build up the Huffman tree by combining the count trees
   // 
   if (!(iRootNode = build_tree( pnNodes )))
   {
     goto CompressReturn;
   }
 
   //
   // Walk the tree for each character symbol and get a code value for
   // each one
   //
   convert_tree_to_code(pnNodes, pcCodes, 0, 0, iRootNode);
 
   //
   // Do the actual compression
   //
   compress_data( pfInputFile, pbfOutputFile, pcCodes );
   
   //
   // We made it!
   //
   fRetVal = TRUE;
 
 CompressReturn:
 
   //
   // Cleanup
   //
   if (puCounts) free(puCounts);
   if (pnNodes)  free(pnNodes);
   if (pcCodes)  free(pcCodes);
 
   return(fRetVal);
 }
 
 
  
 void expand_data(BIT_FILE *pbfInput, FILE *pfOutput, NODE *pnNodes, 
 		 int iRootNode) ;
 BOOL ExpandFile(BIT_FILE *pbfInputFile, FILE *pfOutput)
 {
   NODE *pnNodes;
   int iRootNode;
 
   //
   // Allocate memory for the tree -- 514 is the max sizeof the tree because
   // there are at most 257 leaf nodes and therefore max 256 internal nodes
   // and we use one node position to store a HUGE tree while building it.
   //
   if ((pnNodes = (NODE *) malloc(514 * sizeof(NODE))) == NULL) 
   {
     fprintf(stderr, "Out of memory.\n");
     return(FALSE);
   }
 
   //
   // This routine reads the header count table from the compressed image
   //
   if (!input_counts(pbfInputFile->file, pnNodes))
   {
     free(pnNodes);
     return(FALSE);
   }
 
   //
   // Build the Huffman tree from the table we read in the last step.
   //
   iRootNode = build_tree(pnNodes);
 
   //
   // Do the actual expansion of the compressed file.
   //
   expand_data(pbfInputFile, pfOutput, pnNodes, iRootNode);
 
   // 
   // Cleanup and return
   //
   free(pnNodes);
   return(TRUE);
 }
 
 
 
 //
 // In order for the compressor to build the same model, I have to store
 // the symbol counts in the compressed file so the expander can read
 // them in.  In order to save space, I don't save all 256 symbols
 // unconditionally.  Instead store only characters with a positive count
 // in the format:
 //
 // char, count, char, count, ... 0, 0 (beginning of compressed data)
 //
 // The code from "The Data Compression Book" stores them in a different
 // (more efficient) format.  Unfortunately, I don't think the extra space
 // savings are worth complicating the code.  See the source for a better
 // way of doing this.
 //
 BOOL output_counts(FILE *pfOutputFile, NODE *pnNodes )
 {
   int iFirst = 0;
   int i;
 
   //
   // Write each non-zero count
   //
   for (iFirst = 0; iFirst < 256; iFirst++)
   {
     if (pnNodes[iFirst].uWeight != 0)
     {
       if (putc(iFirst, pfOutputFile) != iFirst)
       {
 	perror("putc");
 	return(FALSE);
       }
 
       if (putc(pnNodes[iFirst].uWeight, pfOutputFile) != 
 	  pnNodes[iFirst].uWeight)
       {
 	perror("putc");
 	return(FALSE);
       }
 
     }
   }
 
   //
   // Write the 0 0 terminator part
   //
   for (iFirst = 0; iFirst < 2; iFirst++) 
   {
     if (putc(0, pfOutputFile) != 0)
     {
       perror("putc");
       return(FALSE);
     }
   }
 
   return(TRUE);
 }
 
 
 
 
 //
 // Read the counts from a compressed file header
 //
 BOOL input_counts(FILE *pfInputFile, NODE *pnNodes)
 {
   int i;
   int iChar;
   int iCount;
 
   //
   // Initialize all counts to zero
   //
   for (i = 0; i < 256; i++)
   {
     pnNodes[i].uWeight = 0;
   }
 
   while(1)
   {
 
     //
     // Read the character
     //
     if ((iChar = getc(pfInputFile)) == EOF) 
     {
       fprintf(stderr, "Error reading byte counts\n");
       return(FALSE);
     }
     
     //
     // Read the count (hi and low byte)
     //
     if ((iCount = getc(pfInputFile)) == EOF)
     {
       fprintf(stderr, "Error reading byte counts\n");
       return(FALSE);
     }
     
     if (iCount) 
     {
       pnNodes[iChar].uWeight = (unsigned) iCount;
     }
     else
     {
       break;
     }
   }
   pnNodes[END_OF_STREAM].uWeight = 1;
   return(TRUE);
 }
 
 
 //
 // This routine counts the frequency of occurence of every byte in
 // the input file.  It marks the place in the input stream where it
 // started, counts up all the bytes, then returns to the place where
 // it started.  In most C implementations, the length of a file
 // cannot exceed an unsigned long, so this routine should always
 // work.
 //
 // This routine could possible be sped up by buffering the input file
 // and counting characters in the buffer.  Hopefully your operating
 // system is doing some buffering behind your back, though...
 //
 void count_bytes(FILE *pfInput, unsigned long *puCounts)
 {
   long lInputMarker;
   int c;
 
   //
   // Preconditions
   //
 
 
   //
   // Remember where the file pointer is now.
   //
   lInputMarker = ftell(pfInput);

   
   //
   // Tally the characters from here to EOF
   //
   while ((c = getc(pfInput)) != EOF)
     puCounts[c]++;
   
   //
   // Put the pointer back where it was.
   //
   fseek(pfInput, lInputMarker, SEEK_SET );
    (ftell(pfInput) == lInputMarker);
 }
 
 
 //
 // In order to limit the size of my Huffman codes to 16 bits, I scale
 // my counts down so they fit in an unsigned char, and then store them
 // all as initial weights in my NODE array.  The only thing to be
 // careful of is to make sure that a node with a non-zero count doesn't
 // get scaled down to 0.  Nodes with values of 0 don't get codes.
 //
 void scale_counts(unsigned long *puCounts, NODE *pnNodes)
 {
   unsigned long uMaxCount;
   int i;
 
   //
   // Preconditions
   //
    (puCounts);
    (pnNodes);
 
   //
   // Run through the counts and look for the one with the greatest weight.
   //
   uMaxCount = 0;
   for (i = 0; i < 256; i++)
   {
     if (puCounts[i] > uMaxCount)
       uMaxCount = puCounts[i];
   }
 
   //
   // If there were no characters in the count table (i.e. we are trying
   // to compress an empty file) make the max count one so the scaling
   // formula works right.
   //
   if (uMaxCount == 0)
   {
     puCounts[0] = 1;
     uMaxCount = 1;
   }
 
   //
   // Now create the node weights for each symbol in the input stream --
   // use the counts scaled down by 1 / uMaxCount
   //
   uMaxCount /= 255;
   uMaxCount += 1;
   for (i = 0; i < 256; i++) 
   {
     pnNodes[i].uWeight = (unsigned int) (puCounts[i] / uMaxCount);
 
     // 
     // As the comment stated, make sure we never scale too much such that
     // a non-zero count byte achieves a scaled weight of zero.
     //
     if ((pnNodes[i].uWeight == 0) && (puCounts[i] != 0))
     {
       pnNodes[i].uWeight = 1;
     }
   }
 
   //
   // Special end of stream symbol
   //
   pnNodes[END_OF_STREAM].uWeight = 1;
 }
 
 
 
 //
 // Building the Huffman tree is fairly simple.  All of the active nodes
 // are scanned in order to locate the two nodes with the minimum
 // weights.  These two weights are added together and assigned to a new
 // node.  The new node makes the two minimum nodes into its 0 child
 // and 1 child.  The two minimum nodes are then marked as inactive.
 // This process repeats until their is only one node left, which is the
 // root node.  The tree is done, and the root node is passed back
 // to the calling routine.
 //
 // Node 513 is used here to arbitratily provide a node with a guaranteed
 // maximum value.  It starts off being min_1 and min_2 and then we look
 // for other nodes two other nodes that have smaller values.  After all 
 // active (i.e. non-zero weight) nodes have been scanned, we can tell if
 // there is really only one active node left in the pool by checking to
 // see if one of the min pointers is still set to 513 (the huge node).
 //
 int build_tree(NODE *pnNodes)
 {
   int iNextFree;
   int i;
   int iMin1;
   int iMin2;
 
    (pnNodes);
 
   //
   // This node is guaranteed max value.
   //
   pnNodes[513].uWeight = 0xffff;
 
   //
   // NextFree will be used to store positions of internal nodes.  Start
   // at 257 (0..256 may be character leaves, 257 is our little end of 
   // stream symbol) and work up.
   //
   for (iNextFree = END_OF_STREAM + 1 ; ; iNextFree++)
   {
      (iNextFree < 513);
 
     //
     // The minimum we have found so far (max possible weight value)
     //
     iMin1 = 513;
     iMin2 = 513;
 
     //
     // This could be done more efficiently by sorting
     //
     for (i = 0; i < iNextFree; i++)
     {
       if (pnNodes[i].uWeight != 0)
       {
 	if (pnNodes[i].uWeight < pnNodes[iMin1].uWeight) 
         {
 	  iMin2 = iMin1;
 	  iMin1 = i;
 	} 
 	else if (pnNodes[i].uWeight < pnNodes[iMin2].uWeight) 
 	{
 	  iMin2 = i;
 	}
       }
     }
 
     //
     // Is there only one tree (the tree and the fat node at 513)?
     //
     if (iMin2 == 513)
     {
       break;
     }
 	
     //
     // Otherwise combine trees
     //
     pnNodes[iNextFree].uWeight = pnNodes[iMin1].uWeight;
     pnNodes[iNextFree].uWeight += pnNodes[iMin2].uWeight;
 
     //
     // These two nodes no longer in consideration for smallest...
     //
     pnNodes[iMin1].uWeight = 0;
     pnNodes[iMin2].uWeight = 0;
 
     pnNodes[iNextFree].iLeftChild = iMin1;
     pnNodes[iNextFree].iRightChild = iMin2;
   }
 
   //
   // Subtract one to get last used -- the root node
   //
   iNextFree--;
   return(iNextFree);
 }
 
 
 
 //
 // Since the Huffman tree is built as a decoding tree, there is
 // no simple way to get the encoding values for each symbol out of
 // it.  This routine recursively walks through the tree, adding the
 // child bits to each code until it gets to a leaf.  When it gets
 // to a leaf, it stores the code value in the CODE element, and
 // returns.
 //
 // This is a really cool routine... check out how it works.
 //
 void convert_tree_to_code(NODE *pnNodes, CODE *pcCodes, 
 			  unsigned int iCodeSoFar, int iBits, int iNode)
 {
   //
   // Preconditions
   //
    (pnNodes);
    (pcCodes);
 
   //
   // If this is a leaf node we are done recursing, assign code and pop stack
   //
   if (iNode <= END_OF_STREAM) 
   {
      (iBits);
      (iCodeSoFar);
 
     //
     // Code
     //
     pcCodes[iNode].uCode = iCodeSoFar;
 
     //
     // Length of code
     //
     pcCodes[iNode].iCodeBits = iBits;
     return;
   }
 
   //
   // Otherwise we are on an internal node and need to keep going
   //
   iCodeSoFar <<= 1;
    ((iCodeSoFar | 0) == iCodeSoFar);
 
   //
   // One more bit about to be added to the length
   //
   iBits++;
 
   //
   // When going right, add a zero to the code so far..
   //
   convert_tree_to_code(pnNodes, pcCodes, iCodeSoFar, iBits,
 		       pnNodes[iNode].iLeftChild);
 
   //
   // When going left add a one..
   //
   convert_tree_to_code(pnNodes, pcCodes, iCodeSoFar | 1, iBits, 
 		       pnNodes[iNode].iRightChild);
 }
 
 
 //
 // Once the tree gets built, and the CODE table is built, compressing
 // the data is a breeze.  Each byte is read in, and its corresponding
 // Huffman code is sent out.
 //
 void compress_data(FILE *pfInput, BIT_FILE *pbfOutput, CODE *pcCodes)
 {
   int ch;               // used for reading data byte by byte from input
 
   //
   // Preconditions
   //
    (pfInput);
    (pbfOutput);
    (pcCodes);
 
   //
   // For each character in input, output the equivalent code
   //
   while ((ch = getc(pfInput)) != EOF)
   {
     OutputBits(pbfOutput, (unsigned long) pcCodes[ch].uCode,
 	       pcCodes[ch].iCodeBits);
   }
 
   //
   // EOS mark -- needed in the expansion process
   //
   OutputBits(pbfOutput, (unsigned long) pcCodes[END_OF_STREAM].uCode,
 	     pcCodes[END_OF_STREAM].iCodeBits);
 }
 
 
 
 //
 // Expanding compressed data is a little harder than the compression
 // phase.  As each new symbol is decoded, the tree is traversed,
 // starting at the root node, reading a bit in, and taking either the
 // iLeftChild or iRightChild path.  Eventually, the tree winds down to a
 // leaf node, and the corresponding symbol is output.  If the symbol
 // is the END_OF_STREAM symbol, it doesn't get written out, and
 // instead the whole process terminates.
 //
 
 void expand_data(BIT_FILE *pbfInput, FILE *pfOutput, NODE *pnNodes, 
 		 int iRootNode) 
 {
   int iNode;                 // the index of the node in the huffman tree for
                              // the current input symbol.
 
   //
   // Preconditions
   //
    (pbfInput);
    (pfOutput);
    (pnNodes);
 
   while (1)
   {
     iNode = iRootNode;
 
     //
     // We will read the input file bit by bit and move the node pointer one
     // level in the tree for each bit read.  We will stop when we get to a
     // leaf node, write the uncompressed character to output, reset the node
     // pointer to the root, and repeat the process.
     //
 
     do 
     {
 
       //
       // If we read a set bit (i.e. a "1") then traverse right else 
       // it's a "0" so left in the Huff tree..
       //
       if (InputBit(pbfInput))
 	iNode = pnNodes[iNode].iRightChild;
       else
 	iNode = pnNodes[iNode].iLeftChild;
      
 
     } 
     while (iNode > END_OF_STREAM);
     //
     // i.e. while we're pointing to an internal node.. stop at a leaf.
     //
 
     //
     // We are now at a leaf node.  Determine if this is a real symbol or
     // the end of file (represented by a made up EOS character).
     //
     if (iNode == END_OF_STREAM)
       break;
 
     //
     // It's a real thing... put it.
     //
     if ((putc(iNode, pfOutput)) != iNode)
       fprintf(stderr, "Error writing to output file!\n");
   }
 }