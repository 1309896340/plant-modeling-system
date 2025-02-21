grammar LSystemParser;
lSystem: (lProducion Newline*)* EOF;
lProducion: symSrc RightMap symDsts;
symSrc: symName '(' params ')';
params: sym (',' sym)*;
symDsts: symDst+;
symDst: symName '(' paramMaps ')'
        | Punct;
paramMaps: mathExpr (',' mathExpr)*;
mathExpr: left=mathExpr op=(Plus | Minus) right=mathItem
        | mathItem;
mathItem: left=mathItem op=(Mul | Div) right=mathFactor
        | mathFactor;
mathFactor: sign=(Plus | Minus)? Number
        | (sign=(Plus | Minus)? sym)
        | '(' mathExpr ')';
symName: String;
sym: String;

Plus: '+';
Minus: '-';
Mul: '*';
Div: '/';
RightMap: '->';
Newline: '\n';
Punct: '[' | ']';
String: [a-zA-Z_]+[0-9]*;
Number: [0-9]+('.'[0-9]*)?'f'?;
