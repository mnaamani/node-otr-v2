#include "otr.hpp"

extern "C" {
    #include <libotr/proto.h>
}


namespace otr {
    v8::Handle<v8::Value> Version(const v8::Arguments& args) {
      v8::HandleScope scope;
      return scope.Close(v8::String::New(otrl_version()));
    }
}

void RegisterModule(v8::Handle<v8::Object> target) {
  OTRL_INIT;

  target->Set(v8::String::NewSymbol("version"), v8::FunctionTemplate::New(otr::Version)->GetFunction());
  
  otr::UserState::Init(target);
  otr::ConnectionCtx::Init(target);
  otr::MessageAppOps::Init(target);      
}


NODE_MODULE(otrnat, RegisterModule)


