
#ifndef __PARSER_RUN_H
#define __PARSER_RUN_H

@begin
include "parser_code.h"
@end

#ifdef __cplusplus
  extern "C" {
#endif

#include <lua5.2/lua.h>
#include <lua5.2/lualib.h>
#include <lua5.2/lauxlib.h>

#ifdef __cplusplus
  }
#endif

/*
 * global lua function callbacks
 */

int rule_body(lua_State *lua_state);

/*
 * definition of generated structures
 */

// -- parser_run_s --
@begin
struct
    <
    pointer:parser_ptr
    pointer:source_string_ptr
    lalr_stack_s:lalr_stack
    unsigned:parse_action
    error_s:error
    >
    additions
{
  bool create_from_parser(parser_s &parser);
  bool parse_source_string(string_s &source_string);
  bool print_error(string_s &source_string);
}
parser_run_s;
@end

/*
 * inline methods of generated structures
 */

// -- parser_run_s --
@begin
inlines parser_run_s
@end

#endif

