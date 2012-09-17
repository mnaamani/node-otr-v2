/*
 *  Off-the-Record Messaging bindings for nodejs
 *  Copyright (C) 2012  Mokhtar Naamani,
 *                      <mokhtar.naamani@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of version 2 of the GNU General Public License as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "otr.hpp"

extern "C" {
    #include <libotr/privkey.h>
    #include <libotr/tlv.h>
}

using namespace v8;

namespace otr {
v8::Persistent<v8::FunctionTemplate> MessageAppOps::constructor;

MessageAppOps::MessageAppOps() {};

MessageAppOps::~MessageAppOps() {    
    delete messageops_;
    ui_event_.Dispose();
};

void MessageAppOps::Init(Handle<Object> target) {
  HandleScope scope;

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  Local<String> name = String::NewSymbol("MessageAppOps");
  
  constructor = Persistent<FunctionTemplate>::New(tpl);
  // ObjectWrap uses the first internal field to store the wrapped pointer.
  constructor->InstanceTemplate()->SetInternalFieldCount(1);
  constructor->SetClassName(name);
  
  // Prototype
  NODE_SET_PROTOTYPE_METHOD(constructor, "messageSending",Message_Sending);
  NODE_SET_PROTOTYPE_METHOD(constructor, "messageReceiving",Message_Receiving);
  NODE_SET_PROTOTYPE_METHOD(constructor, "fragmentAndSend",Fragment_And_Send);
  NODE_SET_PROTOTYPE_METHOD(constructor, "disconnect",Disconnect);
  NODE_SET_PROTOTYPE_METHOD(constructor, "initSMP",Initiate_SMP);
  NODE_SET_PROTOTYPE_METHOD(constructor, "respondSMP",Respond_SMP);

  target->Set(name, constructor->GetFunction());
}

Handle<Value> MessageAppOps::New(const Arguments& args) {
    HandleScope scope;
      
    MessageAppOps* obj = new MessageAppOps();

    obj->messageops_ = new OtrlMessageAppOps();
    obj->messageops_->policy=op_policy;
    obj->messageops_->create_privkey=op_create_privkey;
    obj->messageops_->is_logged_in=op_is_logged_in;
    obj->messageops_->inject_message=op_inject_message;
    obj->messageops_->notify=op_notify;
    obj->messageops_->display_otr_message=op_display_otr_message;
    //obj->messageops_->display_otr_message=NULL;//library will use notify() instead.
    obj->messageops_->update_context_list=op_update_context_list;
    obj->messageops_->protocol_name=op_protocol_name;
    obj->messageops_->protocol_name_free=op_protocol_name_free;
    obj->messageops_->new_fingerprint=op_new_fingerprint;
    obj->messageops_->write_fingerprints=op_write_fingerprints;
    obj->messageops_->gone_secure=op_gone_secure;
    obj->messageops_->gone_insecure=op_gone_insecure;
    obj->messageops_->still_secure=op_still_secure;
    obj->messageops_->log_message=op_log_message;
    obj->messageops_->max_message_size=op_max_message_size;
    obj->messageops_->account_name=op_account_name;
    obj->messageops_->account_name_free=op_account_name_free;

    obj->ui_event_ = Persistent<Function>::New(Local<Function>::Cast(args[0]));
    obj->Wrap(args.This());
    return args.This();
}

Handle<Value> MessageAppOps::Message_Sending(const Arguments& args) {
  HandleScope scope;
  Handle<Value> retvalue;
  MessageAppOps* ops = ObjectWrap::Unwrap<MessageAppOps>(args.This());

  if(!args.Length() > 0 || !args[0]->IsObject()){
    return scope.Close(V8EXCEPTION("Invalid arguments. First argument 'userstate' (UserState) excpected."));
  }
  if(!args.Length() > 1 || !args[1]->IsString()){
    return scope.Close(V8EXCEPTION("Invalid arguments. Second argument 'accountname' (string) excpected."));
  }
  if(!args.Length() > 2 || !args[2]->IsString()){
    return scope.Close(V8EXCEPTION("Invalid arguments. Third argument 'protocol' (string) excpected."));
  }
  if(!args.Length() > 3 || !args[3]->IsString()){
    return scope.Close(V8EXCEPTION("Invalid arguments. Fourth argument 'recipient' (string) excpected."));
  }
  if(!args.Length() > 4 || !args[4]->IsString()){
    return scope.Close(V8EXCEPTION("Invalid arguments. Fifth argument 'message' (string) excpected."));
  }
  
  UserState* user = node::ObjectWrap::Unwrap<UserState>(args[0]->ToObject());
  String::Utf8Value accountname(args[1]->ToString());
  String::Utf8Value protocol(args[2]->ToString());
  String::Utf8Value recipient(args[3]->ToString());
  String::Utf8Value message(args[4]->ToString());

  char *messagep;
  gcry_error_t err = otrl_message_sending(user->userstate_, ops->messageops_,(void *)ops, 
        *accountname, *protocol, *recipient, *message,NULL, &messagep,NULL,NULL);

  if( err ){
    retvalue = scope.Close(Undefined());
  }else{
    retvalue = scope.Close(String::New(messagep));
  }
  if(messagep !=NULL) otrl_message_free(messagep);

  return retvalue;
}

Handle<Value> MessageAppOps::Message_Receiving(const Arguments& args) {
  HandleScope scope;
  Handle<Value> retvalue;
  NextExpectedSMP nextMsg;
  ConnContext *context;
  char *newmessage;
  OtrlTLV *tlvs = NULL;
  OtrlTLV *tlv = NULL;

  MessageAppOps* ops = ObjectWrap::Unwrap<MessageAppOps>(args.This());

  if(!args.Length() > 0 || !args[0]->IsObject()){
    return scope.Close(V8EXCEPTION("Invalid arguments. First argument 'userstate' (UserState) excpected."));
  }
  if(!args.Length() > 1 || !args[1]->IsString()){
    return scope.Close(V8EXCEPTION("Invalid arguments. Second argument 'accountname' (string) excpected."));
  }
  if(!args.Length() > 2 || !args[2]->IsString()){
    return scope.Close(V8EXCEPTION("Invalid arguments. Third argument 'protocol' (string) excpected."));
  }
  if(!args.Length() > 3 || !args[3]->IsString()){
    return scope.Close(V8EXCEPTION("Invalid arguments. Fourth argument 'sender' (string) excpected."));
  }
  if(!args.Length() > 4 || !args[4]->IsString()){
    return scope.Close(V8EXCEPTION("Invalid arguments. Fifth argument 'message' (string) excpected."));
  }


  UserState* user = node::ObjectWrap::Unwrap<UserState>(args[0]->ToObject());
  String::Utf8Value accountname(args[1]->ToString());
  String::Utf8Value protocol(args[2]->ToString());
  String::Utf8Value sender(args[3]->ToString());
  String::Utf8Value message(args[4]->ToString());

  int status = otrl_message_receiving(user->userstate_, ops->messageops_,(void *)ops, 
        *accountname, *protocol, *sender, *message, &newmessage, &tlvs,NULL,NULL);

  context = otrl_context_find(user->userstate_, *sender, *accountname, *protocol, 0, NULL, NULL, NULL);

  //below incoming tlv processing code borrowed from pidgin-otr-3.2.1/otr-plugin.c

  if (context) {
    //LOOK FOR REMOTE SIDE DISCONNECT
    tlv = otrl_tlv_find(tlvs, OTRL_TLV_DISCONNECTED);
    if(tlv){
        notifyRemoteDisconnected(ops,context);
    }else{

	nextMsg = context->smstate->nextExpected;

	if (context->smstate->sm_prog_state == OTRL_SMP_PROG_CHEATED) {

        notifySMPResult(ops, context,"smp_failed");
	    otrl_message_abort_smp(user->userstate_, ops->messageops_, (void *)ops, context);
	    context->smstate->nextExpected = OTRL_SMP_EXPECT1;
	    context->smstate->sm_prog_state = OTRL_SMP_PROG_OK;

	} else {
	
	    tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP1Q);
	    if (tlv) {
		    if (nextMsg != OTRL_SMP_EXPECT1){
    		      otrl_message_abort_smp(user->userstate_, ops->messageops_, (void *)ops, context);
                  notifySMPResult(ops, context,"smp_aborted");
	    	}else {
    		    char *question = (char *)tlv->data;
    		    char *eoq = (char*)memchr(question, '\0', tlv->len);
    		    if (eoq) {
        			notifyIncomingSMPRequest(ops,context,question);
    		    }
    		}
	    }
	    
	    tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP1);
	    if (tlv) {
		    if (nextMsg != OTRL_SMP_EXPECT1){
    		    otrl_message_abort_smp(user->userstate_, ops->messageops_, (void *)ops, context);
                notifySMPResult(ops, context,"smp_aborted");
    		}else {
                notifyIncomingSMPRequest(ops,context,NULL);
    		}
	    }
	    
	    tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP2);
	    if (tlv) {
		    if (nextMsg != OTRL_SMP_EXPECT2){
    		    otrl_message_abort_smp(user->userstate_, ops->messageops_, (void *)ops, context);
                notifySMPResult(ops, context,"smp_aborted");
            }else {
    		    context->smstate->nextExpected = OTRL_SMP_EXPECT4;
	    	}
	    }
	    
	    tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP3);
	    if (tlv) {
		    if (nextMsg != OTRL_SMP_EXPECT3){
    		    otrl_message_abort_smp(user->userstate_, ops->messageops_, (void *)ops, context);
                notifySMPResult(ops, context,"smp_aborted");
    		}else {
                //printf("TLV_SMP3: sm state= %d\n",context->smstate->sm_prog_state);    		    
    		    context->smstate->nextExpected = OTRL_SMP_EXPECT1;
                if(context->smstate->sm_prog_state==1) notifySMPResult(ops, context,"smp_complete");
                else notifySMPResult(ops, context,"smp_failed");
    		}
	    }
	    
	    tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP4);
	    if (tlv) {
            if (nextMsg != OTRL_SMP_EXPECT4){
    		    otrl_message_abort_smp(user->userstate_, ops->messageops_, (void *)ops, context);
                notifySMPResult(ops, context,"smp_aborted");
    		}else {
                //printf("TLV_SMP4: sm state= %d\n",context->smstate->sm_prog_state);    		    
                context->smstate->nextExpected = OTRL_SMP_EXPECT1;
                if(context->smstate->sm_prog_state==1) notifySMPResult(ops, context,"smp_complete");
                else notifySMPResult(ops, context,"smp_failed");
    		}
	    }
	    
	    tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP_ABORT);
	    if (tlv) {
    		context->smstate->nextExpected = OTRL_SMP_EXPECT1;
            notifySMPResult(ops, context,"smp_aborted");
	    }
	}
    }
  }
  
  if(status==1) retvalue = Undefined();
  if(status==0) {
     retvalue = (newmessage==NULL) ? (Handle<Value>)args[3] : String::New(newmessage);
  }
  if(newmessage!=NULL) otrl_message_free(newmessage);
  if(tlvs!=NULL) otrl_tlv_free(tlvs);
  
  return scope.Close(retvalue);
}

void MessageAppOps::notifyRemoteDisconnected(MessageAppOps* ops, ConnContext *context){
    
    otrl_context_force_plaintext(context);  
  
    Local<Object> eobj = Object::New();
    eobj->Set(String::NewSymbol("EVENT"),String::New( "remote_disconnected" ));
    eobj->Set(String::NewSymbol("username"), String::New(context->username));
    eobj->Set(String::NewSymbol("accountname"), String::New(context->accountname));
    eobj->Set(String::NewSymbol("protocol"), String::New(context->protocol));
    eobj->Set(String::NewSymbol("context"), ConnectionCtx::WrapConnectionCtx(context) );
    
    QueEvent(eobj,ops->ui_event_);
}

void MessageAppOps::notifySMPResult(MessageAppOps* ops, ConnContext *context,const char* result){
    Local<Object> eobj = Object::New();
    eobj->Set(String::NewSymbol("EVENT"),String::New( result ));
    eobj->Set(String::NewSymbol("username"), String::New(context->username));
    eobj->Set(String::NewSymbol("accountname"), String::New(context->accountname));
    eobj->Set(String::NewSymbol("protocol"), String::New(context->protocol));
    eobj->Set(String::NewSymbol("context"), ConnectionCtx::WrapConnectionCtx(context) );
    
    QueEvent(eobj,ops->ui_event_);
}
void MessageAppOps::notifyIncomingSMPRequest(MessageAppOps* ops, ConnContext *context, char* question){

    Local<Object> eobj = Object::New();
    eobj->Set(String::NewSymbol("EVENT"),String::New( "smp_request" ));
    eobj->Set(String::NewSymbol("username"), String::New(context->username));
    eobj->Set(String::NewSymbol("accountname"), String::New(context->accountname));
    eobj->Set(String::NewSymbol("protocol"), String::New(context->protocol));
    eobj->Set(String::NewSymbol("context"), ConnectionCtx::WrapConnectionCtx(context) );
    
    if(question==NULL){
        eobj->Set(String::NewSymbol("question"),Undefined());
    }else{
        eobj->Set(String::NewSymbol("question"),String::New(question));
    }

    QueEvent(eobj,ops->ui_event_);
}

Handle<Value> MessageAppOps::Fragment_And_Send(const Arguments& args) { 
  HandleScope scope;  
  MessageAppOps* ops = ObjectWrap::Unwrap<MessageAppOps>(args.This());
  
  if(!args.Length() > 0 || !args[0]->IsObject()){
    return scope.Close(V8EXCEPTION("Invalid arguments. First argument 'context' (ConnContext) excpected."));
  }
  if(!args.Length() > 1 || !args[1]->IsString()){
    return scope.Close(V8EXCEPTION("Invalid arguments. Second argument 'message' (string) excpected."));
  }  

  ConnectionCtx* ctx = node::ObjectWrap::Unwrap<ConnectionCtx>(args[0]->ToObject());
  String::Utf8Value message(args[1]->ToString());
  
  gcry_error_t err = otrl_message_fragment_and_send(ops->messageops_,(void*)ops,ctx->context_,*message,OTRL_FRAGMENT_SEND_ALL, NULL);
  
  return scope.Close(String::New(gcry_strerror(err)));
}
	
Handle<Value> MessageAppOps::Disconnect(const Arguments& args) {
  HandleScope scope;  
  MessageAppOps* ops = ObjectWrap::Unwrap<MessageAppOps>(args.This());

  if(!args.Length() > 0 || !args[0]->IsObject()){
    return scope.Close(V8EXCEPTION("Invalid arguments. First argument 'userstate' (UserState) excpected."));
  }
  if(!args.Length() > 1 || !args[1]->IsString()){
    return scope.Close(V8EXCEPTION("Invalid arguments. Second argument 'accountname' (string) excpected."));
  }
  if(!args.Length() > 2 || !args[2]->IsString()){
    return scope.Close(V8EXCEPTION("Invalid arguments. Third argument 'protocol' (string) excpected."));
  }
  if(!args.Length() > 3 || !args[3]->IsString()){
    return scope.Close(V8EXCEPTION("Invalid arguments. Fourth argument 'recipient' (string) excpected."));
  }
  
  UserState* user = node::ObjectWrap::Unwrap<UserState>(args[0]->ToObject());
  String::Utf8Value accountname(args[1]->ToString());
  String::Utf8Value protocol(args[2]->ToString());
  String::Utf8Value recipient(args[3]->ToString());

  otrl_message_disconnect(user->userstate_,ops->messageops_,(void*)ops,*accountname,*protocol,*recipient);
  return scope.Close(Undefined());
}

Handle<Value> MessageAppOps::Initiate_SMP(const Arguments& args) {
  HandleScope scope;
  MessageAppOps* ops = ObjectWrap::Unwrap<MessageAppOps>(args.This());

  if(!args.Length() > 0 || !args[0]->IsObject()){
    return scope.Close(V8EXCEPTION("Invalid arguments. First argument 'userstate' (UserState) excpected."));
  }
  if(!args.Length() > 1 || !args[1]->IsObject()){
    return scope.Close(V8EXCEPTION("Invalid arguments. Second argument 'context' (ConnContext) excpected."));
  }
  if(!args.Length() > 2 || !args[2]->IsString()){
    return scope.Close(V8EXCEPTION("Invalid arguments. Third argument 'secret' (string) excpected."));
  }
  if(args.Length() > 3 && !args[3]->IsString()){
    return scope.Close(V8EXCEPTION("Invalid arguments. Optional Fourth argument 'question' (string) excpected."));
  }

  UserState* user = node::ObjectWrap::Unwrap<UserState>(args[0]->ToObject());
  ConnectionCtx* ctx = node::ObjectWrap::Unwrap<ConnectionCtx>(args[1]->ToObject());
  String::Utf8Value secret(args[2]->ToString());

  //don't init SMP auth if its is already progressing
  if(ctx->context_->smstate->nextExpected == OTRL_SMP_EXPECT1 ){
      if(args.Length()>3){
          String::Utf8Value question(args[3]->ToString());
          otrl_message_initiate_smp_q(user->userstate_,ops->messageops_,(void*)ops,ctx->context_,(const char*)*question,(const unsigned char*)*secret,strlen(*secret));
      }else{
         otrl_message_initiate_smp(user->userstate_,ops->messageops_,(void*)ops,ctx->context_,(const unsigned char*)*secret,strlen(*secret));
      } 
  }
  return scope.Close(Undefined());
}
Handle<Value> MessageAppOps::Respond_SMP(const Arguments& args) {
  HandleScope scope;
  MessageAppOps* ops = ObjectWrap::Unwrap<MessageAppOps>(args.This());

  if(!args.Length() > 0 || !args[0]->IsObject()){
    return scope.Close(V8EXCEPTION("Invalid arguments. First argument 'userstate' (UserState) excpected."));
  }
  if(!args.Length() > 1 || !args[1]->IsObject()){
    return scope.Close(V8EXCEPTION("Invalid arguments. Second argument 'context' (ConnContext) excpected."));
  }
  if(!args.Length() > 2 || !args[2]->IsString()){
    return scope.Close(V8EXCEPTION("Invalid arguments. Third argument 'secret' (string) excpected."));
  }
  
  UserState* user = node::ObjectWrap::Unwrap<UserState>(args[0]->ToObject());
  ConnectionCtx* ctx = node::ObjectWrap::Unwrap<ConnectionCtx>(args[1]->ToObject());
  String::Utf8Value secret(args[2]->ToString());

  otrl_message_respond_smp(user->userstate_,ops->messageops_,(void*)ops,ctx->context_,(const unsigned char*)*secret,strlen(*secret));
 
  return scope.Close(Undefined());

}

OtrlPolicy MessageAppOps::op_policy(void *opdata, ConnContext *context){
    MessageAppOps* ops = (MessageAppOps*)opdata;
    
    Local<Object> eobj = Object::New();
    eobj->Set(String::NewSymbol("EVENT"),String::New("policy"));
    eobj->Set(String::NewSymbol("username"), String::New(context->username));
    eobj->Set(String::NewSymbol("accountname"), String::New(context->accountname));
    eobj->Set(String::NewSymbol("protocol"), String::New(context->protocol));

    const unsigned argc = 1;
    Local<Value> argv[argc] = { eobj };
    TryCatch try_catch;
    Handle<Value> result = ops->ui_event_->Call(Context::GetCurrent()->Global(), argc, argv);   
    if (try_catch.HasCaught()) {
        node::FatalException(try_catch);
    }
    
    if(!result->IsNumber()) return OTRL_POLICY_ALWAYS;
    
    return (OtrlPolicy)((unsigned int)result->NumberValue());
}

void MessageAppOps::op_create_privkey(void *opdata, const char *accountname, const char *protocol){    
    MessageAppOps* ops = (MessageAppOps*)opdata;
    
    Local<Object> eobj = Object::New();
    eobj->Set(String::NewSymbol("EVENT"),String::New("create_privkey"));
    eobj->Set(String::NewSymbol("accountname"), String::New(accountname));
    eobj->Set(String::NewSymbol("protocol"), String::New(protocol));

    QueEvent(eobj,ops->ui_event_);
}

int MessageAppOps::op_is_logged_in(void *opdata, const char *accountname,const char *protocol, const char *recipient){    
    MessageAppOps* ops = (MessageAppOps*)opdata;
    
    Local<Object> eobj = Object::New();
    eobj->Set(String::NewSymbol("EVENT"),String::New("is_logged_in"));
    eobj->Set(String::NewSymbol("accountname"), String::New(accountname));
    eobj->Set(String::NewSymbol("protocol"), String::New(protocol));
    eobj->Set(String::NewSymbol("recipient"), String::New(recipient));

    const unsigned argc = 1;
    Local<Value> argv[argc] = { eobj };
    TryCatch try_catch;
    Handle<Value> result = ops->ui_event_->Call(Context::GetCurrent()->Global(), argc, argv);
    if (try_catch.HasCaught()) {
        node::FatalException(try_catch);
    }
    
    if(!result->IsNumber()) return 0;
    return (int)result->NumberValue();
}

void MessageAppOps::op_inject_message(void *opdata, const char *accountname, const char *protocol, const char *recipient, const char *message){    
    MessageAppOps* ops = (MessageAppOps*)opdata;
    
    Local<Object> eobj = Object::New();
    eobj->Set(String::NewSymbol("EVENT"),String::New("inject_message"));
    eobj->Set(String::NewSymbol("accountname"), String::New(accountname));
    eobj->Set(String::NewSymbol("protocol"), String::New(protocol));
    eobj->Set(String::NewSymbol("username"), String::New(recipient));
    eobj->Set(String::NewSymbol("message"), String::New(message));

    QueEvent(eobj,ops->ui_event_);
}

void MessageAppOps::op_notify(void *opdata, OtrlNotifyLevel level, const char *accountname, const char *protocol, const char *username, const char *title,
	const char *primary, const char *secondary){
    //This type of notification is more suitable for our evented JS API.
    
    MessageAppOps* ops = (MessageAppOps*)opdata;
    
    Local<Object> eobj = Object::New();
    eobj->Set(String::NewSymbol("EVENT"),String::New("notify"));
    eobj->Set(String::NewSymbol("accountname"), String::New(accountname));
    eobj->Set(String::NewSymbol("protocol"), String::New(protocol));
    eobj->Set(String::NewSymbol("username"), String::New(username));
    eobj->Set(String::NewSymbol("title"), String::New(title));
    eobj->Set(String::NewSymbol("primary"), String::New(primary));
    eobj->Set(String::NewSymbol("secondary"), String::New(secondary));
    /* TODO:pass the OtrlNofifyLevel level along.. */
    /*
    typedef enum {
        OTRL_NOTIFY_ERROR,
        OTRL_NOTIFY_WARNING,
        OTRL_NOTIFY_INFO
    } OtrlNotifyLevel;
    */
    
    QueEvent(eobj,ops->ui_event_);
}

