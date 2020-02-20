#ifndef SRC_STREAM_BASE_H_
#define SRC_STREAM_BASE_H_

#if defined(NODE_WANT_INTERNALS) && NODE_WANT_INTERNALS

#include "env.h"
#include "async_wrap.h"
#include "req_wrap-inl.h"
#include "node.h"
#include "util.h"

#include "v8.h"

namespace node {

// Forward declarations
class ShutdownWrap;
class WriteWrap;
class StreamBase;
class StreamResource;

struct StreamWriteResult {
  bool async;
  int err;
  WriteWrap* wrap;
};


class StreamReq {
 public:
  static constexpr int kStreamReqField = 1;

  explicit StreamReq(StreamBase* stream,
                     v8::Local<v8::Object> req_wrap_obj) : stream_(stream) {
    AttachToObject(req_wrap_obj);
  }

  virtual ~StreamReq() {}
  virtual AsyncWrap* GetAsyncWrap() = 0;
  v8::Local<v8::Object> object();

  void Done(int status, const char* error_str = nullptr);
  void Dispose();

  inline StreamBase* stream() const { return stream_; }

  static StreamReq* FromObject(v8::Local<v8::Object> req_wrap_obj);

 protected:
  virtual void OnDone(int status) = 0;

  void AttachToObject(v8::Local<v8::Object> req_wrap_obj);

 private:
  StreamBase* const stream_;
};

class ShutdownWrap : public StreamReq {
 public:
  ShutdownWrap(StreamBase* stream,
               v8::Local<v8::Object> req_wrap_obj)
    : StreamReq(stream, req_wrap_obj) { }

  void OnDone(int status) override;  // Just calls stream()->AfterShutdown()
};

class WriteWrap : public StreamReq {
 public:
  char* Storage();
  size_t StorageSize() const;
  void SetAllocatedStorage(char* data, size_t size);

  WriteWrap(StreamBase* stream,
            v8::Local<v8::Object> req_wrap_obj)
    : StreamReq(stream, req_wrap_obj) { }

  ~WriteWrap() {
    free(storage_);
  }

  void OnDone(int status) override;  // Just calls stream()->AfterWrite()

 private:
  char* storage_ = nullptr;
  size_t storage_size_ = 0;
};


// This is the generic interface for objects that control Node.js' C++ streams.
// For example, the default `EmitToJSStreamListener` emits a stream's data
// as Buffers in JS, or `TLSWrap` reads and decrypts data from a stream.
class StreamListener {
 public:
  virtual ~StreamListener();

  // This is called when a stream wants to allocate memory immediately before
  // reading data into the freshly allocated buffer (i.e. it is always followed
  // by a `OnStreamRead()` call).
  // This memory may be statically or dynamically allocated; for example,
  // a protocol parser may want to read data into a static buffer if it knows
  // that all data is going to be fully handled during the next
  // `OnStreamRead()` call.
  // The returned buffer does not need to contain `suggested_size` bytes.
  // The default implementation of this method returns a buffer that has exactly
  // the suggested size and is allocated using malloc().
  virtual uv_buf_t OnStreamAlloc(size_t suggested_size);

  // `OnStreamRead()` is called when data is available on the socket and has
  // been read into the buffer provided by `OnStreamAlloc()`.
  // The `buf` argument is the return value of `uv_buf_t`, or may be a buffer
  // with base nullptr in case of an error.
  // `nread` is the number of read bytes (which is at most the buffer length),
  // or, if negative, a libuv error code.
  virtual void OnStreamRead(ssize_t nread,
                            const uv_buf_t& buf) = 0;

  // This is called once a write has finished. `status` may be 0 or,
  // if negative, a libuv error code.
  // By default, this is simply passed on to the previous listener
  // (and raises an assertion if there is none).
  virtual void OnStreamAfterWrite(WriteWrap* w, int status);

  // This is called once a shutdown has finished. `status` may be 0 or,
  // if negative, a libuv error code.
  // By default, this is simply passed on to the previous listener
  // (and raises an assertion if there is none).
  virtual void OnStreamAfterShutdown(ShutdownWrap* w, int status);

  // This is called immediately before the stream is destroyed.
  virtual void OnStreamDestroy() {}

