#ifndef SVM_H_
#define SVM_H_

#include <v8.h>
#include <node.h>
#include <node_buffer.h>
#include <node_version.h>
#include <string>
#include "./node_defs.h"
#include "./svm.h"

namespace node_svm {
  class SVM {
  public:
    SVM();
    ~SVM();

    virtual bool svmTrain() const;    
    
  protected:
    static void Init(v8::Handle<v8::Object> target, v8::Persistent<v8::FunctionTemplate> constructorTemplate);
  };
}

#endif  // SVM_H_
