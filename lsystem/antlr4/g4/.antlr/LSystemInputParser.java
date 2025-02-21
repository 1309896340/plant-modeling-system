// Generated from c:/Users/13098/Desktop/wheat-project/lsystem/antlr4/g4/LSystemInput.g4 by ANTLR 4.13.1
import org.antlr.v4.runtime.atn.*;
import org.antlr.v4.runtime.dfa.DFA;
import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.misc.*;
import org.antlr.v4.runtime.tree.*;
import java.util.List;
import java.util.Iterator;
import java.util.ArrayList;

@SuppressWarnings({"all", "warnings", "unchecked", "unused", "cast", "CheckReturnValue"})
public class LSystemInputParser extends Parser {
	static { RuntimeMetaData.checkVersion("4.13.1", RuntimeMetaData.VERSION); }

	protected static final DFA[] _decisionToDFA;
	protected static final PredictionContextCache _sharedContextCache =
		new PredictionContextCache();
	public static final int
		T__0=1, T__1=2, T__2=3, Newline=4, Punct=5, String=6, Number=7;
	public static final int
		RULE_symSeq = 0, RULE_sym = 1, RULE_symName = 2, RULE_symArgs = 3, RULE_symArg = 4;
	private static String[] makeRuleNames() {
		return new String[] {
			"symSeq", "sym", "symName", "symArgs", "symArg"
		};
	}
	public static final String[] ruleNames = makeRuleNames();

	private static String[] makeLiteralNames() {
		return new String[] {
			null, "'('", "')'", "','", "'\\n'"
		};
	}
	private static final String[] _LITERAL_NAMES = makeLiteralNames();
	private static String[] makeSymbolicNames() {
		return new String[] {
			null, null, null, null, "Newline", "Punct", "String", "Number"
		};
	}
	private static final String[] _SYMBOLIC_NAMES = makeSymbolicNames();
	public static final Vocabulary VOCABULARY = new VocabularyImpl(_LITERAL_NAMES, _SYMBOLIC_NAMES);

	/**
	 * @deprecated Use {@link #VOCABULARY} instead.
	 */
	@Deprecated
	public static final String[] tokenNames;
	static {
		tokenNames = new String[_SYMBOLIC_NAMES.length];
		for (int i = 0; i < tokenNames.length; i++) {
			tokenNames[i] = VOCABULARY.getLiteralName(i);
			if (tokenNames[i] == null) {
				tokenNames[i] = VOCABULARY.getSymbolicName(i);
			}

			if (tokenNames[i] == null) {
				tokenNames[i] = "<INVALID>";
			}
		}
	}

	@Override
	@Deprecated
	public String[] getTokenNames() {
		return tokenNames;
	}

	@Override

	public Vocabulary getVocabulary() {
		return VOCABULARY;
	}

	@Override
	public String getGrammarFileName() { return "LSystemInput.g4"; }

	@Override
	public String[] getRuleNames() { return ruleNames; }

	@Override
	public String getSerializedATN() { return _serializedATN; }

	@Override
	public ATN getATN() { return _ATN; }

	public LSystemInputParser(TokenStream input) {
		super(input);
		_interp = new ParserATNSimulator(this,_ATN,_decisionToDFA,_sharedContextCache);
	}

