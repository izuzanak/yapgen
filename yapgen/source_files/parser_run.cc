
@begin
include "parser_run.h"
@end

/*
 * global lua function callbacks
 */

int rule_body(lua_State *lua_state)
{/*{{{*/
  
  // - retrieve parser run pointer -
  lua_getglobal(lua_state,"_parser_run");
  parser_run_s &parser_run = *((parser_run_s *)lua_touserdata(lua_state,-1));

  // - retrieve index -
  unsigned index = lua_tonumber(lua_state,1);

  parser_s &parser = *((parser_s *)parser_run.parser_ptr);
  string_s &source_string = *((string_s *)parser_run.source_string_ptr);

  // - ERROR -
  cassert(parser_run.parse_action != c_idx_not_exist);

  unsigned rule_body_size = parser.rule_descrs[parser_run.parse_action].body_size;

  // - ERROR -
  cassert(index < rule_body_size);

  // - retrieve lalr stack element -
  lalr_stack_s &lalr_stack = parser_run.lalr_stack;
  lalr_stack_element_s &lse = lalr_stack[lalr_stack.used - rule_body_size + index];
  unsigned data_end = lse.terminal_end - lse.terminal_start;
  char *data = source_string.data + lse.terminal_start;

  // - retrieve rule body string -
  char tmp_char = data[data_end];
  data[data_end] = '\0';

  // - push result to lua stack -
  lua_pushstring(lua_state,data);
  data[data_end] = tmp_char;

  return 1;
}/*}}}*/

/*
 * methods of generated structures
 */

// -- parser_run_s --
@begin
methods parser_run_s
@end

bool parser_run_s::create_from_parser(parser_s &parser)
{/*{{{*/
  clear();

  parser_ptr = &parser;

  return true;
}/*}}}*/

bool parser_run_s::parse_source_string(string_s &source_string)
{/*{{{*/
  parser_s &parser = *((parser_s *)parser_ptr);

  unsigned &end_terminal = parser.end_terminal;
  ui_array_s &skip_terminals = parser.skip_terminals;
  p_rule_descrs_s &rule_descrs = parser.rule_descrs;
  final_automata_s &final_automata = parser.final_automata;
  p_lalr_table_s &lalr_table = parser.lalr_table;

  // - set source string pointer -
  source_string_ptr = &source_string;

  // - initial setup of lalr stack -
  lalr_stack.used = 0;
  lalr_stack.push(0,0,0);

  // - initialize parse action -
  parse_action = c_idx_not_exist;

  // - create and initialize lua state -
  lua_State *lua_state = luaL_newstate();

  // - ERROR -
  if (lua_state == NULL)
  {
    error.type = c_error_PARSER_LUA_NEW_STATE_ERROR;
    return false;
  }

  // - open lua libraries -
  luaL_openlibs(lua_state);

  // - set global _parser_run -
  lua_pushlightuserdata(lua_state,this);
  lua_setglobal(lua_state,"_parser_run");

  // - set global rule_body -
  lua_pushcfunction(lua_state,rule_body);
  lua_setglobal(lua_state,"rule_body");

  // - ERROR -
  if (luaL_dostring(lua_state,parser.init_code.data))
  {
    // - close lua state -
    lua_close(lua_state);

    error.type = c_error_PARSER_LUA_DO_INIT_CODE_ERROR;
    return false;
  }

  // - variables describing state of lexical automata -
  unsigned old_input_idx = 0;
  unsigned input_idx = 0;
  unsigned input_length = source_string.size - 1;
  unsigned ret_term = c_idx_not_exist;

  do
  {
    // - retrieve next terminal from source string -
    while (ret_term == c_idx_not_exist)
    {
      old_input_idx = input_idx;

      ret_term = final_automata.recognize(source_string.data,input_idx,input_length);

      // - ERROR -
      if (ret_term == c_idx_not_exist)
      {
        // - close lua state -
        lua_close(lua_state);

        error.type = c_error_PARSER_PARSE_UNRECOGNIZED_TERMINAL;
        error.params.push(old_input_idx);

        return false;
      }

      if (skip_terminals.get_idx(ret_term) != c_idx_not_exist)
      {
        ret_term = c_idx_not_exist;
      }
    }

    // - retrieve action from action table -
    parse_action = lalr_table.value(ret_term,lalr_stack.last().lalr_state);

    // - ERROR -
    if (parse_action == c_idx_not_exist)
    {
      // - close lua state -
      lua_close(lua_state);

      error.type = c_error_PARSER_PARSE_SYNTAX_ERROR;
      error.params.push(old_input_idx);

      return false;
    }

    // - akce SHIFT -
    if (parse_action < c_lalr_table_reduce_base)
    {
      // - terminate parsing if end terminal was received -
      if (ret_term == end_terminal)
      {
        break;
      }

      // - put new state and terminal position to parser stack -
      lalr_stack.push(parse_action,old_input_idx,input_idx);

      ret_term = c_idx_not_exist;
    }

    // - akce REDUCE -
    else
    {
      parse_action -= c_lalr_table_reduce_base;

      // - ERROR -
      if (luaL_dostring(lua_state,parser.rule_codes[parse_action].data))
      {
        // - close lua state -
        lua_close(lua_state);

        error.type = c_error_PARSER_LUA_DO_RULE_CODE_ERROR;
        error.params.push(parse_action);
        return false;
      }

      p_rule_descr_s &rule_descr = rule_descrs[parse_action];

      // - retrieve nonterminal start and end -
      unsigned nonterm_start = lalr_stack[lalr_stack.used - rule_descr.body_size].terminal_start;
      unsigned nonterm_end = lalr_stack.last().terminal_end;

      lalr_stack.used -= rule_descr.body_size;

      unsigned goto_val = lalr_table.value(rule_descr.head,lalr_stack.last().lalr_state);
      lalr_stack.push(goto_val,nonterm_start,nonterm_end);
    }
  }
  while(1);

  // - close lua state -
  lua_close(lua_state);

  return true;
}/*}}}*/

