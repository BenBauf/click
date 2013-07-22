/*
 * SegSplitElement.hh
 *
 *  Created on: Mar 28, 2013
 *      Author: Benjamin Hesmans
 *
 *       Element to split TCP segment, the options are:
 *       	- SPLITAT int i
 *       		i < 0 : No Split
 *       		i == 0 : Split in 2, (if tcpLen > 1 )
 *       		i > 0 split at i if tcpLen > 0
 *       	- KEEPTCPOPT char c
 *       		c == F : tcp options copied only in the first segment
 *       		c == S : tcp options copied only in the second segment
 *       		c == B : tcp options copied on both
 */

#ifndef CLICK_SEGSPLITOPTIONELEMENT_HH_
#define CLICK_SEGSPLITPOPTIONELEMENT_HH_

#include <click/element.hh>

CLICK_DECLS

class SegSplitElement : public Element {
public:
    ~SegSplitElement();
    SegSplitElement();
    /**
     * cf click documentation
     */
    const char *class_name() const { return "SegSplit"; }
    const char *port_count() const { return PORTS_1_1X2; }
    const char *processing() const { return PUSH; }
    void push(int port, Packet *p);

    /**
     * cf click documentation and :
     * The element's options are :
     */
    int configure(Vector<String> &conf, ErrorHandler *errh);

private:
    /*
     * Check if the given packet must be split or not
     * The packet must be split, if it contains a long enough tcp segment.
     * Long enough def : if the parameter given to this element allow this.
     * If it must be split, then at contains the offset at which we need to split the segment
     */
    bool mustBeSplit(Packet* p_in, int* at);
    /**
     * Split a packet that a contains a long enough tcp segment at at into p1_out and p2_out.
     */
    int splitAt(Packet* p_in, int at, WritablePacket** p1_out, WritablePacket** p2_out);
    int getOffset(int len);
    /**
     * Update packet p (a copy of the original).
     */
    void updateFirst(WritablePacket *p, int to, uint32_t ipLen, int tcpLen);
    /**
     * Update packet p (a copy of the original).
     */
    void updateSecond(WritablePacket *p, int from, int ipLen, int tcpLen);
    /**
     * Rm all tcp options form p.
     */
    void removeTCPOptions(WritablePacket *p);

    bool _verbose;		// Let me blablalba
    int _splitAt;		// Options as described above
    bool _tcpOptFirst;	// tcp options in the first segment ?
    bool _tcpOptSecond; // tcp options in the second segment ?
};

CLICK_ENDDECLS


#endif /* SEGSPLITOPTIONELEMENT_HH_ */

