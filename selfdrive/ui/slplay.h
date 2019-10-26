#ifndef SLPLAY_H
#define SLPLAY_H

#include <stdbool.h>

#ifdef ANDROID

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

typedef struct
{
  const char *uri;
  SLObjectItf player;
  SLPlayItf playInterface;
} uri_player;

#else
typedef struct
{
  const char *uri;
} uri_player;

#endif // ANDROID

void slplay_setup(char **error);
uri_player *slplay_create_player_for_uri(const char *uri, char **error);
void slplay_play(const char *uri, bool loop, char **error);
void slplay_stop_uri(const char *uri, char **error);
void slplay_destroy();

#endif
