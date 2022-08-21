#include <cstdarg>

void *DisMember(size_t size, ...)
{
  if (size != sizeof(void *))
    return nullptr;
  va_list args;
  va_start(args, size);
  void *res = va_arg(args, void *);
  va_end(args);
  return res;
}