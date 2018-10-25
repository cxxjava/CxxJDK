/**
 * @file  eso_lz77.h
 * @brief ES eso zip api.
 */

#include "eso_lz77.h"
#include "eso_libc.h"
#include "eso_util.h"
#include "eso_debug.h"

#define ES_ZIP_BUF_SIZE   2048

// basic unsigned integer types
typedef unsigned char  WORD8;   // unsigned 8bit integer, prefix "b"
typedef unsigned short WORD16;  // unsigned 8bit integer, prefix "w"
typedef unsigned int   WORD32;  // unsigned 8bit integer, prefix "l"

// boolean definitions
typedef signed char BYTEBOOL;  // 8bit boolean, prefix "bl"

#define BOOL_FALSE  0
#define BOOL_TRUE   1

//==============================================================================
// lz77
//==============================================================================

// the condition codes, these are bitflags
#define  LZSS_START  1
#define  LZSS_WORK   2
#define  LZSS_STOP   4

// i/o exception codes
#define LZSS_EOB         257    // end of buffer
#define LZSS_EOD         258    // end of data

// some compressor constants
#define LZSS_N           512    // size of ring buffer
#define LZSS_F            18    // upper limit for match_length
#define LZSS_THRESHOLD     2    // encode string into position and length
// if match_length is greater than this
#define LZSS_NIL      LZSS_N    // index for root of binary search trees

struct LZSSCTX {
	// state freeze for compression
	BYTEBOOL blSaveDone;
	WORD16 nSaveI, nSaveR, nSaveC, nSaveLen, nSaveS;
	WORD16 nSaveLastMatchLength, nSaveCodeBufPtr;
	WORD8 bSaveMask;
	WORD8 saveCode_buf[17];
	// state freeze for decompression (additional necessary data)
	WORD16 nSaveJ, nSaveK;
	WORD16 wSaveFlags;
	// the interrupt point code
	WORD16 wInterruptPoint;

	// general runtime stuff
	WORD8 text_buf[LZSS_N + LZSS_F - 1];	  // ring buffer of size N, with
											  // extra F-1 bytes to facilitate
											  // string comparison
	WORD16 nMatchPosition, nMatchLength;   // of longest match.
	// these are set by the insertNode() procedure.
	WORD16 lson[LZSS_N + 1];	 // left & right children & parents
	WORD16 rson[LZSS_N + 257];	 // these constitute binary search trees
	WORD16 dad[LZSS_N + 1];
	// for readByte() and writeByte()
	WORD32 lSourceSize;
	WORD32 lDrainSize;
	WORD32 lBytesRead;
	WORD32 lBytesWritten;
	WORD8* pDataSource;
	WORD8* pDataDrain;
	// End Of Data (stream)
	BYTEBOOL blEOD;
};

// LZSS context
typedef struct LZSSCTX  LZSSCTX, *PLZSSCTX;

static PLZSSCTX LZSS_Create(void) 
{
	return(PLZSSCTX) eso_malloc(sizeof(LZSSCTX));
}

static void LZSS_Destroy(PLZSSCTX pCtx) 
{
	if (pCtx) eso_free(pCtx);
}

// initialize trees
static void initTree(PLZSSCTX pCtx) 
{
	WORD16 nI;

	// for nI = 0 to LZSS_N - 1, rson[nI] and lson[nI] will be the right and
	// left children of node nI.  These nodes need not be initialized.
	// Also, dad[nI] is the parent of node nI.  These are initialized to
	// LZSS_NIL (= LZSS_N), which stands for 'not used.'
	// For nI = 0 to 255, rson[LZSS_N + nI + 1] is the root of the tree
	// for strings that begin with character nI.  These are initialized
	// to LZSS_NIL.  Note there are 256 trees.
	for (nI = LZSS_N + 1; nI <= (LZSS_N + 256); nI++)
		pCtx->rson[nI] = LZSS_NIL;
	for (nI = 0; nI < LZSS_N; nI++)
		pCtx->dad[nI] = LZSS_NIL;
}

