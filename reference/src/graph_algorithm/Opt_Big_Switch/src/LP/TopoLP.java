/**
 * Author: Nanxi Kang (nkang@cs.princeton.edu) 
 * All rights reserved.
 */
package LP;

import java.io.*;
import java.util.*;

public class TopoLP {

	public static double eps = 1e-6;
	
	public static int MAX_N = 610;
	public static int MAX_PATH = 60000;
	
	public int n_switch, n_path;
	public int P[];
	public double C[];
	
	public int[] H_p[];
	public int[] H_s[];
	public int[] H_idx[];
	
	//public int F[];
	
	public TopoLP(){
	}
	
	public TopoLP(int n_switch, int n_path) {
		this.n_path = n_path;
		this.n_switch = n_switch;
		
		H_p = new int[n_path][];
		P = new int[n_path];
		C = new double[n_switch];
	}
	
	public void load(String filename) throws IOException {
		BufferedReader in = new BufferedReader(new FileReader(filename));
		StringTokenizer token;
		
		token = new StringTokenizer(in.readLine());
		n_switch = Integer.parseInt(token.nextToken());
		n_path = Integer.parseInt(token.nextToken());
		
		System.out.println(n_switch + " " + n_path);
		
		H_p = new int[n_path][];
		P = new int[n_path];
		C = new double[n_switch];
		
		token = new StringTokenizer(in.readLine());
		for (int i = 0; i < n_switch; ++i) {
		    C[i] = Double.parseDouble(token.nextToken());
		}
		
		 int m, x;
		 for (int j = 0; j < n_path; ++j) {
			 token = new StringTokenizer(in.readLine());
			 m = Integer.parseInt(token.nextToken());
			 H_p[j] = new int[m];
		     for (int i = 0; i < m; ++i) {
		         x = Integer.parseInt(token.nextToken());
		         H_p[j][i] = x;
		         
		       }
		 }
		 
		 in.close();
		 
		 compute_Hs();
	}
	
	public void compute_Hs() {
		H_idx = new int[n_path][];
		H_s = new int[n_switch][];
		
		int len[] = new int[n_switch];
		for (int i = 0; i < n_switch; ++i)
			len[i] = 0;
		for (int j = 0; j < n_path; ++j) {
			H_idx[j] = new int[H_p[j].length];
			for (int i = 0; i < H_p[j].length; ++i) {
				int x = H_p[j][i];
				H_idx[j][i] = len[x]++;
			}
		}
		
		for (int i = 0; i < n_switch; ++i)
			H_s[i] = new int[len[i]]; 
		for (int j = 0; j < n_path; ++j)
			for (int i = 0; i < H_p[j].length; ++i) {
				int x = H_p[j][i];
				H_s[x][H_idx[j][i]] = j;
			}
	}
	
	public void readPolicyNum(String filename) throws Exception {
		BufferedReader in = new BufferedReader(new FileReader(filename));
		if (Integer.parseInt(in.readLine()) != n_path)
			throw new Exception("Wrong N_PATH");

		for (int i = 0; i < n_path; ++i) {
			String str = in.readLine();
			P[i] = Integer.parseInt(str);
		}
		
		in.close();	
	}
	
	public void store(String filename) throws Exception {
		PrintWriter fout = new PrintWriter(new FileWriter(filename));
		fout.println(this.n_switch + " " + this.n_path);
		for (int i = 0; i < this.n_path; ++i)
			fout.print(this.P[i] + " ");
		fout.println();
		for (int j = 0; j < this.n_switch; ++j)
			fout.print(this.C[j] + " ");
		fout.println();
		
		for (int i = 0; i < this.n_path; ++i) {
			fout.print(this.H_p[i].length);
			for (int j = 0; j < H_p[i].length; ++j)
				fout.print(" " + H_p[i][j]);
			fout.println();
		}
			
		fout.close();
	}
}
