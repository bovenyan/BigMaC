/**
 * Author: Nanxi Kang (nkang@cs.princeton.edu) 
 * All rights reserved.
 */
package LP;

import gurobi.GRB;
import gurobi.GRBEnv;
import gurobi.GRBException;
import gurobi.GRBLinExpr;
import gurobi.GRBModel;
import gurobi.GRBVar;

public class LP {
	
	// ans
	double[] ans[];

	private TopoLP TopoLP;
	
	//portions
	GRBVar[] R[];

	
	public LP(TopoLP TopoLP) {
		this.TopoLP = TopoLP;
		R = new GRBVar[TopoLP.n_switch][];
		ans = new double[TopoLP.n_switch][];
		for (int i = 0; i < TopoLP.n_switch; ++i) {
			R[i] = new GRBVar[TopoLP.H_s[i].length];
			ans[i] = new double[TopoLP.H_s[i].length];
		}
	}
	

	boolean solveLP(){
		
	    try {
	        GRBEnv env = new GRBEnv();
	        GRBModel model = new GRBModel(env);        
	        
	        for (int i = 0; i< TopoLP.n_switch; ++i)
	            for (int k = 0; k < R[i].length; ++k)
	            	R[i][k] = model.addVar(0.0, 1.0, 0.0, GRB.CONTINUOUS,"");
	                   
	        model.update();
	        
	        GRBLinExpr obj = new GRBLinExpr();
	        for (int i = 0; i < TopoLP.n_switch; ++i) {
	        	for (int j = 0; j < TopoLP.H_s[i].length; ++j) {
	        		int p = TopoLP.H_s[i][j];
	        		int l = TopoLP.H_p[p].length;
	        		for (int k = 0; k < TopoLP.H_p[p].length; ++k) { 
	        			if (TopoLP.H_p[p][k] == i) {
	        				obj.addTerm(1.0 * (l - k) / l, R[i][j]);
	        			}
	        				/*if (k == 0)
	        					obj.addTerm(1.0, R[i][j]);
	        				else if (k == 1)
	        					obj.addTerm(0.5, R[i][j]);*/
	        		}
	        	}
	        }
	            //if (R[i].length >= 1) {
	            	//obj.addTerm(1.0, R[i][0]);
	        model.setObjective(obj, GRB.MAXIMIZE);
            
	        	        
	        //SWITCH CAPCITY
	        GRBLinExpr switchRes[] = new GRBLinExpr[TopoLP.n_switch];
	        for (int i = 0; i < TopoLP.n_switch; ++i) {
	            switchRes[i] = new GRBLinExpr();
	            for (int k = 0; k < R[i].length; ++k) 
	            	switchRes[i].addTerm(1.0, R[i][k]);
	            
	            //model.addConstr(switchRes[i], GRB.LESS_EQUAL, 1.0, "");
	            model.addConstr(switchRes[i], GRB.EQUAL, 1.0, "");
	        }
	        
	        //PATH REQUIREMENT
	        GRBLinExpr pathRes[] = new GRBLinExpr[TopoLP.n_path];
	        for (int j = 0; j < TopoLP.n_path; ++j) {
	        	pathRes[j] = new GRBLinExpr();
	            for (int k = 0; k < TopoLP.H_p[j].length; ++k) {
	            	int i = TopoLP.H_p[j][k];
	            	int idx = TopoLP.H_idx[j][k];
	                pathRes[j].addTerm(TopoLP.C[i], R[i][idx]);
	            }
	            
	            model.addConstr(pathRes[j], GRB.GREATER_EQUAL, TopoLP.P[j], "");
	        }
	        
	        
	        model.optimize();
	        
	        int status = model.get(GRB.IntAttr.Status);
	        if (status == GRB.OPTIMAL) {
	        	System.out.println("OPTIMAL");
	        	
	        	for (int i = 0; i < TopoLP.n_switch; ++i) 
	                for (int k = 0; k < R[i].length; ++k)
	                    ans[i][k] = R[i][k].get(GRB.DoubleAttr.X) * TopoLP.C[i];
	                    
	        	
	        	model.dispose();
	    		env.dispose();
	    	
	            return true;
	        }
	        else if (status == GRB.INFEASIBLE)  {
	            System.out.println("INFEASIBLE");
	            model.dispose();
	    		env.dispose();
	            return false;
	        }
	        else if (status == GRB.UNBOUNDED) {
	            System.out.println("UN_BOUNDED");
	            model.dispose();
	    		env.dispose();
	            return true;
	        }
	        else {
	        	model.dispose();
	    		env.dispose();
	    		
	            System.out.println("LP Stopped with status = " + status);
	        }
	    }
	    catch (GRBException e) {
	        System.out.println("Error code = " + e.getErrorCode());
	        System.out.println(e.getMessage());
	    }
	    catch (Exception e) {
	    	System.out.println("Exception during optimization: " + e.getMessage());
	    }
	    return false;
	}

	public double[][] getAns() {
        return ans;

	}
	
	public void dispose() {
		
	}
	
}