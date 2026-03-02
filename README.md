CSC173 - Project 2

Norman Sackett
nsackett@u.rochester.edu

The code does not need to be built in a particular way and the executable file is included in the directory. Upon running, the program will list the productions for the grammar and will prompt a user input for the recursive-descent parser. This parser will either print the parse tree or "INVALID INPUT" if the string is not accepted by the grammar. The user will then get another input prompt, this time for the table-driven parser. The table-driven parser has similar output to the recursive-descent one. It is important to note that internal nodes on the tree are labelled with numbers based on the syntactic category - it follows the numbering scheme presented upon running the program. Leaves of the tree display exactly the terminal symbol they are associated with unless that terminal is epsilon, in which case it is denoted by "E".
