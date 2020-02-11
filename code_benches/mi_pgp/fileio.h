#define DISKBUFSIZE 4096	/* Size of I/O buffers */

#ifndef SEEK_SET
#define SEEK_SET 0
#define	SEEK_CUR 1
#define SEEK_END 2
#endif

#ifdef VMS
#define PGP_SYSTEM_DIR "PGP$LIBRARY:"
#define FOPRBIN		"rb","ctx=stm"
#define FOPRTXT		"r"
#if 0
#define FOPWBIN		"ab","fop=cif"
#define FOPWTXT		"a","fop=cif"
#else
#define FOPWBIN		"wb"
#define FOPWTXT		"w"
#endif
#define FOPRWBIN	"r+b","ctx=stm"
#define FOPWPBIN	"w+b","ctx=stm"
#else
#ifdef UNIX
/*
 * Directory for system-wide files.  Must end in a /, ready for
 * dumb appending of the filename.  (If not defined, it's not used.)
 */
#ifdef LINUX
#  define PGP_SYSTEM_DIR "/var/lib/pgp/"
#else
#  define PGP_SYSTEM_DIR "/usr/local/lib/pgp/"
#endif
#define FOPRBIN		"r"
#define FOPRTXT		"r"
#define FOPWBIN		"w"
#define FOPWTXT		"w"
#define FOPRWBIN	"r+"
#define FOPWPBIN	"w+"
#else /* !UNIX && !VMS */
#define FOPRBIN		"rb"
#define FOPRTXT		"r"
#define FOPWBIN		"wb"
#define FOPWTXT		"w"
#define FOPRWBIN	"r+b"
#define FOPWPBIN	"w+b"
#endif /* UNIX */
#endif /* VMS */

#define	TMP_WIPE		1
#define	TMP_TMPDIR		4

#define equal_buffers(buf1,buf2,count)	!memcmp( buf1, buf2, count )

/* Returns TRUE iff file is can be opened for reading. */
boolean file_exists(char *filename);

/* Returns TRUE iff file can be opened for writing. Does not harm file! */
boolean file_ok_write(char *filename);

/* Completely overwrite and erase file of given name, so that no sensitive
   information is left on the disk */
int wipefile(char *filename);

/* Return the after-slash part of the filename */
char	*file_tail (char *filename);
/* Returns TRUE if user left off file extension, allowing default */
boolean no_extension(char *filename);

/* Deletes trailing ".xxx" file extension after the period */
void drop_extension(char *filename);

/* Append filename extension if there isn't one already */
void default_extension(char *filename, char *extension);

/* Change the filename extension */
void force_extension(char *filename, char *extension);

/* Get yes/no answer from user, returns TRUE for yes, FALSE for no */
boolean getyesno(char default_answer);

/* If luser consents to it, change the filename extension */
char *maybe_force_extension(char *filename, char *extension);

/* Builds a filename with a complete path specifier from the environmental
   variable PGPPATH */
char *buildfilename(char *result, char *fname);

/* The same, but also searches PGP_SYETEM_DIR */
char *buildsysfilename(char *result, char *fname);

/* Build a path for fileName based on origPath */
int build_path(char *path, char *fileName, char *origPath);

/* Convert filename to canonical form, with slashes as separators */
void file_to_canon(char *filename);

/* Convert filename from canonical to local form */
void file_from_canon(char *filename);

/* Copy file f to file g, for longcount bytes */
int copyfile(FILE *f, FILE *g, word32 longcount);

/* Copy file f to file g, for longcount bytes, positioning f at fpos */
int copyfilepos (FILE *f, FILE *g, word32 longcount, word32 fpos);

/* Copy file f to file g, for longcount bytes.  Convert to canonical form
   as we go.  f is open in text mode.  Canonical form uses crlf's as line
   separators */
int copyfile_to_canon (FILE *f, FILE *g, word32 longcount);

/* Copy file f to file g, for longcount bytes.  Convert from canonical to
   local form as we go.  g is open in text mode.  Canonical form uses crlf's
   as line separators */
int copyfile_from_canon (FILE *f, FILE *g, word32 longcount);

/* Copy srcFile to destFile */
int copyfiles_by_name(char *srcFile, char *destFile);

/* Copy srcFile to destFile, converting to canonical text form */
int make_canonical(char *srcFile, char *destFile);

/* Like rename() but will try to copy the file if the rename fails. This is
   because under OS's with multiple physical volumes if the source and
   destination are on different volumes the rename will fail */
int rename2(char *srcFile, char *destFile);

/* Read the data from stdin to the phantom input file */
int readPhantomInput(char *filename);

/* Write the data from the phantom output file to stdout */
int writePhantomOutput(char *filename);

/* Return the size from the current position of file f to the end */
word32 fsize (FILE *f);

/* Return TRUE if file filename is a text file */
int is_text_file (char *filename);

FILE *fopenbin(char *, char *);
FILE *fopentxt(char *, char *);

VOID *xmalloc(unsigned);

char *tempfile(int);
void rmtemp(char *);
char *savetemp(char *, char *);
char *ck_dup_output(char *, boolean, boolean);
void cleanup_tmpf(void);
int savetempbak(char *, char *);

extern int write_error(FILE *f);
extern void settmpdir(char *path);
extern void setoutdir(char *filename);
extern boolean is_tempfile(char *path);
extern boolean has_extension(char *filename, char *extension);

/* Directories to search for the manuals */
extern char const * const manual_dirs[];
/* Returns non-zero if any manuals are missing */
unsigned manuals_missing(void);

#ifdef __PUREC__
int access(const char *name,int flag);
#endif

#ifdef MACTC5
void mac_cleanup_tmpf(void);
#endif
