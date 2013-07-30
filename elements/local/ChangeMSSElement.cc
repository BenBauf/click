/*
 * ChangeMSSElement.cc
 *
 *  Created on: Apr 26, 2013
 *      Author: Benjamin Hesmans
 */



#include <click/config.h>
#include <click/confparse.hh>
#include "ChangeMSSElement.hh"
#include <clicknet/tcp.h>
#include <clicknet/ip.h>

CLICK_DECLS


ChangeMSSElement::ChangeMSSElement() {
	_verbose = false;
	_delta = 0;
	_div = 0;
}

ChangeMSSElement::~ChangeMSSElement() {
}

int ChangeMSSElement::configure(Vector<String> &conf, ErrorHandler *errh){
    if (cp_va_kparse(conf, this, errh,
                    "VERBOSE", 0, cpBool, &_verbose,
                    "DELTA",0,cpInteger, &_delta,
                    "DIV",0,cpInteger, &_div,
                    cpEnd) < 0)
            return -1;
    else
    {
    	// Why don't you do something ?! (Spears B. 2005)
    }

	return 0;
}

bool ChangeMSSElement::containsMSS(Packet* p_in, int *at){
	int optLen = (p_in->tcp_header()->th_off * 4) - 20, j;
	uint8_t *opt = (uint8_t *)(p_in->tcp_header()+1);
	for(j=0 ; j < optLen && *(opt+j) !=  TCPOPT_MAXSEG; j++)
		if (*(opt+j) != TCPOPT_NOP && *(opt+j) != TCPOPT_EOL )
			j += ((uint8_t) *(opt+j+1)) - 1; //-1 else the j++ will desync.
	*at = j;
	return j!=optLen;
}

bool ChangeMSSElement::shouldBeInspected(Packet* p_in, WritablePacket **p_out, int *at){
	if(p_in->has_network_header()){
		const click_ip *iph = p_in->ip_header();
		//TODO check if it's a syn. MSS only in syns
		if(iph->ip_p == IP_PROTO_TCP && containsMSS(p_in, at)){
			*p_out = p_in->uniqueify();
			return true;
		}
	}
	return false;
}

void ChangeMSSElement::updateMSS(WritablePacket *wp, int at){

	click_tcp *tcph = wp->tcp_header();
	uint16_t mss = ntohs(*( (uint16_t*) ( ((uint8_t *)(tcph+1)) + at + 2)  ) );
	int delta = _div > 0 ? 0 - ( (_div - 1 ) * mss / _div) : _delta;
	if(mss+delta > 65536 || mss+delta < 0)
		return;
	else
		*((uint16_t*) (((uint8_t *)(tcph+1)) + at + 2)) = htons(mss + delta);
}

void ChangeMSSElement::push(int, Packet *p_in) {
	WritablePacket *wp = 0;
	int at;
	if(shouldBeInspected(p_in, &wp, &at)){
		updateMSS(wp,at);
		output(0).push(wp);
	}
	else
		output(0).push(p_in);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(ChangeMSSElement)






