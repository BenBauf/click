/*
 * ChangeMSS.hh
 *
 *	Change tcp option MSS value.
 *
 *  Created on: Apr 26, 2013
 *      Author: Benjamin Hesmans
 */


#ifndef CLICK_CHANGEMSSELEMENT_HH_
#define CLICK_CHANGEMSSWELEMENT_HH_

#include <click/element.hh>

CLICK_DECLS

class ChangeMSSElement : public Element {
public:
    ~ChangeMSSElement();
    ChangeMSSElement();
    /**
     * cf click documentation
     */
    const char *class_name() const { return "ChangeMSS"; }
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
    /**
     * pre : packet is ip/tcp
     *
     * return true if the given packets is a syn tcp packet that contains MSS option, else false.
     * if the returned value is true then at contains the offset (in term of tcp opt) at which MSS is located
     */
    bool containsMSS(Packet* p_in, int *at);
    bool shouldBeInspected(Packet*p_in, WritablePacket **p_out, int *at);
    void updateMSS(WritablePacket *wp, int at);
    bool _verbose;		// Let me blablalba
    int _delta;			// delta
    int _div;
};

CLICK_ENDDECLS

#endif /* CHANGEMSSELEMENT_HH_ */
