grammar LSystemRewriter;
symSeq: sym+ EOF;
sym: symName '(' symArgs? ')'
    | Punct;
symName: String;
symArgs: symArg (',' symArg)*;
symArg: Number;

Newline: '\n';
Punct: '[' | ']';
String: [a-zA-Z_]+[0-9]*;
Number: [+-]?[0-9]+('.'[0-9]*)?'f'?;
