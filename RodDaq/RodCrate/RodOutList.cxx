// File: RodOutList.cxx
// $Header$
//
// Description:
//  Class RodOutList contains the reply lists which are responses from a 
//  primitive list.
//
// @author Tom Meyer (meyer@iastate.edu) - originator
//

// Namespace for the common routines for SCT and PIXEL ROD software.

#include "RodOutList.h"

namespace SctPixelRod {

// Constructors
	/*
	The only constructor.
	*/
	RodOutList::RodOutList( long length) {
	  m_length = length;
	  m_body = new unsigned long[length];
	}

	/*
	The copy constructor.
	*/
	RodOutList::RodOutList( const RodOutList& rhs){
	  if (this == &rhs) return;
	  m_length = rhs.getLength();
	  m_body = rhs.getBody();
	  return;
	}

// Destructor
	/*
	The destructor. We MUST be sure to delete the space allocated on the heap for m_body.
	For safety, we also set the pointer to zero to avoid pointers into la-la land.
	*/
	RodOutList::~RodOutList() 
	{
	  delete [] m_body;
	  m_body = 0;
	}

        ReplyHead* RodOutList::getMsgHead(int n){
	  if(n > nMsgs())return NULL;
	  ReplyHead* replyHead;
	  char * ptr = (char *) getBody();
	  ptr += sizeof(ReplyListHead);
	  for(int i = 0; i < n; i++){
	    replyHead = (ReplyHead*) ptr;
	    ptr += sizeof(ReplyHead) + sizeof(UINT32)*(replyHead->length);
	  }
	  return replyHead;
	}

        int RodOutList::getMsgLength(int n){
	  ReplyHead *replyHead = getMsgHead(n);
	  if(NULL==replyHead) return -1;
	  return replyHead->length;
	}

        int RodOutList::getMsgId(int n){
	  ReplyHead *replyHead = getMsgHead(n);
	  if(NULL==replyHead) return -1;
	  return replyHead->id;
	}

        void* RodOutList::getMsgBody(int n){
	  if(n > nMsgs())return NULL;
	  ReplyHead* replyHead;
	  char * ptr = (char*) getBody();
	  ptr += sizeof(ReplyListHead);
	  char * msgBody;
	  for(int i = 0; i < n; i++){
	    replyHead = (ReplyHead*) ptr;
	    msgBody   = ptr + sizeof(ReplyHead);
	    ptr      += sizeof(ReplyHead) + sizeof(UINT32)*(replyHead->length);
	  }
	  return msgBody;
	}

        int RodOutList::nMsgs(){
	  ReplyListHead* replyListHead;
	  replyListHead = (ReplyListHead*) getBody();
	  return replyListHead->nMsgs;
	}

// Overload = operator
	/*
	Overload the assignment operator to allow us to equate objects of this class. Note the
	special case to handle assigning an object to itself (i.e. A=A).
	*/
	RodOutList& RodOutList::operator=( const RodOutList& rhs)
	{
	  if (this == &rhs) return *this;
	  m_length = rhs.getLength();
	  m_body = rhs.getBody();
	  return *this;
	}



} // End namespace SctPixelRod
