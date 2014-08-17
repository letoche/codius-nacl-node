// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "pipe_wrap.h"

#include "env.h"
#include "env-inl.h"
#include "handle_wrap.h"
#include "async-wrap-inl.h"
#include "node.h"
#include "node_buffer.h"
#include "util-inl.h"
#include "util.h"

#include <unistd.h>

namespace node {

using v8::Boolean;
using v8::Context;
using v8::EscapableHandleScope;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Handle;
using v8::HandleScope;
using v8::Integer;
using v8::Local;
using v8::Object;
using v8::PropertyAttribute;
using v8::String;
using v8::Undefined;
using v8::Value;


int PipeWrap::FileDescriptor() {
  return handle_.GetFileDescriptor();
}


Local<Object> PipeWrap::Instantiate(Environment* env) {
  EscapableHandleScope handle_scope(env->isolate());
  assert(!env->pipe_constructor_template().IsEmpty());
  Local<Function> constructor = env->pipe_constructor_template()->GetFunction();
  assert(!constructor.IsEmpty());
  Local<Object> instance = constructor->NewInstance();
  assert(!instance.IsEmpty());
  return handle_scope.Escape(instance);
}


void PipeWrap::Initialize(Handle<Object> target,
                          Handle<Value> unused,
                          Handle<Context> context) {
  Environment* env = Environment::GetCurrent(context);

  Local<FunctionTemplate> t = FunctionTemplate::New(env->isolate(), New);
  t->SetClassName(FIXED_ONE_BYTE_STRING(env->isolate(), "Pipe"));
  t->InstanceTemplate()->SetInternalFieldCount(1);

  /*enum PropertyAttribute attributes =
      static_cast<PropertyAttribute>(v8::ReadOnly | v8::DontDelete);
  t->InstanceTemplate()->SetAccessor(env->fd_string(),
                                     StreamWrap::GetFD,
                                     NULL,
                                     Handle<Value>(),
                                     v8::DEFAULT,
                                     attributes);*/

//  NODE_SET_PROTOTYPE_METHOD(t, "close", HandleWrap::Close);
//  NODE_SET_PROTOTYPE_METHOD(t, "unref", HandleWrap::Unref);
//  NODE_SET_PROTOTYPE_METHOD(t, "ref", HandleWrap::Ref);
//
//  NODE_SET_PROTOTYPE_METHOD(t, "setBlocking", StreamWrap::SetBlocking);
//
  NODE_SET_PROTOTYPE_METHOD(t, "readStart", ReadStart);
//  NODE_SET_PROTOTYPE_METHOD(t, "readStop", StreamWrap::ReadStop);
//  NODE_SET_PROTOTYPE_METHOD(t, "shutdown", StreamWrap::Shutdown);
//
//  NODE_SET_PROTOTYPE_METHOD(t, "writeBuffer", StreamWrap::WriteBuffer);
//  NODE_SET_PROTOTYPE_METHOD(t,
//                            "writeAsciiString",
//                            StreamWrap::WriteAsciiString);
  NODE_SET_PROTOTYPE_METHOD(t, "writeUtf8String", WriteUtf8String);
//  NODE_SET_PROTOTYPE_METHOD(t, "writeUcs2String", StreamWrap::WriteUcs2String);

  //NODE_SET_PROTOTYPE_METHOD(t, "bind", Bind);
  //NODE_SET_PROTOTYPE_METHOD(t, "listen", Listen);
  NODE_SET_PROTOTYPE_METHOD(t, "connect", Connect);
  NODE_SET_PROTOTYPE_METHOD(t, "open", Open);

#ifdef _WIN32
  NODE_SET_PROTOTYPE_METHOD(t, "setPendingInstances", SetPendingInstances);
#endif

  target->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "Pipe"), t->GetFunction());
  env->set_pipe_constructor_template(t);
}


void PipeWrap::New(const FunctionCallbackInfo<Value>& args) {
  // This constructor should not be exposed to public javascript.
  // Therefore we assert that we are not trying to call this as a
  // normal function.
  assert(args.IsConstructCall());
  HandleScope handle_scope(args.GetIsolate());
  Environment* env = Environment::GetCurrent(args.GetIsolate());
  new PipeWrap(env, args.This(), args[0]->IsTrue());
}


PipeWrap::PipeWrap(Environment* env, Handle<Object> object, bool ipc)
    : HandleWrap(env,
                 object,
                 &handle_,
                 AsyncWrap::PROVIDER_PIPEWRAP) {
  HandleScope scope(env->isolate());
  Wrap<PipeWrap>(object, this);
  handle_.Init(env->event_loop(), ipc);
  /*int r = uv_pipe_init(env->event_loop(), &handle_, ipc);
  assert(r == 0);  // How do we proxy this error up to javascript?
                   // Suggestion: uv_pipe_init() returns void.
  UpdateWriteQueueSize();*/
}




