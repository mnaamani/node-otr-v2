#ifndef __NODE_OTR_CONNCONTEXT_H__
#define __NODE_OTR_CONNCONTEXT_H__

#include "otr.hpp"

extern "C" {
    #include <libotr/context.h>
}

namespace otr {

class ConnectionCtx : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> target);
  static v8::Persistent<v8::FunctionTemplate> constructor;

 protected:
  friend class MessageAppOps;
  ConnContext* context_;

  ConnectionCtx(ConnContext* context);
  ~ConnectionCtx();  
    
  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> WrapConnectionCtx(ConnContext *context);  
  static v8::Handle<v8::Value> ctxGetter(v8::Local<v8::String> property, const v8::AccessorInfo& info);
  static void ctxSetter(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo& info);
  
  
};

}
#endif
