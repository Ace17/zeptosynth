// ALSA implementation
#include "midi_input.h"

#include "alsa/asoundlib.h"
#include "alsa/seq.h"
#include "monotime.h"

namespace
{
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

struct AlsaMidiInput : IMidiInput
{
  AlsaMidiInput(int portNumber)
  {
    if(snd_seq_open(&m_seq, "default", SND_SEQ_OPEN_INPUT | SND_SEQ_OPEN_OUTPUT, 0) != 0)
      throw  Exception { "Can't open ALSA sequencer" };

    auto ret = snd_midi_event_new(MAX_SIZE, &m_event);

    if(ret < 0)
      throw  Exception { "Can't create ALSA parser" };

    snd_seq_set_client_name(m_seq, "Zeptosynth");

    int alsaPort = snd_seq_create_simple_port(m_seq, "ZeptosynthInput", SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE, SND_SEQ_PORT_TYPE_SYNTH);

    // Connect to portNumber
    {
      snd_seq_port_info_t* pinfo;
      snd_seq_port_info_alloca(&pinfo);

      if(!portInfo(m_seq, pinfo, SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ, portNumber))
        throw Exception { "Can't get port info" };

      snd_seq_addr_t sender {};
      sender.client = snd_seq_port_info_get_client(pinfo);
      sender.port = snd_seq_port_info_get_port(pinfo);

      snd_seq_addr_t receiver {};
      receiver.client = snd_seq_client_id(m_seq);

      snd_seq_port_subscribe_t* subs;
      snd_seq_port_subscribe_malloc(&subs);
      snd_seq_port_subscribe_set_sender(subs, &sender);
      snd_seq_port_subscribe_set_dest(subs, &receiver);

      if(snd_seq_subscribe_port(m_seq, subs))
        throw  Exception { "Can't subscribe to input events" };
    }
  }

  ~AlsaMidiInput()
  {
    snd_midi_event_free(m_event);
  }

  int read(uint8_t* buffer, double& timestamp) override
  {
    if(snd_seq_event_input_pending(m_seq, 1) == 0)
      return 0;

    timestamp = get_monotonic_time();

    snd_seq_event_t* e = nullptr;
    auto i = snd_seq_event_input(m_seq, &e);

    if(!e || i < 0 || e->type == SND_SEQ_EVENT_USR0)
      return -1; // stop thread?

    if(!isMidiEvent((snd_seq_event_type)e->type))
      return 0;

    auto len = snd_midi_event_decode(m_event, buffer, MAX_SIZE, e);

    if(len < 0)
    {
      fprintf(stderr, "midi event decode error : %d\n", len);
      return -2;
    }

    return len;
  }

  snd_seq_t* m_seq;
  snd_midi_event_t* m_event;
};
}

Unique<IMidiInput> createMidiInput(int portNumber)
{
  return makeUnique<AlsaMidiInput>(portNumber);
}

