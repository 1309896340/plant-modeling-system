// Generated from c:/Users/13098/Desktop/wheat-project/lsystem/antlr4/g4/LSystemInput.g4 by ANTLR 4.13.1
import org.antlr.v4.runtime.tree.ParseTreeListener;

/**
 * This interface defines a complete listener for a parse tree produced by
 * {@link LSystemInputParser}.
 */
public interface LSystemInputListener extends ParseTreeListener {
	/**
	 * Enter a parse tree produced by {@link LSystemInputParser#symSeq}.
	 * @param ctx the parse tree
	 */
	void enterSymSeq(LSystemInputParser.SymSeqContext ctx);
	/**
	 * Exit a parse tree produced by {@link LSystemInputParser#symSeq}.
	 * @param ctx the parse tree
	 */
	void exitSymSeq(LSystemInputParser.SymSeqContext ctx);
	/**
	 * Enter a parse tree produced by {@link LSystemInputParser#sym}.
	 * @param ctx the parse tree
	 */
	void enterSym(LSystemInputParser.SymContext ctx);
	/**
	 * Exit a parse tree produced by {@link LSystemInputParser#sym}.
	 * @param ctx the parse tree
	 */
	void exitSym(LSystemInputParser.SymContext ctx);
	/**
	 * Enter a parse tree produced by {@link LSystemInputParser#symName}.
	 * @param ctx the parse tree
	 */
	void enterSymName(LSystemInputParser.SymNameContext ctx);
	/**
	 * Exit a parse tree produced by {@link LSystemInputParser#symName}.
	 * @param ctx the parse tree
	 */
	void exitSymName(LSystemInputParser.SymNameContext ctx);
	/**
	 * Enter a parse tree produced by {@link LSystemInputParser#symArgs}.
	 * @param ctx the parse tree
	 */
	void enterSymArgs(LSystemInputParser.SymArgsContext ctx);
	/**
	 * Exit a parse tree produced by {@link LSystemInputParser#symArgs}.
	 * @param ctx the parse tree
	 */
	void exitSymArgs(LSystemInputParser.SymArgsContext ctx);
	/**
	 * Enter a parse tree produced by {@link LSystemInputParser#symArg}.
	 * @param ctx the parse tree
	 */
	void enterSymArg(LSystemInputParser.SymArgContext ctx);
	/**
	 * Exit a parse tree produced by {@link LSystemInputParser#symArg}.
	 * @param ctx the parse tree
	 */
	void exitSymArg(LSystemInputParser.SymArgContext ctx);
}