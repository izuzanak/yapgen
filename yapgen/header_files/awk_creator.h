
/*
 * code of function generating parser in language C
 */

cc_source.clear();

const int buffer_size = 2048;
char buffer[buffer_size];
int chars_writed;

PUSH_CODE(
"\n"
"function _init_ord_()\n"
"{#{{{\n"
"  for (value = 0;value <= 127;value++)\n"
"  {\n"
"    _ord_[sprintf(\"%c\",value)] = value\n"
"  }\n"
"}#}}}\n"
"\n"
);

PUSH_CODE(
"function _init_lalr_( rhi_init,rbl_init,lalr_init,rhi_length,rbl_length,lalr_length,rhi,rbl,lalr)\n"
"{#{{{\n"
);

PUSH_CODE(
"  rhi_init = \""
);

if (rule_descrs.used != 0)
{
  p_rule_descr_s *rd_ptr = rule_descrs.data;
  p_rule_descr_s *rd_ptr_end = rd_ptr + rule_descrs.used;

  do
  {
    PUSH_FORMAT_CODE("%d %d ",MPAR(rd_ptr - rule_descrs.data,rd_ptr->head));
  }
  while(++rd_ptr < rd_ptr_end);
}

PUSH_CODE(
"\"\n"
"  rbl_init = \""
);

if (rule_descrs.used != 0)
{
  p_rule_descr_s *rd_ptr = rule_descrs.data;
  p_rule_descr_s *rd_ptr_end = rd_ptr + rule_descrs.used;

  do
  {
    PUSH_FORMAT_CODE("%d %d ",MPAR(rd_ptr - rule_descrs.data,rd_ptr->body_size));
  }
  while(++rd_ptr < rd_ptr_end);
}

PUSH_CODE(
"\"\n"
"  lalr_init = \""
);

unsigned lalr_init_length = 0;

{
  unsigned *row_ptr = lalr_table.table.data;
  unsigned *row_ptr_end = row_ptr + lalr_table.y_size*lalr_table.x_size;
  unsigned table_idx = 0;

  do
  {
    unsigned col_idx = 0;

    // - write shift/reduce actions -
    do
    {
      if (row_ptr[col_idx] != c_idx_not_exist)
      {
        PUSH_FORMAT_CODE("%u %u ",MPAR(table_idx,row_ptr[col_idx]));
        lalr_init_length += 2;
      }
    }
    while(++table_idx,++col_idx < terminal_cnt);

    // - write goto actions -
    do
    {
      if (row_ptr[col_idx] != c_idx_not_exist)
      {
        PUSH_FORMAT_CODE("%u %u ",MPAR(table_idx,row_ptr[col_idx]));
        lalr_init_length += 2;
      }
    }
    while(++table_idx,++col_idx < lalr_table.x_size);
  }
  while((row_ptr += lalr_table.x_size) < row_ptr_end);
}

PUSH_CODE(
"\"\n"
"\n"
);

PUSH_FORMAT_CODE(
"  split(rhi_init,rhi)\n"
"  for (rhi_idx = 1;rhi_idx <= %d;rhi_idx += 2)\n"
"  {\n"
"    rule_head_idxs[rhi[rhi_idx]] = rhi[rhi_idx + 1]\n"
"  }\n"
"\n"
,rule_descrs.used << 1);

PUSH_FORMAT_CODE(
"  split(rbl_init,rbl)\n"
"  for (rbl_idx = 1;rbl_idx <= %d;rbl_idx += 2)\n"
"  {\n"
"    rule_body_lengths[rbl[rbl_idx]] = rbl[rbl_idx + 1]\n"
"  }\n"
"\n"
,rule_descrs.used << 1);

PUSH_FORMAT_CODE(
"  split(lalr_init,lalr)\n"
"  for (lalr_idx = 1;lalr_idx <= %d;lalr_idx += 2)\n"
"  {\n"
"    lalr_table[lalr[lalr_idx]] = lalr[lalr_idx + 1]\n"
"  }\n"
,lalr_init_length);

PUSH_CODE(
"}#}}}\n"
"\n"
);

PUSH_CODE(
"function next_char()\n"
"{#{{{\n"
"  if (input_idx <= source_length) {\n"
"    in_char = _ord_[source_chars[input_idx]]\n"
"  }\n"
"  else {\n"
"    in_char = 0\n"
"  }\n"
"}#}}}\n"
"\n"
);

