
/*
 * code of function generating parser in JavaScript
 */

cc_source.clear();

const unsigned buffer_size = 2048;
char buffer[buffer_size];
const unsigned sub_buffer_size = 256;
char sub_buffer[sub_buffer_size];
unsigned chars_writed;

PUSH_CODE(
"#![allow(dead_code)]\n"
"\n"
"use std::fs::File;\n"
"use std::io::Read;\n"
"use std::io::Seek;\n"
"use std::io::SeekFrom;\n"
"\n"
"// - parse constants -\n"
"const IDX_NOT_EXIST:u32 = std::u32::MAX;\n"
"\n"
);

PUSH_FORMAT_CODE(
"const RULE_CNT:usize = %d;\n"
"const RULE_HEAD_IDXS:[u32;RULE_CNT] = ["
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
"];\n"
);

PUSH_CODE(
"const RULE_BODY_LENGTHS:[usize;RULE_CNT] = ["
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
"const BLANK:u32 = IDX_NOT_EXIST;\n"
"macro_rules! shift  { ($e:expr) => ($e) }\n"
"macro_rules! reduce { ($e:expr) => (LALR_TABLE_REDUCE_BASE + $e) }\n"
"macro_rules! goto   { ($e:expr) => ($e) }\n"
"\n"
"const LALR_TABLE_REDUCE_BASE:u32 = 0x%x;\n"
"const TERMINAL_PLUS_NONTERMINAL_CNT:u32 = %d;\n"
"const LALR_STATE_CNT:usize = %d;\n"
"\n"
"const LALR_TABLE:[u32;LALR_STATE_CNT*(TERMINAL_PLUS_NONTERMINAL_CNT as usize)] =\n"
"[/*{{{*/\n"
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
        PUSH_CODE("        BLANK,");
      }
      else if (row_ptr[col_idx] < c_lalr_table_reduce_base)
      {
        SUB_FORMAT("%d",row_ptr[col_idx]);
        PUSH_SPACES(4 - chars_writed);
        PUSH_FORMAT_CODE(" shift!(%s),",sub_buffer);
      }
      else
      {
        SUB_FORMAT("%d",row_ptr[col_idx] - c_lalr_table_reduce_base);
        PUSH_SPACES(4 - chars_writed);
        PUSH_FORMAT_CODE("reduce!(%s),",sub_buffer);
      }
    }
    while(++col_idx < terminal_cnt);

    // - write goto actions -
    do
    {
      if (row_ptr[col_idx] == c_idx_not_exist)
      {
        PUSH_CODE("        BLANK,");
      }
      else
      {
        SUB_FORMAT("%d",row_ptr[col_idx]);
        PUSH_SPACES(4 - chars_writed);
        PUSH_FORMAT_CODE("  goto!(%s),",sub_buffer);
      }
    }
    while(++col_idx < lalr_table.x_size);
    PUSH_CODE("\n");

  }
  while((row_ptr += lalr_table.x_size) < row_ptr_end);
}

PUSH_CODE(
"];/*}}}*/\n"
"\n"
);

PUSH_CODE(
"struct LalrStackElement {\n"
"    lalr_state:u32,\n"
"    terminal_start:usize,\n"
"    terminal_end:usize,\n"
"}\n"
"\n"
"impl LalrStackElement {\n"
"    fn new(state:u32,term_start:usize,term_end:usize) -> LalrStackElement {\n"
"        LalrStackElement{lalr_state:state,terminal_start:term_start,terminal_end:term_end}\n"
"    }\n"
"\n"
"    fn new_state(state:u32) -> LalrStackElement {\n"
"        LalrStackElement{lalr_state:state,terminal_start:0,terminal_end:0}\n"
"    }\n"
"}\n"
"\n"
);

