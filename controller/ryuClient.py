
"""
Query Format
    ----       ----     ----
    type (4) | len(4) | query (0-36)
    ----       ----     ----
    type = 0: (PacketIn)
            'query' Format
            ----        ----       ----        ----       ----
            dpid(4) | header(4) | header(4) | header(4) | header(4)
            ----        ----       ----        ----       ----

    type = 1: (Route)
            'query' Format
            ----
            NULL(0)
            ----

Answer Format
     ----       ----     ----
    type (4) | len(4) | answers (4-1000)
     ----       ----     ----
    type = 0:
            'answers' format:
            ----            ----          ----            ----
           tableID  |  dpid/routeID (4) | field.val(4) |  field.mask(4) | next..
            ----             ----          ----            ----

    type = 1:
            'answers' format:
               ----       ----      ----
            routeID(4) | dpid(4) | dpid(4)
               ----       ----      ----
    type = 2:
            'answers' format:
            None;
"""

import socket
from struct import pack, unpack
import struct.error as structErr
import socket.error as sockErr
# import logging
# logger = logging.getLogger("tcp")


class query:
    def __init__(self, typeID, dpid, headers):
        self.typeID = typeID
        self.dpid = dpid
        self.headers = headers

    def toMsg(self):
        msg = None

        if self.typeID == 0:
            formatStr = '!II' + 'I' * len(self.headers)
            msg = pack(formatStr, self.typeID, len(self.headers)*4,
                       self.dpid, *self.headers)

        elif self.typeID == 1:
            formatStr = '!II'
            msg = pack(self.formatStr, self.typeID, 0)

        else:
            pass

        return msg


class answer:
    def __init__(self, rawInput, fieldNo):
        self.typeID = None
        self.rawInput = rawInput
        self.fieldNo = 2 * fieldNo

    def parseMsg(self):
        result = None

        typeID, length = unpack("!II", self.rawInput[0:4*2])

        ruleLen = 8 + self.fieldNo * 8
        ruleFormat = "!II" + 'I' * (self.fieldNo * 2)

        if typeID == 0:
            ruleNo = (length - 8) / ruleLen
            assert (length - 8) % ruleLen == 0

            result = [typeID]

            for idx in range(ruleNo):
                rule = unpack(ruleFormat,
                              self.rawInput[(8 + idx * ruleLen):
                                            (8 + (idx + 1) * ruleLen)])
                result.append(rule)

        elif typeID == 1:
            dpidNo = (length - 8) / 4

            assert (length - 8) % 4 == 0
            result = [unpack('!I', self.rawInput[8:12])]

            route = unpack('!'+'I'*dpidNo,
                           self.rawInput[12: 12+4*dpidNo])

            result.append(route)

        elif typeID == 2:
            result = [None]

        else:
            pass

        return result


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
    eth_list = [hex(integer >> (44 - (n * 8)) & 15)[2:] +
                hex(integer >> (40 - (n * 8)) & 15)[2:] for n in range(6)]
    return ':'.join(eth_list)


def eth_mask_to_str(integer):
    eth_list = [hex(integer >> (44 - (n * 8)) & 15)[2:] +
                hex(integer >> (40 - (n * 8)) & 15)[2:] for n in range(6)]
    mask_temp = ':'.join(eth_list)
    return 'ff:ff:'+mask_temp[6:]


def eth_to_int(string):
    eth = string.split(':')
    assert len(eth) == 6
    i = 0
    for b in eth:
        b = int(b, 16)
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
                # logger.info("server connected : %s %s",
                #            self.server_ip,
                #            self.server_port)
            except sockErr:
                # logger.debug("error connected %s,%s", e.errno, e.message)
                self.handle_error()

    def handle_error(self):
        if not (self.skt is None):
            try:
                self.skt.close()
            finally:
                self.skt = None

    def packetIn(self, dpid, headers):  # forward packetIn to BigMac
        if self.skt is None:
            self.create_connection()
        if self.skt is None:
            return None

        queryObj = query(0, dpid, headers)
        message = queryObj.toMsg()
        rules = None

        try:
            self.skt.send(message)

            bodyraw = self.skt.recv()
            answerObj = answer(bodyraw, self.header_size)
            rules = answerObj.parseMsg()

        except socket.error, (value, message):
            # logger.error("TCP ERROR:\t%s %s", value, message)
            # logger.info("TCP INFO:\ttry to re-connect " +
            #            self.server_ip + " : " + str(self.server_port))
            self.handle_error()
            return None

        except structErr:
            self.handle_error()
            return None

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
    request = query(0, 1, [src, dst, 4000, 8000])

    cab = ryuClient()
    cab.create_connection()
    rules = cab.query(request)

    for i in rules:
        print i
