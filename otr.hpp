#ifndef __NODE_OTR_H__
#define __NODE_OTR_H__

#include <node.h>
#include <v8.h>
#include <stdio.h>
#include <string>

#include "userstate.hpp"
#include "message.hpp"
#include "context.hpp"

#define GCRY_EXCEPTION(error) v8::ThrowException(v8::Exception::Error(v8::String::New(gcry_strerror(error))))
#define V8EXCEPTION(error) v8::ThrowException(v8::Exception::Error(v8::String::New(error)))

#ifndef NODE_SET_PROTOTYPE_ACCESSOR
#define NODE_SET_PROTOTYPE_ACCESSOR(templ, name, getter, setter)          \
do {                                                                      \
  templ->PrototypeTemplate()->SetAccessor(v8::String::NewSymbol(name),    \
                                  getter, setter);                        \
} while (0)
#endif


#define IfStrEqual(a,b) if(a.compare(b)==0)


#endif
