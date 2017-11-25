
/*
 * code of function generating parser in language C
 */

cc_source.clear();

const unsigned buffer_size = 2048;
char buffer[buffer_size];
const unsigned sub_buffer_size = 256;
char sub_buffer[sub_buffer_size];
unsigned chars_writed;

PUSH_CODE(
"\n"
"#include <stdlib.h>\n"
"#include <stdio.h>\n"
"#include <assert.h>\n"
"#include <string.h>\n"
"\n"
);

PUSH_CODE(
"#define DEBUG_ASSERT 1\n"
"\n"
"#ifdef DEBUG_ASSERT\n"
"#define debug_assert(CONDITION) assert(CONDITION)\n"
"#else\n"
"#define debug_assert(CONDITION)\n"
"#endif\n"
"\n"
"#define cmalloc(SIZE) malloc(SIZE)\n"
"#define cfree(LOCATION) free(LOCATION)\n"
"\n"
);

PUSH_CODE(
"// - parse constants -\n"
"const unsigned c_idx_not_exist = 0xffffffff;\n"
);

PUSH_FORMAT_CODE(
"const unsigned c_rule_cnt = %d;\n"
"const unsigned rule_head_idxs[c_rule_cnt] = {"
  ,rule_descrs.used);

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
"};\n"
);

PUSH_CODE(
"const unsigned rule_body_lengths[c_rule_cnt] = {"
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
"};\n"
"\n"
);

PUSH_FORMAT_CODE(
"// - lalr parse table - \n"
"#define blank c_idx_not_exist\n"
"#define SHIFT(VALUE) VALUE\n"
"#define REDUCE(VALUE) c_lalr_table_reduce_base + VALUE\n"
"#define GOTO(VALUE) VALUE\n"
"\n"
"const unsigned c_lalr_table_reduce_base = 0x%x;\n"
"const unsigned c_terminal_plus_nonterminal_cnt = %d;\n"
"const unsigned lalr_state_cnt = %d;\n"
"\n"
"const unsigned lalr_table[lalr_state_cnt*c_terminal_plus_nonterminal_cnt] =\n"
"{/*{{{*/\n"
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
        SUB_FORMAT("%d",row_ptr[col_idx]);
        PUSH_SPACES(4 - chars_writed);
        PUSH_FORMAT_CODE(" SHIFT(%s),",sub_buffer);
      }
      else
      {
        SUB_FORMAT("%d",row_ptr[col_idx] - c_lalr_table_reduce_base);
        PUSH_SPACES(4 - chars_writed);
        PUSH_FORMAT_CODE("REDUCE(%s),",sub_buffer);
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
        SUB_FORMAT("%d",row_ptr[col_idx]);
        PUSH_SPACES(4 - chars_writed);
        PUSH_FORMAT_CODE("  GOTO(%s),",sub_buffer);
      }
    }
    while(++col_idx < lalr_table.x_size);
    PUSH_CODE("\n");

  }
  while((row_ptr += lalr_table.x_size) < row_ptr_end);
}

PUSH_CODE(
"};/*}}}*/\n"
"\n"
);

