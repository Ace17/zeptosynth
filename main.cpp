#include <stdio.h>
#include <stdint.h>
#include <cassert>
#include "SDL.h"
#include "alsa/asoundlib.h"
#include "alsa/seq.h"

namespace
{
struct Exception
{
  const char* msg;
};

#define DebugTrace printf

bool isMidiEvent(snd_seq_event_type t)
{
  switch(t)
  {
  case SND_SEQ_EVENT_NOTEOFF:
  case SND_SEQ_EVENT_NOTEON:
  case SND_SEQ_EVENT_CONTROLLER:
  case SND_SEQ_EVENT_PITCHBEND:
    return true;
  default:
    return false;
  }
}

void readEvents(snd_seq_t* seq)
{
  int ret;

  uint8_t buffer[4096];

  snd_midi_event_t* parser;
  ret = snd_midi_event_new(sizeof buffer, &parser);

  if(ret < 0)
  {
    DebugTrace("snd_midi_event_new error : %d", ret);
    return;
  }

  for(;;)
  {
    if(snd_seq_event_input_pending(seq, 1) == 0)
    {
      // printf("no data\n");
      usleep(1000);
      continue;
    }

    snd_seq_event_t* e = nullptr;
    DebugTrace("readEvents: waiting for one event");
    auto i = snd_seq_event_input(seq, &e);

    if(e == nullptr || i < 0 || e->type == SND_SEQ_EVENT_USR0)
    {
      DebugTrace("readEvents: stopping");
      break;
    }

    DebugTrace("readEvents: got event (%d)\n", i);

    if(isMidiEvent((snd_seq_event_type)e->type))
    {
      auto len = snd_midi_event_decode(parser, buffer, sizeof buffer, e);

      if(len < 0)
      {
        DebugTrace("midi event decode error : %d\n", len);
      }

      printf("got midi event!\n");
      fflush(stdout);
    }
  }

  snd_midi_event_free(parser);

  DebugTrace("readEvents is exiting...");
}

// This function is used to count or get the pinfo structure for a given port number.
unsigned int portInfo(snd_seq_t* seq, snd_seq_port_info_t* pinfo, unsigned int type, int portNumber)
{
  snd_seq_client_info_t* cinfo;
  int count = 0;
  snd_seq_client_info_alloca(&cinfo);

  snd_seq_client_info_set_client(cinfo, -1);

  while(snd_seq_query_next_client(seq, cinfo) >= 0)
  {
    int client = snd_seq_client_info_get_client(cinfo);

    if(!client)
      continue;

    // Reset query info
    snd_seq_port_info_set_client(pinfo, client);
    snd_seq_port_info_set_port(pinfo, -1);

    while(snd_seq_query_next_port(seq, pinfo) >= 0)
    {
      auto atyp = snd_seq_port_info_get_type(pinfo);

      if((atyp & SND_SEQ_PORT_TYPE_MIDI_GENERIC) == 0)
        continue;

      auto caps = snd_seq_port_info_get_capability(pinfo);

      if((caps & type) != type)
        continue;

      if(count == portNumber)
        return 1;

      ++count;
    }
  }

  return 0;
}

const int PORT_NUMBER = 1;

void safeMain()
{
  snd_seq_t* seq;

  if(snd_seq_open(&seq, "default", SND_SEQ_OPEN_INPUT | SND_SEQ_OPEN_OUTPUT, 0) != 0)
    throw  Exception { "Can't open ALSA sequencer" };

  snd_seq_set_client_name(seq, "Zeptosynth");

  int alsaPort = snd_seq_create_simple_port(seq, "ZeptosynthInput", SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE, SND_SEQ_PORT_TYPE_SYNTH);

  // Connect to PORT_NUMBER
  {
    snd_seq_port_info_t* pinfo;
    snd_seq_port_info_alloca(&pinfo);

    if(!portInfo(seq, pinfo, SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ, PORT_NUMBER))
      throw Exception { "Can't get port info" };

    snd_seq_addr_t sender {};
    sender.client = snd_seq_port_info_get_client(pinfo);
    sender.port = snd_seq_port_info_get_port(pinfo);

    snd_seq_addr_t receiver {};
    receiver.client = snd_seq_client_id(seq);

    snd_seq_port_subscribe_t* subs;
    snd_seq_port_subscribe_malloc(&subs);
    snd_seq_port_subscribe_set_sender(subs, &sender);
    snd_seq_port_subscribe_set_dest(subs, &receiver);

    if(snd_seq_subscribe_port(seq, subs))
      throw  Exception { "Can't subscribe to input events" };
  }

  readEvents(seq);

  snd_seq_close(seq);

  printf("OK\n");
}
}

int main()
{
  try
  {
    safeMain();
    return 0;
  }
  catch(Exception const& e)
  {
    fprintf(stderr, "Fatal: %s\n", e.msg);
    return 1;
  }
}