// inserts a mode into tree
static void insertNode(PLZSSCTX pCtx, WORD16 nR) 
{
	// copy some context members to local members to
	// increase the execution speed
	WORD8* text_buf = pCtx->text_buf;
	WORD16* lson = pCtx->lson;
	WORD16* rson = pCtx->rson;
	WORD16* dad = pCtx->dad;
	WORD16 nMatchPosition = pCtx->nMatchPosition;
	WORD16 nMatchLength = 0;

	// local variables
	WORD8* pKey = &text_buf[nR];
	WORD16 nI;
	WORD16 nP = LZSS_N + 1 + pKey[0];
	int nCmp = 1;

	// (Inserts string of length LZSS_F, text_buf[nR..nR + LZSS_F - 1], into one
	// of the trees (text_buf[nR]'th tree) and returns the longest-match position
	// and length via the global variables nMatchPosition and nMatchLength. If 
	// nMatchLength = F, then removes the old node in favor of the new one,
	// because the old one will be deleted sooner. Note nR plays double role, as
	// tree node and position in buffer.)

	lson[nR] = rson[nR] = LZSS_NIL;
	for (;;) {
		if (nCmp >= 0) {
			if (rson[nP] != LZSS_NIL)
				nP = rson[nP];
			else {
				rson[nP] = nR;
				dad[nR] = nP;
				pCtx->nMatchPosition = nMatchPosition;
				pCtx->nMatchLength = nMatchLength;
				return;
			}
		} else {
			if (lson[nP] != LZSS_NIL)
				nP = lson[nP];
			else {
				lson[nP] = nR;
				dad[nR] = nP;
				pCtx->nMatchPosition = nMatchPosition;
				pCtx->nMatchLength = nMatchLength;
				return;
			}
		}
		for (nI = 1; nI < LZSS_F; nI++) {
			if ((nCmp = pKey[nI] - text_buf[nP + nI]) != 0)	 break;
		}
		if (nI > nMatchLength) {
			nMatchPosition = nP;
			if ((nMatchLength = nI) >= LZSS_F) break;
		}
	}
	dad[nR] = dad[nP];
	lson[nR] = lson[nP];
	rson[nR] = rson[nP];
	dad[lson[nP]] = nR;
	dad[rson[nP]] = nR;
	if (rson[dad[nP]] == nP) rson[dad[nP]] = nR;
	else					 lson[dad[nP]] = nR;
	dad[nP] = LZSS_NIL;	// remove nP

	pCtx->nMatchPosition = nMatchPosition;
	pCtx->nMatchLength = nMatchLength;
}

// deletes node p from tree
static void deleteNode(PLZSSCTX pCtx, WORD16 nP)
{

	// copy some context members to local members to
	// to increase the execution speed
	WORD16* lson = pCtx->lson;
	WORD16* rson = pCtx->rson;
	WORD16* dad = pCtx->dad;

	// local variable
	WORD16 nQ;

	// start...
	if (dad[nP] == LZSS_NIL) return;  // not in tree
	if (rson[nP] == LZSS_NIL)
		nQ = lson[nP];
	else {
		if (lson[nP] == LZSS_NIL)
			nQ = rson[nP];
		else {
			nQ = lson[nP];
			if (rson[nQ] != LZSS_NIL) {
				do {
					nQ = rson[nQ];
				} 
				while (rson[nQ] != LZSS_NIL);

				rson[dad[nQ]] = lson[nQ];
				dad[lson[nQ]] = dad[nQ];
				lson[nQ] = lson[nP];
				dad[lson[nP]] = nQ;
			}
			rson[nQ] = rson[nP];
			dad[rson[nP]] = nQ;
		}
	}
	dad[nQ] = dad[nP];
	if (rson[dad[nP]] == nP) rson[dad[nP]] = nQ;
	else					 lson[dad[nP]] = nQ;
	dad[nP] = LZSS_NIL;
}

// reads a byte from the input buffer and checks if the 
// buffer run out of data
static WORD16 readByte(PLZSSCTX pCtx) 
{
	// enough bytes?
	if (pCtx->lBytesRead < pCtx->lSourceSize)
		// return the actual byte in the buffer
		return pCtx->pDataSource[pCtx->lBytesRead++];
	if (pCtx->blEOD)
		return LZSS_EOD; // end of data
	else
		return LZSS_EOB; // end of buffer, but we'll be back
}

