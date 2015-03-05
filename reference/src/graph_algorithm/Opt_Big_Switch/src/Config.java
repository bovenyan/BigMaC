/**
 * Author: Nanxi Kang (nkang@cs.princeton.edu) 
 * All rights reserved.
 */
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.util.StringTokenizer;


public class Config {
	
	public static final String ALLOC_FILE_T = "%salloc%d.txt"; 
	public static final String ALLOC_FILE_P_T = "%salloc_p%d.txt";
	public static final String POLICY_NUM_FILE_T = "%srule_num%d.txt";
	public static final String RATIO_FILE_T = "%sratio%d.txt";
	public static final String RES_FILE_T = "%sresult%d.txt"; 
	public static final String RES_FILE_P_T = "%sresult%d_%d.txt";
	
	public static final int MAX_HOP = 20;
	
	private static String PATH_POLICY_NUM_FILE_T = "%spath_num.txt";

	private String dir;
	public String pathEXEFile = "..//path";
	public String pathPolicyFile;
	public String pathPolicyNumFile;
	public String topoFile;
	
	public double[] eta;
	public int[] hops;
	
	public int start_itr = 0;
	public int end_itr = 100;
	public int processNum = 1;
	
	public double eta_base = 1.1;
	public int hop_base = 5;
	
		
	public void load(String[] args) throws Exception {
	   dir = args[0];
	   pathPolicyFile = args[1];
	   topoFile = args[2];
	   pathPolicyNumFile = String.format(PATH_POLICY_NUM_FILE_T, dir);
	   createPathPolicyNumFile(pathPolicyFile, pathPolicyNumFile);
	   
	   int idx;
	   idx = Utils.locate(args, "-f");
	   if (idx >= 0 && idx + 1 < args.length) {
		   pathEXEFile = args[idx + 1];
	   }
	   
	   idx = Utils.locate(args, "-p");
	   if (idx >= 0 && idx + 1 < args.length) {
		   processNum = Integer.parseInt(args[idx + 1]);
	   }
	   
	   idx = Utils.locate(args, "-i");
	   if (idx >= 0 && idx + 2 < args.length) {
		   start_itr = Integer.parseInt(args[idx + 1]);
		   end_itr = Integer.parseInt(args[idx + 2]);
	   }
	   
	   idx = Utils.locate(args, "-e");
	   if (idx >= 0 && idx + 2 < args.length) {
		   eta_base = Double.parseDouble(args[idx + 1]);
		   hop_base = Integer.parseInt(args[idx + 2]);		   
	   }
	   computeEta();
	}
	
    private void computeEta() {
    	hops = new int[MAX_HOP / hop_base + 2];
    	eta = new double[MAX_HOP + 1];
    	
    	for (int i = 0; i <= MAX_HOP / hop_base + 1; ++i) 
    		hops[i] = i * hop_base + 1;
    	eta[0] = 1.0;
    	for (int i = 1; i <= MAX_HOP; ++i)
    		eta[i] = eta[i - 1] * eta_base;
    }
	
	private void createPathPolicyNumFile(String policyFile, String policyNumFile) throws Exception{
		BufferedReader fin = new BufferedReader(new FileReader(policyFile));
		PrintWriter fout = new PrintWriter(new FileWriter(policyNumFile));
		
		int n_policy = Integer.parseInt(fin.readLine());
		fout.println(n_policy);
		// the position of count in the rule 
		int CNT_IDX = 6;
		
		for (int i = 0; i < n_policy; ++i) {
			fin.readLine();
			int n_rule = Integer.parseInt(fin.readLine());
			//count the rules (take the projection into consideration)
			int total_rules = 0;
			for (int j = 0; j < n_rule; ++j) {
				String str = fin.readLine();
				StringTokenizer tokens = new StringTokenizer(str);
				for (int k = 0; k < CNT_IDX; ++k)
					tokens.nextToken();
				int cnt = Integer.parseInt(tokens.nextToken());
				total_rules = total_rules + cnt;
			}
			fout.println(total_rules);
		}
		fin.close();
		fout.close();
	}
	
	public String ratioFile(int k) {
		return String.format(RATIO_FILE_T, dir, k);
	}
	
	public String policyNumFile(int k) {
		return String.format(POLICY_NUM_FILE_T, dir, k);
	}
	
	public String allocFile(int k) {
		return String.format(ALLOC_FILE_T, dir, k);
	}
	
	public String allocFile(int k, int p) {
		return String.format(ALLOC_FILE_P_T, dir, p);
	}
	
	public String resFile(int k) {
		return String.format(RES_FILE_T, dir, k);
	}
	
	public String resFile(int k, int p) {
		return String.format(RES_FILE_P_T, dir, k, p);
	}
	
	
}