#define AWK_PROCESS_STATE(STATE_IDX) \
{/*{{{*/\
  fa_state_s &state = states[STATE_IDX];\
  ui_array_s &state_moves = final_automata.state_moves[STATE_IDX];\
  \
  if (state.moves.used != 0) {\
    PUSH_CODE(\
              "  next_char()\n"\
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
                               "  if (in_char < %d)\n"\
                               "    return state_%d()\n"\
                               ,MPAR(e_idx,target_state));\
            }\
            else {\
              PUSH_FORMAT_CODE(\
                               "\n"\
                               "  if (in_char >= %d && in_char < %d)\n"\
                               "    return state_%d()\n"\
                               ,MPAR(MPAR(b_idx,e_idx),target_state));\
            }\
          }\
          else {\
            PUSH_FORMAT_CODE(\
                             "\n"\
                             "  if (in_char == %d)\n"\
                             "    return state_%d()\n"\
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
                           "  if (in_char < %d)\n"\
                           "    return state_%d()\n"\
                           ,MPAR(e_idx,target_state));\
        }\
        else {\
          PUSH_FORMAT_CODE(\
                           "\n"\
                           "  if (in_char >= %d && in_char < %d)\n"\
                           "    return state_%d()\n"\
                           ,MPAR(MPAR(b_idx,e_idx),target_state));\
        }\
      }\
      else {\
        PUSH_FORMAT_CODE(\
                         "\n"\
                         "  if (in_char == %d)\n"\
                         "    return state_%d()\n"\
                         ,MPAR(b_idx,target_state));\
      }\
    }\
  }\
  \
  if (state.final != c_idx_not_exist) {\
    PUSH_FORMAT_CODE(\
                     "\n"\
                     "  return %d\n"\
                     ,state.final);\
  }\
  else {\
    PUSH_CODE(\
              "\n"\
              "  return c_idx_not_exist\n"\
             );\
  }\
}/*}}}*/

{
  fa_states_s &states = final_automata.states;

  PUSH_CODE(
"# - STATE 0 -\n"
"function state_0( a_no_close)\n"
"{#{{{\n"
"  if (a_no_close == \"\") {\n"
  );
  if (states[0].final != c_idx_not_exist)
  {
    PUSH_FORMAT_CODE(
"    if (in_char == 0)\n"
"      return %d\n"
"\n"
"    ++input_idx\n"
      ,states[0].final);
  }
  else
  {
    PUSH_CODE(
"    if (in_char == 0)\n"
"      return c_idx_not_exist\n"
"\n"
"    ++input_idx\n"
      );
  }
  PUSH_CODE(
"  }\n"
"\n"
  );
  AWK_PROCESS_STATE(0);
  PUSH_CODE(
"}#}}}\n"
"\n"
  );

  unsigned s_idx = 1;
  do
  {
    PUSH_FORMAT_CODE(
"# - STATE %d -\n"
"function state_%d()\n"
"{#{{{\n"
      ,MPAR(s_idx,s_idx));
    if (states[s_idx].final != c_idx_not_exist)
    {
      PUSH_FORMAT_CODE(
"  if (in_char == 0)\n"
"    return %d\n"
"\n"
"  ++input_idx\n"
        ,states[s_idx].final);
    }
    else
    {
      PUSH_CODE(
"  if (in_char == 0)\n"
"    return c_idx_not_exist\n"
"\n"
"  ++input_idx\n"
      );
    }
    AWK_PROCESS_STATE(s_idx);
  PUSH_CODE(
"}#}}}\n"
"\n"
  );
  }
  while(++s_idx < states.used);
}

PUSH_CODE(
"# - parse source string -\n"
"function parse_source( lalr_stack_depth,lalr_stack,old_input_idx,ret_term,parse_action)\n"
"{#{{{\n"
"  lalr_stack_depth = 0\n"
"\n"
"  lalr_stack[lalr_stack_depth++] = 0\n"
"  lalr_stack[lalr_stack_depth++] = c_idx_not_exist\n"
"  lalr_stack[lalr_stack_depth++] = c_idx_not_exist\n"
"\n"
"  old_input_idx = 1\n"
"  input_idx = 1\n"
"  ret_term = c_idx_not_exist\n"
"\n"
"  do {\n"
"\n"
"    while (ret_term == c_idx_not_exist) {\n"
"      old_input_idx = input_idx\n"
"\n"
"      ret_term = state_0(1)\n"
"      if (ret_term == c_idx_not_exist)\n"
"      {\n"
"        print \"Unrecognized terminal\" > \"/dev/stderr\"\n"
"        exit 1\n"
"      }\n"
);

