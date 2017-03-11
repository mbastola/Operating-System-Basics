#Meet XSSH

The xssh shell is a bash wannabe. It either takes -f flag with a FILEIn and/or variables, runs the scripts and then exits to bash or it is invoked without a -f flag when user can input continuously till they invoke exit command. This means once inside the xssh shell, the user is not prompted for a -f flag. Although the user can invoke another child xssh shell with -f inside the xssh shell to run the scripts. howevr since the stdredirection isnt implemented this will cause errors and crashes the program.

*std redirection not implemented. Dup2 in progress (doesnt give errors when < or > is fed but does nothing as well)
*when a bad external command is invoked, the xssh opens up a new child that fails to close, thus you will have to exit twice to close the shell.

Design choices:
#Assumes every line of stdin or fin to be of max size 1024. This seemed a bit high for a command size but safe size and eradicated complications that would have brought by mallocing.
#Assumes each command size be of max 256. ie.if the total command is "ls -al" Then ls and ls each would be stored in a char array of size 256. Again, this is too big but eradicates complications and is a large enough number for safety.

