
/*
 * code of function generating parser in PHP
 */

cc_source.clear();

const int buffer_size = 2048;
char buffer[buffer_size];
const int sub_buffer_size = 256;
char sub_buffer[sub_buffer_size];
int chars_writed;

PUSH_CODE(
"\n"
"<?php\n"
"\n"
);

PUSH_CODE(
"// - parse constants -\n"
"const c_idx_not_exist = -1;\n"
);

PUSH_CODE(
"const rule_head_idxs = ["
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
"const rule_body_lengths = ["
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
"const blank = c_idx_not_exist;\n"
"function SHIFT($VALUE) { return $VALUE; }\n"
"function REDUCE($VALUE) { return c_lalr_table_reduce_base + $VALUE; }\n"
"function TRANS($VALUE) { return $VALUE; }\n"
"\n"
"const c_lalr_table_reduce_base = 0x08000000;\n"
"const c_terminal_plus_nonterminal_cnt = %d;\n"
"const lalr_state_cnt = %d;\n"
"\n"
"$lalr_table =\n"
"[//{{{\n"
  ,MPAR(lalr_table.x_size,lalr_table.y_size));

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
        SUB_FORMAT("TRANS(%d)",row_ptr[col_idx]);
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

PUSH_CODE(
"// - global variables -\n"
"$source_string_length;\n"
"$source_string;\n"
"$input_idx;\n"
"$in_char;\n"
"\n"
);

PUSH_CODE(
"function get_next_char() \n"
"{//{{{\n"
"   global $source_string_length;\n"
"   global $source_string;\n"
"   global $input_idx;\n"
"   global $in_char;\n"
"\n"
"   if ($input_idx < $source_string_length) {\n"
"      $in_char = ord($source_string[$input_idx]);\n"
"   }\n"
"   else {\n"
"      $in_char = 0;\n"
"   }\n"
"}//}}}\n"
"\n"
"function close_char($ret_term_idx) \n"
"{//{{{\n"
"   global $input_idx;\n"
"   global $in_char;\n"
"\n"
"   if ($in_char == 0) {\n"
"      return $ret_term_idx;\n"
"   }\n"
"\n"
"   $input_idx++;\n"
"}//}}}\n"
"\n"
);

PUSH_CODE(
"// - recognize next terminal - \n"
"function recognize_terminal()\n"
"{/*{{{*/\n"
);

#define PHP_PROCESS_STATE(STATE_IDX) \
  {/*{{{*/\
    fa_state_s &state = states[STATE_IDX];\
    ui_array_s &state_moves = final_automata.state_moves[STATE_IDX];\
    \
    if (state.moves.used != 0) {\
      PUSH_CODE(\
                "   get_next_char();\n"\
                "\n"\
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
                                 "   if ($in_char < %d)\n"\
                                 "      goto state_%d_label;\n"\
                                 "\n"\
                                 ,MPAR(e_idx,target_state));\
              }\
              else {\
                PUSH_FORMAT_CODE(\
                                 "   if ($in_char >= %d && $in_char < %d)\n"\
                                 "      goto state_%d_label;\n"\
                                 "\n"\
                                 ,MPAR(MPAR(b_idx,e_idx),target_state));\
              }\
            }\
            else {\
              PUSH_FORMAT_CODE(\
                               "   if ($in_char == %d)\n"\
                               "      goto state_%d_label;\n"\
                               "\n"\
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
                             "   if ($in_char < %d)\n"\
                             "      goto state_%d_label;\n"\
                             "\n"\
                             ,MPAR(e_idx,target_state));\
          }\
          else {\
            if (e_idx < 256) {\
              PUSH_FORMAT_CODE(\
                               "   if ($in_char >= %d && $in_char < %d)\n"\
                               "      goto state_%d_label;\n"\
                               "\n"\
                               ,MPAR(MPAR(b_idx,e_idx),target_state));\
            }\
            else {\
              cassert(e_idx == 256);\
              PUSH_FORMAT_CODE(\
                               "   if ($in_char >= %d)\n"\
                               "      goto state_%d_label;\n"\
                               "\n"\
                               ,MPAR(b_idx,target_state));\
            }\
          }\
        }\
        else {\
          PUSH_FORMAT_CODE(\
                           "   if ($in_char == %d)\n"\
                           "      goto state_%d_label;\n"\
                           "\n"\
                           ,MPAR(b_idx,target_state));\
        }\
      }\
    }\
    \
    if (state.final != c_idx_not_exist) {\
      PUSH_FORMAT_CODE(\
                       "   return %d;\n"\
                       "\n"\
                       ,state.final);\
    }\
    else {\
      PUSH_CODE(\
                "   return c_idx_not_exist;\n"\
                "\n"\
               );\
    }\
  }/*}}}*/

  PUSH_CODE(
"   global $in_char;\n"
"\n"
"   goto fa_start_label;\n"
"\n"
);
{
  fa_states_s &states = final_automata.states;

  PUSH_CODE(
"// - STATE 0 -\n"
"state_0_label:\n"
  );
  if (states[0].final != c_idx_not_exist)
  {
    PUSH_FORMAT_CODE(
"   close_char(%d);\n"
      ,states[0].final);
  }
  else
  {
    PUSH_CODE(
"   close_char(c_idx_not_exist);\n"
    );
  }
  PUSH_CODE(
"\n"
"fa_start_label:\n"
  );
  PHP_PROCESS_STATE(0);

  unsigned s_idx = 1;
  do
  {
    PUSH_FORMAT_CODE(
"// - STATE %d -\n"
"state_%d_label:\n"
      ,MPAR(s_idx,s_idx));
    if (states[s_idx].final != c_idx_not_exist)
    {
      PUSH_FORMAT_CODE(
"   close_char(%d);\n"
        ,states[s_idx].final);
    }
    else
    {
      PUSH_CODE(
"   close_char(c_idx_not_exist);\n"
      );
    }
    PHP_PROCESS_STATE(s_idx);
  }
  while(++s_idx < states.used);
}
PUSH_CODE(
"}/*}}}*/\n"
"\n"
);