int MessageAppOps::op_display_otr_message(void *opdata, const char *accountname, const char *protocol, const char *username, const char *msg){   
    //Messages received here are intended to be displayed inline in a chat session
    //as human readable message
    
    MessageAppOps* ops = (MessageAppOps*)opdata;
   
    Local<Object> eobj = Object::New();
    eobj->Set(String::NewSymbol("EVENT"),String::New("display_otr_message"));
    eobj->Set(String::NewSymbol("accountname"), String::New(accountname));
    eobj->Set(String::NewSymbol("protocol"), String::New(protocol));
    eobj->Set(String::NewSymbol("username"), String::New(username));
    eobj->Set(String::NewSymbol("message"), String::New(msg));

    QueEvent(eobj,ops->ui_event_);
    
    /* for libotr to fire op_notify() as well, return non-zero */
    return 1;
    return 0;
}

void MessageAppOps::op_update_context_list(void *opdata){    
    MessageAppOps* ops = (MessageAppOps*)opdata;
    
    Local<Object> eobj = Object::New();
    eobj->Set(String::NewSymbol("EVENT"),String::New("update_context_list"));

    QueEvent(eobj,ops->ui_event_);
}

const char *MessageAppOps::op_protocol_name(void *opdata, const char *protocol){
    //puts("protocol_name()");
    return protocol;
}

