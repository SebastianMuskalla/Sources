/*
*	mcast-put-tree.c - put the MP trees found in the input file to several
*                          slaves (mcast-get-tree) each of which prints the 
*                          tree to a local file and returns its id indicating 
*                          it is done. 
* 
*	See also mcast-get-tree.c
*/

#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include "pvm3.h"
#include "gmp.h"
#include "MP.h"
#include "MP_PvmTransp.h"

EXTERN MP_TranspOps_t pvm_ops;

#define ENCODING  PvmDataRaw
#define SLAVENAME "mcast-get-tree"

#include "pvm_imp_getdata.c" 

/**********************************************************************
 * FUNCTION: imp_file2tree
 * ARGUMENT: link - pointer to a stream structure
 *           fname - If not empty, this is a string giving the name 
 *                   of the input file (as given from the command line).  
 *                   If the string is empty, the routine asks for a
 *                   filename.  If the filename cannot be opened for
 *                   reading we print an error message and croak.
 *           stid  - tids to which to send the data.
 * RETURN:   None.
 * PURPOSE:  Read successive trees from an input file and send them to
 *           the data stream associated with link.  
 *
 * COMMENT:  Note that MP_EndMsg() is called after writing each
 *           tree to the buffer.  This flushes the buffer to the 
 *           network and sets the last fragment bit in the buffer 
 *           header.  The receiver must do a corrsponding m_skiprecord()
 *           PRIOR to retrieving EACH tree from the data stream.
 **********************************************************************/
void
#ifdef __STDC__
imp_file2tree(MP_Link_pt link, char *fname, int *stid )
#else
imp_file2tree(link, fname, stid)
MP_Link_pt link;
char *fname;
int *stid;
#endif
{
  FILE *fd;
  int more = 0;

  if (fname == NULL) {
    fname = malloc(64);
    printf("Enter name of input file: ");
    scanf("%s", fname);
    }

  if ((fd = fopen(fname, "r")) == NULL) {
    fprintf(stderr, "Can't open %s for reading\n", fname);
    return;
    }

  do {
    pvm_initsend(PvmDataRaw);
/*    pvm_initsend(PvmDataInPlace);*/
    more = imp_get_term(link, fd);
    MP_EndMsg(link);
    pvm_mcast(stid, 2, 0);
    MP_ResetLink(link);
  } while (more);
  free (fname);
  return;

bail:
  pvm_exit(); exit(1);
  free (fname);
  fclose(fd);
}



main(argc, argv)
  int argc;
  char *argv[];
{
  int mytid;   /* my task id */
  int who;
  int i, n = 0, nproc = 2, tids[2];
  int spawn_flag = PvmTaskDefault;
  char *infile, *rhost = NULL;
  MP_Link_pt link = NULL;
  MP_Env_pt  env = NULL;

  if ((infile = IMP_GetCmdlineArg(argc, argv, "-infile")) == NULL) {
    fprintf(stderr, "missing -infile argument!\n");
    exit (1);
  }
  pvm_setopt(PvmRoute, PvmRouteDirect);
  pvm_setopt(PvmAutoErr, 0);      /* tell PVM not to print error to stdout */
  mytid = pvm_mytid();            /* enroll in pvm */

  env = MP_AllocateEnv();
  MP_SetEnvOption(env, MP_BUFFER_SIZE_OPT, pvm_getopt(PvmFragSize));
  env = MP_InitializeEnv(env);
  if (env == NULL) {
    fprintf(stderr, "%s: MP_InitializeEnv() failed!\n", argv[0]);
    exit(1);
    }
   MP_AddEnvTranspDevice(env, MP_PvmTransportDev, &pvm_ops);

  if ((link = MP_OpenLink(env, argc, argv)) == NULL) {
    fprintf(stderr, "%s: MP_OpenLink() failed!\n", argv[0]);
    exit(1);
    }
  MP_SetLinkOption(link, MP_LINK_LOG_MASK_OPT, MP_LOG_INIT_EVENTS);
  MP_SetLinkOption(link, MP_LINK_LOG_MASK_OPT, MP_LOG_WRITE_EVENTS);
  MP_SetLinkOption(link, MP_LINK_LOG_MASK_OPT, MP_LOG_READ_EVENTS);
  /* need the following to use PvmDataInPlace encoding mechanism 
  MP_SetLinkOption(link, MP_LINK_SEND_MODE_OPT, MP_SEND_WHOLE_MSG);*/

  if ((mytid = pvm_mytid()) < 0) exit(1);

  rhost = IMP_GetCmdlineArg(argc, argv, "-remhost");
  if (rhost != NULL) spawn_flag = PvmTaskHost;
  /* start up slave task */
  pvm_spawn(SLAVENAME, argv, spawn_flag, rhost, nproc, tids);
  
  pvm_initsend(PvmDataRaw);
  pvm_pkint(&nproc, 1, 1);
  pvm_pkint(tids, nproc, 1);
  pvm_mcast(tids, nproc, 0);

  imp_file2tree(link, infile, tids);  /* do the work */

  for(i = 0; i < nproc; i++) {
    pvm_recv(-1, 5);   /* make sure message type matches that of sender */
    pvm_upkint(&who, 1, 1);
    printf("Received done signal from %d\n", who);
    }

  MP_CloseLink(link); /* cleanup and exit */
  MP_ReleaseEnv(env);
bail: 
  pvm_exit();
  exit(0);
}
