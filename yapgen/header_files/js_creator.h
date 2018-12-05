
/*
 * code of function generating parser in JavaScript
 */

cc_source.clear();

const int buffer_size = 2048;
char buffer[buffer_size];
const int sub_buffer_size = 256;
char sub_buffer[sub_buffer_size];
int chars_writed;

PUSH_CODE(
"\n"
"'use strict';\n"
"\n"
"var fs = require('fs');\n"
"\n"
);

PUSH_CODE(
"// - parse constants -\n"
"var c_idx_not_exist = -1;\n"
);

PUSH_CODE(
"var rule_head_idxs = ["
);

if (rule_descrs.used != 0)
{
  p_rule_descr_s *rd_ptr = rule_descrs.data;
  p_rule_descr_s *rd_ptr_end = rd_ptr + rule_descrs.used;

  do
  {
    PUSH_FORMAT_CODE("%d, ",rd_ptr->head);
  }
  while(++rd_ptr < rd_ptr_end);
}

PUSH_CODE(
"];\n"
);

PUSH_CODE(
"var rule_body_lengths = ["
);

if (rule_descrs.used != 0)
{
  p_rule_descr_s *rd_ptr = rule_descrs.data;
  p_rule_descr_s *rd_ptr_end = rd_ptr + rule_descrs.used;

  do
  {
    PUSH_FORMAT_CODE("%d, ",rd_ptr->body_size);
  }
  while(++rd_ptr < rd_ptr_end);
}

PUSH_CODE(
"];\n"
"\n"
);

PUSH_FORMAT_CODE(
"// - lalr parse table - \n"
"var blank = c_idx_not_exist;\n"
"function SHIFT(VALUE) { return VALUE; }\n"
"function REDUCE(VALUE) { return c_lalr_table_reduce_base + VALUE; }\n"
"function GOTO(VALUE) { return VALUE; }\n"
"\n"
"var c_lalr_table_reduce_base = 0x%x;\n"
"var c_terminal_plus_nonterminal_cnt = %d;\n"
"var lalr_state_cnt = %d;\n"
"\n"
"var lalr_table =\n"
"[//{{{\n"
  ,MPAR(MPAR(c_lalr_table_reduce_base,lalr_table.x_size),lalr_table.y_size));

{
  unsigned *row_ptr = lalr_table.table.data;
  unsigned *row_ptr_end = row_ptr + lalr_table.y_size*lalr_table.x_size;

  do
  {
    unsigned col_idx = 0;

    // - write shift/reduce actions -
    do
    {
      if (row_ptr[col_idx] == c_idx_not_exist)
      {
        PUSH_CODE("       blank,");
      }
      else if (row_ptr[col_idx] < c_lalr_table_reduce_base)
      {
        SUB_FORMAT("SHIFT(%d)",row_ptr[col_idx]);
        PUSH_FORMAT_CODE("%12s,",sub_buffer);
      }
      else
      {
        SUB_FORMAT("REDUCE(%d)",row_ptr[col_idx] - c_lalr_table_reduce_base);
        PUSH_FORMAT_CODE("%12s,",sub_buffer);
      }
    }
    while(++col_idx < terminal_cnt);

    // - write goto actions -
    do
    {
      if (row_ptr[col_idx] == c_idx_not_exist)
      {
        PUSH_CODE("       blank,");
      }
      else
      {
        SUB_FORMAT("GOTO(%d)",row_ptr[col_idx]);
        PUSH_FORMAT_CODE("%12s,",sub_buffer);
      }
    }
    while(++col_idx < lalr_table.x_size);
    PUSH_CODE("\n");

  }
  while((row_ptr += lalr_table.x_size) < row_ptr_end);
}

PUSH_CODE(
"];//}}}\n"
"\n"
);