void MessageAppOps::op_protocol_name_free(void *opdata, const char *protocol_name){
    //puts("protocol_name_free()");
    return;
}

void MessageAppOps::op_new_fingerprint(void *opdata, OtrlUserState us,	const char *accountname, const char *protocol, const char *username, unsigned char fingerprint[20]){    
    MessageAppOps* ops = (MessageAppOps*)opdata;
     
    Local<Object> eobj = Object::New();
    eobj->Set(String::NewSymbol("EVENT"),String::New("new_fingerprint"));
    eobj->Set(String::NewSymbol("accountname"), String::New(accountname));
    eobj->Set(String::NewSymbol("protocol"), String::New(protocol));
    eobj->Set(String::NewSymbol("username"), String::New(username));
    eobj->Set(String::NewSymbol("userstate"), UserState::WrapUserState(us) );
    
    char human[45];
    otrl_privkey_hash_to_human(human, fingerprint);
    eobj->Set(String::NewSymbol("fingerprint"), String::New(human));

    QueEvent(eobj,ops->ui_event_);
}

void MessageAppOps::op_write_fingerprints(void *opdata){    
    MessageAppOps* ops = (MessageAppOps*)opdata;
    
    Local<Object> eobj = Object::New();
    eobj->Set(String::NewSymbol("EVENT"),String::New("write_fingerprints"));

    QueEvent(eobj,ops->ui_event_);
}

