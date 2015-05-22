
@begin
include "yapgen.h"
@end

const unsigned arg_option_cnt = 3;
const char *arg_option_names[arg_option_cnt] =
{
   "--parser_descr",
   "--parser_save_cc",
   "--source"
};

const unsigned c_arg_parser_descr = 0;
const unsigned c_arg_parser_save_cc = 1;
const unsigned c_arg_source = 2;

/*
 * rozpozna v argumentech ruzne vstupni a vystupni soubory programu
 */

bool parse_arguments(int argc,char **argv,unsigned *arg_file_idxs)
{/*{{{*/
  if (argc > 1)
  {
    int a_idx = 1;
    do {

      unsigned o_idx = 0;
      do {
        if (strcmp(argv[a_idx],arg_option_names[o_idx]) == 0)
        {
          if (++a_idx < argc)
          {
            arg_file_idxs[o_idx] = (unsigned)a_idx;
          }
          else
          {
            return false;
          }

          break;
        }
      } while(++o_idx < arg_option_cnt);

      if (o_idx >= arg_option_cnt)
      {
        return false;
      }
    } while(++a_idx < argc);
  }

  return true;
}/*}}}*/

int main(int argc,char **argv)
{/*{{{*/
  unsigned arg_file_idxs[6] = {0,0,0,0,0,0};

  // -- ziska indexy vst. a vystupnich souboru v argumentech programu --
  if (!parse_arguments(argc,argv,arg_file_idxs)) {
    fprintf(stderr,
        "main: bad arguments format\n"
        "arguments: --parser_descr <file>   - create parser from description file\n"
        "           --parser_save_cc <file> - save parser source in c language to file\n"
        "           --source <file>         - load and parse source file\n"
        );
    exit(0);
  }

  // -- promenne popisujici nacteny prekladac a docasne retezce --
  bool parser_exist = false;
  parser_s parser;
  parser.init();

  string_s str;
  str.init();

  // -- vygenerovani prekladace podle jeho popisu --
  if (arg_file_idxs[c_arg_parser_descr])
  {
    if (!str.load_text_file(argv[arg_file_idxs[c_arg_parser_descr]]))
    {
      fprintf(stderr,"main: --parser_descr: Cannot open file\n");
    }
    else
    {
      if (!parser.create_from_rule_string(str))
      {
        parser.print_error(str);
      }
      else
      {
        parser_exist = true;
      }

      str.clear();
    }
  }

  // -- vygenerovani a ulozeni popisu prekladace v jazyce c --
  if (arg_file_idxs[c_arg_parser_save_cc])
  {
    if (!parser_exist)
    {
      fprintf(stderr,"main: --parser_save_cc: Parser doesnt exist\n");
    }
    else
    {
      bc_array_s cc_source;
      cc_source.init();

      parser.create_cc_source(cc_source);

      FILE *f = fopen(argv[arg_file_idxs[c_arg_parser_save_cc]],"wb");
      if (f == NULL)
      {
        fprintf(stderr,"main: --parser_save_cc: Cannot save parser source to file\n");
      }
      else
      {
        if (cc_source.used != 0)
        {
          fwrite(cc_source.data,cc_source.used,1,f);
        }

        fclose(f);
      }

      cc_source.clear();
    }
  }

  // -- execute source code --
  if (arg_file_idxs[c_arg_source])
  {
    if (!str.load_text_file(argv[arg_file_idxs[c_arg_source]]))
    {
      fprintf(stderr,"main: --source: Cannot load source text\n");
    }
    else
    {
      if (!parser_exist)
      {
        fprintf(stderr,"main: --source: Parser doesnt exist\n");
      }
      else
      {
        parser_run_s parser_run;
        parser_run.init();

        if (!parser_run.create_from_parser(parser))
        {
          fprintf(stderr,"main: --source: Cannot create parser run for parser\n");
        }
        else
        {
          if (!parser_run.parse_source_string(str))
          {
            parser_run.print_error(str);
          }
        }

        parser_run.clear();
      }

      str.clear();
    }
  }

  // -- uvolneni prekladace z pameti --
  parser.clear();

  return 0;
}/*}}}*/

