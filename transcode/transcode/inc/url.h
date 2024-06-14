#ifndef URL_H_INCLUDED
#define URL_H_INCLUDED

struct url
{
   const char* protocol;
   const char* host;
   const char* port;
   const char* path;
   const char* file;
   const char* format;
   const char* params;
};

#endif // URL_H_INCLUDED
