// File: RodPrimitive.cxx
// $Header$

// Description:
// This class contains the primitives which are sent as a list to the RODs. They control 
// almost all aspects of the ROD DSP behavior. New primitives can be defined to perform
// user-defined actions, but corresponding DSP code must also be written, compiled and
// loaded into the DSPs before the primitive can be executed.
//
// @author Tom Meyer (meyer@iastate.edu) - originator

//! Namespace for the common routines for SCT and PIXEL ROD software.

#include <string.h>
#include "RodPrimitive.h"


namespace SctPixelRod {

// Constructors
	/*
	The default constructor, provided explicitly.
	*/
	RodPrimitive::RodPrimitive()
	{
	  m_header.length = 0;
	  m_header.index  = 0;
	  m_header.id     = 0;
	  m_body          = 0;
	}

	/*
	An alternate constructor, which initializes the data members 
	from arguments.
	*/
	RodPrimitive::RodPrimitive(long bodysize, long index, long id, 
				   long revision, long* body)
	{
	  //size in words
	  m_header.length   = bodysize + sizeof(m_header)/sizeof(UINT32);
	  m_header.index    = index;
	  m_header.id       = id;
	  m_header.revision = revision;
	  m_body            = body;
	}

	/*
	The copy constructor, which uses a deep copy since one data member is referenced
	by a pointer.
	*/
	RodPrimitive::RodPrimitive( const RodPrimitive& rhs)
	{
	  if (this == &rhs) return;
	  m_header.length   = rhs.getLength();
	  m_header.index    = rhs.getIndex();
	  m_header.id       = rhs.getId();
	  m_header.revision = rhs.getRevision();
	  m_body            = rhs.getBody();
	  return;
	}

// Destructor
	/*
	The destructor. We MUST be sure to delete the space allocated on the heap for m_body.
	For safety, we also set the pointer to zero to avoid pointers into la-la land.
	*/
	RodPrimitive::~RodPrimitive() 
	{
	  m_body = 0;
	}

// Overload = operator
	/*
	Overload the assignment operator to allow us to equate objects of this class. Note the
	special case to handle assigning an object to itself (i.e. A=A).
	*/
	RodPrimitive& RodPrimitive::operator=( const RodPrimitive& rhs)
	{
	  if (this == &rhs) return *this;
	  m_header.length   = rhs.getLength();
	  m_header.index    = rhs.getIndex();
	  m_header.id       = rhs.getId();
	  m_header.revision = rhs.getRevision();
	  m_body            = rhs.getBody();
	  return *this;
	}

// Copy Prim into supplied buffer
 
  long RodPrimitive::copyToBuffer(long unsigned int *buffer){
    memcpy(buffer, &m_header, sizeof(m_header));
    memcpy(buffer + sizeof(m_header)/sizeof(UINT32), m_body, 
	   sizeof(UINT32)*getBodySize());
    return m_header.length;
  }

 


} // End namespace SctPixelRod
