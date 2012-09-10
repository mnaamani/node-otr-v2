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
