/*
 * Benjamin Hesmans : modified version of delayunqueue element to support tcp coalescing.
 *
 * Coalesce.{cc,hh} -- element pulls packets from input, delays pushing
 * the packet to output port.
 *
 * Copyright (c) 1999-2001 Massachusetts Institute of Technology
 * Copyright (c) 2002 International Computer Science Institute
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 */

#include <click/config.h>
#include <click/error.hh>
#include <click/args.hh>
#include <click/glue.hh>
#include "CoalesceElement.hh"
#include <click/standard/scheduleinfo.hh>
#include <clicknet/tcp.h>
#include <clicknet/ip.h>
CLICK_DECLS

Coalesce::Coalesce()
    : _p(0), _p2(0), _mss(1500), _task(this), _timer(&_task)
{
}

Coalesce::~Coalesce()
{
}

int
Coalesce::configure(Vector<String> &conf, ErrorHandler *errh)
{
    return Args(conf, this, errh).read_mp("DELAY", _delay).complete();
}

int
Coalesce::initialize(ErrorHandler *errh)
{
    ScheduleInfo::initialize_task(this, &_task, errh);
    _timer.initialize(this);
    _signal = Notifier::upstream_empty_signal(this, 0, &_task);
    return 0;
}

void
Coalesce::cleanup(CleanupStage)
{
    if (_p)
	_p->kill();
}

bool
Coalesce::run_task(Task *)
{
    bool worked = false;

  retry:
    // read a packet
    if (!_p && (_p = input(0).pull())) {
	if (!_p->timestamp_anno().sec()) // get timestamp if not set
	    _p->timestamp_anno().assign_now();
	_p->timestamp_anno() += _delay;
    }

    if (_p) {
	Timestamp now = Timestamp::now();
	if (_p->timestamp_anno() <= now) {
	    // packet ready for output
		// TODO may be changed to do a reverse lookup in the previous queue element
		// IDEA use yank1 on casted input(0) simple queue
		_p2 = input(0).pull();
		if(mustBeCoalesced()){
			//actual coalescing
			//click_chatter("Will coalesce");
			WritablePacket *p_out;
			doCoalesce(&p_out);
			_p2->kill();
			_p=0;
			_p2=0;
			output(0).push(p_out);
		}
		else{
			//click_chatter("I should not do it !");
			output(0).push(_p);
			_p = _p2;
			_p2 = 0;
		}
	    worked = true;
	    goto retry;
	}

	Timestamp expiry = _p->timestamp_anno() - Timer::adjustment();
	if (expiry <= now)
	    // small delta, reschedule Task
	    /* Task rescheduled below */;
	else {
	    // large delta, schedule Timer
	    _timer.schedule_at(expiry);
	    return false;		// without rescheduling
	}
    } else {
	// no Packet available
	if (!_signal)
	    return false;		// without rescheduling
    }

    _task.fast_reschedule();
    return worked;
}

bool Coalesce::doCoalesce(WritablePacket **p_out){
	int lentcp2 = ntohs((_p2)->ip_header()->ip_len) - 4*((_p2)->ip_header()->ip_hl) - 4*(_p2->tcp_header()->th_off);
	*p_out = _p->put(lentcp2);
	uint8_t* data2 = (uint8_t *)_p2->ip_header() + ntohs(_p2->ip_header()->ip_len);
	uint8_t* begindata2 = (uint8_t *) (*p_out)->ip_header() + ntohs((*p_out)->ip_header()->ip_len);
	for(int i=0;i<lentcp2;i++)
		*(begindata2+i)=*(data2+i);
	(*p_out)->ip_header()->ip_len = htons(ntohs((*p_out)->ip_header()->ip_len)+ lentcp2);

	return true;
}

bool Coalesce::areFromSameFlow(){
	const click_ip *iph = _p->ip_header();
	const click_ip *iph2 = _p2->ip_header();
	const click_tcp *tcph = _p->tcp_header();
	const click_tcp *tcph2 = _p2->tcp_header();
	if (iph->ip_dst!=iph2->ip_dst || iph->ip_src != iph2->ip_src || tcph->th_dport != tcph2->th_dport  || tcph->th_sport != tcph2->th_sport)
		return false;
	return true;
}
bool Coalesce::containSmallData(){
	int lentcp  = ntohs((_p)->ip_header()->ip_len)  - 4*((_p)->ip_header()->ip_hl)  - 4*(_p->tcp_header()->th_off);
	int lentcp2 = ntohs((_p2)->ip_header()->ip_len) - 4*((_p2)->ip_header()->ip_hl) - 4*(_p2->tcp_header()->th_off);
	return lentcp > 0 && lentcp2 > 0 && lentcp + lentcp2 <= _mss;
}
bool Coalesce::follows(){
	int lentcp  = ntohs((_p)->ip_header()->ip_len)  - 4*((_p)->ip_header()->ip_hl)  - 4*(_p->tcp_header()->th_off);
	return ntohl(_p->tcp_header()->th_seq) + lentcp == ntohl(_p2->tcp_header()->th_seq);
}

bool Coalesce::mustBeCoalesced(){
	if(!_p2 ||  !areFromSameFlow() || !containSmallData() ||  !follows())
		return false;
	return true;
}


void
Coalesce::add_handlers()
{
    add_data_handlers("delay", Handler::OP_READ | Handler::OP_WRITE, &_delay, true);
    add_task_handlers(&_task);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(Coalesce)
