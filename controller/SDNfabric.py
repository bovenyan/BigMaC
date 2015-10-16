from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import OVSController

# parsing the input file
def parseTopoFile(filename):
    links = set()
    nodeNo = 0

    obj = open(filename, 'r')
    lines = obj.readlines()

    for line in lines:
        line = line[:-1]

        if 'node' in line:
            line.split(':')
            nodeNo = int(nodeNo)

        if '-' in lines:
            pairStr = line.split('-')
            pair = (pairStr[0], pairStr[1])
            pairRev = (pairStr[1], pairStr[0])

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

        nodeNo, links = parseTopoFile(topoFile)

        hosts = [None] * nodeNo
        switches = [None] * nodeNo

        # add hosts and switches
        for idx in range(nodeNo):
            print idx
            hosts[idx] = self.addHost('h'+str(idx))
            switches[idx] = self.addSwitch('s'+str(idx))

            self.addLink(hosts[idx], switches[idx])  # connect a host to each

        # add links
        for link in links:
            if (link[0] < nodeNo and link[1] < nodeNo):
                self.addLink(hosts[idx], switches[idx])
            else:
                print "invalid link: " + str(link)

""" Trigger test
"""
def runTest():
    # generate topology
    topo = fabric(topoFile="../topology/test.topo")

    #net = Mininet(topo=topo, controller=OVSController)
    #net.start()

    #print topo.nodeInfo('s1')

    #net.stop()


if __name__ == "__main__":
    runTest()
