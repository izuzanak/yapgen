# yapgen - parser generator

Program yapgen is designed for fast prototyping, testing and generating of
parsers consisting from lexical and syntactic analyzers.

## Basic function

Parser is described by one input file containing description of lexical and
syntactic analyzers.  Lexical analyzer is denoted by set of regular
expressions, syntactic analyzer is denoted by SLR(1) grammar and semantic rules
of parser can be expressed by scripts, implemented in programming language LUA.
From description of language is generated parser, that can be tested on input
strings, or can be stored as source code in language C/C++.

### Program yapgen usage

Program yapgen accepts following arguments:

* Argument `--parser_descr <file>` identifies file containing description of
  parser to be generated. This file contains regular expressions of terminal
  symbols and rules of language grammar.
* Argument `--parser_save_cc <file>` identifies file to which is stored code of
  parser generated in language C/C++. Target file will contain implementation
  of lexical and syntactic analyzer of parser.
* Last argument `--source <file>` identifies file containing string, which is
  used as test input of generated parser.

## Description of language

### Regular expressions

Terminal symbols of language grammar are described by regular expressions.
Lexical analyzer based on final state automata is generated from these regular
expressions.

#### Terminal symbols description

Descriptions of terminal symbols corresponds to following format:

```
<id> {<regexp>}
```

Where `<id>` represents terminal symbol identifier, and `<regexp>`
represents code describing regular expression. Identifiers of terminal symbols
can be of same format as identifiers of language C, e.g. `('_'+l).('_'+l+d)*`.

```
oct_i32_const {'0'.<07>*}
dec_i32_const {<19>.d*}
hex_i32_const {'0'.[xX].(<09>+<af>+<AF>).(<09>+<af>+<AF>)*}

_SKIP_    {w.w*}
_SKIP__   {'#'.!'\n'*.'\n'}
_SKIP___  {"//".!'\n'*.'\n'}
_SKIP____ {"/*".(!'*'+('*'.!'/'))*."*/"}
_END_     {'\0'}
```

Terminal symbols which name contains substring `_SKIP_` are marked as ignored
by syntactic analysis of generated parser. This property is used for removal of
white symbols and comments from syntactic processing.

Terminal symbols which name contains substring `_END_` are marked as parse
terminating terminals.  Parse of source string is terminated after accepting
one of these terminals by shift action.  Character `\0` is often used as
terminating terminal of generated languages.

#### Base components of regular expressions

Regular expressions describing language terminal symbols are composed from
following components.

**Placeholder symbol**

Represents one character from predefined set of characters.

  * `e` - epsilon, represents string with zero length.
  * `w` - white, one of white characters (space, tabulator, or new line character).
  * `d` - digit, one digit `0-9`.
  * `l` - letter, one letter `a-z`, `A-Z`.

**Character**

  * `'a'` - represents character in apostrophes.
  * `'\60'` - represents character with ordinal value same as number in octal.
  * `'\x30'` - represents character with ordinal value same as number in hexadecimal.
  * `'\n'` - represents character with special meaning identified by symbol
    after backslash.  Following symbols can be located after backslash:
    `abfnrtv\?'">}]|`.

**Except character**

Represents one character different than character in apostrophes. For
characters between apostrophes can be used same notation as in component
Character: `!'<char>'`.

```
!'a'
'"'.(!'"'+'\\'.'"')*.'"'
```

**String of characters**

Represents string of characters, which is determined by characters used at
positions of string. For characters on individual positions can be used same
notation as in component Character: `"<char><char>..."`.

```
"if"
"else"
"multi\nline"
"/*".(!'*'+('*'.!'/'))*."*/"
```

**Character range**

Represents character, which ordinal value is between ordinal values of first
and last ordinal values of given range. For identification of first and last
value is used same notation as in component Character: `<<char><char>>`.

```
<19>.d*
'0'.[xX].(<09>+<af>+<AF>).(<09>+<af>+<AF>)*
```

**Character set**

Represents one character of given character set. For identification of
characters in set is used same notation as in component Character:
`[<char><char>...]`.

```
'\''.'\\'.[abfnrtv\\?\'"].'\''
'0'.[xX].(<09>+<af>+<AF>).(<09>+<af>+<AF>)*.[lL]
```

**Except character set**

Represents one character which is not contained in given character set.  For
identification of characters in set is used same notation as in component
Character: `|<char><char>...|`.

```
'"'.(|"|+'\\'.'"')*.'"'
```

#### Operators of regular expressions

Described components can be combined by following operators to construct more
complex regular expressions.

**Concatenation**

Binary operator `.`.  In order to accept resulting regular expression, left
operand and then right operand of operator must be accepted.

![Concatenation graph image](sources/op_conc.png)

