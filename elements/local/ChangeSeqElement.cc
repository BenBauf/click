/*
 * ChangeSeq.cc
 *
 *  Created on: Apr 3, 2013
 *      Author: Benjamin Hesmans
 */


#include <click/config.h>
#include <click/confparse.hh>
#include "ChangeSeqElement.hh"
#include <clicknet/tcp.h>
#include <clicknet/ip.h>

CLICK_DECLS


ChangeSeqElement::ChangeSeqElement() {
	_verbose = false;
	_reverse = false;
	_delta = 0;
}

ChangeSeqElement::~ChangeSeqElement() {
}

int ChangeSeqElement::configure(Vector<String> &conf, ErrorHandler *errh){
    if (cp_va_kparse(conf, this, errh,
                    "VERBOSE", 0, cpBool, &_verbose,
                    "DELTA",0,cpInteger, &_delta,
                    "REVERSE", 0, cpBool, &_reverse,
                    cpEnd) < 0)
            return -1;
    else
    {
    	// Why don't you do something ?! (Spears B. 2005)
    }

	return 0;
}

int ChangeSeqElement::getDelta(Packet* /*p_in*/){
	if(_delta != 0)
		return _reverse ? 0 - _delta : _delta;
	else{
		return 0; // TODO update to hash
	}
}

bool ChangeSeqElement::shouldBeInspected(Packet*p_in, WritablePacket **p_out){
	if(p_in->has_network_header()){
		const click_ip *iph = p_in->ip_header();
		if(iph->ip_p == IP_PROTO_TCP){
			*p_out = p_in->uniqueify();
			return true;
		}
	}
	return false;
}

void ChangeSeqElement::updateSeq(WritablePacket *wp, int delta){
	click_tcp *tcph = wp->tcp_header();
	if(_reverse)
		tcph->th_ack =  htonl(ntohl(tcph->th_ack) + delta);
	else
		tcph->th_seq = htonl(ntohl(tcph->th_seq) + delta);
}

void ChangeSeqElement::push(int, Packet *p_in) {
	WritablePacket *wp = 0;
	if(shouldBeInspected(p_in, &wp)){
		int delta = getDelta(wp);
		updateSeq(wp,delta);
		output(0).push(wp);
	}
	else
		output(0).push(p_in);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(ChangeSeqElement)
