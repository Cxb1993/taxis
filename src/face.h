#ifndef __FACE_H__
#define __FACE_H__

#include "vec.h"

class Face
{
   public:
      Face () {};
      Face (const Face&);
      unsigned int vertex[2];
      int          lcell, rcell;
      Vector       normal;
      Vector       centroid;
      int          type;
      double       area;

      bool operator== (const Face& face) const;
};

// copy constructor
inline
Face::Face (const Face& face)
{
   vertex[0] = face.vertex[0];
   vertex[1] = face.vertex[1];
   lcell     = face.lcell;
   rcell     = face.rcell;
   normal    = face.normal;
   centroid  = face.centroid;
   type      = face.type;
   area      = face.area;
}

// Check if two faces are same by checking their vertices
inline
bool Face::operator== (const Face& face) const
{

   if ( vertex[0]==face.vertex[0] &&
        vertex[1]==face.vertex[1]) return true;

   if ( vertex[0]==face.vertex[1] &&
        vertex[1]==face.vertex[0]) return true;

   return false;
}

#endif
