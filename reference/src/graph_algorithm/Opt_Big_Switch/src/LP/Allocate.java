/**
 * Author: Nanxi Kang (nkang@cs.princeton.edu) 
 * All rights reserved.
 */
package LP;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;

public class Allocate {
	
	public static double eps = 1e-6;
	
	//input: TopoLPlogy, Capacity T, PolicyNum
	//output: Allocation
	public boolean work(String topoFile, String policyNumFile, String allocFile) {
		TopoLP TopoLP;
		try {
			TopoLP = load(topoFile);
			readPolicyNum(policyNumFile, TopoLP);
				
			LP model = new LP(TopoLP);
			boolean success = model.solveLP();
			if (success) {
				writeAllocation(allocFile, TopoLP, model);
				return true;
			}
			else {
				File file = new File(allocFile);
				file.delete();
				return false;
			}
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return false;
		}
			
	}
	
	private TopoLP load(String filename) throws IOException {
		
		TopoLP TopoLP = new TopoLP();
		TopoLP.load(filename);
		return TopoLP;
	}
	
	private void readPolicyNum(String filename, TopoLP TopoLP) throws Exception {
		TopoLP.readPolicyNum(filename);
	}
	
	private void writeAllocation(String filename, TopoLP TopoLP, LP model) throws IOException {
		PrintWriter fout = new PrintWriter(new FileWriter(filename));
		
		fout.println(TopoLP.n_path);
		for (int j = 0; j < TopoLP.n_path; ++j) {
			
			fout.print(TopoLP.H_p[j].length);
			fout.print(' ');
			for (int k = 0; k < TopoLP.H_p[j].length; ++k) {
				int i = TopoLP.H_p[j][k];
				int idx = TopoLP.H_idx[j][k];
				fout.print((int)(model.ans[i][idx] + eps));
				fout.print(' ');
			}
			fout.println();
		}
		
		fout.close();
	}
}