// writes a byte to the output buffer, used for compression,
// does no range check because compressed data won't be much
// larger (in the worst case) than the original data (and an
// additional check will cost too much overhead)
static void cWriteByte(PLZSSCTX pCtx, WORD8 bVal) 
{
	// just write the byte to the output buffer
	pCtx->pDataDrain[pCtx->lBytesWritten++] = bVal;
}

// writes a byte to the output buffer and checks if the buffer
// is full, used for decompression (because just a hundred of
// compressed bytes might create millions of original bytes)
static BYTEBOOL dWriteByte(PLZSSCTX pCtx, WORD8 bVal) 
{
	// enough free space?
	if (pCtx->lBytesWritten < pCtx->lDrainSize) {
		pCtx->pDataDrain[pCtx->lBytesWritten++] = bVal;
		return BOOL_TRUE;
	}
	return BOOL_FALSE;
}

// macro to save the local variables in a context, used in LZSS_Compress()
#define COMPRESS_SAVE_LOCAL_VAR pCtx->blSaveDone = blDone;                     \
                                pCtx->nSaveI = nI;                             \
                                pCtx->nSaveC = nC;                             \
                                pCtx->nSaveLen = nLen;                         \
                                pCtx->nSaveR = nR;                             \
                                pCtx->nSaveS = nS;                             \
                                pCtx->nSaveLastMatchLength = nLastMatchLength; \
                                pCtx->nSaveCodeBufPtr = nCodeBufPtr;           \
                                pCtx->bSaveMask = bMask;                       \
                                memcpy(pCtx->saveCode_buf, code_buf, 17);