void MessageAppOps::op_gone_secure(void *opdata, ConnContext *context){    
    contextSecureStatusUpdate(opdata,context,"gone_secure");
}

void MessageAppOps::op_gone_insecure(void *opdata, ConnContext *context){
    contextSecureStatusUpdate(opdata,context,"gone_insecure");
}

void MessageAppOps::op_still_secure(void *opdata, ConnContext *context, int is_reply){
    contextSecureStatusUpdate(opdata,context,"still_secure");
}

void MessageAppOps::op_log_message(void *opdata, const char *message){    
    MessageAppOps* ops = (MessageAppOps*)opdata;
    Local<Object> eobj = Object::New();
    eobj->Set(String::NewSymbol("EVENT"),String::New("log_message"));
    eobj->Set(String::NewSymbol("message"), String::New(message));

    QueEvent(eobj,ops->ui_event_);
}

int MessageAppOps::op_max_message_size(void *opdata, ConnContext *context){    
    MessageAppOps* ops = (MessageAppOps*)opdata;
    
    Local<Object> eobj = Object::New();
    eobj->Set(String::NewSymbol("EVENT"),String::New("max_message_size"));
    eobj->Set(String::NewSymbol("username"), String::New(context->username));
    eobj->Set(String::NewSymbol("accountname"), String::New(context->accountname));
    eobj->Set(String::NewSymbol("protocol"), String::New(context->protocol));
    eobj->Set(String::NewSymbol("context"), ConnectionCtx::WrapConnectionCtx(context) );

    const unsigned argc = 1;
    Local<Value> argv[argc] = { eobj };
    TryCatch try_catch;
    Handle<Value> result = ops->ui_event_->Call(Context::GetCurrent()->Global(), argc, argv);
    if (try_catch.HasCaught()) {
        node::FatalException(try_catch);
    }
    
    if(!result->IsNumber()) return 1450;//default max message size    
    return (int)result->NumberValue();
}

