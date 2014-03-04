#ifndef _OBJECT_TYPE_H_
#define _OBJECT_TYPE_H_

#include <boost/intrusive_ptr.hpp>

//! Intrusive pointer used to manage StelObject with smart pointers
class ObjectBase;
typedef boost::intrusive_ptr<ObjectBase> ObjectBaseP;

#endif