PUSH_CODE(
"macro_rules! get_next_char {\n"
"    ($s:expr) => (\n"
"        $s.in_char = if $s.input_idx < $s.data.len() { $s.data[$s.input_idx] } else { 0 }\n"
"    )\n"
"}\n"
"\n"
"macro_rules! close_char {\n"
"    ($s:expr,$e:expr) => (\n"
"        if $s.in_char == 0 {\n"
"            return $e;\n"
"        }\n"
"\n"
"        $s.input_idx += 1;\n"
"    )\n"
"}\n"
"\n"
"struct TermSource<'a> {\n"
"    input_idx:usize,\n"
"    data:&'a[u8],\n"
"    in_char:u8,\n"
"    start:bool,\n"
"}\n"
"\n"
"impl<'a> TermSource<'a> {\n"
"\n"
"    fn new(source:&[u8]) -> TermSource {\n"
"        TermSource{input_idx:0,data:source,in_char:0,start:true}\n"
"    }\n"
"\n"
"    fn recognize_terminal(&mut self) -> u32 {\n"
"      self.start = true;\n"
"      self.state_0()\n"
"    }\n"
"\n"
);

#define RUST_PROCESS_STATE(STATE_IDX) \
{/*{{{*/\
  fa_state_s &state = states[STATE_IDX];\
  ui_array_s &state_moves = final_automata.state_moves[STATE_IDX];\
  \
  if (state.moves.used != 0) {\
    PUSH_CODE(\
              "        get_next_char!(self);\n"\
              "\n"\
              "        match self.in_char {\n"\
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
                               "            0x00 ... 0x%2.2x => self.state_%d(),\n"\
                               ,MPAR(e_idx - 1,target_state));\
            }\
            else {\
              PUSH_FORMAT_CODE(\
                               "            0x%2.2x ... 0x%2.2x => self.state_%d(),\n"\
                               ,MPAR(MPAR(b_idx,e_idx - 1),target_state));\
            }\
          }\
          else {\
            PUSH_FORMAT_CODE(\
                             "            0x%2.2x          => self.state_%d(),\n"\
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
                           "            0x00 ... 0x%2.2x => self.state_%d(),\n"\
                           ,MPAR(e_idx - 1,target_state));\
        }\
        else {\
          PUSH_FORMAT_CODE(\
                           "            0x%2.2x ... 0x%2.2x => self.state_%d(),\n"\
                           ,MPAR(MPAR(b_idx,e_idx - 1),target_state));\
        }\
      }\
      else {\
        PUSH_FORMAT_CODE(\
                         "            0x%2.2x          => self.state_%d(),\n"\
                         ,MPAR(b_idx,target_state));\
      }\
    }\
\
    if (state.final != c_idx_not_exist) {\
      PUSH_FORMAT_CODE(\
                       "              _           => %d\n"\
                       ,state.final);\
    }\
    else {\
      PUSH_CODE(\
                "              _           => IDX_NOT_EXIST\n"\
               );\
    }\
    PUSH_CODE(\
              "        }\n"\
             );\
  }\
  else {\
    if (state.final != c_idx_not_exist) {\
      PUSH_FORMAT_CODE(\
                       "\n"\
                       "        %d\n"\
                       ,state.final);\
    }\
    else {\
      PUSH_CODE(\
                "\n"\
                "        IDX_NOT_EXIST\n"\
               );\
    }\
  }\
}/*}}}*/

{
  fa_states_s &states = final_automata.states;

  PUSH_CODE(
"    // - STATE 0 -\n"
"    fn state_0(&mut self) -> u32\n"
"    {//{{{\n"
"        if !self.start {\n"
  );
  if (states[0].final != c_idx_not_exist)
  {
    PUSH_FORMAT_CODE(
"            close_char!(self,%d);\n"
      ,states[0].final);
  }
  else
  {
    PUSH_CODE(
"            close_char!(self,IDX_NOT_EXIST);\n"
    );
  }
  PUSH_CODE(
"        }\n"
"        else {\n"
"            self.start = false;\n"
"        }\n"
"\n"
  );
  RUST_PROCESS_STATE(0);
  PUSH_CODE(
"    }//}}}\n"
"\n"
  );

  unsigned s_idx = 1;
  do
  {
    PUSH_FORMAT_CODE(
"    // - STATE %d -\n"
"    fn state_%d(&mut self) -> u32\n"
"    {//{{{\n"
      ,MPAR(s_idx,s_idx));
    if (states[s_idx].final != c_idx_not_exist)
    {
      PUSH_FORMAT_CODE(
"        close_char!(self,%d);\n"
        ,states[s_idx].final);
    }
    else
    {
      PUSH_CODE(
"        close_char!(self,IDX_NOT_EXIST);\n"
      );
    }
    RUST_PROCESS_STATE(s_idx);
    PUSH_CODE(
"    }//}}}\n"
"\n"
  );
  }
  while(++s_idx < states.used);
}

