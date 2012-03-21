default:
	cd makeprocs; make
	cd hello_world; make

clean:
	cd makeprocs; make clean
	cd hello_world; make clean

run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -D impf -u userprog.dlx.obj; ee469_fixterminal
