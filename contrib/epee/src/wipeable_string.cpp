// Copyright (c) 2017, The Monero Project
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <string.h>
#include "misc_log_ex.h"
#include "wipeable_string.h"

namespace epee
{

void *(*wipeable_string::wipefunc)(void*, size_t) = NULL;

wipeable_string::wipeable_string(const wipeable_string &other):
  buffer(other.buffer)
{
}

wipeable_string::wipeable_string(wipeable_string &&other)
{
  if (&other == this)
    return;
  buffer = std::move(other.buffer);
}

wipeable_string::wipeable_string(const std::string &other)
{
  grow(other.size());
  memcpy(buffer.data(), other.c_str(), size());
}

wipeable_string::wipeable_string(std::string &&other)
{
  CHECK_AND_ASSERT_THROW_MES(wipefunc, "wipefunc is not set");
  grow(other.size());
  memcpy(buffer.data(), other.c_str(), size());
  if (!other.empty())
  {
    wipefunc(&other[0], other.size()); // we're kinda left with this again aren't we
    other = std::string();
  }
}

wipeable_string::wipeable_string(const char *s)
{
  grow(strlen(s));
  memcpy(buffer.data(), s, size());
}

wipeable_string::~wipeable_string()
{
  wipe();
}

void wipeable_string::wipe()
{
  CHECK_AND_ASSERT_THROW_MES(wipefunc, "wipefunc is not set");
  wipefunc(buffer.data(), buffer.size() * sizeof(char));
}

void wipeable_string::grow(size_t sz, size_t reserved)
{
  CHECK_AND_ASSERT_THROW_MES(wipefunc, "wipefunc is not set");
  if (reserved == 0)
    reserved = sz;
  CHECK_AND_ASSERT_THROW_MES(reserved >= sz, "reserved < sz");
  if (reserved <= buffer.capacity())
    return;
  size_t old_sz = buffer.size();
  std::unique_ptr<char[]> tmp{new char[old_sz]};
  memcpy(tmp.get(), buffer.data(), old_sz * sizeof(char));
  wipefunc(buffer.data(), old_sz * sizeof(char));
  buffer.reserve(reserved);
  buffer.resize(sz);
  memcpy(buffer.data(), tmp.get(), sz * sizeof(char));
  wipefunc(tmp.get(), old_sz * sizeof(char));
}

void wipeable_string::push_back(char c)
{
  grow(size() + 1);
  buffer.push_back(c);
}

void wipeable_string::pop_back()
{
  resize(size() - 1);
}

void wipeable_string::resize(size_t sz)
{
  CHECK_AND_ASSERT_THROW_MES(wipefunc, "wipefunc is not set");
  if (sz < buffer.size())
    wipefunc(buffer.data() + sz, buffer.size() - sz);
  grow(sz);
}

void wipeable_string::reserve(size_t sz)
{
  grow(size(), sz);
}

void wipeable_string::clear()
{
  resize(0);
}

wipeable_string &wipeable_string::operator=(wipeable_string &&other)
{
  if (&other != this)
    buffer = std::move(other.buffer);
  return *this;
}

wipeable_string &wipeable_string::operator=(const wipeable_string &other)
{
  if (&other != this)
    buffer = other.buffer;
  return *this;
}

}