void PipeWrap::Open(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args.GetIsolate());
  HandleScope scope(env->isolate());

  PipeWrap* wrap = Unwrap<PipeWrap>(args.Holder());

  int fd = args[0]->Int32Value();

  wrap->handle_.Open(fd);

  // TODO-CODIUS: Do stuff...
  /*
  int err = uv_pipe_open(&wrap->handle_, fd);

  if (err != 0)
    env->isolate()->ThrowException(UVException(err, "uv_pipe_open"));
  */
}


void PipeWrap::Connect(const FunctionCallbackInfo<Value>& args) {
  HandleScope scope(args.GetIsolate());
  Environment* env = Environment::GetCurrent(args.GetIsolate());

  PipeWrap* wrap = Unwrap<PipeWrap>(args.Holder());

  assert(args[0]->IsObject());
  assert(args[1]->IsString());

  Local<Object> req_wrap_obj = args[0].As<Object>();
  node::Utf8Value name(args[1]);

  // TODO-CODIUS: Do stuff...

  args.GetReturnValue().Set(0);  // uv_pipe_connect() doesn't return errors.
}


void PipeWrap::WriteUtf8String(const FunctionCallbackInfo<Value>& args) {
  HandleScope scope(args.GetIsolate());
  Environment* env = Environment::GetCurrent(args.GetIsolate());

  PipeWrap* wrap = Unwrap<PipeWrap>(args.Holder());

  assert(args[0]->IsObject());
  assert(args[1]->IsString());

  node::Utf8Value name(args[1]);

  write(wrap->handle_.GetFileDescriptor(), *name, name.length());
  // TODO-CODIUS: Do stuff...

  args.GetReturnValue().Set(0);  // uv_pipe_connect() doesn't return errors.
}


void PipeWrap::OnAlloc(EventLoop::Handle* handle,
                       size_t suggested_size,
                       EventLoop::Buffer* buf) {
  PipeWrap* wrap = static_cast<PipeWrap*>(handle->data);
  assert(&wrap->handle_ == reinterpret_cast<EventLoop::PipeHandle*>(handle));
  buf->base = static_cast<char*>(malloc(suggested_size));
  buf->len = suggested_size;

  if (buf->base == NULL && suggested_size > 0) {
    FatalError(
        "node::PipeWrap::OnAlloc(EventLoop::Handle*, size_t, EventLoop::Buffer*)",
        "Out Of Memory");
  }
}

void PipeWrap::OnRead(EventLoop::PipeHandle* handle,
                      ssize_t nread,
                      const EventLoop::Buffer* buf) {
  PipeWrap* wrap = static_cast<PipeWrap*>(handle->data);
  Environment* env = wrap->env();
  HandleScope handle_scope(env->isolate());
  Context::Scope context_scope(env->context());

  // We should not be getting this callback if someone as already called
  // Close() on the handle.
  assert(wrap->persistent().IsEmpty() == false);

  Local<Value> argv[] = {
    Integer::New(env->isolate(), nread),
    Undefined(env->isolate()),
    Undefined(env->isolate())
  };

  if (nread < 0)  {
    if (buf->base != NULL)
      free(buf->base);
    wrap->MakeCallback(env->onread_string(), ARRAY_SIZE(argv), argv);
    return;
  }

  if (nread == 0) {
    if (buf->base != NULL)
      free(buf->base);
    return;
  }

  char* base = static_cast<char*>(realloc(buf->base, nread));
  assert(static_cast<size_t>(nread) <= buf->len);
  argv[1] = Buffer::Use(env, base, nread);

  Local<Object> pending_obj;

  // TODO-CODIUS: Do something like uv_accept where we can accept a handle,
  // wrap it and then put it in pending_obj.
//  if (wrap->is_named_pipe_ipc() &&
//      uv_pipe_pending_count(reinterpret_cast<uv_pipe_t*>(handle)) > 0) {
//    type = uv_pipe_pending_type(reinterpret_cast<uv_pipe_t*>(handle));
//  }

  if (!pending_obj.IsEmpty()) {
    argv[2] = pending_obj;
  }

  wrap->MakeCallback(env->onread_string(), ARRAY_SIZE(argv), argv);
}

void PipeWrap::ReadStart(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args.GetIsolate());
  HandleScope scope(env->isolate());

  PipeWrap* wrap = Unwrap<PipeWrap>(args.Holder());

  // TODO-CODIUS: Implement PipeWrap::ReadStart
//  int err = uv_read_start(wrap->stream(), OnAlloc, OnRead);
  int err = 0;

  args.GetReturnValue().Set(err);
}


}  // namespace node

NODE_MODULE_CONTEXT_AWARE_BUILTIN(pipe_wrap, node::PipeWrap::Initialize)