PUSH_CODE(
"// - parse source string -\n"
"function parser_parse_source_string()\n"
"{/*{{{*/\n"
"   global $lalr_table;\n"
"   global $source_string_length;\n"
"   global $source_string;\n"
"   global $input_idx;\n"
"\n"
"   $lalr_stack = [0,c_idx_not_exist,c_idx_not_exist];\n"
"\n"
"   $old_input_idx = 0;\n"
"   $input_idx = 0;\n"
"   $ret_term = c_idx_not_exist;\n"
"\n"
"   do {\n"
"\n"
"      while ($ret_term == c_idx_not_exist) {\n"
"         $old_input_idx = $input_idx;\n"
"\n"
"         $ret_term = recognize_terminal();\n"
"         assert($ret_term != c_idx_not_exist);\n"
);

if (skip_terminals.used > 0)
{
  PUSH_FORMAT_CODE(
"\n"
"         // - skipping of _SKIP_ terminals -\n"
"         if ($ret_term == %d"
    ,skip_terminals[0]);
  if (skip_terminals.used > 1)
  {
    unsigned st_idx = 1;
    do
    {
      PUSH_FORMAT_CODE(" || $ret_term == %d",skip_terminals[st_idx]);
    }
    while(++st_idx < skip_terminals.used);
  }
  PUSH_CODE(") {\n"
"            $ret_term = c_idx_not_exist;\n"
"         }\n"
           );
}

PUSH_CODE(
"      }\n"
"\n"
"      // - retrieve action from table of actions -\n"
"      $parse_action = $lalr_table[$lalr_stack[count($lalr_stack) - 3]*c_terminal_plus_nonterminal_cnt + $ret_term];\n"
"      assert(parse_action != c_idx_not_exist);\n"
"\n"
"      // - action SHIFT -\n"
"      if ($parse_action < c_lalr_table_reduce_base) {\n"
"\n"
);
PUSH_FORMAT_CODE(
"         // - end on _END_ terminal -\n"
"         if ($ret_term == %d) {\n"
"            break;\n"
"         }\n"
"\n"
  ,end_terminal);
PUSH_CODE(
"         // - push new state to lalr stack -\n"
"         array_push($lalr_stack,$parse_action,$old_input_idx,$input_idx);\n"
"         $ret_term = c_idx_not_exist;\n"
"      }\n"
"\n"
"      // - action REDUCE -\n"
"      else {\n"
"         $parse_action -= c_lalr_table_reduce_base;\n"
"\n"
"         // - print index of reduction rule to output -\n"
"         echo \"$parse_action, \";\n"
"\n"
"         // - remove rule body from stack -\n"
"         array_splice($lalr_stack,count($lalr_stack) - 3*rule_body_lengths[$parse_action]);\n"
"\n"
"         // - push new state to lalr stack -\n"
"         $goto_val = $lalr_table[$lalr_stack[count($lalr_stack) - 3]*c_terminal_plus_nonterminal_cnt + rule_head_idxs[$parse_action]];\n"
"         array_push($lalr_stack,$goto_val,c_idx_not_exist,c_idx_not_exist);\n"
"      }\n"
"\n"
"   } while(true);\n"
"}/*}}}*/\n"
"\n"
);

PUSH_CODE(
"if ($argc <= 1)\n"
"{\n"
"  fwrite(STDERR,\"Error: expected one argument\\n\");\n"
"  die();\n"
"}\n"
"\n"
"$source_string = file_get_contents($argv[1]);\n"
"$source_string_length = strlen($source_string);\n"
"\n"
"parser_parse_source_string();\n"
);

PUSH_CODE(
"?>\n"
"\n"
);

