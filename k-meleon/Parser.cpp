#include "StdAfx.h"

#include "Plugins.h"

#include "MfcEmbed.h"
extern CMfcEmbedApp theApp;

#include "Parser.h"

// this is basically a copy of strtok, but it doesn't use a static var, so it
// doesn't step on anyone's toes
// there was a problem when a plugin tried to use strtok when loading

// changed: we need to do this differently because the other version didn't
// like international characters?!?

char * CParser::strtok (char * string, const char * control)
{
#if 0
   char *str;
   const char *ctrl = control;

   unsigned char map[32];
   int count;

   /* Clear control map */
   for (count = 0; count < 32; count++)
      map[count] = 0;

   /* Set bits in delimiter table */
   do {
      map[*ctrl >> 3] |= (1 << (*ctrl & 7));
   } while (*ctrl++);

   /* Initialize str. If string is NULL, set str to the saved
    * pointer (i.e., continue breaking tokens out of the string
    * from the last strtok call) */
   if (string)
      str = string;
   else
      str = nextoken;

   /* Find beginning of token (skip over leading delimiters). Note that
    * there is no token iff this loop sets str to point to the terminal
    * null (*str == '\0') */
   while ( (map[*str >> 3] & (1 << (*str & 7))) && *str )
      str++;

   string = str;

   /* Find the end of the token. If it is not the end of the string,
    * put a null there. */
   for ( ; *str ; str++ )
      if ( map[*str >> 3] & (1 << (*str & 7)) ) {
         *str++ = '\0';
         break;
      }

   /* Update nextoken */
   nextoken = str;

   /* Determine if a token has been found. */
   if ( string == str )
      return NULL;
   else
      return string;
#else
   if (string)
      nextoken = string;

   char *tok;
   if ( (tok = strstr(nextoken, control)) ) {
      *tok = 0;
      char *ret = nextoken;
      nextoken = tok + strlen(control);
      return ret;
   }
   else return 0;
#endif
}

CParser::Load(CString &filename)
{
   CFile *file;
   TRY {
      file = new CFile(filename, CFile::modeRead);
   }
   CATCH (CFileException, e) {
      file = NULL;
      return 0;
   }
   END_CATCH

   long length = file->GetLength();
   char *buffer = new char[length + 1];
   file->Read(buffer, length);
   // force the terminating 0
   buffer[length] = 0;

   file->Close();
   delete file;
   file = NULL;

   int pauseParsing = 0;

   char *p = strtok(buffer, "\r\n");
   while (p){
      if (p[0] == '#'){
      }
      else if (p[0] == '%'){
         if (strnicmp(p+1, "strict", 6) == 0){
            LOG_STRICT();
         }
         else if (strnicmp(p+1, "verbose", 7) == 0) {
            LOG_VERBOSE();
         }
         else if (strnicmp(p+1, "ifplugin", 8) == 0) {
            char *plugin = p+9;
            kmeleonPlugin * kPlugin = theApp.plugins.Load(plugin);
            if (!kPlugin || !kPlugin->loaded) {
               pauseParsing = 1;
            }
         }
         else if (strcmpi(p+1, "endif") == 0) {
            pauseParsing = 0;
         }
         else if (strcmpi(p+1, "else") == 0) {
            pauseParsing = !pauseParsing;
         }
      }
      else if (pauseParsing){
      }
      else {
         Parse(p);
      }
      p = strtok(NULL, "\r\n");
   } // while

   delete [] buffer;

   return true;
}
