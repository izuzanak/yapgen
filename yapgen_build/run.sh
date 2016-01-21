#!/bin/sh

# - uclang_parser -
#echo "uclang_parser.rules" 1>&2
./yapgen --parser_descr rules/uclang_parser.rules --source rules/uclang_parser.src

# - docu_comment -
#echo "docu_comment.rules" 1>&2
#./yapgen --parser_descr rules/docu_comment.rules --source rules/docu_comment.src

# - docu_extract -
#echo "docu_extract.rules" 1>&2
#./yapgen --parser_descr rules/docu_extract.rules --source rules/docu_extract.src

# - jit_parser -
#echo "jit_parser.rules" 1>&2
#./yapgen --parser_descr rules/jit_parser.rules --source rules/jit_parser.src

# - jit_types -
#echo "jit_types.rules" 1>&2
#./yapgen --parser_descr rules/jit_types.rules --source rules/jit_types.src

# - json -
#echo "json.rules" 1>&2
#./yapgen --parser_descr rules/json.rules --source rules/json.src

# - pack_code -
#echo "pack_code.rules" 1>&2
#./yapgen --parser_descr rules/pack_code.rules --source rules/pack_code.src

# - string_format -
#echo "string_format.rules" 1>&2
#./yapgen --parser_descr rules/string_format.rules --source rules/string_format.src

# - demo_exp -
#echo "demo_exp.rules" 1>&2
#./yapgen --parser_descr rules/demo_exp.rules --source rules/demo_exp.src

# - demo_parse -
#echo "demo_parse.rules" 1>&2
#./yapgen --parser_descr rules/demo_parse/1.rules --source rules/demo_parse/1.src
#./yapgen --parser_descr rules/demo_parse/2.rules --source rules/demo_parse/2.src
#./yapgen --parser_descr rules/demo_parse/3.rules --source rules/demo_parse/3.src

# - regexp -
#echo "regexp.rules" 1>&2
#./yapgen --parser_descr rules/regexp.rules --source rules/regexp.src

# - packet_parser -
#echo "packet_parser.rules" 1>&2
#./yapgen --parser_descr rules/packet_parser.rules --source rules/packet_parser.src

# - cont -
#echo "cont.rules" 1>&2
#./yapgen --parser_descr rules/cont.rules --source rules/cont.src

# - cont_c -
#echo "cont_c.rules" 1>&2
#./yapgen --parser_descr rules/cont_c.rules --source rules/cont_c.src