static WORD32 LZSS_Compress(PLZSSCTX pCtx, 
							const void* pSource, 
							void* pTarget, 
							WORD32 lNumOfBytes, 
							WORD8 bCondition) 
{
	BYTEBOOL blDone;
	WORD16 nI, nC, nLen, nR, nS, nLastMatchLength, nCodeBufPtr;
	WORD8 bMask;
	WORD8 code_buf[17];
	WORD16 wTemp;  // (this variable must not be saved)

	// first setup the i/o pointers and the counters
	pCtx->pDataSource = (WORD8*) pSource;
	pCtx->pDataDrain = (WORD8*) pTarget;
	pCtx->lSourceSize = lNumOfBytes;
	pCtx->lBytesRead = 0;
	pCtx->lBytesWritten = 0;

	// end of data stream?
	if ((bCondition & LZSS_STOP) == LZSS_STOP) pCtx->blEOD = BOOL_TRUE;
	else pCtx->blEOD = BOOL_FALSE;

	// must we first launch the compression engine?
	if ((bCondition & LZSS_START) == LZSS_START) {
		// (populate state things, otherwise the debug version will fail because
		//  of assumed missing initialization)
		nC = nLen = nR = nS = 0;
		nLastMatchLength = nCodeBufPtr = 0;
		goto ENTRYPOINT1;
	}

	// no, restore the local variables...
	blDone = pCtx->blSaveDone;
	nI = pCtx->nSaveI;
	nC = pCtx->nSaveC;
	nLen = pCtx->nSaveLen;
	nR = pCtx->nSaveR;
	nS = pCtx->nSaveS;
	nLastMatchLength = pCtx->nSaveLastMatchLength;
	nCodeBufPtr = pCtx->nSaveCodeBufPtr;
	bMask = pCtx->bSaveMask;
	memcpy(code_buf, pCtx->saveCode_buf, 17);

	// ...and jump to the last interruption point
	switch (pCtx->wInterruptPoint) {
	case 2 : goto ENTRYPOINT2;
	case 3 : goto ENTRYPOINT3;
	}

	// here we start with the engine setup
	ENTRYPOINT1:
	initTree(pCtx);	 // initialize trees
	code_buf[0] = 0;  // code_buf[1..16] saves eight units of code, and
					  // code_buf[0] works as eight flags, "1" representing that
					  // the unit is an unencoded letter (1 byte), "0" a position
					  // -and-length pair (2 bytes).  Thus, eight units require at
					  // most 16 bytes of code.
	nCodeBufPtr = bMask =1;
	nS = 0;
	nR = LZSS_N - LZSS_F;

	// clear the buffer with any character that might appear often (SPACE)
	for (nI = nS; nI < nR; nI++) pCtx->text_buf[nI] = ' ';
	nLen = 0;
	blDone = BOOL_FALSE;

	while (blDone == BOOL_FALSE) {
		// the following entry point will guarantee, that the readByte() call will
		// be repeated when the engine is re-activated, same technique used below
		ENTRYPOINT2:
		wTemp = readByte(pCtx);
		// must we interrupt the engine?
		if (wTemp == LZSS_EOB) {
			COMPRESS_SAVE_LOCAL_VAR
			pCtx->wInterruptPoint = 2;
			// return the number of bytes written (will be zero here)
			return pCtx->lBytesWritten;
		}

		// has the stream come to its end?
		if (wTemp == LZSS_EOD) blDone = BOOL_TRUE;
		else {
			// read LZSS_F bytes into the last LZSS_F bytes of the buffer
			pCtx->text_buf[nR + nLen] = (WORD8) wTemp; 
			if (++nLen >= LZSS_F) blDone = BOOL_TRUE;
		}
	}

	// nothing to compress at all?
	if (nLen == 0) return 0;

	// Insert the LZSS_F strings, each of which begins with one or more 'space'
	// characters. Note the order in which these strings are inserted. This way,
	// degenerated trees will be less likely to occur.
	for (nI = 1; nI <= LZSS_F; nI++) insertNode(pCtx, (WORD16)(nR - nI)); 

	// Finally, insert the whole string just read. The context variables
	// nMatchLength and nMatchPosition are set.
	insertNode(pCtx, nR);

	do {
		// match_length may be spuriously long near the end of text.
		if (pCtx->nMatchLength > nLen) pCtx->nMatchLength = nLen;

		if (pCtx->nMatchLength <= LZSS_THRESHOLD) {
			pCtx->nMatchLength = 1;	 // not long enough match, send one byte
			code_buf[0] |= bMask;  // "send one byte" flag
			code_buf[nCodeBufPtr++] = pCtx->text_buf[nR];  // send uncoded
		} else {
			code_buf[nCodeBufPtr++] = (WORD8) pCtx->nMatchPosition;

			// send position and length pair, note nMatchLength > LZSS_THRESHOLD.
			code_buf[nCodeBufPtr++] = (WORD8) (((pCtx->nMatchPosition >> 4) & 0xf0) |
											   (pCtx->nMatchLength - (LZSS_THRESHOLD + 1)));
		}
		// shift mask left one bit
		if ((bMask <<= 1) == 0) {
			// send at most 8 units of code together
			for (nI = 0; nI < nCodeBufPtr; nI++)
				cWriteByte(pCtx, code_buf[nI]);
			code_buf[0] = 0;
			nCodeBufPtr = bMask = 1;
		}
		nLastMatchLength = pCtx->nMatchLength;
		nI = 0;
		blDone = BOOL_FALSE;
		while (blDone == BOOL_FALSE) {
			ENTRYPOINT3:
			wTemp = readByte(pCtx);

			// must we interrupt the engine?
			if (wTemp == LZSS_EOB) {
				COMPRESS_SAVE_LOCAL_VAR
				pCtx->wInterruptPoint = 3;

				// return the number of bytes written
				return pCtx->lBytesWritten;
			}
			// has the stream come to its end?
			if (wTemp == LZSS_EOD)
				blDone = BOOL_TRUE;
			else {
				nC = wTemp;

				// delete old strings and read new bytes
				deleteNode(pCtx, nS);    
				pCtx->text_buf[nS] = (WORD8) nC;

				// if the position is near the end of buffer, extend the buffer to
				// make string comparison easier
				if (nS < (LZSS_F - 1)) pCtx->text_buf[nS + LZSS_N] = (WORD8) nC;

				// since this is a ring buffer, increment the position modulo LZSS_N
				nS = (nS + 1) & (LZSS_N - 1);  
				nR = (nR + 1) & (LZSS_N - 1);  

				// register the string in text_buf[nR..nR + LZSS_F - 1]
				insertNode(pCtx, nR);   
				if (++nI >= nLastMatchLength) blDone = BOOL_TRUE;
			}
		}

		// after the end of text,no need to read, but buffer may not be empty
		while (nI++ < nLastMatchLength) {
			deleteNode(pCtx, nS);                     
			nS = (nS + 1) & (LZSS_N - 1);
			nR = (nR + 1) & (LZSS_N - 1);
			if (--nLen)	insertNode(pCtx, nR);
		}
	} 
	while (nLen > 0); // (until all data has been compressed)

	// send remaining code
	if (nCodeBufPtr > 1)
		for (nI = 0; nI < nCodeBufPtr; nI++) cWriteByte(pCtx, code_buf[nI]);

	// remember that lCodeSize just contains the number of all compressed bytes
	return pCtx->lBytesWritten;
}