PUSH_CODE(
"}\n"
"\n"
);

PUSH_CODE(
"// - parse source string -\n"
"fn parse_source_string(source:&[u8])\n"
"{/*{{{*/\n"
"    let mut term_source = TermSource::new(&source);\n"
"    let mut lalr_stack = vec![LalrStackElement::new_state(0)];\n"
"\n"
"    let mut old_input_idx:usize = 0;\n"
"    let mut ret_term:u32 = IDX_NOT_EXIST;\n"
"\n"
"    loop {\n"
"\n"
"        while ret_term == IDX_NOT_EXIST {\n"
"            old_input_idx = term_source.input_idx;\n"
"\n"
"            ret_term = term_source.recognize_terminal();\n"
"            assert!(ret_term != IDX_NOT_EXIST);\n"
);

if (skip_terminals.used > 0)
{
  PUSH_FORMAT_CODE(
"\n"
"            // - skipping of _SKIP_ terminals -\n"
"            if ret_term == %d"
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
  PUSH_CODE(" {\n"
"                ret_term = IDX_NOT_EXIST;\n"
"            }\n"
           );
}

PUSH_CODE(
"        }\n"
"\n"
"        // - retrieve action from table of actions -\n"
"        let mut parse_action = LALR_TABLE[(lalr_stack.last().unwrap().lalr_state*TERMINAL_PLUS_NONTERMINAL_CNT + ret_term) as usize];\n"
"        assert!(parse_action != IDX_NOT_EXIST);\n"
"\n"
"        // - action SHIFT -\n"
"        if parse_action < LALR_TABLE_REDUCE_BASE {\n"
"\n"
);
PUSH_FORMAT_CODE(
"            // - end on _END_ terminal -\n"
"            if ret_term == %d {\n"
"                break;\n"
"            }\n"
"\n"
  ,end_terminal);
PUSH_CODE(
"            // - push new state to lalr stack -\n"
"            lalr_stack.push(LalrStackElement::new(parse_action,old_input_idx,term_source.input_idx));\n"
"            ret_term = IDX_NOT_EXIST;\n"
"        }\n"
"\n"
"        // - action REDUCE -\n"
"        else {\n"
"            parse_action -= LALR_TABLE_REDUCE_BASE;\n"
"\n"
"            // - print index of reduction rule to output -\n"
"            print!(\"{}, \",parse_action);\n"
"\n"
"            // - remove rule body from stack -\n"
"            let truncate_size = lalr_stack.len() - RULE_BODY_LENGTHS[parse_action as usize];\n"
"            lalr_stack.truncate(truncate_size);\n"
"\n"
"            // - push new state to lalr stack -\n"
"            let goto_val:u32 = LALR_TABLE[(lalr_stack.last().unwrap().lalr_state*TERMINAL_PLUS_NONTERMINAL_CNT + RULE_HEAD_IDXS[parse_action as usize]) as usize];\n"
"            lalr_stack.push(LalrStackElement::new_state(goto_val));\n"
"        }\n"
"\n"
"    }\n"
"}/*}}}*/\n"
"\n"
);

// -----

PUSH_CODE(
"// - program entry function -\n"
"fn main()\n"
"{/*{{{*/\n"
"    let args: Vec<_> = std::env::args().collect();\n"
"\n"
"    let mut file = File::open(&args[1]).unwrap();\n"
"    let size = file.seek(SeekFrom::End(0)).unwrap();\n"
"    file.seek(SeekFrom::Start(0)).unwrap();\n"
"\n"
"    let mut buffer = vec![0u8; size as usize];\n"
"    file.read_exact(buffer.as_mut_slice()).unwrap();\n"
"\n"
"    parse_source_string(&buffer);\n"
"}/*}}}*/\n"
"\n"
);

