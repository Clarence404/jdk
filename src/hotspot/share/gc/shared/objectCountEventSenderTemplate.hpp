#ifndef SHARE_GC_SHARED_OBJECTCOUNTEVENTSENDERTEMPLATE_HPP
#define SHARE_GC_SHARED_OBJECTCOUNTEVENTSENDERTEMPLATE_HPP

#include "gc/shared/gcId.hpp"
#include "jfr/jfrEvents.hpp"
#include "memory/heapInspection.hpp"
#include "utilities/macros.hpp"
#include "utilities/ticks.hpp"
#include "iostream"

#if INCLUDE_SERVICES

class KlassInfoEntry;
class Klass;

template <typename Event>
class ObjectCountEventSenderTemplate : public AllStatic {
  static bool _should_send_requestable_event;

  template <typename T>
  static void send_event_if_enabled(Klass* klass, jlong count, julong size, const Ticks& timestamp);

 public:
  static void enable_requestable_event();
  static void disable_requestable_event();

  static void send(const KlassInfoEntry* entry, const Ticks& timestamp);
  static bool should_send_event();
};

typedef ObjectCountEventSenderTemplate<EventObjectCount> ObjectCountEventSender;
typedef ObjectCountEventSenderTemplate<EventObjectCountAfterGC> ObjectCountAfterGCEventSender;

template <typename Event>
bool ObjectCountEventSenderTemplate<Event>::should_send_event() {
#if INCLUDE_JFR
  return _should_send_requestable_event || Event::is_enabled();
#else
  return false;
#endif // INCLUDE_JFR
}

template <typename Event>
bool ObjectCountEventSenderTemplate<Event>::_should_send_requestable_event = false;

template <typename Event>
void ObjectCountEventSenderTemplate<Event>::enable_requestable_event() {
  _should_send_requestable_event = true;
}


template <typename Event>
void ObjectCountEventSenderTemplate<Event>::disable_requestable_event() {
  _should_send_requestable_event = false;
}

template <typename Event>
template <typename T>
void ObjectCountEventSenderTemplate<Event>::send_event_if_enabled(Klass* klass, jlong count, julong size, const Ticks& timestamp) {
  T event(UNTIMED);
  if (event.should_commit()) {
    event.set_starttime(timestamp);
    event.set_endtime(timestamp);
    event.set_gcId(GCId::current());
    event.set_objectClass(klass);
    event.set_count(count);
    event.set_totalSize(size);
    event.commit();
  }
}

template <typename Event>
void ObjectCountEventSenderTemplate<Event>::send(const KlassInfoEntry* entry, const Ticks& timestamp) {
  Klass* klass = entry->klass();
  jlong count = entry->count();
  julong total_size = entry->words() * BytesPerWord;
  
  send_event_if_enabled<Event>(klass, count, total_size, timestamp);
  // If sending ObjectCountAfterGCEvent, check if ObjectCount is enabled and send event data to ObjectCount
  if (std::is_same<Event, EventObjectCountAfterGC>::value && ObjectCountEventSender::should_send_event()) { 
    send_event_if_enabled<EventObjectCount>(klass, count, total_size, timestamp);
  }
}
#endif // INCLUDE_SERVICES

#endif // SHARE_GC_SHARED_OBJECTCOUNTEVENTSENDERTEMPLATE_HPP
