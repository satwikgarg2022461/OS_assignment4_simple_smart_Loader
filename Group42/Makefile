#invoke make inside following directories and in this order: loader, launch, fib
#move the lib_simpleloader.so and launch binaries inside bin directory
#Provide the command for cleanup
all:
	make -C test
	make -C loader
	make -C launcher
	echo "Type this command in terminal before running the executable file in bin directory"
	export LD_LIBRARY_PATH=./bin:$ LD_LIBRARY_PATH
#export LD_LIBRARY_PATH=./bin:$LD_LIBRARY_PATH
#Type this command in terminal before running the executable file in bin directory
clean:
	make clean -C test
	make clean -C loader
	make clean -C launcher