	@SuppressWarnings("CheckReturnValue")
	public static class SymSeqContext extends ParserRuleContext {
		public TerminalNode EOF() { return getToken(LSystemInputParser.EOF, 0); }
		public List<SymContext> sym() {
			return getRuleContexts(SymContext.class);
		}
		public SymContext sym(int i) {
			return getRuleContext(SymContext.class,i);
		}
		public SymSeqContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_symSeq; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LSystemInputListener ) ((LSystemInputListener)listener).enterSymSeq(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LSystemInputListener ) ((LSystemInputListener)listener).exitSymSeq(this);
		}
	}

	public final SymSeqContext symSeq() throws RecognitionException {
		SymSeqContext _localctx = new SymSeqContext(_ctx, getState());
		enterRule(_localctx, 0, RULE_symSeq);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(11); 
			_errHandler.sync(this);
			_la = _input.LA(1);
			do {
				{
				{
				setState(10);
				sym();
				}
				}
				setState(13); 
				_errHandler.sync(this);
				_la = _input.LA(1);
			} while ( _la==Punct || _la==String );
			setState(15);
			match(EOF);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class SymContext extends ParserRuleContext {
		public SymNameContext symName() {
			return getRuleContext(SymNameContext.class,0);
		}
		public SymArgsContext symArgs() {
			return getRuleContext(SymArgsContext.class,0);
		}
		public TerminalNode Punct() { return getToken(LSystemInputParser.Punct, 0); }
		public SymContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_sym; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LSystemInputListener ) ((LSystemInputListener)listener).enterSym(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LSystemInputListener ) ((LSystemInputListener)listener).exitSym(this);
		}
	}

	public final SymContext sym() throws RecognitionException {
		SymContext _localctx = new SymContext(_ctx, getState());
		enterRule(_localctx, 2, RULE_sym);
		int _la;
		try {
			setState(25);
			_errHandler.sync(this);
			switch (_input.LA(1)) {
			case String:
				enterOuterAlt(_localctx, 1);
				{
				setState(17);
				symName();
				setState(18);
				match(T__0);
				setState(20);
				_errHandler.sync(this);
				_la = _input.LA(1);
				if (_la==Number) {
					{
					setState(19);
					symArgs();
					}
				}

				setState(22);
				match(T__1);
				}
				break;
			case Punct:
				enterOuterAlt(_localctx, 2);
				{
				setState(24);
				match(Punct);
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class SymNameContext extends ParserRuleContext {
		public TerminalNode String() { return getToken(LSystemInputParser.String, 0); }
		public SymNameContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_symName; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LSystemInputListener ) ((LSystemInputListener)listener).enterSymName(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LSystemInputListener ) ((LSystemInputListener)listener).exitSymName(this);
		}
	}

	public final SymNameContext symName() throws RecognitionException {
		SymNameContext _localctx = new SymNameContext(_ctx, getState());
		enterRule(_localctx, 4, RULE_symName);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(27);
			match(String);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class SymArgsContext extends ParserRuleContext {
		public List<SymArgContext> symArg() {
			return getRuleContexts(SymArgContext.class);
		}
		public SymArgContext symArg(int i) {
			return getRuleContext(SymArgContext.class,i);
		}
		public SymArgsContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_symArgs; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LSystemInputListener ) ((LSystemInputListener)listener).enterSymArgs(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LSystemInputListener ) ((LSystemInputListener)listener).exitSymArgs(this);
		}
	}

	public final SymArgsContext symArgs() throws RecognitionException {
		SymArgsContext _localctx = new SymArgsContext(_ctx, getState());
		enterRule(_localctx, 6, RULE_symArgs);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(29);
			symArg();
			setState(34);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while (_la==T__2) {
				{
				{
				setState(30);
				match(T__2);
				setState(31);
				symArg();
				}
				}
				setState(36);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class SymArgContext extends ParserRuleContext {
		public TerminalNode Number() { return getToken(LSystemInputParser.Number, 0); }
		public SymArgContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_symArg; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LSystemInputListener ) ((LSystemInputListener)listener).enterSymArg(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LSystemInputListener ) ((LSystemInputListener)listener).exitSymArg(this);
		}
	}

	public final SymArgContext symArg() throws RecognitionException {
		SymArgContext _localctx = new SymArgContext(_ctx, getState());
		enterRule(_localctx, 8, RULE_symArg);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(37);
			match(Number);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static final String _serializedATN =
		"\u0004\u0001\u0007(\u0002\u0000\u0007\u0000\u0002\u0001\u0007\u0001\u0002"+
		"\u0002\u0007\u0002\u0002\u0003\u0007\u0003\u0002\u0004\u0007\u0004\u0001"+
		"\u0000\u0004\u0000\f\b\u0000\u000b\u0000\f\u0000\r\u0001\u0000\u0001\u0000"+
		"\u0001\u0001\u0001\u0001\u0001\u0001\u0003\u0001\u0015\b\u0001\u0001\u0001"+
		"\u0001\u0001\u0001\u0001\u0003\u0001\u001a\b\u0001\u0001\u0002\u0001\u0002"+
		"\u0001\u0003\u0001\u0003\u0001\u0003\u0005\u0003!\b\u0003\n\u0003\f\u0003"+
		"$\t\u0003\u0001\u0004\u0001\u0004\u0001\u0004\u0000\u0000\u0005\u0000"+
		"\u0002\u0004\u0006\b\u0000\u0000&\u0000\u000b\u0001\u0000\u0000\u0000"+
		"\u0002\u0019\u0001\u0000\u0000\u0000\u0004\u001b\u0001\u0000\u0000\u0000"+
		"\u0006\u001d\u0001\u0000\u0000\u0000\b%\u0001\u0000\u0000\u0000\n\f\u0003"+
		"\u0002\u0001\u0000\u000b\n\u0001\u0000\u0000\u0000\f\r\u0001\u0000\u0000"+
		"\u0000\r\u000b\u0001\u0000\u0000\u0000\r\u000e\u0001\u0000\u0000\u0000"+
		"\u000e\u000f\u0001\u0000\u0000\u0000\u000f\u0010\u0005\u0000\u0000\u0001"+
		"\u0010\u0001\u0001\u0000\u0000\u0000\u0011\u0012\u0003\u0004\u0002\u0000"+
		"\u0012\u0014\u0005\u0001\u0000\u0000\u0013\u0015\u0003\u0006\u0003\u0000"+
		"\u0014\u0013\u0001\u0000\u0000\u0000\u0014\u0015\u0001\u0000\u0000\u0000"+
		"\u0015\u0016\u0001\u0000\u0000\u0000\u0016\u0017\u0005\u0002\u0000\u0000"+
		"\u0017\u001a\u0001\u0000\u0000\u0000\u0018\u001a\u0005\u0005\u0000\u0000"+
		"\u0019\u0011\u0001\u0000\u0000\u0000\u0019\u0018\u0001\u0000\u0000\u0000"+
		"\u001a\u0003\u0001\u0000\u0000\u0000\u001b\u001c\u0005\u0006\u0000\u0000"+
		"\u001c\u0005\u0001\u0000\u0000\u0000\u001d\"\u0003\b\u0004\u0000\u001e"+
		"\u001f\u0005\u0003\u0000\u0000\u001f!\u0003\b\u0004\u0000 \u001e\u0001"+
		"\u0000\u0000\u0000!$\u0001\u0000\u0000\u0000\" \u0001\u0000\u0000\u0000"+
		"\"#\u0001\u0000\u0000\u0000#\u0007\u0001\u0000\u0000\u0000$\"\u0001\u0000"+
		"\u0000\u0000%&\u0005\u0007\u0000\u0000&\t\u0001\u0000\u0000\u0000\u0004"+
		"\r\u0014\u0019\"";
	public static final ATN _ATN =
		new ATNDeserializer().deserialize(_serializedATN.toCharArray());
	static {
		_decisionToDFA = new DFA[_ATN.getNumberOfDecisions()];
		for (int i = 0; i < _ATN.getNumberOfDecisions(); i++) {
			_decisionToDFA[i] = new DFA(_ATN.getDecisionState(i), i);
		}
	}
}