// macro to save the local variables in a context, used in LZSS_Decompress()
#define DECOMPRESS_SAVE_LOCAL_VAR pCtx->blSaveDone = blDone;    \
                                  pCtx->nSaveI = nI;            \
                                  pCtx->nSaveJ = nJ;            \
                                  pCtx->nSaveK = nK;            \
                                  pCtx->nSaveR = nR;            \
                                  pCtx->nSaveC = nC;            \
                                  pCtx->wSaveFlags = wFlags;

static WORD32 LZSS_Decompress(PLZSSCTX pCtx, 
							  const void* pSource, 
							  void* pTarget, 
							  WORD32 lNumOfBytes,
							  WORD32 lSizeOfOutputBuffer, 
							  WORD8 bCondition, 
							  BYTEBOOL* pblRepeatMe) 
{
	BYTEBOOL blDone;
	WORD16 nI, nJ, nK, nR, nC;
	WORD16 wFlags;
	WORD16 wTemp; // (this variable mustn't be saved)

	// first set up the i/o pointers and the counters
	pCtx->pDataSource = (WORD8*) pSource;
	pCtx->pDataDrain = (WORD8*) pTarget;
	pCtx->lDrainSize = lSizeOfOutputBuffer;
	pCtx->lSourceSize = lNumOfBytes;
	if (*pblRepeatMe == BOOL_FALSE)	pCtx->lBytesRead = 0;
	pCtx->lBytesWritten = 0;

	// end of data stream?
	if ((bCondition & LZSS_STOP) == LZSS_STOP)
		pCtx->blEOD = BOOL_TRUE;
	else
		pCtx->blEOD	= BOOL_FALSE;

	// must we first launch the compression engine?
	if ((bCondition & LZSS_START) == LZSS_START) {
		blDone = BOOL_FALSE;
		nJ = nK = nC = 0;
		goto ENTRYPOINT1;
	}

	// if not, restore the local variables...
	blDone = pCtx->blSaveDone;
	nI = pCtx->nSaveI;
	nJ = pCtx->nSaveJ;
	nK = pCtx->nSaveK;
	nR = pCtx->nSaveR;
	nC = pCtx->nSaveC;
	wFlags = pCtx->wSaveFlags;

	// ...and jump to the last interrupt point
	switch (pCtx->wInterruptPoint) {
	case 2 : goto ENTRYPOINT2;
	case 3 : goto ENTRYPOINT3;
	case 4 : goto ENTRYPOINT4;
	case 5 : goto ENTRYPOINT5;
	case 6 : goto ENTRYPOINT6;
	case 7 : goto ENTRYPOINT7;
	}

	// here we start with the engine setup
	ENTRYPOINT1:
	for (nI = 0; nI < LZSS_N - LZSS_F; nI++)
		pCtx->text_buf[nI] = ' ';
	nR = LZSS_N - LZSS_F;
	wFlags = 0;
	for (;;) {
		if (((wFlags >>= 1) & 256) == 0) {
			ENTRYPOINT2:
			wTemp = readByte(pCtx);
			// must we interrupt the engine?
			if (wTemp == LZSS_EOB) {
				DECOMPRESS_SAVE_LOCAL_VAR
				pCtx->wInterruptPoint = 2;
				*pblRepeatMe = BOOL_FALSE;
				return pCtx->lBytesWritten;
			}
			// has the stream come to its end?
			if (wTemp == LZSS_EOD) {
				// yes, everything's finished now
				*pblRepeatMe = BOOL_FALSE;
				return pCtx->lBytesWritten;
			}
			nC = wTemp;
			wFlags = (WORD16) (nC | 0xff00); // uses higher byte cleverly to count 8
		}
		if (wFlags & 1) {
			ENTRYPOINT3:
			wTemp = readByte(pCtx);
			// (same input technique as above)
			if (wTemp == LZSS_EOB) {
				DECOMPRESS_SAVE_LOCAL_VAR
				pCtx->wInterruptPoint = 3;
				*pblRepeatMe = BOOL_FALSE;
				return pCtx->lBytesWritten;
			}
			if (wTemp == LZSS_EOD) {
				*pblRepeatMe = BOOL_FALSE;
				return pCtx->lBytesWritten;
			}
			nC = wTemp;
			// can we put out a byte?
			ENTRYPOINT4:
			if (dWriteByte(pCtx, (WORD8) nC) == BOOL_FALSE) {
				// no -> set the repeat flag and interrupt the routine
				DECOMPRESS_SAVE_LOCAL_VAR
				pCtx->wInterruptPoint = 4;
				*pblRepeatMe = BOOL_TRUE;
				return pCtx->lBytesWritten;
			}
			pCtx->text_buf[nR++] = (WORD8) nC;
			nR &= (LZSS_N - 1);
		} else {
			// (same input techniques as above)
			ENTRYPOINT5:
			wTemp = readByte(pCtx);
			if (wTemp == LZSS_EOB) {
				DECOMPRESS_SAVE_LOCAL_VAR
				pCtx->wInterruptPoint = 5;
				*pblRepeatMe = BOOL_FALSE;
				return pCtx->lBytesWritten;
			}
			if (wTemp == LZSS_EOD) {
				*pblRepeatMe = BOOL_FALSE;
				return pCtx->lBytesWritten;
			}
			nI = wTemp;
			ENTRYPOINT6:
			wTemp = readByte(pCtx);
			if (wTemp == LZSS_EOB) {
				DECOMPRESS_SAVE_LOCAL_VAR
				pCtx->wInterruptPoint = 6;
				*pblRepeatMe = BOOL_FALSE;
				return pCtx->lBytesWritten;
			}
			if (wTemp == LZSS_EOD) {
				*pblRepeatMe = BOOL_FALSE;
				return pCtx->lBytesWritten;
			}
			nJ = wTemp;
			nI |= ((nJ & 0xf0) << 4);
			nJ = (nJ & 0x0f) + LZSS_THRESHOLD;
			nK = 0;
			while (nK <= nJ) {
				nC = pCtx->text_buf[(nI + nK) & (LZSS_N - 1)];
				ENTRYPOINT7:
				// (same output technique as above)
				if (dWriteByte(pCtx, (WORD8) nC) == BOOL_FALSE) {
					DECOMPRESS_SAVE_LOCAL_VAR
					pCtx->wInterruptPoint = 7;
					*pblRepeatMe = BOOL_TRUE;
					return pCtx->lBytesWritten;
				}
				pCtx->text_buf[nR++] = (WORD8) nC;
				nR &= (LZSS_N - 1);
				nK++;
			}
		}
	} // of while()
}

