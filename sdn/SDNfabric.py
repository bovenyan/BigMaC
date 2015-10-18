from mininet.topo import Topo
from mininet.net import Mininet
from mininet.cli import CLI
from mininet.node import RemoteController


# parsing the input file
def parseTopoFile(filename):
    links = set()
    nodeNo = 0

    obj = open(filename, 'r')
    lines = obj.readlines()

    for line in lines:
        line = line[:-1]

        print line
        if 'node' in line:
            temp = line.split(':')
            nodeNo = int(temp[1])

        if '-' in line:
            pairStr = line.split('-')
            pair = (int(pairStr[0])-1, int(pairStr[1])-1)
            pairRev = (int(pairStr[1])-1, int(pairStr[0])-1)

            # remove duplicate links
            if not (pairRev in links):
                links.add(pair)
            else:
                print "duplicate link: " + str(pair)

    obj.close()

    return (nodeNo, links)


class fabric(Topo):
    def __init__(self, topoFile):
        Topo.__init__(self)

        nodeNo, self.linkset = parseTopoFile(topoFile)

        hosts = [None] * nodeNo
        switches = [None] * nodeNo

        # add hosts and switches
        for idx in range(nodeNo):
            hosts[idx] = self.addHost('h'+str(idx+1))
            switches[idx] = self.addSwitch('s'+str(idx+1))

            self.addLink(hosts[idx], switches[idx])  # connect a host to each

        # add links
        print "nNode " + str(nodeNo)
        for link in self.linkset:
            if (link[0] < nodeNo and link[1] < nodeNo):
                print "link" + str(link)

                self.addLink(switches[link[0]], switches[link[1]])
            else:
                print "invalid link: " + str(link)

        # record switch info
        # dpid, name, {next-hop: physical port}

        self.switchInfo = [None] * nodeNo

        for idx in range(nodeNo):
            self.switchInfo[idx] = [idx+1, switches[idx], {}]

        for link in self.linkset:
            print "link: " + str((switches[link[0]], switches[link[1]]))
            sPort, dPort = self.port(switches[link[0]], switches[link[1]])
            print "ports: " + str((sPort, dPort))
            self.switchInfo[link[0]][2][link[1]+1] = sPort
            self.switchInfo[link[1]][2][link[0]+1] = dPort

        print self.switchInfo


def runTest():
    # generate topology
    topo = fabric(topoFile="../topology/test.topo")

    net = Mininet(topo=topo, controller=RemoteController("ryu"))
    # net = Mininet(topo=topo, controller=OVSController)
    net.start()

    CLI(net)

    net.stop()

if __name__ == "__main__":
    runTest()
