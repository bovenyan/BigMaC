/**
 * Author: Nanxi Kang (nkang@cs.princeton.edu) 
 * All rights reserved.
 */
import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;


public class Utils {
	
	public static void copyFile(File srcFile, File destFile) throws IOException {
            InputStream oInStream = new FileInputStream(srcFile);
            OutputStream oOutStream = new FileOutputStream(destFile);

            // Transfer bytes from in to out
            byte[] oBytes = new byte[1024];
            int nLength;
            BufferedInputStream oBuffInputStream = 
                            new BufferedInputStream( oInStream );
            while ((nLength = oBuffInputStream.read(oBytes)) > 0) 
            {
                oOutStream.write(oBytes, 0, nLength);
            }
            oInStream.close();
            oOutStream.close();
    }
	
	public static int maxInt(int a, int b) {
		if (a > b)
			return a;
		return b;
	}
	
	public static long maxLong(long a, long b) {
		if (a > b)
			return a;
		return b;
	}
	
	public static long getFather(long k) {
		return (k - 1) / 2;
	}

	public static long getCommonAncestor(long x, long y) {
		while (x != y) {
			if (x > y)
				x = getFather(x);
			else 
				y = getFather(y);
		}
		return x;
	}
	
	public static boolean overlap(long x, long y) {
		long z = getCommonAncestor(x, y);
		return (z == x || z == y);
	}
	
	public static boolean intersect(long x1, long y1, long x2, long y2) {
		return overlap(x1, x2) && overlap(y1, y2);
	}
	
	public static String toIP(long x) {
		int mask = 0;
		long addr = 1;
		for (mask = 0, addr = 1; addr * 2 - 1 <= x; ++mask, addr = addr * 2);
		
		x = x - addr + 1;
		int delta = ((mask - 1)/ 8  + 1) * 8 - mask;
		for (int i = 0; i < delta; ++i)
			x = x * 2;
		
		int len = (mask + delta)/ 8;
		int p[] = new int[]{0, 0, 0, 0};
		for (int i = len - 1;  i >= 0; --i) {
			p[i] = (int)(x % 256);
			x /= 256;			
		}
		
		String str = String.format("%d.%d.%d.%d/%d", p[0], p[1], p[2], p[3], mask);
		return str;
		
	}
	
	public static int locate(String[] strs, String pattern) {
		for (int i = 0; i < strs.length; ++i)
			if (strs[i].equals(pattern)) {
				return i;
			}
		return -1;
	}
}