//==============================================================================

es_int32_t eso_lz77_zip(es_ostream_t *outStream, es_istream_t *inStream)
{
	es_byte_t bCond;
	es_bool_t blEOF;
	PLZSSCTX ctx;
	es_int32_t lRes, lLen;
	es_size_t lRead;
	es_byte_t *inbuf;
	es_byte_t *outbuf;
	
	lLen = -1;
	if (!outStream || !inStream)
		return -1;
	
	// create the context
	ctx = LZSS_Create();
	inbuf = (es_byte_t*)eso_malloc(ES_ZIP_BUF_SIZE);
	outbuf = (es_byte_t*)eso_malloc(ES_ZIP_BUF_SIZE * 2);
	if (!inbuf || !outbuf) {
		goto DONE;
	}
	
	lLen = 0;
	bCond = LZSS_START | LZSS_WORK;
	blEOF = BOOL_FALSE;
	do {
		lRead = ES_ZIP_BUF_SIZE;
		lRes = inStream->read(inStream, inbuf, &lRead);
		if (lRes != 0) goto DONE;
		
		if (lRead < ES_ZIP_BUF_SIZE)
			blEOF = BOOL_TRUE;
		if (blEOF == BOOL_TRUE)	bCond |= LZSS_STOP;
		lRes = LZSS_Compress(ctx, 
							 inbuf, 
							 outbuf, 
							 lRead, 
							 bCond);
		bCond &= ~LZSS_START;
		lLen += outStream->write(outStream, outbuf, lRes);
	}
	while (blEOF == BOOL_FALSE);

DONE:
	// destroy the context
	LZSS_Destroy(ctx);
	eso_free(inbuf);
	eso_free(outbuf);

	return lLen;
}

