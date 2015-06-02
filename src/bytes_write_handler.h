// Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_BYTES_WRITE_HANDLER_H_
#define CEF_TESTS_CEFCLIENT_BYTES_WRITE_HANDLER_H_
#pragma once

// #include "include/base/cef_lock.h"
// #include "include/cef_stream.h"
#include "include_wrapper_cef.h"

class BytesWriteHandler : public CefWriteHandler {
 public:
  explicit BytesWriteHandler(size_t grow);
  virtual ~BytesWriteHandler();

  virtual size_t Write(const void* ptr, size_t size, size_t n) OVERRIDE;
  virtual int Seek(int64 offset, int whence) OVERRIDE;
  virtual int64 Tell() OVERRIDE;
  virtual int Flush() OVERRIDE;
  virtual bool MayBlock() OVERRIDE { return false; }

  void* GetData() { return data_; }
  int64 GetDataSize() { return offset_; }

 protected:
  size_t Grow(size_t size);

  size_t grow_;
  void* data_;
  int64 datasize_;
  int64 offset_;

  base::Lock lock_;

  IMPLEMENT_REFCOUNTING(BytesWriteHandler);
};

#endif  // CEF_TESTS_CEFCLIENT_BYTES_WRITE_HANDLER_H_
