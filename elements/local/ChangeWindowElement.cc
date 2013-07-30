/*
 * ChangeWindowElement.cc
 *
 *  Created on: Apr 17, 2013
 *      Author: Benjamin Hesmans
 */


#include <click/config.h>
#include <click/confparse.hh>
#include "ChangeWindowElement.hh"
#include <clicknet/tcp.h>
#include <clicknet/ip.h>

CLICK_DECLS


ChangeWindowElement::ChangeWindowElement() {
	_verbose = false;
	_delta = 0;
}

ChangeWindowElement::~ChangeWindowElement() {
}

int ChangeWindowElement::configure(Vector<String> &conf, ErrorHandler *errh){
    if (cp_va_kparse(conf, this, errh,
                    "VERBOSE", 0, cpBool, &_verbose,
                    "DELTA",0,cpInteger, &_delta,
                    cpEnd) < 0)
            return -1;
    else
    {
    	// Why don't you do something ?! (Spears B. 2005)
    }

	return 0;
}

bool ChangeWindowElement::shouldBeInspected(Packet*p_in, WritablePacket **p_out){
	if(p_in->has_network_header()){
		const click_ip *iph = p_in->ip_header();
		if(iph->ip_p == IP_PROTO_TCP){
			*p_out = p_in->uniqueify();
			return true;
		}
	}
	return false;
}

void ChangeWindowElement::updateWindow(WritablePacket *wp){
	click_tcp *tcph = wp->tcp_header();
	uint16_t cwin = ntohs(tcph->th_win);
	if(cwin+_delta > 65536 || cwin+_delta < 0)
		return;
	else
		tcph->th_win = htons(cwin + _delta);
}

void ChangeWindowElement::push(int, Packet *p_in) {
	WritablePacket *wp = 0;
	if(shouldBeInspected(p_in, &wp)){
		updateWindow(wp);
		output(0).push(wp);
	}
	else
		output(0).push(p_in);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(ChangeWindowElement)