es_int32_t eso_lz77_unzip(es_ostream_t *outStream, es_istream_t *inStream)
{
	es_byte_t bCond;
	es_bool_t blEOF;
	PLZSSCTX ctx;
	es_uint32_t lRes, lLen;
	es_size_t lRead;
	es_byte_t *inbuf;
	es_byte_t *outbuf;
	es_bool_t blMustRepeat;
	
	lLen = -1;
	if (!outStream || !inStream)
		return -1;
	
	// create the context
	ctx = LZSS_Create();
	inbuf = (es_byte_t*)eso_malloc(ES_ZIP_BUF_SIZE);
	outbuf = (es_byte_t*)eso_malloc(ES_ZIP_BUF_SIZE * 2);
	if (!inbuf || !outbuf) {
		goto DONE;
	}

	lLen = 0;
	bCond = LZSS_START | LZSS_WORK;
	blEOF = BOOL_FALSE;
	do {
		lRead = ES_ZIP_BUF_SIZE;
		lRes = inStream->read(inStream, inbuf, &lRead);
		if (lRes != 0) goto DONE;
		
		if (lRead < ES_ZIP_BUF_SIZE)
			blEOF = BOOL_TRUE;
		if (blEOF == BOOL_TRUE)	bCond |= LZSS_STOP;
		
		blMustRepeat = BOOL_FALSE; // this is a must
		do {
			es_uint32_t lDecomp = LZSS_Decompress(ctx, 
												  inbuf, 
												  outbuf, 
												  lRead,
												  ES_ZIP_BUF_SIZE * 2, 
												  bCond, 
												  &blMustRepeat);
			bCond &= ~LZSS_START;
			lRes = outStream->write(outStream, outbuf, lDecomp);
			if (lRes != lDecomp) goto DONE;
			lLen += lRes;
		}
		while (blMustRepeat == BOOL_TRUE);
	}
	while (blEOF == BOOL_FALSE);

DONE:
	// destroy the context
	LZSS_Destroy(ctx);
	eso_free(inbuf);
	eso_free(outbuf);

	return lLen;
}

//==============================================================================

//#define ES_ZIP_TEST

#ifdef ES_ZIP_TEST

typedef struct {
	es_istream_t s;
	es_file_t *file;
} CFileInStream;

typedef struct {
	es_ostream_t s;
	es_file_t *file;
} CFileOutStream;

static es_int32_t stream_read_file(void *p, void *buf, es_size_t *size)
{
	CFileInStream *inStream = (CFileInStream*)p;
	return (*size = eso_fread(buf, *size, inStream->file)) >= 0 ? 0 : -1;
}

static es_size_t stream_write_file(void *p, const void *buf, es_size_t size)
{
	CFileOutStream *outStream = (CFileOutStream*)p;
	return eso_fwrite(buf, size, outStream->file);
}


typedef struct {
	es_istream_t s;
	es_byte_t *ptr;
	es_int32_t size;
	es_int32_t pos;
} CMemInStream;

typedef struct {
	es_ostream_t s;
	es_byte_t *ptr;
	es_int32_t size;
	es_int32_t pos;
} CMemOutStream;