 protected:
  // Pass along a read error to the `StreamListener` instance that was active
  // before this one. For example, a protocol parser does not care about read
  // errors and may instead want to let the original handler
  // (e.g. the JS handler) take care of the situation.
  void PassReadErrorToPreviousListener(ssize_t nread);

  StreamResource* stream_ = nullptr;
  StreamListener* previous_listener_ = nullptr;

  friend class StreamResource;
};


// An (incomplete) stream listener class that calls the `.oncomplete()`
// method of the JS objects associated with the wrap objects.
class ReportWritesToJSStreamListener : public StreamListener {
 public:
  void OnStreamAfterWrite(WriteWrap* w, int status) override;
  void OnStreamAfterShutdown(ShutdownWrap* w, int status) override;

 private:
  void OnStreamAfterReqFinished(StreamReq* req_wrap, int status);
};


// A default emitter that just pushes data chunks as Buffer instances to
// JS land via the handle’s .ondata method.
class EmitToJSStreamListener : public ReportWritesToJSStreamListener {
 public:
  void OnStreamRead(ssize_t nread, const uv_buf_t& buf) override;
};


// A generic stream, comparable to JS land’s `Duplex` streams.
// A stream is always controlled through one `StreamListener` instance.
class StreamResource {
 public:
  virtual ~StreamResource();

  // These need to be implemented on the readable side of this stream:

  // Start reading from the underlying resource. This is called by the consumer
  // when more data is desired. Use `EmitAlloc()` and `EmitData()` to
  // pass data along to the consumer.
  virtual int ReadStart() = 0;
  // Stop reading from the underlying resource. This is called by the
  // consumer when its buffers are full and no more data can be handled.
  virtual int ReadStop() = 0;

  // These need to be implemented on the writable side of this stream:
  // All of these methods may return an error code synchronously.
  // In that case, the finish callback should *not* be called.

  // Perform a shutdown operation, and call req_wrap->Done() when finished.
  virtual int DoShutdown(ShutdownWrap* req_wrap) = 0;
  // Try to write as much data as possible synchronously, and modify
  // `*bufs` and `*count` accordingly. This is a no-op by default.
  virtual int DoTryWrite(uv_buf_t** bufs, size_t* count);
  // Perform a write of data, and call req_wrap->Done() when finished.
  virtual int DoWrite(WriteWrap* w,
                      uv_buf_t* bufs,
                      size_t count,
                      uv_stream_t* send_handle) = 0;

  // Optionally, this may provide an error message to be used for
  // failing writes.
  virtual const char* Error() const;
  // Clear the current error (i.e. that would be returned by Error()).
  virtual void ClearError();

  // Transfer ownership of this stream to `listener`. The previous listener
  // will not receive any more callbacks while the new listener was active.
  void PushStreamListener(StreamListener* listener);
  // Remove a listener, and, if this was the currently active one,
  // transfer ownership back to the previous listener.
  void RemoveStreamListener(StreamListener* listener);

 protected:
  // Call the current listener's OnStreamAlloc() method.
  uv_buf_t EmitAlloc(size_t suggested_size);
  // Call the current listener's OnStreamRead() method and update the
  // stream's read byte counter.
  void EmitRead(ssize_t nread, const uv_buf_t& buf = uv_buf_init(nullptr, 0));
  // Call the current listener's OnStreamAfterWrite() method.
  void EmitAfterWrite(WriteWrap* w, int status);
  // Call the current listener's OnStreamAfterShutdown() method.
  void EmitAfterShutdown(ShutdownWrap* w, int status);

  StreamListener* listener_ = nullptr;
  uint64_t bytes_read_ = 0;

  friend class StreamListener;
};


class StreamBase : public StreamResource {
 public:
  enum Flags {
    kFlagNone = 0x0,
    kFlagHasWritev = 0x1,
    kFlagNoShutdown = 0x2
  };

  template <class Base>
  static inline void AddMethods(Environment* env,
                                v8::Local<v8::FunctionTemplate> target,
                                int flags = kFlagNone);

  virtual bool IsAlive() = 0;
  virtual bool IsClosing() = 0;
  virtual bool IsIPCPipe();
  virtual int GetFD();

  void CallJSOnreadMethod(ssize_t nread, v8::Local<v8::Object> buf);

  // This is named `stream_env` to avoid name clashes, because a lot of
  // subclasses are also `BaseObject`s.
  Environment* stream_env() const;

