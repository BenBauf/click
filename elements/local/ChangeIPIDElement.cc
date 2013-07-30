/*
 * ChangeIPIDElement.cc
 *
 *  Created on: Apr 17, 2013
 *      Author: Benjamin Hesmans
 */


#include <click/config.h>
#include <click/confparse.hh>
#include "ChangeIPIDElement.hh"
#include <clicknet/tcp.h>
#include <clicknet/ip.h>

CLICK_DECLS


ChangeIPIDElement::ChangeIPIDElement() {
	_verbose = false;
	_delta = 0;
}

ChangeIPIDElement::~ChangeIPIDElement() {
}

int ChangeIPIDElement::configure(Vector<String> &conf, ErrorHandler *errh){
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

bool ChangeIPIDElement::shouldBeInspected(Packet*p_in, WritablePacket **p_out){
	if(p_in->has_network_header()){
		*p_out = p_in->uniqueify();
		return true;
	}
	return false;
}

void ChangeIPIDElement::updateIPID(WritablePacket *wp){
	wp->ip_header()->ip_id =  htons(ntohs(wp->ip_header()->ip_id) + _delta);
}

void ChangeIPIDElement::push(int, Packet *p_in) {
	WritablePacket *wp = 0;
	if(shouldBeInspected(p_in, &wp)){
		updateIPID(wp);
		output(0).push(wp);
	}
	else
		output(0).push(p_in);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(ChangeIPIDElement)

