
@begin
include "parser_run.h"
@end

/*
 * methods of generated structures
 */

// -- parser_run_s --
@begin
methods parser_run_s
@end

bool parser_run_s::create_from_parser(parser_s &a_parser)
{/*{{{*/
  clear();

  parser_ptr = &a_parser;

  return true;
}/*}}}*/

bool parser_run_s::parse_source_string(string_s &a_source_string)
{/*{{{*/
  parser_s &parser = *((parser_s *)parser_ptr);

  unsigned &end_terminal = parser.end_terminal;
  ui_array_s &skip_terminals = parser.skip_terminals;
  p_rule_descrs_s &rule_descrs = parser.rule_descrs;
  final_automata_s &final_automata = parser.final_automata;
  p_lalr_table_s &lalr_table = parser.lalr_table;

  // - create and initialize lua state -
  lua_State *lua_state = luaL_newstate();

  // FIXME TODO process error ...
  cassert(lua_state != NULL);

  // - open lua libraries -
  luaL_openlibs(lua_state);

  // - run init semantic code -
  luaL_dostring(lua_state,parser.init_code.data);

  // - vychozi nastaveni lalr_stavoveho zasobniku -
  lalr_stack.used = 0;
  lalr_stack.push(0,0,0);

  // - promenne popisujici stav konecneho lexikalniho automatu -
  unsigned old_input_idx = 0;
  unsigned input_idx = 0;
  unsigned input_length = a_source_string.size - 1;
  unsigned ret_term = c_idx_not_exist;

  do
  {
    // - rozpoznani dalsiho terminalu na vstupu -
    while (ret_term == c_idx_not_exist)
    {
      old_input_idx = input_idx;

      ret_term = final_automata.recognize(a_source_string.data,input_idx,input_length);

      // - ERROR -
      if (ret_term == c_idx_not_exist)
      {
        // - close lua state -
        lua_close(lua_state);

        /*c_error_PARSER_PARSE_UNRECOGNIZED_TERMINAL*/
        /*old_input_idx*/
        cassert(0);

        return false;
      }

      if (skip_terminals.get_idx(ret_term) != c_idx_not_exist)
      {
        ret_term = c_idx_not_exist;
      }
    }

    // - nalezeni akce v tabulce akci -
    unsigned parse_action = lalr_table.value(ret_term,lalr_stack.last().lalr_state);

    // - ERROR -
    if (parse_action == c_idx_not_exist)
    {
      // - close lua state -
      lua_close(lua_state);

      /*c_error_PARSER_PARSE_SYNTAX_ERROR*/
      /*old_input_idx*/
      cassert(0);

      return false;
    }

    // - akce SHIFT -
    if (parse_action < c_lalr_table_reduce_base)
    {
      // - kdyz byl prijmut koncovy terminal ukonci rozklad -
      if (ret_term == end_terminal)
      {
        break;
      }

      // - vlozi na zasobnik novy stav a pozici terminalu ve zdrojovem retezci -
      lalr_stack.push(parse_action,old_input_idx,input_idx);

      ret_term = c_idx_not_exist;
    }

    // - akce REDUCE -
    else
    {
      parse_action -= c_lalr_table_reduce_base;

      // - run rule semantic code -
      luaL_dostring(lua_state,parser.rule_codes[parse_action].data);

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

