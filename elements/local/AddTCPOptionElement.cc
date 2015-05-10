/*
 * AddTCPOptionElement.cc
 *
 *  Created on: May 1, 2015
 *      Author: Baufays Benoit
 *
 *  More comments in the header file.
 */


#include <click/config.h>
#include <click/confparse.hh>
#include "AddTCPOptionElement.hh"
#include <clicknet/tcp.h>
#include <clicknet/ip.h>

CLICK_DECLS

#define IP_MPTCP 30

AddTCPOptionElement::AddTCPOptionElement() {
	_killNotInspected = false;
	_verbose = false;
	_tcpOptions = 0;
	_tcpOptionsC = 0;
	_mptcpOptionsC = 0;
	_mptcpOptions = 0;
	_rmSyn = true;
	_rmData = true;
	_delayRM = 0;
	_ttl = 6000;
}

AddTCPOptionElement::~AddTCPOptionElement() {
	CLICK_LFREE(_tcpOptions,sizeof(uint8_t) * _tcpOptionsC);
	CLICK_LFREE(_mptcpOptions,sizeof(uint8_t) * _mptcpOptionsC);
}

int AddTCPOptionElement::configure(Vector<String> &conf, ErrorHandler *errh){
    String optTableS,mptcpSubS;

    // See doc for the options in header file.
    if (cp_va_kparse(conf, this, errh,
                    "KILLNOTINSPECTED", 0, cpBool, &_killNotInspected,
                    "VERBOSE", 0, cpBool, &_verbose,
                    "RMSYN", 0, cpBool, &_rmSyn,
                    "RMDATA", 0, cpBool, &_rmData,
                    "TCPOPTIONS",0,cpString, &optTableS,
                    "MPTCPOPTIONS",0,cpString, &mptcpSubS,
                    "DELAY",0,cpInteger, &_delayRM,
                    "TTL",0,cpInteger, &_ttl,
                    cpEnd) < 0)
            return -1;
    else
    {
    	_tcpOptions = 0;

    	// Normal TCP Options
    	_tcpOptionsC = checkOptTable(optTableS);
    	if(_tcpOptionsC<0){
    		click_chatter("Error during tcp options table init : %s",optTableS.c_str());
    		return -1;
    	}
    	else
    		initOptTable(&_tcpOptions,optTableS,_tcpOptionsC);

    	// MPTCP options
    	_mptcpOptionsC = checkOptTable(mptcpSubS);
    	if(_mptcpOptionsC < 0){
    		click_chatter("Error during mtcp options table init  : %s",mptcpSubS.c_str());
    		return -1;
    	}
    	else
    		initOptTable(&_mptcpOptions,mptcpSubS,_mptcpOptionsC);

    	if(_delayRM < 0)
    		return -1;

    }

	return 0;
}

int AddTCPOptionElement::initOptTable(uint8_t **table, String &s, int count){
	int start = 0;
	*table =  (uint8_t *) CLICK_LALLOC(sizeof(uint8_t) * count);
	if(*table){
		for(int i=0;i<count;i++){
			(*table)[i] = getOptionAt(s,&start);
		}
		return 0;
	}
	return -1;
}

uint8_t AddTCPOptionElement::getOptionAt(String &s, int *start){
	uint8_t ret = 0;
	while(*start < s.length() && s.at(*start) != '/'){
		ret = ret * 10;
		ret += s.at(*start) - '0';
		*start = *start +1;
	}
	*start = *start +1;
	return ret;
}

void AddTCPOptionElement::printOptTable(uint8_t *table,int len){
	for(int i=0;i<len;i++){
		click_chatter("Option %d is set to %d",i,table[i]);
	}
}

int AddTCPOptionElement::checkOptTable(String &s){
	int nbOptions=0;
	if(s){
		for(int i=0;i<s.length();i++){
			if(s.at(i) != '/' && !(s.at(i)>='0' && s.at(i)<='9')){
				return -1;
			}
			else if(s.at(i)=='/'){
				nbOptions++;
			}
		}
		nbOptions++;
	}
	return nbOptions;
}