#define JS_PROCESS_STATE(STATE_IDX) \
{/*{{{*/\
  fa_state_s &state = states[STATE_IDX];\
  ui_array_s &state_moves = final_automata.state_moves[STATE_IDX];\
  \
  if (state.moves.used != 0) {\
    PUSH_CODE(\
              "    this.next_char();\n"\
             );\
    unsigned b_idx = 0;\
    unsigned e_idx = 0;\
    unsigned target_state = state_moves[b_idx];\
    do {\
      if (state_moves[e_idx] != target_state) {\
        if (target_state != c_idx_not_exist) {\
          if (e_idx - b_idx > 1) {\
            if (b_idx == 0) {\
              PUSH_FORMAT_CODE(\
                               "\n"\
                               "    if (this.in_char < %d)\n"\
                               "      return this.state_%d();\n"\
                               ,MPAR(e_idx,target_state));\
            }\
            else {\
              PUSH_FORMAT_CODE(\
                               "\n"\
                               "    if (this.in_char >= %d && this.in_char < %d)\n"\
                               "      return this.state_%d();\n"\
                               ,MPAR(MPAR(b_idx,e_idx),target_state));\
            }\
          }\
          else {\
            PUSH_FORMAT_CODE(\
                             "\n"\
                             "    if (this.in_char == %d)\n"\
                             "      return this.state_%d();\n"\
                             ,MPAR(b_idx,target_state));\
          }\
        }\
        \
        b_idx = e_idx;\
        target_state = state_moves[b_idx];\
      }\
    } while(++e_idx < c_base_char_cnt);\
    \
    if (target_state != c_idx_not_exist) {\
      if (e_idx - b_idx > 1) {\
        if (b_idx == 0) {\
          PUSH_FORMAT_CODE(\
                           "\n"\
                           "    if (this.in_char < %d)\n"\
                           "      return this.state_%d();\n"\
                           ,MPAR(e_idx,target_state));\
        }\
        else {\
          PUSH_FORMAT_CODE(\
                           "\n"\
                           "    if (this.in_char >= %d && this.in_char < %d)\n"\
                           "      return this.state_%d();\n"\
                           ,MPAR(MPAR(b_idx,e_idx),target_state));\
        }\
      }\
      else {\
        PUSH_FORMAT_CODE(\
                         "\n"\
                         "    if (this.in_char == %d)\n"\
                         "      return this.state_%d();\n"\
                         ,MPAR(b_idx,target_state));\
      }\
    }\
  }\
  \
  if (state.final != c_idx_not_exist) {\
    PUSH_FORMAT_CODE(\
                     "\n"\
                     "    return %d;\n"\
                     ,state.final);\
  }\
  else {\
    PUSH_CODE(\
              "\n"\
              "    return c_idx_not_exist;\n"\
             );\
  }\
}/*}}}*/

PUSH_CODE(
"function source_s(a_data)\n"
"{\n"
"  this.input_idx = 0;\n"
"  this.data = a_data;\n"
"  this.in_char = 0;\n"
"\n"
"  this.next_char = function()\n"
"  {//{{{\n"
"    if (this.input_idx < this.data.length) {\n"
"      this.in_char = this.data[this.input_idx];\n"
"    }\n"
"    else {\n"
"      this.in_char = 0;\n"
"    }\n"
"  }//}}}\n"
"\n"
);

{
  fa_states_s &states = final_automata.states;

  PUSH_CODE(
"  // - STATE 0 -\n"
"  this.state_0 = function(a_no_close)\n"
"  {//{{{\n"
"    if (a_no_close === undefined) {\n"
  );
  if (states[0].final != c_idx_not_exist)
  {
    PUSH_FORMAT_CODE(
"      if (this.in_char == 0)\n"
"        return %d;\n"
"\n"
"      ++this.input_idx;\n"
      ,states[0].final);
  }
  else
  {
    PUSH_CODE(
"      if (this.in_char == 0)\n"
"        return c_idx_not_exist;\n"
"\n"
"      ++this.input_idx;\n"
      );
  }
  PUSH_CODE(
"    }\n"
"\n"
  );
  JS_PROCESS_STATE(0);
  PUSH_CODE(
"  }//}}}\n"
"\n"
  );

  unsigned s_idx = 1;
  do
  {
    PUSH_FORMAT_CODE(
"  // - STATE %d -\n"
"  this.state_%d = function()\n"
"  {//{{{\n"
      ,MPAR(s_idx,s_idx));
    if (states[s_idx].final != c_idx_not_exist)
    {
      PUSH_FORMAT_CODE(
"    if (this.in_char == 0)\n"
"      return %d;\n"
"\n"
"    ++this.input_idx;\n"
        ,states[s_idx].final);
    }
    else
    {
      PUSH_CODE(
"    if (this.in_char == 0)\n"
"      return c_idx_not_exist;\n"
"\n"
"    ++this.input_idx;\n"
      );
    }
    JS_PROCESS_STATE(s_idx);
  PUSH_CODE(
"  }//}}}\n"
"\n"
  );
  }
  while(++s_idx < states.used);
}

