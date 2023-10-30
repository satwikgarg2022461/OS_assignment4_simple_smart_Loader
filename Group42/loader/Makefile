#Create lib_simpleloader.so from loader.c
all:
	gcc -m32 -c -fPIC loader.c
	gcc  -m32 -shared -o libsimpleloader.so loader.o
	mv libsimpleloader.so ../bin

#Provide the command for cleanup
clean:
	-@rm -f loader.o