bool AddTCPOptionElement::shouldBeInspected(Packet*p_in, WritablePacket **p_out){
	if(p_in->has_network_header()){
		const click_ip *iph = p_in->ip_header();
		if(iph->ip_p == IP_PROTO_TCP){
			const click_tcp *tcph = p_in->tcp_header();
			//bool startRm = updateFlow(iph->ip_src,tcph->th_sport,iph->ip_dst,tcph->th_dport);
			 // th_off = TCP header length in word. -20 to remove static header elements.
			uint16_t optLen = tcph->th_off * 4 - 20;
			bool syn = (tcph->th_flags & 0x2) == 0x2;
			if(tcph->th_off * 4 > 20
					&& optLen > 0 // at least one option
					//&& (startRm) // Do we already need to rm some option (delay)
					&& ((syn && _rmSyn)||(!syn && _rmData)) // do the user ask to inspect this segment  ?
					){
				//we are in good shape
				*p_out = p_in->uniqueify();
				return true;
			}
		}
	}
	return false;
}

void AddTCPOptionElement::push(int, Packet *p_in) {
	WritablePacket *p = 0;
	if(shouldBeInspected(p_in,&p)){
		const click_tcp *tcph = p->tcp_header();
		uint16_t optLen = p->tcp_header()->th_off * 4 - 20;

		if(_verbose) click_chatter("---TCP options---");

		uint8_t *nextOpt = (uint8_t *)(tcph+1);
		//while( nextOpt < (uint8_t *)(tcph+1) + optLen ){
		//	nextOpt = rmTCPOpt(nextOpt,(uint8_t *)(tcph+1)+optLen);
		//}
		*(uint8_t *)(tcph+1)+optLen = TCPOPT_MAXSEG;

		if(_verbose) click_chatter("---End TCP options---");

		output(0).push(p);
		return;
	}
	else{ //Not inspected -> p_in not uniqueify !
		if(_killNotInspected)
			p_in->kill();
		else
			output(0).push(p_in);
	}
}


bool AddTCPOptionElement::mustBeRemoved(uint8_t *opt){
	if(*opt==IP_MPTCP){
		for(int i=0;i<_mptcpOptionsC;i++){
			if((*(opt+2))>>4 == _mptcpOptions[i]) //mptcp sub type : 4 higher bits of the third byte of the TCP option
				return true;
		}
	}
	// no else so that it's still possible to say : remove all MPTCP options without regarding the sub-type
	for(int i=0;i<_tcpOptionsC;i++){
		if(_tcpOptions[i]==*opt)
			return true;
	}
	return false;
}

uint8_t * AddTCPOptionElement::rmTCPOpt(uint8_t *nextOpt, uint8_t * max){
	uint8_t kind = *(nextOpt);
	uint8_t *ret = 0;

	if(kind == TCPOPT_EOL){
		if(_verbose) click_chatter("END ");
		ret = max;
	}
	else if(kind == TCPOPT_NOP){
		if(_verbose) click_chatter("NOPE");
		ret = nextOpt + 1;
	}
	else{
		// TLV standard
		uint8_t len = nextOpt +  1 < max ? *(nextOpt + 1) : 0;

		if(_verbose)  click_chatter("Opt 0x%02x",*(nextOpt));
		if(_verbose)  click_chatter("Len 0x%02x (%d)",len,len);

		if(nextOpt+len > max){
			if(_verbose)  click_chatter("Bad len option ... would go beyond the header");
			nextOpt = max;
		}
		else{
			bool rm = mustBeRemoved(nextOpt);
			if(rm){
				for(int i=0;i<len;i++){
					if(_verbose) click_chatter("    %d : 0x%02x",i,*(nextOpt+i));
					if(_verbose)  click_chatter("changing ...%d",i);
					*(nextOpt+i) = TCPOPT_MAXSEG;
				}
			}
			ret  = nextOpt + len;
		}
	}
	return ret;
}



CLICK_ENDDECLS
EXPORT_ELEMENT(AddTCPOptionElement)

