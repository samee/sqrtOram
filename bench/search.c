/* Common parts:
   * Parse server:port, connect TCP
   * Parse <linear|sqrt|ckt>
   * Initialize with zeroed/garbage ORAMs (utils)
   * Runtime reporting
   Uncommon parts:
   * Concrete inputs (test mode)
   * Input size, test count (bench mode)
   Todo:
   * Write search/bfs, refactor later
   * Tons of boilerplate, to refactor later

   ./search test :1234 -t lin 3 2 4 4 9 2
   # mode and netspec can be flipped
   # '-x' options can appear pretty much anywhere
   ./search :1234 test 3 9 2 8 3 4 -t lin
   ./search bench server:1234 --type=sqrt -z 499 -c 100
   Bench shows timing, test uses return value or one-line error.

   There should be a "mismatched params" error.
   */

#include<error.h>
#include<obliv.h>
#include<stdbool.h>
#include<stdio.h>

#include<util.h>
#include"search.h"

const char cmdUsage[] =
    "\n"
    "Usage: search test        :port <options> <haystack>\n"
    "   or: search test  server:port <options> <needles>\n"
    "   or: search bench --size=# --axcount=# :port\n"
    "   or: search bench                server:port\n"
    "\n"
    "Options:\n"
    "  -t <type>         ORAM type. One of linear, sqrt, or ckt.\n"
    "  --oramtype=<type>\n"
    "\n"
    "Test mode:\n"
    "  Runs ORAM search on manually entered data. The search is followed by\n"
    "  a plaintext  evaluation that checks correctness.\n"
    "  <haystack>        A sequence of integers. Must be sorted.\n"
    "  <needle>          A sequence of integers to search for.\n"
    "  --showResult, +o  Show output locations (default).\n"
    "  --hideResult, -o  Don't show output locations.\n"
    "\n"
    "Benchmark mode:\n"
    "  --size=n, -z n    Number of integers in autogenerated array\n"
    "  --axcount=n, -c n Number of random searches to perform\n"
    ;

bool searchCmdSizeCount(CmdParseState* s,void* vio)
{
  SearchIO* io=vio;
  return cmdParseSingleInt(s,'z',"size",&io->n,-1)
      || cmdParseSingleInt(s,'c',"axcount",&io->axc,-1);
}

int main(int argc,char *argv[])
{
  SearchIO io = {.data=NULL,.indices=NULL,.outputs=NULL,.n=-1,.axc=-1};
  ProtocolDesc pd;
  CmdParseState ps = cmdParseInit(argc,argv);
  cmdSetUsage(cmdUsage);
  cmdParseCommon(&ps,searchCmdSizeCount,&io);
  if(cmdMode==cmdModeTest)
  { if(cmdMeServing())
    { cmdParseTermInts(&ps,&io.data,&io.n);
      for(int i=1;i<io.n;++i) if(io.data[i-1]>io.data[i])
        error(-1,0,"Input data should be sorted\n");
    }
    else cmdParseTermInts(&ps,&io.indices,&io.axc);
  }
  cmdParseEnd(&ps);
  cmdConnectOrDie(&pd);
  execYaoProtocol(&pd,searchProto,&io);
  // Check if output makes sense if we are in test mode
  free(io.data); free(io.indices); free(io.outputs);
  return 0;
}
