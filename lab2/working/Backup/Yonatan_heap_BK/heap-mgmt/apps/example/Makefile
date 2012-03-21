default:
	cd heap_test; make
	#	cd makeprocs; make
	#	cd hello_world; make
	#	cd delay_prog; make
	#	cd invalid_page; make
	#	cd invalid_address; make
	#	cd stack_grow; make

clean:
	cd heap_test; make clean
#	cd makeprocs; make clean
#	cd hello_world; make clean
#	cd delay_prog; make clean
#	cd invalid_page; make clean
#	cd invalid_address; make clean
#	cd stack_grow; make clean

run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -D impf -u heap_test.dlx.obj; ee469_fixterminal
	# cd ../../bin; dlxsim -x os.dlx.obj -a -D impf -u userprog.dlx.obj; ee469_fixterminal
	# cd ../../bin; dlxsim -x os.dlx.obj -a -D impf -u makeprocs.dlx.obj 5 5; ee469_fixterminal
	# cd ../../bin; dlxsim -x os.dlx.obj -a -u invalid_page.dlx.obj; ee469_fixterminal
	# cd ../../bin; dlxsim -x os.dlx.obj -a -u invalid_address.dlx.obj; ee469_fixterminal
	
