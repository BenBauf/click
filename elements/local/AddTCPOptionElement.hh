/*
 * AddTCPOptionElement.cc
 *
 *  Created on: May 1, 2015
 *      Author: Baufays Benoit
 *
 *  add option
 *  The TCP checksum is NOT updated : you can use SetTCPChecksum to set it.
 *
 */

#ifndef CLICK_AddTCPOptionElement_HH_
#define CLICK_AddTCPOptionElement_HH_

#include <click/element.hh>

#include <click/hashtable.hh>
#include <click/ipflowid.hh>
#include <click/timer.hh>
#include <click/ipaddress.hh>


CLICK_DECLS

class AddTCPOptionElement;


class AddTCPOptionElement : public Element { public:
	//typedef HashContainer<FlowIDrm> Map;

    ~AddTCPOptionElement();
    AddTCPOptionElement();
    /**
     * cf click documentation
     */
    const char *class_name() const { return "AddTCPOpt"; }
    const char *port_count() const { return PORTS_1_1; }
    const char *processing() const { return PUSH; }
    void push(int port, Packet *p);

    /**
     * cf click documentation and :
     * The element's options are :
     *	- bool KILLNOTCP		: (optional) if true, kill traffic that is not Inspected : eases debugging !
     *  - bool VERBOSE			: (optional) if true, speaks a lot
     *  - String TCPTOPTIONS	: (optional) String format : "a/b/c/.../j" a..j are tcp options to remove ( should be 0<=n<=255)
     *  - String MPTCPOPTIONS	: (optional) String format : "a/b/c/.../j" a..j are mptcp options to remove ( should be 0<=n<=15)
     */
    int configure(Vector<String> &conf, ErrorHandler *errh);
    //Map _map;
private:
    bool shouldBeInspected(Packet*p_in, WritablePacket **p_out);
    /**
     * Check if the option located at nextOpt must be removed. If it must be removed then the whole option is changed by NOP TCP option.
     * If an option has a invalid length, return max without changing anything
     * return the address of the next Option or max if nextOption points to END tcpOption.
     *
     * max point to the true end of the header. We used this pointer ensure that a bad length in an option can be detected.
     */
    uint8_t *  rmTCPOpt(uint8_t *nextOpt, uint8_t *max);
    /**
     * Check the string and return the number of opt, or -1 if the string is not well formed.
     * A well formed string follow the pattern : "a/b/c/.../j"  where a ..j are numbers.
     * In the case of TCP options, the numbers should be 0 <= tcpOpt <= 255
     * In the case of MPTCP options, the numbers should be 0 <= mptcpOpt <=15.
     *
     * rem : number range is NOT checked. User should know use correct option numbers...
     */
    int checkOptTable(String &s);
    /**
     * Create the option table, based on string s.
     * Pre : s should be well formed (see above), and count should be the number of options present in s
     */
    int initOptTable(uint8_t **table, String &s, int count);


    /**
     * return true if the given option must be removed else return false.
     * A TCP option must be removed if it's present in tcpOptions table
     * A MPTCP option must be removed if the tcp option is 30 (0x1c) AND the subtype is present in mptcpOptions
     */
    bool mustBeRemoved(uint8_t *opt);

    /**
     * return option located at start in s. At the end, start points to the next option or end if there is no more option.
     */
    uint8_t getOptionAt(String &s, int *start);

    /**
     * Print a table of options. Debugging purposes.
     */
    void printOptTable(uint8_t *table, int len);

    // Administrative stuff
    bool _killNotInspected;
    bool _verbose;

    // Options to be removed
    uint8_t *_tcpOptions; 	// TCP options to be removed table
    int _tcpOptionsC; 		// Count of TCP options
    uint8_t *_mptcpOptions;	// mptcp (30) sub options to be removed
    int _mptcpOptionsC;		// Count of MPTCP options
    bool _rmSyn;			// do we rm options from syn segment ?
    bool _rmData;			// do we rm options from data segment ?
    int _delayRM; 			// How many packet do we wait before actually removing specified opt ?
    int _ttl;				// How long can an idle connection state live ?
};

CLICK_ENDDECLS


#endif /* PRINTTCPOPTIONELEMENT_HH_ */
