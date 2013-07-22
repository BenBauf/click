/*
 * ChangeWindowElement.hh
 *
 *  Created on: Apr 17, 2013
 *      Author: Benjamin Hesmans
 */

#ifndef CLICK_CHANGEWINDOWELEMENT_HH_
#define CLICK_CHANGEWINDOWELEMENT_HH_

#include <click/element.hh>

CLICK_DECLS

class ChangeWindowElement : public Element {
public:
    ~ChangeWindowElement();
    ChangeWindowElement();
    /**
     * cf click documentation
     */
    const char *class_name() const { return "ChangeWin"; }
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
    void updateWindow(WritablePacket *wp);
    bool _verbose;		// Let me blablalba
    int _delta;			// delta
};

CLICK_ENDDECLS

#endif /* CHANGEWINDOWELEMENT_HH_ */