bool parser_run_s::print_error(string_s &source_string)
{/*{{{*/
  switch (error.type)
  {
  case c_error_PARSER_PARSE_UNRECOGNIZED_TERMINAL:
  case c_error_PARSER_PARSE_SYNTAX_ERROR:
    {
      unsigned source_pos = error.params[0];

      fprintf(stderr," ---------------------------------------- \n");
      fprintf(stderr,"Exception: ERROR:\n");
      fprintf(stderr,"Source string on line: %u\n",source_string.get_character_line(source_pos));

      switch (error.type)
      {
      case c_error_PARSER_PARSE_UNRECOGNIZED_TERMINAL:
        print_error_show_line(source_string,source_pos);
        fprintf(stderr,"Unrecognized terminal in parsed source string\n");
        fprintf(stderr," ---------------------------------------- \n");
        break;
      case c_error_PARSER_PARSE_SYNTAX_ERROR:
        print_error_show_line(source_string,source_pos);
        fprintf(stderr,"Syntax error in parsed source string\n");
        fprintf(stderr," ---------------------------------------- \n");
        break;
      }
    }
    break;
  case c_error_PARSER_LUA_NEW_STATE_ERROR:
    fprintf(stderr," ---------------------------------------- \n");
    fprintf(stderr,"Exception: ERROR:\n");
    fprintf(stderr,"\nCannot create new LUA state\n");
    fprintf(stderr," ---------------------------------------- \n");
    break;
  case c_error_PARSER_LUA_DO_INIT_CODE_ERROR:
    fprintf(stderr," ---------------------------------------- \n");
    fprintf(stderr,"Exception: ERROR:\n");
    fprintf(stderr,"\nError while executing LUA init code\n");
    fprintf(stderr," ---------------------------------------- \n");
    break;
  case c_error_PARSER_LUA_DO_RULE_CODE_ERROR:
    fprintf(stderr," ---------------------------------------- \n");
    fprintf(stderr,"Exception: ERROR:\n");
    fprintf(stderr,"\nError while executing LUA code of rule %u\n",error.params[0]);
    fprintf(stderr," ---------------------------------------- \n");
    break;
  default:
    return false;
  }

  return true;
}/*}}}*/