PUSH_CODE(
"  // - parse source string -\n"
"  this.parse_source = function()\n"
"  {//{{{\n"
"    var lalr_stack = [[0,-1,-1]];\n"
"\n"
"    var old_input_idx = 0;\n"
"    this.input_idx = 0;\n"
"    var ret_term = c_idx_not_exist;\n"
"    var parse_action;\n"
"\n"
"    do {\n"
"\n"
"      while (ret_term == c_idx_not_exist) {\n"
"        old_input_idx = this.input_idx;\n"
"\n"
"        ret_term = this.state_0(false);\n"
"        //assert(ret_term != c_idx_not_exist);\n"
);

if (skip_terminals.used > 0)
{
  PUSH_FORMAT_CODE(
"\n"
"        // - skipping of _SKIP_ terminals -\n"
"        if (ret_term == %d"
    ,skip_terminals[0]);
  if (skip_terminals.used > 1)
  {
    unsigned st_idx = 1;
    do
    {
      PUSH_FORMAT_CODE(" || ret_term == %d",skip_terminals[st_idx]);
    }
    while(++st_idx < skip_terminals.used);
  }
  PUSH_CODE(") {\n"
"          ret_term = c_idx_not_exist;\n"
"        }\n"
           );
}

PUSH_CODE(
"      }\n"
"\n"
"      // - retrieve action from table of actions -\n"
"      parse_action = lalr_table[lalr_stack[lalr_stack.length - 1][0]*c_terminal_plus_nonterminal_cnt + ret_term];\n"
"      //assert(parse_action != c_idx_not_exist);\n"
"\n"
"      // - action SHIFT -\n"
"      if (parse_action < c_lalr_table_reduce_base) {\n"
"\n"
);
PUSH_FORMAT_CODE(
"        // - end on _END_ terminal -\n"
"        if (ret_term == %d) {\n"
"          break;\n"
"        }\n"
"\n"
  ,end_terminal);
PUSH_CODE(
"        // - push new state to lalr stack -\n"
"        lalr_stack.push([parse_action,old_input_idx,this.input_idx]);\n"
"        ret_term = c_idx_not_exist;\n"
"      }\n"
"\n"
"      // - action REDUCE -\n"
"      else {\n"
"        parse_action -= c_lalr_table_reduce_base;\n"
"\n"
"        // - print index of reduction rule to output -\n"
"        process.stdout.write(parse_action + ', ');\n"
"\n"
"        // - remove rule body from stack -\n"
"        var rb_length = rule_body_lengths[parse_action];\n"
"        lalr_stack.splice(lalr_stack.length - rb_length,rb_length);\n"
"\n"
"        // - push new state to lalr stack -\n"
"        var goto_val = lalr_table[lalr_stack[lalr_stack.length - 1][0]*c_terminal_plus_nonterminal_cnt + rule_head_idxs[parse_action]];\n"
"        lalr_stack.push([goto_val,-1,-1]);\n"
"      }\n"
"\n"
"    } while(true);\n"
"  }//}}}\n"
);

PUSH_CODE(
"}\n"
"\n"
);

PUSH_CODE(
"// - program entry point -\n"
"if (process.argv.length <= 2)\n"
"{\n"
"  process.stderr.write('Expecting at least one argument\\n');\n"
"  process.exit(1);\n"
"}\n"
"\n"
"fs.readFile(process.argv[2],function(err,data)\n"
"{\n"
"  // - ERROR -\n"
"  if (err != null)\n"
"  {\n"
"    process.stderr.write(err.toString() + '\\n');\n"
"    process.exit(1);\n"
"  }\n"
"\n"
"  var source = new source_s(new Int8Array(data));\n"
"  source.parse_source();\n"
"});\n"
"\n"
);

