# Prova Finale di Algoritmi e Strutture Dati a.a. 2019-2020

The objective of this project is to provide an implementation of a text editor with multiple Undo/Redo optimizing running time and memory usage.

# Input commands
The program accepts the following inputs:
* (ind1, ind2)c
* (ind1, ind2)d
* (ind1, ind2)p
* (num)u
* (num)r
* q

The **change** command changes the text between lines ind1 and ind2. The command must be followed by a number of lines equals to ind2-ind1+1 and ind1 must be the address of a line already present in the text, or the first empty address after the last line present.

The **delete** command deletes the lines present between ind1 and ind2. The lines after ind2 (if presents) are shifted in order to do not leave any empty block.

The **print** command prints the lines between ind1 and ind2, included. In case the line to print is not present in the text, a '.' is printed.

The **undo** command does the undo of the last "num" c or d commands.

The **redo** command does the redo. If the number of commands to redo is higher than the undo that has been done previously, only the possible number of redo are completed.

The **quit** command ends the execution of the editor.

# Test cases
The implementation has been tested with several test, in order to evaluate the efficiency in terms of running time and memory usage.

