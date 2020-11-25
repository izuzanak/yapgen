
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define DEBUG_ASSERT 1

#ifdef DEBUG_ASSERT
#define debug_assert(CONDITION) assert(CONDITION)
#else
#define debug_assert(CONDITION)
#endif

#define cmalloc(SIZE) malloc(SIZE)
#define cfree(LOCATION) free(LOCATION)

// - parse constants -
const unsigned c_idx_not_exist = 0xffffffff;
const unsigned c_rule_cnt = 2;
const unsigned rule_head_idxs[c_rule_cnt] = {2, 3, };
const unsigned rule_body_lengths[c_rule_cnt] = {1, 1, };

// - lalr parse table - 
#define blank c_idx_not_exist
#define SHIFT(VALUE) VALUE
#define REDUCE(VALUE) c_lalr_table_reduce_base + VALUE
#define GOTO(VALUE) VALUE

const unsigned c_lalr_table_reduce_base = 0x80000000;
const unsigned c_terminal_plus_nonterminal_cnt = 4;
const unsigned lalr_state_cnt = 3;

const unsigned lalr_table[lalr_state_cnt*c_terminal_plus_nonterminal_cnt] = {
       blank,    SHIFT(2),       blank,     GOTO(1),
       blank,       blank,       blank,       blank,
       blank,       blank,       blank,       blank,
};

// - lalr parser stack -
const unsigned c_lalr_stack_size_add = 64;

struct lalr_stack_element_s
{
   unsigned lalr_state;
   unsigned terminal_start;
   unsigned terminal_end;
};

struct lalr_stack_s
{
   unsigned size;
   unsigned used;
   lalr_stack_element_s *data;

   inline void init();
   inline void clear();
   void copy_resize(unsigned a_size);
   inline void push(unsigned a_lalr_state);
   inline void push(unsigned a_lalr_state,unsigned a_terminal_start,unsigned a_terminal_end);
   inline lalr_stack_element_s &last();
};

inline void lalr_stack_s::init()
{
   size = 0;
   used = 0;
   data = NULL;
}

inline void lalr_stack_s::clear()
{
   if (data != NULL) {
      cfree(data);
   }

   init();
}

void lalr_stack_s::copy_resize(unsigned a_size)
{
   assert(a_size >= used);

   lalr_stack_element_s *n_data;

   if (a_size == 0) {
      n_data = NULL;
   }
   else {
      n_data = (lalr_stack_element_s *)cmalloc(a_size*sizeof(lalr_stack_element_s));
   }

   if (used != 0) {
      memcpy(n_data,data,used*sizeof(lalr_stack_element_s));
   }

   if (size != 0) {
      cfree(data);
   }

   data = n_data;
   size = a_size;
}

inline void lalr_stack_s::push(unsigned a_lalr_state)
{
   if (used >= size) {
      copy_resize(size + c_lalr_stack_size_add);
   }

   lalr_stack_element_s &target = data[used++];

   target.lalr_state = a_lalr_state;
}

inline void lalr_stack_s::push(unsigned a_lalr_state,unsigned a_terminal_start,unsigned a_terminal_end)
{
   if (used >= size) {
      copy_resize(size + c_lalr_stack_size_add);
   }

   lalr_stack_element_s &target = data[used++];

   target.lalr_state = a_lalr_state;
   target.terminal_start = a_terminal_start;
   target.terminal_end = a_terminal_end;
}

inline lalr_stack_element_s &lalr_stack_s::last()
{
   debug_assert(used != 0);
   return data[used - 1];
}

// - global variables -
unsigned source_string_length;
char *source_string;

lalr_stack_s lalr_stack;
unsigned parse_action;

// - recognize next terminal - 
unsigned recognize_terminal(unsigned &input_idx)
{
#define GET_NEXT_CHAR() \
{\
   if (input_idx < source_string_length) {\
      in_char = (unsigned char)source_string[input_idx];\
   }\
   else {\
      in_char = '\0';\
   }\
}

#define CLOSE_CHAR(RET_TERM_IDX) \
{\
   if (in_char == '\0') {\
      return RET_TERM_IDX;\
   }\
\
   input_idx++;\
}

   unsigned short in_char;
   goto fa_start_label;

// - STATE 0 -
state_0_label:
   CLOSE_CHAR(c_idx_not_exist);

fa_start_label:
   GET_NEXT_CHAR();

   if (in_char == 0)
      goto state_1_label;

   if (in_char >= 9 && in_char < 11)
      goto state_2_label;

   if (in_char == 32)
      goto state_2_label;

   return c_idx_not_exist;

// - STATE 1 -
state_1_label:
   CLOSE_CHAR(1);
   return 1;

// - STATE 2 -
state_2_label:
   CLOSE_CHAR(0);
   GET_NEXT_CHAR();

   if (in_char >= 9 && in_char < 11)
      goto state_2_label;

   if (in_char == 32)
      goto state_2_label;

   return 0;

}

// - parse source string -
void parser_parse_source_string()
{
   lalr_stack.used = 0;
   lalr_stack.push(0);

   unsigned old_input_idx = 0;
   unsigned input_idx = 0;
   unsigned ret_term = c_idx_not_exist;

   do {

      while (ret_term == c_idx_not_exist) {
         old_input_idx = input_idx;

         ret_term = recognize_terminal(input_idx);
         assert(ret_term != c_idx_not_exist);

         // - skipping of _SKIP_ terminals -
         if (ret_term == 0) {
            ret_term = c_idx_not_exist;
         }
      }

      // - retrieve action from table of actions -
      parse_action = lalr_table[lalr_stack.last().lalr_state*c_terminal_plus_nonterminal_cnt + ret_term];
      assert(parse_action != c_idx_not_exist);

      // - action SHIFT -
      if (parse_action < c_lalr_table_reduce_base) {

         // - end on _END_ terminal -
         if (ret_term == 1) {
            break;
         }

         // - push new state to lalr stack -
         lalr_stack.push(parse_action,old_input_idx,input_idx);
         ret_term = c_idx_not_exist;
      }

      // - action REDUCE -
      else {
         parse_action -= c_lalr_table_reduce_base;

         // - print index of reduction rule to output -
         printf("%d, ",parse_action);

         // - remove rule body from stack -
         lalr_stack.used -= rule_body_lengths[parse_action];

         // - push new state to lalr stack -
         unsigned goto_val = lalr_table[lalr_stack.last().lalr_state*c_terminal_plus_nonterminal_cnt + rule_head_idxs[parse_action]];
         lalr_stack.push(goto_val);
      }

   } while(1);

   lalr_stack.clear();
}

// - program entry function -
int main(int argc,char **argv)
{
   // - initialize global variables -
   source_string_length = 0;
   source_string = NULL;
   lalr_stack.init();

   // - read source string -
   assert(argc > 1 && argv[1] != NULL);

   FILE *f = fopen(argv[1],"rb");
   assert(f != NULL);

   fseek(f,0,SEEK_END);
   source_string_length = ftell(f);
   fseek(f,0,SEEK_SET);

   source_string = (char *)cmalloc((source_string_length + 1)*sizeof(char));
   fread(source_string,source_string_length,1,f);
   source_string[source_string_length] = '\0';

   fclose(f);

   // - parse source string -
   parser_parse_source_string();

   // - release global variables -
   if (source_string != NULL) {
     cfree(source_string);
     source_string = NULL;
   }

   lalr_stack.clear();

   return 0;
}

