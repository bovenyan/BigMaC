
"""
Query Format
    ----
    type (4) | len(4) | query (0-36)
    ----
    type = 0:
            PacketIn Query Format
            ----        ----       ----        ----       ----
            dpid(4) | header(4) | header(4) | header(4) | header(4)
            ----        ----       ----        ----       ----

    type = 1: Route Query Format
            ----
            NULL(0)
            ----

Answer Format
     ----       ----
    type (4) | len(4) | answers (0-1000)
     ----       ----
    type = 0:
            Rule Format:
            dpid | table | rule_0.field[0].val |rule_0.field[0].mask |
                       ----       ----       ----        ----
    Higher bits ...   dpid(s) | ruletype | fieldLen | fieldType
                       ----       ----       ----        ----
            ----            ----         ----        ----
            routeID(4) |   dpid(4)  |  dpid(4)  |   dpid(4)  |  ... | CNTL
            ----            ----         ----        ----
"""


import socket
import struct
import logging
logger = logging.getLogger("tcp")

class pkt_h:
    def __init__(self,ip_src=0, ip_dst = 0, port_src = 0, port_dst = 0):
        self.ip_src = ip_src
        self.ip_dst = ip_dst
        self.port_src = port_src
        self.port_dst = port_dst

class bktOrR(object):
    def __init__(self, ip_src = 0, ip_src_mask = 0, ip_dst = 0, ip_dst_mask = 0, port_src = 0,
                 port_src_mask = 0, port_dst = 0, port_dst_mask = 0, priority = 0):
        self.ip_src = ip_src
        self.ip_src_mask = ip_src_mask
        self.ip_dst = ip_dst
        self.ip_dst_mask = ip_dst_mask
        self.port_src = port_src
        self.port_src_mask = port_src_mask
        self.port_dst = port_dst
        self.port_dst_mask = port_dst_mask
        self.priority = priority

    def __str__(self):
        return "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t" % (self.ip_src, self.ip_src_mask, self.ip_dst, self.ip_dst_mask, self.port_src,
                                                         self.port_src_mask, self.port_dst, self.port_dst_mask, self.priority)

def ipv4_to_str(integre):
    ip_list = [str((integre >> (24 - (n * 8)) & 255)) for n in range(4)]
    return '.'.join(ip_list)

def ipv4_to_int(string):
    ip = string.split('.')
    assert len(ip) == 4
    i = 0
    for b in ip:
        b = int(b)
        i = (i << 8) | b
    return i

def eth_to_str(integer):
    eth_list = [hex(integer >> (44 - (n * 8)) & 15)[2:] + hex(integer >> (40 - (n * 8)) & 15)[2:] for n in range(6)]
    return ':'.join(eth_list)

def eth_mask_to_str(integer):
    eth_list = [hex(integer >> (44 - (n * 8)) & 15)[2:] + hex(integer >> (40 - (n * 8)) & 15)[2:] for n in range(6)]
    mask_temp = ':'.join(eth_list)
    return 'ff:ff:'+mask_temp[6:]

def eth_to_int(string):
    eth = string.split(':')
    assert len(eth) == 6
    i = 0
    for b in eth:
        b = int(b,16)
        i = (i << 8) | b
    return i


class ryuClient:
    def __init__(self, configFile="./config"):
        config = open(configFile)
        lines = config.readlines()

        self.server_ip = "127.0.0.1"
        self.server_port = 9000
        self.header_size = 4
        self.skt = None

        for line in lines:
            line = line[:-1]
            temp = line.split('\t')
            if (temp[0] == "server IP"):
                self.server_ip = str(temp[1])
            if (temp[0] == "server Port"):
                self.server_port = int(temp[1])

    def create_connection(self):
        if self.skt is None:
            try:
                self.skt = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.skt.connect((self.server_ip, self.server_port))
                logger.info("server connected : %s %s",
                            self.server_ip,
                            self.server_port)
            except socket.error as e:
                logger.debug("error connected %s,%s", e.errno, e.message)
                self.handle_error()

    def handle_error(self):
        if not (self.skt is None):
            try:
                self.skt.close()
            finally:
                self.skt = None

    def query(self, dpid, header):  # forward packetIn to BigMac
        if self.skt is None:
            self.create_connection()
        if self.skt is None:
            return None

        request = [dpid] + header

        reqLen = 4*len(request)
        reqFormat = '!I' + 'I'*len(request)

        message = struct.pack(reqFormat, reqLen, *request)

        try:
            # send query
            self.skt.send(message)

            # recv len
            body_len_raw = self.skt.recv(self.header_size)
            (body_len, ) = struct.unpack('!I', body_len_raw)
            # recv body
            body_raw = self.skt.recv(body_len)

        except socket.error, (value, message):
            logger.error("TCP ERROR:\t%s %s", value, message)
            logger.info("TCP INFO:\ttry to re-connect " +
                        self.server_ip + " : " + str(self.server_port))
            self.handle_error()
            return None

        except struct.error:
            self.handle_error()
            return None

        rules_num = body_len/36
        rules = []

        for i in range(rules_num):
            rules.append(bktOrR())
            (rules[i].ip_src, rules[i].ip_src_mask, rules[i].ip_dst,
             rules[i].ip_dst_mask, rules[i].port_src, rules[i].port_src_mask,
             rules[i].port_dst, rules[i].port_dst_mask, rules[i].priority) = \
                struct.unpack('!IIIIIIIII', body_raw[i*36:(i*36+36)])

        return rules


if __name__ == "__main__":
    """debugging
    """
    src = ipv4_to_int('10.0.0.1')
    dst = ipv4_to_int('10.0.0.2')
    src_str = ipv4_to_str(src)
    dst_str = ipv4_to_str(dst)
    print "int : %s %s" % (src, dst)
    print "str : %s %s" % (src_str, dst_str)
    request = pkt_h(src, dst, 4000, 8000)

    cab = cab_client()
    cab.create_connection()
    rules = cab.query(request)

    for i in rules:
        print i
