/**
 * Author: Nanxi Kang (nkang@cs.princeton.edu) 
 * All rights reserved.
 */
import java.util.*;

public class PortEntry {
		
		public Map<String, Integer> port_map = new HashMap<String, Integer>();
		
		public static final int MIN_PORT = 0;
		public static final int MAX_PORT = 65535;
		
		PortOp op;
		int v1, v2;
		int start, end;
		
		public PortEntry() {
			op = PortOp.All;
		}
		
		private int parsePort(String str, Map<String, Integer> port_map) {
			Integer v = port_map.get(str);			
			if (v == null) {
				try {
					return Integer.parseInt(str);
				}
				catch (NumberFormatException e) {
					return 514;
				}
			}
			else 
				return v;
		}
		
		private int countRange(int start, int end) {
			this.start = start;
			this.end = end;
			if (start > end)
				return 0;
			
			int l, now = start, next, c = 0;
			while (now <= end) {
				++c;
				int k = -1;
				for (int i = 0; ; ++i) {
					l = (1<<i) - 1;
					next = (now | l);
					if (next <= end && next - l == now) {
						k = next;
					}
					else {
						now = k + 1;
						break;
					}
				}
			}
			
			return c;
		}
		
		public int count() {
			if (op == PortOp.Eq) {
				 start = end = v1;
				 return 1;
			}
			else if (op == PortOp.All) {
				start = MIN_PORT;
				end = MAX_PORT;
				return 1;
			}
			else if (op == PortOp.Lt) 
				return countRange(MIN_PORT, v1 - 1);
			else if (op == PortOp.Gt)
				return countRange(v1 + 1, MAX_PORT);
			else if (op == PortOp.Neq)
				return countRange(MIN_PORT, v1 - 1) + countRange(v1 + 1, MAX_PORT);
			else if (op == PortOp.Range)
				return countRange(v1, v2);
			return 0;
		}
		
		public boolean load(StringTokenizer token, String op_str, Map<String, Integer> port_map) {
			if (op_str.equalsIgnoreCase("lt")) {
				op = PortOp.Lt;
				v1 = parsePort(token.nextToken(), port_map);
				return true;
			}
			else if (op_str.equalsIgnoreCase("gt")) {
				op = PortOp.Gt;
				v1 = parsePort(token.nextToken(), port_map);
				return true;
			}
			else if (op_str.equalsIgnoreCase("Eq")) {
				op = PortOp.Eq;
				v1 = parsePort(token.nextToken(), port_map);
				return true;
			}
			else if (op_str.equalsIgnoreCase("Neq")) {
				op = PortOp.Neq;
				v1 = parsePort(token.nextToken(), port_map);
				return true;
			}
			else if (op_str.equalsIgnoreCase("Range")) {
				op = PortOp.Range;
				v1 = parsePort(token.nextToken(), port_map);
				v2 = parsePort(token.nextToken(), port_map);
				return true;
			}
			
			return false;
		}		
	}