  // Shut down the current stream. This request can use an existing
  // ShutdownWrap object (that was created in JS), or a new one will be created.
  int Shutdown(v8::Local<v8::Object> req_wrap_obj = v8::Local<v8::Object>());

  // Write data to the current stream. This request can use an existing
  // WriteWrap object (that was created in JS), or a new one will be created.
  // This will first try to write synchronously using `DoTryWrite()`, then
  // asynchronously using `DoWrite()`.
  // If the return value indicates a synchronous completion, no callback will
  // be invoked.
  StreamWriteResult Write(
      uv_buf_t* bufs,
      size_t count,
      uv_stream_t* send_handle = nullptr,
      v8::Local<v8::Object> req_wrap_obj = v8::Local<v8::Object>());

  // These can be overridden by subclasses to get more specific wrap instances.
  // For example, a subclass Foo could create a FooWriteWrap or FooShutdownWrap
  // (inheriting from ShutdownWrap/WriteWrap) that has extra fields, like
  // an associated libuv request.
  virtual ShutdownWrap* CreateShutdownWrap(v8::Local<v8::Object> object);
  virtual WriteWrap* CreateWriteWrap(v8::Local<v8::Object> object);

  // One of these must be implemented
  virtual AsyncWrap* GetAsyncWrap() = 0;
  virtual v8::Local<v8::Object> GetObject();

 protected:
  explicit StreamBase(Environment* env);

  // JS Methods
  int ReadStartJS(const v8::FunctionCallbackInfo<v8::Value>& args);
  int ReadStopJS(const v8::FunctionCallbackInfo<v8::Value>& args);
  int Shutdown(const v8::FunctionCallbackInfo<v8::Value>& args);
  int Writev(const v8::FunctionCallbackInfo<v8::Value>& args);
  int WriteBuffer(const v8::FunctionCallbackInfo<v8::Value>& args);
  template <enum encoding enc>
  int WriteString(const v8::FunctionCallbackInfo<v8::Value>& args);

  template <class Base>
  static void GetFD(const v8::FunctionCallbackInfo<v8::Value>& args);

  template <class Base>
  static void GetExternal(const v8::FunctionCallbackInfo<v8::Value>& args);

  template <class Base>
  static void GetBytesRead(const v8::FunctionCallbackInfo<v8::Value>& args);

  template <class Base,
            int (StreamBase::*Method)(
      const v8::FunctionCallbackInfo<v8::Value>& args)>
  static void JSMethod(const v8::FunctionCallbackInfo<v8::Value>& args);

 private:
  Environment* env_;
  EmitToJSStreamListener default_listener_;

  // These are called by the respective {Write,Shutdown}Wrap class.
  void AfterShutdown(ShutdownWrap* req, int status);
  void AfterWrite(WriteWrap* req, int status);

  template <typename Wrap, typename EmitEvent>
  void AfterRequest(Wrap* req_wrap, EmitEvent emit);

  friend class WriteWrap;
  friend class ShutdownWrap;
};


// These are helpers for creating `ShutdownWrap`/`WriteWrap` instances.
// `OtherBase` must have a constructor that matches the `AsyncWrap`
// constructors’s (Environment*, Local<Object>, AsyncWrap::Provider) signature
// and be a subclass of `AsyncWrap`.
template <typename OtherBase>
class SimpleShutdownWrap : public ShutdownWrap, public OtherBase {
 public:
  SimpleShutdownWrap(StreamBase* stream,
                     v8::Local<v8::Object> req_wrap_obj);
  ~SimpleShutdownWrap();

  AsyncWrap* GetAsyncWrap() override { return this; }
  size_t self_size() const override { return sizeof(*this); }
};

template <typename OtherBase>
class SimpleWriteWrap : public WriteWrap, public OtherBase {
 public:
  SimpleWriteWrap(StreamBase* stream,
                  v8::Local<v8::Object> req_wrap_obj);
  ~SimpleWriteWrap();

  AsyncWrap* GetAsyncWrap() override { return this; }
  size_t self_size() const override { return sizeof(*this) + StorageSize(); }
};

}  // namespace node

#endif  // defined(NODE_WANT_INTERNALS) && NODE_WANT_INTERNALS

#endif  // SRC_STREAM_BASE_H_