PUSH_CODE(
"// - lalr parser stack -\n"
"const unsigned c_lalr_stack_size_add = 64;\n"
"\n"
"struct lalr_stack_element_s\n"
"{/*{{{*/\n"
"   unsigned lalr_state;\n"
"   unsigned terminal_start;\n"
"   unsigned terminal_end;\n"
"};/*}}}*/\n"
"\n"
"struct lalr_stack_s\n"
"{/*{{{*/\n"
"   unsigned size;\n"
"   unsigned used;\n"
"   lalr_stack_element_s *data;\n"
"\n"
"   inline void init();\n"
"   inline void clear();\n"
"   void copy_resize(unsigned a_size);\n"
"   inline void push(unsigned a_lalr_state);\n"
"   inline void push(unsigned a_lalr_state,unsigned a_terminal_start,unsigned a_terminal_end);\n"
"   inline lalr_stack_element_s &last();\n"
"};/*}}}*/\n"
"\n"
"inline void lalr_stack_s::init()\n"
"{/*{{{*/\n"
"   size = 0;\n"
"   used = 0;\n"
"   data = NULL;\n"
"}/*}}}*/\n"
"\n"
"inline void lalr_stack_s::clear()\n"
"{/*{{{*/\n"
"   if (data != NULL) {\n"
"      cfree(data);\n"
"   }\n"
"\n"
"   init();\n"
"}/*}}}*/\n"
"\n"
"void lalr_stack_s::copy_resize(unsigned a_size)\n"
"{/*{{{*/\n"
"   assert(a_size >= used);\n"
"\n"
"   lalr_stack_element_s *n_data;\n"
"\n"
"   if (a_size == 0) {\n"
"      n_data = NULL;\n"
"   }\n"
"   else {\n"
"      n_data = (lalr_stack_element_s *)cmalloc(a_size*sizeof(lalr_stack_element_s));\n"
"   }\n"
"\n"
"   if (used != 0) {\n"
"      memcpy(n_data,data,used*sizeof(lalr_stack_element_s));\n"
"   }\n"
"\n"
"   if (size != 0) {\n"
"      cfree(data);\n"
"   }\n"
"\n"
"   data = n_data;\n"
"   size = a_size;\n"
"}/*}}}*/\n"
"\n"
"inline void lalr_stack_s::push(unsigned a_lalr_state)\n"
"{/*{{{*/\n"
"   if (used >= size) {\n"
"      copy_resize(size + c_lalr_stack_size_add);\n"
"   }\n"
"\n"
"   lalr_stack_element_s &target = data[used++];\n"
"\n"
"   target.lalr_state = a_lalr_state;\n"
"}/*}}}*/\n"
"\n"
"inline void lalr_stack_s::push(unsigned a_lalr_state,unsigned a_terminal_start,unsigned a_terminal_end)\n"
"{/*{{{*/\n"
"   if (used >= size) {\n"
"      copy_resize(size + c_lalr_stack_size_add);\n"
"   }\n"
"\n"
"   lalr_stack_element_s &target = data[used++];\n"
"\n"
"   target.lalr_state = a_lalr_state;\n"
"   target.terminal_start = a_terminal_start;\n"
"   target.terminal_end = a_terminal_end;\n"
"}/*}}}*/\n"
"\n"
"inline lalr_stack_element_s &lalr_stack_s::last()\n"
"{/*{{{*/\n"
"   debug_assert(used != 0);\n"
"   return data[used - 1];\n"
"}/*}}}*/\n"
"\n"
);

PUSH_CODE(
"// - global variables -\n"
"unsigned source_string_length;\n"
"char *source_string;\n"
"\n"
"lalr_stack_s lalr_stack;\n"
"unsigned parse_action;\n"
"\n"
);

PUSH_CODE(
"// - recognize next terminal - \n"
"unsigned recognize_terminal(unsigned &input_idx)\n"
"{/*{{{*/\n"
);

#define CC_PROCESS_STATE(STATE_IDX) \
  {/*{{{*/\
    fa_state_s &state = states[STATE_IDX];\
    ui_array_s &state_moves = final_automata.state_moves[STATE_IDX];\
    \
    if (state.moves.used != 0) {\
      PUSH_CODE(\
                "   GET_NEXT_CHAR();\n"\
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
                                 "   if (in_char < %d)\n"\
                                 "      goto state_%d_label;\n"\
                                 "\n"\
                                 ,MPAR(e_idx,target_state));\
              }\
              else {\
                PUSH_FORMAT_CODE(\
                                 "   if (in_char >= %d && in_char < %d)\n"\
                                 "      goto state_%d_label;\n"\
                                 "\n"\
                                 ,MPAR(MPAR(b_idx,e_idx),target_state));\
              }\
            }\
            else {\
              PUSH_FORMAT_CODE(\
                               "   if (in_char == %d)\n"\
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
                             "   if (in_char < %d)\n"\
                             "      goto state_%d_label;\n"\
                             "\n"\
                             ,MPAR(e_idx,target_state));\
          }\
          else {\
            if (e_idx < 256) {\
              PUSH_FORMAT_CODE(\
                               "   if (in_char >= %d && in_char < %d)\n"\
                               "      goto state_%d_label;\n"\
                               "\n"\
                               ,MPAR(MPAR(b_idx,e_idx),target_state));\
            }\
            else {\
              cassert(e_idx == 256);\
              PUSH_FORMAT_CODE(\
                               "   if (in_char >= %d)\n"\
                               "      goto state_%d_label;\n"\
                               "\n"\
                               ,MPAR(b_idx,target_state));\
            }\
          }\
        }\
        else {\
          PUSH_FORMAT_CODE(\
                           "   if (in_char == %d)\n"\
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
"#define GET_NEXT_CHAR() \\\n"
"{\\\n"
"   if (input_idx < source_string_length) {\\\n"
"      in_char = (unsigned char)source_string[input_idx];\\\n"
"   }\\\n"
"   else {\\\n"
"      in_char = '\\0';\\\n"
"   }\\\n"
"}\n"
"\n"
"#define CLOSE_CHAR(RET_TERM_IDX) \\\n"
"{\\\n"
"   if (in_char == '\\0') {\\\n"
"      return RET_TERM_IDX;\\\n"
"   }\\\n"
"\\\n"
"   input_idx++;\\\n"
"}\n"
"\n"
"   unsigned char in_char;\n"
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
"   CLOSE_CHAR(%d);\n"
      ,states[0].final);
  }
  else
  {
    PUSH_CODE(
"   CLOSE_CHAR(c_idx_not_exist);\n"
    );
  }
  PUSH_CODE(
"\n"
"fa_start_label:\n"
  );
  CC_PROCESS_STATE(0);

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
"   CLOSE_CHAR(%d);\n"
        ,states[s_idx].final);
    }
    else
    {
      PUSH_CODE(
"   CLOSE_CHAR(c_idx_not_exist);\n"
      );
    }
    CC_PROCESS_STATE(s_idx);
  }
  while(++s_idx < states.used);
}
PUSH_CODE(
"}/*}}}*/\n"
"\n"
);

