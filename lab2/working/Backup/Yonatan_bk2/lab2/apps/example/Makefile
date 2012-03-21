default:
	cd makeprocs; make
	cd hello_world; make
	cd userprog; make
	cd test2; make

clean:
	cd makeprocs; make clean
	cd hello_world; make clean
	cd userprog; make clean
	cd test2; make clean

run:
#	cd ../../bin; dlxsim -x os.dlx.obj -a -D impf -u userprog.dlx.obj; ee469_fixterminal
#	cd ../../bin; dlxsim -x os.dlx.obj -a  -u userprog.dlx.obj; ee469_fixterminal
#	cd ../../bin; dlxsim -x os.dlx.obj -a  -u makeprocs.dlx.obj; ee469_fixterminal
	cd ../../bin; dlxsim -x os.dlx.obj -a -D impft -u test2.dlx.obj; ee469_fixterminal
#impf i- information m- memory p-process f - filesystem