```
"Hello".' '."world"
<19>.d*
'x'.(<09>+<af>+<AF>).(<09>+<af>+<AF>+e)
```

**Alternative**

Binary operator `+`.  In order to accept resulting regular expression, left
operand or right operand of operator must be accepted.

![Alternative graph image](sources/op_alter.png)

```
"i8"+"i16"+"i32"+"i64"+"u8"+"u16"+"u32"+"u64"+"f32"+"f64"
('_'+l).('_'+l+d)*
'x'.(<09>+<af>+<AF>).(<09>+<af>+<AF>+e)
```

**Iteration**

Unary operator `*`. In order to accept resulting regular expression operand of
operator must be accepted zero or n-times. Operand to which is operator applied
is located before operator symbol.

![Iteration graph image](sources/op_iter.png)

```
<19>.d*
('_'+l).('_'+l+d)*
```

#### Brackets in regular expressions

Round brackets `()` increases priority of regular subexpression similarly as in
brackets in arithmetic expressions. Brackets also combines complicated regular
subexpression to one component, to which above mentioned operators can be
applied.

### Grammar rules

Language of generated parser is described by SLR(1) grammar, which is strong
enough to describe general programming language, but not as strong as grammars
of LALR(1) or LR(1) parsers. Grammar of language is described by list of accepted
terminal symbols and by rules of grammar composed from terminal and nonterminal
symbols.

Nonterminal symbols of grammar are enclosed in angle brackets.

```
<start>
<program>
```

Rules of grammar are denoted by following format:

```
<head> -> <item> <item> ... ->> {<code>}
```

Where, `<head>` is nonterminal representing rule head. Head of rule is from
rule body separated by characters `->`. Rule of body contains sequence of
terminal and nonterminal symbols (`<item>`), which are from semantic code of
rule separated by characters `->>`. Semantic code expressed in language LUA is
enclosed in curly brackets `{}`.  When there is need to use symbol `}` in
semantic code it should be preceded by backslash character `\}`.

```
<command> -> <if_condition> <if_body> ->> {}
<if_condition> -> if <condition> ->> {print('if condition')}
<if_body> -> <command>           ->> {print('if statement')}
<if_body> -> <if_else> <command> ->> {print('if else statement')}
<if_else> -> <command> else      ->> {print('if else')}
```

### Semantic rules of language

Semantic rules of language are in phase of prototyping (gradual construction of
parser) expressed by scripts in language LUA.

It is possible to retrieve substring of source string, that represents n-th
component of rule body according to which reduction was performed, by call of
function `rule_body` with parameter `n`.

### Parser properties

Generated parser is one-pass parser. Terminal symbols recognized by lexical
analyzer are immediately processed by syntactic analyzer, without need to store
them between two steps.

## Example of parser construction

In this section will be presented gradual construction of parser, that will
accept simple arithmetic expressions on its input.

### Skeleton of parser

Initial set of rules describes parser, which accepts language of strings composed
from white characters (space, tabulator and new line character).

```
init_code: {}

terminals:
  _SKIP_ {w.w*}
  _END_ {'\0'}

nonterminals:
  <start>
  <end_check>

rules:
   <start> -> <end_check> ->> {}
   <end_check> -> _END_ ->> {}
```

Parser recognizes two terminal symbols. First symbol `_SKIP_` represents
sequence of white characters and second symbol `_END_` represents character
denoting end of input string.  Call of function implementing lexical analyzer
accepts pointer to input string characters and length of input string. When all
characters of input string are processed, character `\0` is then generated as
next input character.  Value of terminating terminal is not restricted to
character `\0`, but can be represented by any value that can be expressed by
regular expression.

As was mentioned in preceding text. Terminal symbols which identifier contains
substring `_SKIP_` are ignored by syntactic analysis.  When terminal symbol
which identifiers contain substring `_END_` is shifted to `LALR` parse stack
(so nonterminal symbol preceding this terminal of rule body was fully accepted)
parse of input string is terminated.

This assumptions imply, that parser generated from this set of rules just waits
on terminating character `\0`.

### List of terminal symbols

Second set of rules describes parser, which recognizes terminal symbols
representing integer constants.

```
init_code: {print('init_code')}

terminals:
  oct_int {'0'.<07>*}
  dec_int {<19>.d*}
  hex_int {'0'.[xX].(<09>+<af>+<AF>).(<09>+<af>+<AF>)*}

  _SKIP_ {w.w*}
  _END_ {'\0'}

nonterminals:
  <start>
  <end_check>
  <program>
  <item_list>
  <item>
  <A>

rules:
  <start> -> <end_check> ->> {}

  <end_check> -> <program> _END_ ->> {}
  <end_check> -> _END_ ->> {}

  <program> -> <item_list> ->> {}

  <item_list> -> <item> <item_list> ->> {}
  <item_list> -> <item> ->> {}

  <item> -> <A> ->> {}

  <A> -> oct_int ->> {print('constant: '..rule_body(0))}
  <A> -> dec_int ->> {print('constant: '..rule_body(0))}
  <A> -> hex_int ->> {print('constant: '..rule_body(0))}
```

