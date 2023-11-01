# OS_assignment4_simple_smart_Loader

# Contribution
This code is made by Saurav Haldar(2022464) and Satwik Garg(2022461).   
Bonus is part is done by Satwik Garg (2022461) and Saurav Haldar (2022464).  
  
Github Link - https://github.com/satwikgarg2022461/OS_assignment4_simple_smart_Loader.git  
  
# Implementation
1. Loader first catches the fragmentation error by using signal.  
2. Loader first allocate memory of size 4 KB(4096 byte) irrespective the size of fragmented segment.  
3. Then copy that segment by using memcopy.  
4. This process continues until no fragmentation occur.  

# How to Run the code
1. Open terminal  
2. Move to Group42 folder  
3. Type command "make"  
4. Type command "export LD_LIBRARY_PATH=./bin:$LD_LIBRARY_PATH"  
5. Type command "./launch ../test/fib"  