if (skip_terminals.used > 0)
{
  PUSH_FORMAT_CODE(
"\n"
"      # - skipping of _SKIP_ terminals -\n"
"      if (ret_term == %d"
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
"        ret_term = c_idx_not_exist\n"
"      }\n"
           );
}

PUSH_CODE(
"    }\n"
"\n"
"    # - retrieve action from table of actions -\n"
"    parse_action = lalr_table[lalr_stack[lalr_stack_depth - 3]*c_terminal_plus_nonterminal_cnt + ret_term]\n"
"    if (parse_action == c_idx_not_exist)\n"
"    {\n"
"      print \"Undefined parse action\" > \"/dev/stderr\"\n"
"      exit 1\n"
"    }\n"
"\n"
"    # - action SHIFT -\n"
"    if (parse_action < c_lalr_table_reduce_base) {\n"
"\n"
);
PUSH_FORMAT_CODE(
"      # - end on _END_ terminal -\n"
"      if (ret_term == %d) {\n"
"        break\n"
"      }\n"
"\n"
  ,end_terminal);
PUSH_CODE(
"      # - push new state to lalr stack -\n"
"      lalr_stack[lalr_stack_depth++] = parse_action\n"
"      lalr_stack[lalr_stack_depth++] = old_input_idx\n"
"      lalr_stack[lalr_stack_depth++] = input_idx\n"
"\n"
"      ret_term = c_idx_not_exist\n"
"    }\n"
"\n"
"    # - action REDUCE -\n"
"    else {\n"
"      parse_action -= c_lalr_table_reduce_base\n"
"\n"
"      # - print index of reduction rule to output -\n"
"      printf(\"%d, \",parse_action)\n"
"\n"
"      # - remove rule body from stack -\n"
"      rb_length = rule_body_lengths[parse_action]\n"
"      lalr_stack_depth -= 3*rb_length\n"
"\n"
"      # - push new state to lalr stack -\n"
"      goto_val = lalr_table[lalr_stack[lalr_stack_depth - 3]*c_terminal_plus_nonterminal_cnt + rule_head_idxs[parse_action]]\n"
"\n"
"      lalr_stack[lalr_stack_depth++] = goto_val\n"
"      lalr_stack[lalr_stack_depth++] = c_idx_not_exist\n"
"      lalr_stack[lalr_stack_depth++] = c_idx_not_exist\n"
"    }\n"
"\n"
"  } while(1)\n"
"}#}}}\n"
"\n"
);

PUSH_CODE(
"BEGIN {\n"
"  # - set RS to something unlikely -\n"
"  RS = \"\\v\"\n"
"\n"
"  # - source array index -\n"
"  source_idx = 1\n"
"\n"
"  # - initialize _ord_ array -\n"
"  _init_ord_()\n"
"\n"
"  # - parse constants -\n"
"  c_idx_not_exist = \"\"\n"
"\n"
);

PUSH_FORMAT_CODE(
"  # - lalr parse table - \n"
"  c_lalr_table_reduce_base = 0x%x\n"
"  c_terminal_plus_nonterminal_cnt = %d\n"
"  lalr_state_cnt = %d\n"
"\n"
"  # - initialize lalr parse table - \n"
"  _init_lalr_()\n"
  ,MPAR(MPAR(c_lalr_table_reduce_base,lalr_table.x_size),lalr_table.y_size));

PUSH_CODE(
"}\n"
"\n"
);

PUSH_CODE(
"{\n"
"  # - construct source array -\n"
"  source_array[source_idx++] = $0\n"
"}\n"
"\n"
);

PUSH_CODE(
"END {\n"
"  # - concatenate source array to source -\n"
"  source_string = source_array[1]\n"
"  for (i = 2;i < source_idx;++i)\n"
"  {\n"
"    source_string = source_string RS source_array[i]\n"
"  }\n"
"\n"
"  # - retrieve source length -\n"
"  source_length = length(source_string)\n"
"\n"
"  # - retrieve source characters -\n"
"  split(source_string,source_chars,\"\")\n"
"\n"
"  # - parse source string -\n"
"  parse_source()\n"
"}\n"
"\n"
);

