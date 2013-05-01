// Copyright (c) 2013, David B. Moffett
//
// Author David B. Moffett <davidbmoffett@gmail.com>
//
// All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#define BUILDING_NODE_EXTENSION
    // #include <node.h>
#include <v8.h>
#include "svm.h"
#include "node_defs.h"

    using namespace v8;

struct svm_train_data {
  svm_problem *prob;
  svm_paramter *param;
  Persistent<Function> *callback;
};

void doSvmTrain(uv_work_t *req){
  svm_train_data *td = (svm_train_data *)req->data;
  svm_train(td->prob, td->param);
}

void afterSvmTrain(uv_work_t *req){
  HandleScope scope;

  svm_train_data *id = (my_struct *)req->data;

  Handle<Value> argv[1];
  argv[0] = Integer::New(r->rtn);

  TryCatch try_catch;
  td->callback->Call(Context::GetCurrent()->Global(), 1, argv);

  // cleanup
  td->callback.Dispose();
  delete td;
  delete req;

  if (try_catch.HasCaught())
    FatalException(try_catch);
}


bool node_svm::SVM::getHostname(const Arguments &args) const {
  HandleScope scope;

  if(args.Length() < 2){
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
    return scope.Close(Undefined());
  }
  
  if(args[0]->IsObject() && args[1]->IsObject()){
    v8::Local<v8::Object> prob = args[0]->ToObject();

    // Checks for value and creates key for value.
    ARG_CHECK_OBJECT_ATTR_NUMBER(prob, y);
    v8::Local<v8::Value> y = prob->Get(y_key);
    printf("y = %f\n", y->NumberValue());

    ARG_CHECK_OBJECT_ATTR_ARRAY(prob, svm_node);
    v8::Handle<v8::Array> nodes = v8::Handle<v8::Array>::Cast(prob->Get(svm_node_key));
    int index, count = nodes->Length();
    for(index = 0; index < count; index++){
      printf("value: %d at %d\n", nodes->Get(index)->IntegerValue(), index);
    }

    // Start work in background 
    uv_work_ *req = new uv_work_t;
    svm_train_data *td = new svm_train_data;
    req->data = td;
    td->callback = Persistent<Function>::New(Local<Function>::Cast(args[0]));

    uv_queue_work(uv_default_loop(), req, doSvmTrain, afterSvmTrain);
    
    
  } else {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
    return scope.Close(Undefined());
  }
  
  return scope.Close(Undefined());
}

void Init(Handle<Object> exports){
  exports->Set(String::NewSymbol("svm_train"), FunctionTemplate::New(SvmTrain)->GetFunction());
}

NODE_MODULE(svm, Init)
