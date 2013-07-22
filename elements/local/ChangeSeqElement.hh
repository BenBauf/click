/*
 * ChangeSeq.hh
 *
 *  Created on: Apr 3, 2013
 *      Author: Benjamin Hesmans
 */

#ifndef CLICK_CHANGESEQELEMENT_HH_
#define CLICK_CHANGESEQELEMENT_HH_

#include <click/element.hh>

CLICK_DECLS

class ChangeSeqElement : public Element {
public:
    ~ChangeSeqElement();
    ChangeSeqElement();
    /**
     * cf click documentation
     */
    const char *class_name() const { return "ChangeSeq"; }
    const char *port_count() const { return PORTS_1_1; }
    const char *processing() const { return PUSH; }
    void push(int port, Packet *p);

    /**
     * cf click documentation and :
     * The element's options are :
     */
    int configure(Vector<String> &conf, ErrorHandler *errh);

private:
    int getDelta(Packet *p_in);
    bool shouldBeInspected(Packet*p_in, WritablePacket **p_out);
    void updateSeq(WritablePacket *wp, int delta);
    bool _verbose;		// Let me blablalba
    int _delta;			// delta : 0 means hash the flowid, else use the delta (may be pos or neg)
    bool _reverse;		// l to r : false ; r to l : true
};

CLICK_ENDDECLS

#endif /* CHANGESEQELEMENT_HH_ */
