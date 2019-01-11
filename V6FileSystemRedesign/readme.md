################# Modified V6 file system #####################
The new v6 file system supports file with maximum capacity of 4GB.

################## Compile instructions #######################
Run --> gcc fsaccess.c -o fsaccess -lm

################## Code Execution Step ########################

After compilation, Run the output file --> ./V6Redesign.  
It will open the v6 command line interface. 

The following commands can be run on the command line interface.
1. initfs 
2. cpin
3. cpout
4. mkdir
5. Rm
6. q

################## initfs command ##################

initfs takes four arguments.
initfs <filename> < no.of.blocks assigned to the file>  < no of inodes assigned to the file>

e.g. --> initfs <v6file> <8000> <200>

################### cpin command ##################

Command format --> "cpin < from filename> < to filename>" 

################### cpout command ##################

Command Format --> cpout < from filename> < to filename> 

################### Rm command ##################

Command format --> Rm <filename>

################### mkdir command ################## 

Command Format --> mkdir <filename>

################### q command ##################

q exits the filesystem saving all the changes. 


