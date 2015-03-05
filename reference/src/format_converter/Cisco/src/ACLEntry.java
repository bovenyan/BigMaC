/**
 * Author: Nanxi Kang (nkang@cs.princeton.edu) 
 * All rights reserved.
 */
import java.io.*;
import java.util.*;
import java.util.StringTokenizer;

public class ACLEntry {
		
	String acl_name;
	int line_number;
	boolean isExtended;
	boolean isPermit;
	String protocol;
	String srcIP;
	String dstIP;
	PortEntry srcPort;
	PortEntry dstPort;
	boolean isActive;
	
	long srcIPNum;
	long dstIPNum;
	int srcPortCnt;
	int dstPortCnt;
	
	public boolean load(String str, Map<String, Integer> port_map) {
		StringTokenizer tokens = new StringTokenizer(str);
		String now;
		
		now = tokens.nextToken();
		if (!now.equalsIgnoreCase("access-list"))
			return false;
		this.acl_name = tokens.nextToken();
		
		now = tokens.nextToken();
		if (now.equalsIgnoreCase("line")) {
			line_number = Integer.parseInt(tokens.nextToken());
			now = tokens.nextToken();
		}
		else 
			line_number = -1;
		
		if (now.equalsIgnoreCase("extended")) {
			isExtended = true;
			now = tokens.nextToken();
		}
		else 
			isExtended = false;
		
		if (now.equalsIgnoreCase("permit")) {
			isPermit = true;
			now = tokens.nextToken();
		}
		else if (now.equalsIgnoreCase("deny")) {
			isPermit = false;
			now = tokens.nextToken();
		}
		else return false;
		
		protocol = now;
		now = tokens.nextToken();
		
		srcIP = now;
		srcPort = new PortEntry();
		now = tokens.nextToken();
		if (srcPort.load(tokens, now, port_map)) 
			now = tokens.nextToken();
		
		dstIP = now;
		dstPort = new PortEntry();
		
		if (tokens.hasMoreElements()) {			
			now = tokens.nextToken();
			if (dstPort.load(tokens, now, port_map)) {
				if (!tokens.hasMoreElements()) { 
					isActive = true;			
					return true;
				}
				else 
						now = tokens.nextToken();
			}
			if (now.equalsIgnoreCase("inactive"))
				isActive = false;
			else
				return false;
		}
		else
			isActive = true;
		
		
		return true;
	}
	
	
	private static int PARTS = 4;
	private static int MI = 256;
	
	private long parseIP(String ip) {
		if (ip.equalsIgnoreCase("any"))
			return 0;
		
		StringTokenizer tokens;
		tokens = new StringTokenizer(ip, "./");
		
		long tmp = 0;
		for (int i = 0; i < PARTS; ++i)
			if (tokens.hasMoreElements())
				tmp = tmp * MI + Integer.parseInt(tokens.nextToken());
			
		int mask;
		if (tokens.hasMoreElements())
			mask = Integer.parseInt(tokens.nextToken());
		else 
			mask = 32;
		
		long value = 1;
		for (int i = 0; i < mask; ++i)
			value = value * 2;
		return value - 1 + tmp;
	}
	
	public boolean parse() {
		this.srcIPNum = parseIP(this.srcIP);
		this.dstIPNum = parseIP(this.dstIP);
		this.srcPortCnt = this.srcPort.count();
		this.dstPortCnt = this.dstPort.count();
		
		return true;
	}
	
	public void show() {
		System.out.println(">>>>>>>");
		System.out.println(this.srcIP);
		System.out.println(this.dstIP);
		System.out.println("<<<<<<<");
	}
}



