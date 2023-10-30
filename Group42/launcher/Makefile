#Compile the launch.c by linking it with the lib_simpleloader.so
all:
	gcc -m32 -L../bin -o launch launch.c -lsimpleloader
	mv launch ../bin


#Provide the command for cleanup
clean:
	-@rm -f launch
	