static es_int32_t stream_read_mem(void *p, void *buf, es_size_t *size)
{
	CMemInStream *inStream = (CMemInStream*)p;
	
	if (inStream->pos > inStream->size) {
		return -1;
	}
	
	*size = ES_MIN(*size, inStream->size - inStream->pos);
	memcpy(buf, inStream->ptr+inStream->pos, *size);
	inStream->pos += *size;
	
	return 0;
}

static es_size_t stream_write_mem(void *p, const void *buf, es_size_t size)
{
	CMemOutStream *outStream = (CMemOutStream*)p;
	
	if (!outStream->ptr) {
		outStream->ptr = (es_byte_t *)eso_malloc(size);
		if (!outStream->ptr) {
			return -1;
		}
		outStream->size = size;
	}
	else {
		//realloc
		es_byte_t *p = outStream->ptr;
		outStream->ptr = (es_byte_t *)eso_malloc(outStream->size + size);
		memcpy(outStream->ptr, p, outStream->size);
		eso_free(p);
		outStream->size += size;
	}
	memcpy(outStream->ptr + outStream->pos, buf, size);
	outStream->pos += size;
	
	return size;
}

void eso_unzip_test(void)
{
#if 0
	CFileInStream inStream;
	CFileOutStream outStream;
	
	inStream.s.read = stream_read_file;
	outStream.s.write = stream_write_file;

	inStream.file = eso_fopen("e:\\bmw\\tst.z7", "rb");
	outStream.file = eso_fopen("e:\\bmw\\tst.u7", "wb");
	
	eso_lz77_unzip((es_ostream_t*)&outStream, (es_istream_t*)&inStream);
	
	eso_fclose(outStream.file);
	eso_fclose(inStream.file);
#else
	es_file_t *pfI, *pfO;
	CMemInStream inStream;
	CMemOutStream outStream;
	
	memset(&inStream, 0, sizeof(CMemInStream));
	memset(&outStream, 0, sizeof(CMemOutStream));
	
	inStream.s.read = stream_read_mem;
	outStream.s.write = stream_write_mem;
	
	pfI = eso_fopen("e:\\bmw\\tst.z7", "rb");
	inStream.size = eso_fsize(pfI);
	inStream.ptr = (es_byte_t *)eso_malloc(inStream.size);
	eso_fread(inStream.ptr, inStream.size, pfI);
	eso_fclose(pfI);
	
	eso_lz77_unzip((es_ostream_t*)&outStream, (es_istream_t*)&inStream);
	
	pfO = eso_fopen("e:\\bmw\\tst.u7", "wb");
	eso_fwrite(outStream.ptr, outStream.size, pfO);
	eso_free(outStream.ptr);
	eso_fclose(pfO);
#endif
}

void eso_zip_test(void)
{
#if 0
	CFileInStream inStream;
	CFileOutStream outStream;
	
	inStream.s.read = stream_read_file;
	outStream.s.write = stream_write_file;

	inStream.file = eso_fopen("e:\\bmw\\tst.u7", "rb");
	outStream.file = eso_fopen("e:\\bmw\\tst.z7", "wb");
	
	eso_lz77_zip((es_ostream_t*)&outStream, (es_istream_t*)&inStream);
	
	eso_fclose(outStream.file);
	eso_fclose(inStream.file);
#else
	es_file_t *pfI, *pfO;
	CMemInStream inStream;
	CMemOutStream outStream;
	
	memset(&inStream, 0, sizeof(CMemInStream));
	memset(&outStream, 0, sizeof(CMemOutStream));
	
	inStream.s.read = stream_read_mem;
	outStream.s.write = stream_write_mem;
	
	pfI = eso_fopen("e:\\bmw\\tst.u7", "rb");
	inStream.size = eso_fsize(pfI);
	inStream.ptr = (es_byte_t *)eso_malloc(inStream.size);
	eso_fread(inStream.ptr, inStream.size, pfI);
	eso_fclose(pfI);
	
	eso_lz77_zip((es_ostream_t*)&outStream, (es_istream_t*)&inStream);
	
	pfO = eso_fopen("e:\\bmw\\tst.z7", "wb");
	eso_fwrite(outStream.ptr, outStream.size, pfO);
	eso_free(outStream.ptr);
	eso_fclose(pfO);
#endif
}

#endif //!ES_ZIP_TEST
