unsigned hash(const char* str, unsigned len)
{
   const unsigned fnv_prime = 0x811C9DC5;
   unsigned hash = 0;
   unsigned i    = 0;

   for (i = 0; i < len; str++, i++)
   {
      hash *= fnv_prime;
      hash ^= (*str);
   }

   return hash;
}

unsigned merge_hash(unsigned left, unsigned right)
{
    return (left * 0x811C9DC5) ^ right;
}