Example of string accepted by generated parser:

```
10
0x10 125 02
07
```

Output of parser executed on input string:

```
init_code
constant: 10
constant: 0x10
constant: 125
constant: 02
constant: 07
```

Set of terminal symbols was extended by representations of integer numbers.
Rules of language grammar which generates list of integers contained in
accepted source string were added.

Semantic codes were assigned to last free rules of grammar. These codes will be
executed every time when reduction of rule to which this code is assigned will
occur. Semantic codes introduced in this example prints informations about
recognized terminals.

File contains initializing semantic code too. This code is located after
keyword `init_code`, and is executed at start of source parse.

### Evaluation of expressions

Last set of rules describes parser, which parses and by its semantic codes
evaluates simple arithmetic expressions over integer numbers.

```
init_code: {s = {\}}

terminals:
  oct_int {'0'.<07>*}
  dec_int {<19>.d*}
  hex_int {'0'.[xX].(<09>+<af>+<AF>).(<09>+<af>+<AF>)*}

  lr_br {'('}
  rr_br {')'}

  plus {'+'}
  minus {'-'}
  asterisk {'*'}
  slash {'/'}
  percent {'%'}

  _SKIP_ {w.w*}
  _END_ {'\0'}

nonterminals:
  <start>
  <end_check>
  <program>
  <item_list>
  <item>
  <C>
  <B>
  <A>

rules:
  <start> -> <end_check> ->> {}

  <end_check> -> <program> _END_ ->> {}
  <end_check> -> _END_ ->> {}

  <program> -> <item_list> ->> {}

  <item_list> -> <item> <item_list> ->> {}
  <item_list> -> <item> ->> {}

  <item> -> <C>           ->> {print("result: "..s[#s])}
  <C> -> <C> plus <B>     ->> {s[#s-1] = s[#s-1] + table.remove(s)}
  <C> -> <C> minus <B>    ->> {s[#s-1] = s[#s-1] - table.remove(s)}
  <C> -> <B>              ->> {}
  <B> -> <B> asterisk <A> ->> {s[#s-1] = s[#s-1] * table.remove(s)}
  <B> -> <B> slash <A>    ->> {s[#s-1] = s[#s-1] / table.remove(s)}
  <B> -> <B> percent <A>  ->> {s[#s-1] = s[#s-1] % table.remove(s)}
  <B> -> <A>              ->> {}
  <A> -> lr_br <C> rr_br  ->> {}

  <A> -> oct_int ->> {table.insert(s,tonumber(rule_body(0),8))}
  <A> -> dec_int ->> {table.insert(s,tonumber(rule_body(0),10))}
  <A> -> hex_int ->> {table.insert(s,tonumber(rule_body(0)))}
```

Example of string accepted by generated parser:

```
10 - 010
5*(10 + 5) - 0x10
10*3 - 10*2
```

Output of parser executed on input string:

```
result: 2
result: 59
result: 10
```

Set of terminal symbols was extended by round brackets and five arithmetic
operators. Rules of language grammar which describes arithmetic operations
were added. Evaluation of arithmetic expressions is performed on LUA stack as
follows:

  * Initializing semantic code creates variable `s`, that represents stack of
    evaluated expression.
  * When reduction by rule representing one integer number is performed. String
    representing integer is converted to number which is stored on top of stack.
  * When reduction by rule representing one of operators is performed. Operands
    of operation are retrieved from top of stack, operation is performed and
    result is stored on stack top.
  * When reduction by rule accepting complete expression is performed. Number
    on top of stack is printed to standard output as expression result.

Priority of operators is denoted by sequence of nonterminal symbols `<A>`,`<B>`
a `<C>`.

### Compilation of C like code

Next example will describe parser of simple JIT language derived from C.
Strings of JIT language describes functions, whose code is compiled to native
machine code.

```
i32 fact(i32 n)
{
  i32 res = 1;
  while (n > 1) res *= n--;
  return res;
}
```

```
i32 test(i8 *data,u32 count)
{
  i64 *ptr = (i64 *)data;
  i64 *ptr_end = ptr + count;
  do {
    *ptr = fact(*ptr);
  } while(++ptr < ptr_end);

  return 0;
}
```

File containing set of rules of JIT language can be found
[here](https://github.com/izuzanak/yapgen/blob/master/yapgen_build/rules/jit_parser.rules).
This example was introduced, to show that yapgen can be used for creation of
real parsers, not just simple toys.