PUSH_CODE(
"// - parse source string -\n"
"void parser_parse_source_string()\n"
"{/*{{{*/\n"
"   lalr_stack.used = 0;\n"
"   lalr_stack.push(0);\n"
"\n"
"   unsigned old_input_idx = 0;\n"
"   unsigned input_idx = 0;\n"
"   unsigned ret_term = c_idx_not_exist;\n"
"\n"
"   do {\n"
"\n"
"      while (ret_term == c_idx_not_exist) {\n"
"         old_input_idx = input_idx;\n"
"\n"
"         ret_term = recognize_terminal(input_idx);\n"
"         assert(ret_term != c_idx_not_exist);\n"
);

if (skip_terminals.used > 0)
{
  PUSH_FORMAT_CODE(
"\n"
"         // - skipping of _SKIP_ terminals -\n"
"         if (ret_term == %d"
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
"            ret_term = c_idx_not_exist;\n"
"         }\n"
           );
}

PUSH_CODE(
"      }\n"
"\n"
"      // - retrieve action from table of actions -\n"
"      parse_action = lalr_table[lalr_stack.last().lalr_state*c_terminal_plus_nonterminal_cnt + ret_term];\n"
"      assert(parse_action != c_idx_not_exist);\n"
"\n"
"      // - action SHIFT -\n"
"      if (parse_action < c_lalr_table_reduce_base) {\n"
"\n"
);
PUSH_FORMAT_CODE(
"         // - end on _END_ terminal -\n"
"         if (ret_term == %d) {\n"
"            break;\n"
"         }\n"
"\n"
  ,end_terminal);
PUSH_CODE(
"         // - push new state to lalr stack -\n"
"         lalr_stack.push(parse_action,old_input_idx,input_idx);\n"
"         ret_term = c_idx_not_exist;\n"
"      }\n"
"\n"
"      // - action REDUCE -\n"
"      else {\n"
"         parse_action -= c_lalr_table_reduce_base;\n"
"\n"
"         // - print index of reduction rule to output -\n"
"         printf(\"%d, \",parse_action);\n"
"\n"
"         // - remove rule body from stack -\n"
"         lalr_stack.used -= rule_body_lengths[parse_action];\n"
"\n"
"         // - push new state to lalr stack -\n"
"         unsigned goto_val = lalr_table[lalr_stack.last().lalr_state*c_terminal_plus_nonterminal_cnt + rule_head_idxs[parse_action]];\n"
"         lalr_stack.push(goto_val);\n"
"      }\n"
"\n"
"   } while(1);\n"
"\n"
"   lalr_stack.clear();\n"
"}/*}}}*/\n"
"\n"
);

PUSH_CODE(
"// - program entry function -\n"
"int main(int argc,char **argv)\n"
"{/*{{{*/\n"
"   // - initialize global variables -\n"
"   source_string_length = 0;\n"
"   source_string = NULL;\n"
"   lalr_stack.init();\n"
"\n"
"   // - read source string -\n"
"   assert(argc > 1 && argv[1] != NULL);\n"
"\n"
"   FILE *f = fopen(argv[1],\"rb\");\n"
"   assert(f != NULL);\n"
"\n"
"   fseek(f,0,SEEK_END);\n"
"   source_string_length = ftell(f);\n"
"   fseek(f,0,SEEK_SET);\n"
"\n"
"   source_string = (char *)cmalloc((source_string_length + 1)*sizeof(char));\n"
"   fread(source_string,source_string_length,1,f);\n"
"   source_string[source_string_length] = \'\\0\';\n"
"\n"
"   fclose(f);\n"
"\n"
"   // - parse source string -\n"
"   parser_parse_source_string();\n"
"\n"
"   // - release global variables -\n"
"   if (source_string != NULL) {\n"
"     cfree(source_string);\n"
"     source_string = NULL;\n"
"   }\n"
"\n"
"   lalr_stack.clear();\n"
"\n"
"   return 0;\n"
"}/*}}}*/\n"
"\n"
);