const char *MessageAppOps::op_account_name(void *opdata, const char *account, const char *protocol){
    //puts("account_name()");
    return account;
}

void MessageAppOps::op_account_name_free(void *opdata, const char *account_name){
    //puts("account_name_free()");
    return;
}

void MessageAppOps::contextSecureStatusUpdate(void *opdata, ConnContext* context, const char* event){
    MessageAppOps* ops = (MessageAppOps*)opdata;
    
    Local<Object> eobj = Object::New();
    eobj->Set(String::NewSymbol("EVENT"),String::New( event ));
    eobj->Set(String::NewSymbol("username"), String::New(context->username));
    eobj->Set(String::NewSymbol("accountname"), String::New(context->accountname));
    eobj->Set(String::NewSymbol("protocol"), String::New(context->protocol));     
    eobj->Set(String::NewSymbol("context"), ConnectionCtx::WrapConnectionCtx(context) );   

    QueEvent(eobj,ops->ui_event_);
}

void MessageAppOps::QueEvent(Local<Object> obj, Persistent<Function> callback){
    EventBaton *baton = new EventBaton();
    baton->request.data = baton;
    //baton->callback = Persistent<Function>::New(callback);
    baton->callback = callback;
    baton->event = Persistent<Object>::New(obj);
    int status = uv_queue_work(uv_default_loop(), &baton->request, NULL, FireEvent);
    assert(status == 0);
}
void MessageAppOps::FireEvent(uv_work_t* req){
    HandleScope scope;
    EventBaton* baton = static_cast<EventBaton*>(req->data);
    const unsigned argc = 1;
    Local<Object> obj_ = Local<Object>::New(baton->event);
    Local<Value> argv[argc] = { obj_ };
    TryCatch try_catch;
    baton->callback->Call(Context::GetCurrent()->Global(), argc, argv);   
    if (try_catch.HasCaught()) {
        puts(">> Node Fatal Exception <<");
        node::FatalException(try_catch);
        
    }
    baton->event.Dispose();    
    delete baton;
}

}
