/*
 * SegSplitElement.cc
 *
 *  Created on: Mar 28, 2013
 *      Author: Benjamin Hesmans
 *
 *      Element to split TCP segment, more details in the header file
 */


#include <click/config.h>
#include <click/confparse.hh>
#include "SegSplitElement.hh"
#include <clicknet/tcp.h>
#include <clicknet/ip.h>

CLICK_DECLS


SegSplitElement::SegSplitElement() {
	_verbose = false;
	_splitAt = 0;
	_tcpOptFirst = true;
	_tcpOptSecond = true;
}

SegSplitElement::~SegSplitElement() {
}

int SegSplitElement::configure(Vector<String> &conf, ErrorHandler *errh){
    String s;
    // See doc for the options in header file.
    if (cp_va_kparse(conf, this, errh,
                    "VERBOSE", 0, cpBool, &_verbose,
                    "SPLITAT",0,cpInteger, &_splitAt,
                    "KEEPTCPOPT",0,cpString, &s,
                    cpEnd) < 0)
            return -1;
    else if(s)
    {
    	if(s.at(0) == 'F')
    		_tcpOptSecond = false;
    	else if (s.at(0) == 'S' )
    		_tcpOptFirst = false;
    	else if (s.at(0) != 'B'){
    		click_chatter("KEEPTCPOPT should be S B or F, %c given", s.at(0));
    		return -1;
    	}
    }
	return 0;
}

int SegSplitElement::getOffset(int len){
	if(_splitAt == 0 && len > 1)
		return len / 2;
	if(_splitAt > 0 && len > _splitAt)
		return _splitAt;
	return len;
}

bool SegSplitElement::mustBeSplit(Packet* p_in, int* at){
	if(p_in->has_network_header()){
		const click_ip *iph = p_in->ip_header();
		if(iph->ip_p == IP_PROTO_TCP){
			int tcpLen = ntohs(iph->ip_len) - 4*(iph->ip_hl) - 4*(p_in->tcp_header()->th_off);
			*at = getOffset(tcpLen);
			return *at != tcpLen;
		}
	}
	return false;
}

int SegSplitElement::splitAt(Packet* p_in, int at, WritablePacket** p1_out, WritablePacket** p2_out){
	Packet *tmp = p_in->clone();
	*p1_out = p_in->uniqueify();
	*p2_out = tmp->uniqueify();
	uint32_t ipLen = ntohs((*p1_out)->ip_header()->ip_len);
	int tcpLen = ipLen - 4*((*p1_out)->ip_header()->ip_hl) - 4*(p_in->tcp_header()->th_off);
	updateFirst(*p1_out,at, ipLen, tcpLen);
	updateSecond(*p2_out,at, ipLen, tcpLen);
	return 0;
}

void SegSplitElement::updateFirst(WritablePacket *p, int to,uint32_t ipLen, int tcpLen){
	p->ip_header()->ip_len =  htons(ipLen - (tcpLen - to));
	p->take(tcpLen - to);
	//always reset FIN in first segment.
	p->tcp_header()->th_flags = ((~0x01) & (p->tcp_header()->th_flags));
	if(!_tcpOptFirst)
		removeTCPOptions(p);
}

void SegSplitElement::updateSecond(WritablePacket *p, int from, int ipLen, int tcpLen){
	p->ip_header()->ip_len = htons(ipLen - from);
	uint8_t *data_new = ((uint8_t*)p->tcp_header()) + 4 * p->tcp_header()->th_off;
	uint8_t *data_old = data_new + from;
	for(int j=0; j<tcpLen-from;j++)
		*(data_new+j)=*(data_old+j);
	p->take(from);
	p->tcp_header()->th_seq = htonl(ntohl(p->tcp_header()->th_seq) + from);
	if(!_tcpOptSecond)
		removeTCPOptions(p);
}

void SegSplitElement::removeTCPOptions(WritablePacket *p){
	int optLen = (p->tcp_header()->th_off * 4) - 20;
	uint8_t *opt = (uint8_t *)(p->tcp_header()+1);
	for(int j=0 ; j < optLen ; j++)
		*(opt+j) = TCPOPT_NOP;
}

void SegSplitElement::push(int, Packet *p_in) {
	int at;
	WritablePacket *p1,*p2;
	if(mustBeSplit(p_in, &at)){
		splitAt(p_in, at, &p1, &p2);
		output(0).push(p1);
		if(noutputs() == 2)
			output(1).push(p2);
		else
			output(0).push(p2);
	}
	else{
		output(0).push(p_in);
	}
}

CLICK_ENDDECLS
EXPORT_ELEMENT(SegSplitElement)




