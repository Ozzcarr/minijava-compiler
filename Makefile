compiler: lex.yy.c parser.tab.o main.cc
		g++ -g -w -ocompiler parser.tab.o lex.yy.c main.cc SymbolTable.cc SymbolTableBuilder.cc SemanticAnalyzer.cc IntermediateRepresentation.cc BytecodeGenerator.cc -std=c++14
interpreter:
		g++ -g -w -ointerpreter StackMachineInterpreter.cc -std=c++14
parser.tab.o: parser.tab.cc
		g++ -g -w -c parser.tab.cc -std=c++14
parser.tab.cc: parser.yy
		bison parser.yy
lex.yy.c: lexer.flex parser.tab.cc
		flex lexer.flex
tree:
		dot -Tpdf tree.dot -otree.pdf
cfg:
		dot -Tpdf cfg.dot -ocfg.pdf
clean:
		rm -f parser.tab.* lex.yy.c* compiler interpreter stack.hh position.hh location.hh tree.dot tree.pdf cfg.dot cfg.pdf output.bc
interpreterclean:
		rm -f interpreter
