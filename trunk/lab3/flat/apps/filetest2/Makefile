default:
	cd filetest2; make

clean:
	cd filetest2; make clean

run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -D FD -u filetest2.dlx.obj; ee469_fixterminal

runwithcache:
	cd ../../bin; dlxsim -x os.dlx.obj -a -D FDC -u filetest2.dlx.obj; ee469_fixterminal
