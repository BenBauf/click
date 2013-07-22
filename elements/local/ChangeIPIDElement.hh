/*
 * ChangeIPIDElement.hh
 *
 *  Created on: Apr 17, 2013
 *      Author: bhesmans
 */

#ifndef CLICK_CHANGEIPIDELEMENT_HH_
#define CLICK_CHANGEIPIDELEMENT_HH_

#include <click/element.hh>

CLICK_DECLS

class ChangeIPIDElement : public Element {
public:
    ~ChangeIPIDElement();
    ChangeIPIDElement();
    /**
     * cf click documentation
     */
    const char *class_name() const { return "ChangeIPID"; }
    const char *port_count() const { return PORTS_1_1; }
    const char *processing() const { return PUSH; }
    void push(int port, Packet *p);

    /**
     * cf click documentation and :
     * The element's options are :
     */
    int configure(Vector<String> &conf, ErrorHandler *errh);

private:
    //int getDelta(Packet *p_in);
    bool shouldBeInspected(Packet*p_in, WritablePacket **p_out);
    void updateIPID(WritablePacket *wp);
    bool _verbose;		// Let me blablalba
    int _delta;			// delta
};

CLICK_ENDDECLS

#endif /* CHANGEIPIDELEMENT_HH_ */
