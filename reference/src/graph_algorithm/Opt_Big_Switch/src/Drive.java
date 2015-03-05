/**
 * Author: Nanxi Kang (nkang@cs.princeton.edu) 
 * All rights reserved.
 */
import java.io.File;
import LP.Allocate;

public class Drive {
	
	public static String INPUT = "";
	
	public static boolean SAVE_ALLOC = true;
	
	public static double RATIO[][] ={
		{1.0, 1.4, 1.7, 2.0, 2.5, 3.0, 3.5, 4.0},
	};
		
	public static String PATH_POLICY_FILE;
	public static String PATH_POLICY_NUM_FILE;
	public static String TOPOLOGY_FILE;
	public static String PATH_EXE_FILE;
	
	/***Command Line***/
	public static String DFS_COMM_P =  "%s %s %s %s %d %d";
	
	public static void main(String args[]) {
		try {
			Config config = new Config();
			config.load(args);		
			
			Procedure.CUS_RATIO = config.eta;
			Procedure.CUS_HOP = config.hops;

			PATH_POLICY_FILE = config.pathPolicyFile;
			PATH_POLICY_NUM_FILE = config.pathPolicyNumFile;
			TOPOLOGY_FILE = config.topoFile;
			PATH_EXE_FILE = config.pathEXEFile;
			
			ResultRecord r = work(config);
			System.out.println("It takes " + r.Iterations + " iterations to complete.");
			System.out.println("In one iteration, LP takes " + r.timeLP + " seconds, and path algorithm takes " + r.timeSolver + " seconds");
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	//Input: Global Policy, TOPOLOGY
	//Output: xxxxx
	public static ResultRecord work(Config config) {
		ResultRecord ret = new ResultRecord();
		ret.Iterations = -1;
		
		Allocate allocworker = new Allocate();
		try {
			int start = config.start_itr;
			int end = config.end_itr;
			if (start == 0)
				initRatio(config);
			
			int procNum = config.processNum;
			
			// Declare output files
			String now_policyNum, now_alloc, now_res, now_ratio, next_ratio;
			String[] resFiles = new String[procNum];
			String[] allocFiles = new String[procNum];
						
			String[] batchCommLines = new String[procNum];
			Process[] batches = new Process[procNum];
			
			int round, exitVal;
			int status;
			
			for (round = start; round < end; ++round) {
				System.out.println("Round " + round);
				now_policyNum = config.policyNumFile(round);
				now_alloc = config.allocFile(round);
				now_res = config.resFile(round);
				now_ratio = config.ratioFile(round);
				next_ratio = config.ratioFile(round + 1); 
				for (int i = 0; i < procNum; ++i) {
					resFiles[i] = config.resFile(round, i);
					allocFiles[i] =config.allocFile(round, i); 
				}	
				System.out.println(now_ratio);
				//Generate estimated policy_num
				//PATH_POLICY + RATIO -> POLICY_NUM
				System.out.println("Generate Policy Num...");
				Procedure.Control(PATH_POLICY_NUM_FILE, now_ratio, now_policyNum);
				
				//Call LP, provision switch capacities for paths
				//TOPOLOGY + POLICY_NUM_FILE -> ALLOC
				long startProvision = System.currentTimeMillis();
				boolean successLP = allocworker.work(TOPOLOGY_FILE, now_policyNum, now_alloc);
				long endProvision = System.currentTimeMillis();
				ret.timeLP = (endProvision - startProvision) * 1.0 / 1000;
				
				if (successLP) {
					//works out whether capacities can satisfy paths
					//ALLOC +  PATH_POLICY_FILE -> RES
					long startSolver = 0, endSolver = 0;
						
					// Copy Input for multi-process
					for (int i = 0; i < procNum; ++i) {
						batchCommLines[i] = String.format(DFS_COMM_P, PATH_EXE_FILE, allocFiles[i], PATH_POLICY_FILE, resFiles[i], procNum, i);
							
						File src = new File(now_alloc);
						File dst = new File(allocFiles[i]);
						Utils.copyFile(src, dst);
					}
					System.out.println("BATCH...");
					for (int i = 0; i < procNum; ++i) 
						System.out.println(batchCommLines[i]);
	
					startSolver = System.currentTimeMillis();
						
					for (int i = 0; i < procNum; ++i) {
						batches[i] = Runtime.getRuntime().exec(batchCommLines[i]);
					}
						
					exitVal = 0;
					for (int i = 0; i < procNum; ++i) {
						exitVal = exitVal | batches[i].waitFor();
					}
					
					endSolver = System.currentTimeMillis();
										 
					ret.timeSolver = (endSolver - startSolver) * 1.0 / 1000;
					
					System.out.println("Run...DONE");
					
					//check feasibility
					//RES + RATIO -> RATIO'
					System.out.println("Check and Adjust");
					System.out.println(now_ratio);
					System.out.println(next_ratio);
					status = Procedure.CheckAdjustP(resFiles, now_res, now_ratio, next_ratio);
					
				}
				else status = -1;
				
				if (status == 0) {
					System.out.println("Round " + round + " : Try Again");
				}				
				else {
					if (status > 0) {
						ret.Iterations = round;
						System.out.println("Round " + round + " : Success :)");
					}
					else 
						System.out.println("Round " + round + " : Failed...");
					break;
				}
			}
			System.out.println("Done!");
			
		}
		catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		return ret;
	}
		
	//PATH_POLICY -> RATIO
	private static void initRatio(Config config) throws Exception {
		String now_ratio = config.ratioFile(0); 
		System.out.println("Init Ratio...");
		Procedure.InitRatio(PATH_POLICY_NUM_FILE, TOPOLOGY_FILE, now_ratio);
	}
}
