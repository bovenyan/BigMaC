/**
 * Author: Nanxi Kang (nkang@cs.princeton.edu) 
 * All rights reserved.
 */
import java.io.*;
import java.util.*;

public class CiscoDrive {

	// read literal to port number mappings
	private static Map<String, Integer> readPortMap(String filename) throws IOException {
		BufferedReader fin = new BufferedReader(new FileReader(filename));
		Map<String, Integer> port_map = new HashMap<String, Integer>();
		
		String line, key, value;		
		StringTokenizer tokens;
		while (true) {
			line = fin.readLine();
			if (line == null)
				break;
			tokens = new StringTokenizer(line, ",");
			key = tokens.nextToken();
			value = tokens.nextToken();
			while (tokens.hasMoreElements())
				value = tokens.nextToken();
			port_map.put(key, Integer.parseInt(value));
		}
		
		fin.close();
		
		return port_map;
	}
	
	public static void main(String[] argv) {
		try {
			Map<String, Integer> port_map = readPortMap(argv[2]);
			
			BufferedReader fin = new BufferedReader(new FileReader(argv[0]));		
			PrintWriter fout = new PrintWriter(new FileWriter(argv[1]));
		
			Vector<ACLEntry> acl = new Vector<ACLEntry>();
		
			
			String line;
			ACLEntry entry;
			while (true) {
				line = fin.readLine();
				if (line == null)
					break;
				if (line.equals(""))
					continue;
				
				entry = new ACLEntry();
				if (!entry.load(line, port_map) || !entry.parse()) {
					fout.println("Invalid entry " + line);
					return;
				}
				if (!entry.isActive)
					continue;
				
				if (entry.line_number >= 0)
					acl.add(entry.line_number - 1, entry);
				else 
					acl.add(entry);
			}
			
			
			Enumeration<ACLEntry> e = acl.elements();
			int act;
			
			fout.println(acl.size() + " 2");
			while (e.hasMoreElements()) {
				entry = e.nextElement();
				if (entry.isPermit)
					act = 1;
				else act = 0;
				fout.println(entry.srcIPNum + " " + entry.dstIPNum + " " + entry.srcPort.start + " " + entry.srcPort.end +
						" " + entry.dstPort.start + " " + entry.dstPort.end + " " + (entry.srcPortCnt * entry.dstPortCnt) + 
						" " + act);
			}
			
			fin.close();
			fout.close();
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
	}
}
