default:
	cd makeprocs; make
	cd hello_world; make
	cd delay_prog; make
	cd invalid_page; make
	cd invalid_address; make
	cd stack_grow; make
	cd excess_pages; make

clean:
	cd makeprocs; make clean
	cd hello_world; make clean
	cd delay_prog; make clean
	cd invalid_page; make clean
	cd invalid_address; make clean
	cd stack_grow; make clean
	cd excess_pages; make clean

run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 100 30; ee469_fixterminal
	cd ../../bin; dlxsim -x os.dlx.obj -a -u invalid_page.dlx.obj; ee469_fixterminal
	cd ../../bin; dlxsim -x os.dlx.obj -a -u invalid_address.dlx.obj; ee469_fixterminal
	cd ../../bin; dlxsim -x os.dlx.obj -a -u excess_pages.dlx.obj; ee469_fixterminal
