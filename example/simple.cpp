#include "../jsmn.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

/*
 * A small example of jsmn parsing when JSON structure is known and number of
 * tokens is predictable.
 */

std::string JSON_STRING("{\"user\": \"johndoe\", \"admin\": false, \"uid\": 1000,\n \"groups\": [\"users\", \"wheel\", \"audio\", \"video\"]}");

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
  if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

int main() {
  int i;
  int r;
  jsmn_parser p;
  // jsmntok_t t[128]; /* We expect no more than 128 tokens */

  p.init(JSON_STRING.c_str());
  r = p.parse();
  if (r < 0) {
    printf("Failed to parse JSON: %d\n", r);
    return 1;
  }

  jsmntok_t * t = p.get_tokens();

  /* Assume the top-level element is an object */
  if (r < 1 || t[0].type != JSMN_OBJECT) {
    printf("Object expected\n");
    return 1;
  }

  /* Loop over all keys of the root object */
  for (i = 1; i < r; i++) {
    if (jsoneq(JSON_STRING.c_str(), &t[i], "user") == 0) {
      /* We may use strndup() to fetch string value */
      printf("- User: %.*s\n", t[i + 1].end - t[i + 1].start,
             JSON_STRING.c_str() + t[i + 1].start);
      i++;
    } else if (jsoneq(JSON_STRING.c_str(), &t[i], "admin") == 0) {
      /* We may additionally check if the value is either "true" or "false" */
      printf("- Admin: %.*s\n", t[i + 1].end - t[i + 1].start,
             JSON_STRING.c_str() + t[i + 1].start);
      i++;
    } else if (jsoneq(JSON_STRING.c_str(), &t[i], "uid") == 0) {
      /* We may want to do strtol() here to get numeric value */
      printf("- UID: %.*s\n", t[i + 1].end - t[i + 1].start,
             JSON_STRING.c_str() + t[i + 1].start);
      i++;
    } else if (jsoneq(JSON_STRING.c_str(), &t[i], "groups") == 0) {
      int j;
      printf("- Groups:\n");
      if (t[i + 1].type != JSMN_ARRAY) {
        continue; /* We expect groups to be an array of strings */
      }
      for (j = 0; j < t[i + 1].size; j++) {
        jsmntok_t *g = &t[i + j + 2];
        printf("  * %.*s\n", g->end - g->start, JSON_STRING.c_str() + g->start);
      }
      i += t[i + 1].size + 1;
    } else {
      printf("Unexpected key: %.*s\n", t[i].end - t[i].start,
             JSON_STRING.c_str() + t[i].start);
    }
  }
  return EXIT_SUCCESS;
}