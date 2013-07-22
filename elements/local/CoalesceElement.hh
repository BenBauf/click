#ifndef CLICK_COALESCE_HH
#define CLICK_COALESCE_HH
#include <click/element.hh>
#include <click/task.hh>
#include <click/timer.hh>
#include <click/notifier.hh>
CLICK_DECLS

/*
 * Benjamin Hesmans : modified version of delayunqueue element to support tcp coalescing.
 *
=c

Coalesce(DELAY)

=s shaping

delay-inducing pull-to-push converter

=d

Pulls packets from the single input port. Delays them for at least DELAY
seconds, with microsecond precision. A packet with timestamp T will be emitted
no earlier than time (T + DELAY). On output, the packet's timestamp is set to
the delayed time.

Coalesce listens for upstream notification, such as that available from
Queue.

=h delay read/write

Return or set the DELAY parameter.

=a Queue, Unqueue, RatedUnqueue, BandwidthRatedUnqueue, LinkUnqueue,
DelayShaper, SetTimestamp */

class Coalesce : public Element { public:

    Coalesce();
    ~Coalesce();

    const char *class_name() const	{ return "Coalesce"; }
    const char *port_count() const	{ return PORTS_1_1; }
    const char *processing() const	{ return PULL_TO_PUSH; }

    int configure(Vector<String> &, ErrorHandler *);
    int initialize(ErrorHandler *);
    void cleanup(CleanupStage);
    void add_handlers();

    bool areFromSameFlow();
    bool follows();
    bool containSmallData();

    bool doCoalesce(WritablePacket **p_out);

    bool mustBeCoalesced();


    bool run_task(Task *);

  private:

    Packet *_p;
    Packet *_p2;
    Timestamp _delay;
    Task _task;
    Timer _timer;
    uint32_t _mss;
    NotifierSignal _signal;

};

CLICK_ENDDECLS
#endif
