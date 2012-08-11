#ifndef __NODE_OTR_USERSTATE_H__
#define __NODE_OTR_USERSTATE_H__

#include "otr.hpp"

extern "C" {
    #include <libotr/userstate.h>
}

namespace otr {
class UserState : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> target);
  static v8::Persistent<v8::FunctionTemplate> constructor;

 protected:
  friend class MessageAppOps;
  friend class ConnectionCtx;
  OtrlUserState userstate_;
  bool reference;
  
  UserState(OtrlUserState userstate);
  ~UserState();
  
  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> WrapUserState(OtrlUserState userstate);
  static v8::Handle<v8::Value> Destroy(const v8::Arguments &args);
  //Async
  static v8::Handle<v8::Value> Generate_Key(const v8::Arguments& args);
  static v8::Handle<v8::Value> Read_Keys(const v8::Arguments& args);
  static v8::Handle<v8::Value> Read_Fingerprints(const v8::Arguments& args);
  static v8::Handle<v8::Value> Write_Fingerprints(const v8::Arguments& args);
  //Sync
  static v8::Handle<v8::Value> Fingerprint(const v8::Arguments& args);
  static v8::Handle<v8::Value> Accounts(const v8::Arguments& args);
  static v8::Handle<v8::Value> Get_Key(const v8::Arguments& args);     
  static v8::Handle<v8::Value> Read_Keys_Sync(const v8::Arguments& args);
  static v8::Handle<v8::Value> Read_Fingerprints_Sync(const v8::Arguments& args);
  static v8::Handle<v8::Value> Write_Fingerprints_Sync(const v8::Arguments& args);  
  //Workers
  static void Worker_Generate_Key (uv_work_t* req);
  static void Worker_Read_Keys (uv_work_t* req);
  static void Worker_Read_Fingerprints (uv_work_t* req);
  static void Worker_Write_Fingerprints (uv_work_t* req);
  static void Worker_After (uv_work_t* req);  
  
};

//information about the asynchronous "work request".
struct Baton {    
    uv_work_t request;
    bool hasCallback;
    v8::Persistent<v8::Function> callback;
    gcry_error_t error;
    OtrlUserState  userstate;
    std::string arg0;
    std::string arg1;
    std::string arg2;
};

}
#endif
