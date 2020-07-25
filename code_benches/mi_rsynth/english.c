#include <config.h>

/* $Id: english.c,v 1.13 1994/11/08 13:30:50 a904209 Exp a904209 $
 */
char *english_id = "$Id: english.c,v 1.13 1994/11/08 13:30:50 a904209 Exp a904209 $";

/*
   **      English to Phoneme rules.
   **
   **      Derived from:
   **
   **           AUTOMATIC TRANSLATION OF ENGLISH TEXT TO PHONETICS
   **                  BY MEANS OF LETTER-TO-SOUND RULES
   **
   **                      NRL Report 7948
   **
   **                    January 21st, 1976
   **          Naval Research Laboratory, Washington, D.C.
   **
   **
   **      Published by the National Technical Information Service as
   **      document "AD/A021 929".
   **
   **
   **
   **      The Phoneme codes:
   **
   **              IY      bEEt            IH      bIt
   **              EY      gAte            EH      gEt
   **              AE      fAt             AA      fAther
   **              AO      lAWn            OW      lOne
   **              UH      fUll            UW      fOOl
   **              ER      mURdER          AX      About
   **              AH      bUt             AY      hIde
   **              AW      hOW             OY      tOY
   **
   **              p       Pack            b       Back
   **              t       Time            d       Dime
   **              k       Coat            g       Goat
   **              f       Fault           v       Vault
   **              TH      eTHer           DH      eiTHer
   **              s       Sue             z       Zoo
   **              SH      leaSH           ZH      leiSure
   **              HH      How             m       suM
   **              n       suN             NG      suNG
   **              l       Laugh           w       Wear
   **              y       Young           r       Rate
   **              CH      CHar            j       Jar
   **              WH      WHere
   **
   **
   **      Rules are made up of four parts:
   **
   **              The left context.
   **              The text to match.
   **              The right context.
   **              The phonemes to substitute for the matched text.
   **
   **      Procedure:
   **
   **              Seperate each block of letters (apostrophes included)
   **              and add a space on each side.  For each unmatched
   **              letter in the word, look through the rules where the
   **              text to match starts with the letter in the word.  If
   **              the text to match is found and the right and left
   **              context patterns also match, output the phonemes for
   **              that rule and skip to the next unmatched letter.
   **
   **
   **      Special Context Symbols:
   **
   **              #       One or more vowels
   **              :       Zero or more consonants
   **              ^       One consonant.
   **              .       One of B, D, V, G, J, L, M, N, R, W or Z (voiced
   **                      consonants)
   **              %       One of ER, E, ES, ED, ING, ELY (a suffix)
   **                      (Found in right context only)
   **              +       One of E, I or Y (a "front" vowel)
   **
 */


/* Context definitions */
static char Anything[] = "";
 /* No context requirement */

static char Nothing[] = " ";
 /* Context is beginning or end of word */

static char Silent[] = "";
 /* No phonemes */


#define LEFT_PART       0
#define MATCH_PART      1
#define RIGHT_PART      2
#define OUT_PART        3

typedef char *Rule[4];
 /* Rule is an array of 4 character pointers */


/*0 = Punctuation */
/*
   **      LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
 */


static Rule punct_rules[] =
{
 {Anything, " ", Anything, " "},
 {Anything, "-", Anything, ""},
 {".", "'S", Anything, "z"},
 {"#:.E", "'S", Anything, "z"},
 {"#", "'S", Anything, "z"},
 {Anything, "'", Anything, ""},
 {Anything, ",", Anything, " "},
 {Anything, ".", Anything, " "},
 {Anything, "?", Anything, " "},
 {Anything, "!", Anything, " "},
 {Anything, 0, Anything, Silent},
};

static Rule A_rules[] =
{
 {Anything, "A", Nothing, "@"},
 {Nothing, "ARE", Nothing, "0r"},
 {Nothing, "AR", "O", "@r"},
 {Anything, "AR", "#", "er"},
 {"^", "AS", "#", "eIs"},
 {Anything, "A", "WA", "@"},
 {Anything, "AW", Anything, "O"},
 {" :", "ANY", Anything, "eni"},
 {Anything, "A", "^+#", "eI"},
 {"#:", "ALLY", Anything, "@li"},
 {Nothing, "AL", "#", "@l"},
 {Anything, "AGAIN", Anything, "@gen"},
 {"#:", "AG", "E", "IdZ"},
 {Anything, "A", "^+:#", "&"},
 {" :", "A", "^+ ", "eI"},
 {Anything, "A", "^%", "eI"},
 {Nothing, "ARR", Anything, "@r"},
 {Anything, "ARR", Anything, "&r"},
 {" :", "AR", Nothing, "0r"},
 {Anything, "AR", Nothing, "3"},
 {Anything, "AR", Anything, "0r"},
 {Anything, "AIR", Anything, "er"},
 {Anything, "AI", Anything, "eI"},
 {Anything, "AY", Anything, "eI"},
 {Anything, "AU", Anything, "O"},
 {"#:", "AL", Nothing, "@l"},
 {"#:", "ALS", Nothing, "@lz"},
 {Anything, "ALK", Anything, "Ok"},
 {Anything, "AL", "^", "Ol"},
 {" :", "ABLE", Anything, "eIb@l"},
 {Anything, "ABLE", Anything, "@b@l"},
 {Anything, "ANG", "+", "eIndZ"},
 {"^", "A", "^#", "eI"},
 {Anything, "A", Anything, "&"},
 {Anything, 0, Anything, Silent},
};

static Rule B_rules[] =
{
 {Nothing, "BE", "^#", "bI"},
 {Anything, "BEING", Anything, "biIN"},
 {Nothing, "BOTH", Nothing, "b@UT"},
 {Nothing, "BUS", "#", "bIz"},
 {Anything, "BUIL", Anything, "bIl"},
 {Anything, "B", Anything, "b"},
 {Anything, 0, Anything, Silent},
};

static Rule C_rules[] =
{
 {Nothing, "CH", "^", "k"},
 {"^E", "CH", Anything, "k"},
 {Anything, "CH", Anything, "tS"},
 {" S", "CI", "#", "saI"},
 {Anything, "CI", "A", "S"},
 {Anything, "CI", "O", "S"},
 {Anything, "CI", "EN", "S"},
 {Anything, "C", "+", "s"},
 {Anything, "CK", Anything, "k"},
 {Anything, "COM", "%", "kVm"},
 {Anything, "C", Anything, "k"},
 {Anything, 0, Anything, Silent},
};

static Rule D_rules[] =
{
 {"#:", "DED", Nothing, "dId"},
 {".E", "D", Nothing, "d"},
 {"#:^E", "D", Nothing, "t"},
 {Nothing, "DE", "^#", "dI"},
 {Nothing, "DO", Nothing, "du"},
 {Nothing, "DOES", Anything, "dVz"},
 {Nothing, "DOING", Anything, "duIN"},
 {Nothing, "DOW", Anything, "daU"},
 {Anything, "DU", "A", "dZu"},
 {Anything, "D", Anything, "d"},
 {Anything, 0, Anything, Silent},
};

static Rule E_rules[] =
{
 {"#:", "E", Nothing, ""},
 {"':^", "E", Nothing, ""},
 {" :", "E", Nothing, "i"},
 {"#", "ED", Nothing, "d"},
 {"#:", "E", "D ", ""},
 {Anything, "EV", "ER", "ev"},
 {Anything, "E", "^%", "i"},
 {Anything, "ERI", "#", "iri"},
 {Anything, "ERI", Anything, "erI"},
 {"#:", "ER", "#", "3"},
 {Anything, "ER", "#", "er"},
 {Anything, "ER", Anything, "3"},
 {Nothing, "EVEN", Anything, "iven"},
 {"#:", "E", "W", ""},
 {"T", "EW", Anything, "u"},
 {"S", "EW", Anything, "u"},
 {"R", "EW", Anything, "u"},
 {"D", "EW", Anything, "u"},
 {"L", "EW", Anything, "u"},
 {"Z", "EW", Anything, "u"},
 {"N", "EW", Anything, "u"},
 {"J", "EW", Anything, "u"},
 {"TH", "EW", Anything, "u"},
 {"CH", "EW", Anything, "u"},
 {"SH", "EW", Anything, "u"},
 {Anything, "EW", Anything, "ju"},
 {Anything, "E", "O", "i"},
 {"#:S", "ES", Nothing, "Iz"},
 {"#:C", "ES", Nothing, "Iz"},
 {"#:G", "ES", Nothing, "Iz"},
 {"#:Z", "ES", Nothing, "Iz"},
 {"#:X", "ES", Nothing, "Iz"},
 {"#:J", "ES", Nothing, "Iz"},
 {"#:CH", "ES", Nothing, "Iz"},
 {"#:SH", "ES", Nothing, "Iz"},
 {"#:", "E", "S ", ""},
 {"#:", "ELY", Nothing, "li"},
 {"#:", "EMENT", Anything, "ment"},
 {Anything, "EFUL", Anything, "fUl"},
 {Anything, "EE", Anything, "i"},
 {Anything, "EARN", Anything, "3n"},
 {Nothing, "EAR", "^", "3"},
 {Anything, "EAD", Anything, "ed"},
 {"#:", "EA", Nothing, "i@"},
 {Anything, "EA", "SU", "e"},
 {Anything, "EA", Anything, "i"},
 {Anything, "EIGH", Anything, "eI"},
 {Anything, "EI", Anything, "i"},
 {Nothing, "EYE", Anything, "aI"},
 {Anything, "EY", Anything, "i"},
 {Anything, "EU", Anything, "ju"},
 {Anything, "E", Anything, "e"},
 {Anything, 0, Anything, Silent},
};

static Rule F_rules[] =
{
 {Anything, "FUL", Anything, "fUl"},
 {Anything, "F", Anything, "f"},
 {Anything, 0, Anything, Silent},
};

static Rule G_rules[] =
{
 {Anything, "GIV", Anything, "gIv"},
 {Nothing, "G", "I^", "g"},
 {Anything, "GE", "T", "ge"},
 {"SU", "GGES", Anything, "gdZes"},
 {Anything, "GG", Anything, "g"},
 {" B#", "G", Anything, "g"},
 {Anything, "G", "+", "dZ"},
 {Anything, "GREAT", Anything, "greIt"},
 {"#", "GH", Anything, ""},
 {Anything, "G", Anything, "g"},
 {Anything, 0, Anything, Silent},
};

static Rule H_rules[] =
{
 {Nothing, "HAV", Anything, "h&v"},
 {Nothing, "HERE", Anything, "hir"},
 {Nothing, "HOUR", Anything, "aU3"},
 {Anything, "HOW", Anything, "haU"},
 {Anything, "H", "#", "h"},
 {Anything, "H", Anything, ""},
 {Anything, 0, Anything, Silent},
};

static Rule I_rules[] =
{
 {Nothing, "IAIN", Nothing, "I@n"},
 {Nothing, "ING", Nothing, "IN"},
 {Nothing, "IN", Anything, "In"},
 {Nothing, "I", Nothing, "aI"},
 {Anything, "IN", "D", "aIn"},
 {Anything, "IER", Anything, "i3"},
 {"#:R", "IED", Anything, "id"},
 {Anything, "IED", Nothing, "aId"},
 {Anything, "IEN", Anything, "ien"},
 {Anything, "IE", "T", "aIe"},
 {" :", "I", "%", "aI"},
 {Anything, "I", "%", "i"},
 {Anything, "IE", Anything, "i"},
 {Anything, "I", "^+:#", "I"},
 {Anything, "IR", "#", "aIr"},
 {Anything, "IZ", "%", "aIz"},
 {Anything, "IS", "%", "aIz"},
 {Anything, "I", "D%", "aI"},
 {"+^", "I", "^+", "I"},
 {Anything, "I", "T%", "aI"},
 {"#:^", "I", "^+", "I"},
 {Anything, "I", "^+", "aI"},
 {Anything, "IR", Anything, "3"},
 {Anything, "IGH", Anything, "aI"},
 {Anything, "ILD", Anything, "aIld"},
 {Anything, "IGN", Nothing, "aIn"},
 {Anything, "IGN", "^", "aIn"},
 {Anything, "IGN", "%", "aIn"},
 {Anything, "IQUE", Anything, "ik"},
 {"^", "I", "^#", "aI"},
 {Anything, "I", Anything, "I"},
 {Anything, 0, Anything, Silent},
};

static Rule J_rules[] =
{
 {Anything, "J", Anything, "dZ"},
 {Anything, 0, Anything, Silent},
};

static Rule K_rules[] =
{
 {Nothing, "K", "N", ""},
 {Anything, "K", Anything, "k"},
 {Anything, 0, Anything, Silent},
};

static Rule L_rules[] =
{
 {Anything, "LO", "C#", "l@U"},
 {"L", "L", Anything, ""},
 {"#:^", "L", "%", "@l"},
 {Anything, "LEAD", Anything, "lid"},
 {Anything, "L", Anything, "l"},
 {Anything, 0, Anything, Silent},
};

static Rule M_rules[] =
{
 {Anything, "MOV", Anything, "muv"},
 {"#", "MM", "#", "m"},
 {Anything, "M", Anything, "m"},
 {Anything, 0, Anything, Silent},
};

static Rule N_rules[] =
{
 {"E", "NG", "+", "ndZ"},
 {Anything, "NG", "R", "Ng"},
 {Anything, "NG", "#", "Ng"},
 {Anything, "NGL", "%", "Ng@l"},
 {Anything, "NG", Anything, "N"},
 {Anything, "NK", Anything, "Nk"},
 {Nothing, "NOW", Nothing, "naU"},
 {"#", "NG", Nothing, "Ng"},
 {Anything, "N", Anything, "n"},
 {Anything, 0, Anything, Silent},
};

static Rule O_rules[] =
{
 {Anything, "OF", Nothing, "@v"},
 {Anything, "OROUGH", Anything, "3@U"},
 {"#:", "OR", Nothing, "3"},
 {"#:", "ORS", Nothing, "3z"},
 {Anything, "OR", Anything, "Or"},
 {Nothing, "ONE", Anything, "wVn"},
 {Anything, "OW", Anything, "@U"},
 {Nothing, "OVER", Anything, "@Uv3"},
 {Anything, "OV", Anything, "Vv"},
 {Anything, "O", "^%", "@U"},
 {Anything, "O", "^EN", "@U"},
 {Anything, "O", "^I#", "@U"},
 {Anything, "OL", "D", "@Ul"},
 {Anything, "OUGHT", Anything, "Ot"},
 {Anything, "OUGH", Anything, "Vf"},
 {Nothing, "OU", Anything, "aU"},
 {"H", "OU", "S#", "aU"},
 {Anything, "OUS", Anything, "@s"},
 {Anything, "OUR", Anything, "Or"},
 {Anything, "OULD", Anything, "Ud"},
 {"^", "OU", "^L", "V"},
 {Anything, "OUP", Anything, "up"},
 {Anything, "OU", Anything, "aU"},
 {Anything, "OY", Anything, "oI"},
 {Anything, "OING", Anything, "@UIN"},
 {Anything, "OI", Anything, "oI"},
 {Anything, "OOR", Anything, "Or"},
 {Anything, "OOK", Anything, "Uk"},
 {Anything, "OOD", Anything, "Ud"},
 {Anything, "OO", Anything, "u"},
 {Anything, "O", "E", "@U"},
 {Anything, "O", Nothing, "@U"},
 {Anything, "OA", Anything, "@U"},
 {Nothing, "ONLY", Anything, "@Unli"},
 {Nothing, "ONCE", Anything, "wVns"},
 {Anything, "ON'T", Anything, "@Unt"},
 {"C", "O", "N", "0"},
 {Anything, "O", "NG", "O"},
 {" :^", "O", "N", "V"},
 {"I", "ON", Anything, "@n"},
 {"#:", "ON", Nothing, "@n"},
 {"#^", "ON", Anything, "@n"},
 {Anything, "O", "ST ", "@U"},
 {Anything, "OF", "^", "Of"},
 {Anything, "OTHER", Anything, "VD3"},
 {Anything, "OSS", Nothing, "Os"},
 {"#:^", "OM", Anything, "Vm"},
 {Anything, "O", Anything, "0"},
 {Anything, 0, Anything, Silent},
};

static Rule P_rules[] =
{
 {Anything, "PH", Anything, "f"},
 {Anything, "PEOP", Anything, "pip"},
 {Anything, "POW", Anything, "paU"},
 {Anything, "PUT", Nothing, "pUt"},
 {Anything, "P", Anything, "p"},
 {Anything, 0, Anything, Silent},
};

static Rule Q_rules[] =
{
 {Anything, "QUAR", Anything, "kwOr"},
 {Anything, "QU", Anything, "kw"},
 {Anything, "Q", Anything, "k"},
 {Anything, 0, Anything, Silent},
};

static Rule R_rules[] =
{
 {Nothing, "RE", "^#", "ri"},
 {Anything, "R", Anything, "r"},
 {Anything, 0, Anything, Silent},
};

static Rule S_rules[] =
{
 {Anything, "SH", Anything, "S"},
 {"#", "SION", Anything, "Z@n"},
 {Anything, "SOME", Anything, "sVm"},
 {"#", "SUR", "#", "Z3"},
 {Anything, "SUR", "#", "S3"},
 {"#", "SU", "#", "Zu"},
 {"#", "SSU", "#", "Su"},
 {"#", "SED", Nothing, "zd"},
 {"#", "S", "#", "z"},
 {Anything, "SAID", Anything, "sed"},
 {"^", "SION", Anything, "S@n"},
 {Anything, "S", "S", ""},
 {".", "S", Nothing, "z"},
 {"#:.E", "S", Nothing, "z"},
 {"#:^##", "S", Nothing, "z"},
 {"#:^#", "S", Nothing, "s"},
 {"U", "S", Nothing, "s"},
 {" :#", "S", Nothing, "z"},
 {Nothing, "SCH", Anything, "sk"},
 {Anything, "S", "C+", ""},
 {"#", "SM", Anything, "zm"},
 {"#", "SN", "'", "z@n"},
 {Anything, "S", Anything, "s"},
 {Anything, 0, Anything, Silent},
};

static Rule T_rules[] =
{
 {Nothing, "THE", Nothing, "D@"},
 {Anything, "TO", Nothing, "tu"},
 {Anything, "THAT", Nothing, "D&t"},
 {Nothing, "THIS", Nothing, "DIs"},
 {Nothing, "THEY", Anything, "DeI"},
 {Nothing, "THERE", Anything, "Der"},
 {Anything, "THER", Anything, "D3"},
 {Anything, "THEIR", Anything, "Der"},
 {Nothing, "THAN", Nothing, "D&n"},
 {Nothing, "THEM", Nothing, "Dem"},
 {Anything, "THESE", Nothing, "Diz"},
 {Nothing, "THEN", Anything, "Den"},
 {Anything, "THROUGH", Anything, "Tru"},
 {Anything, "THOSE", Anything, "D@Uz"},
 {Anything, "THOUGH", Nothing, "D@U"},
 {Nothing, "THUS", Anything, "DVs"},
 {Anything, "TH", Anything, "T"},
 {"#:", "TED", Nothing, "tId"},
 {"S", "TI", "#N", "tS"},
 {Anything, "TI", "O", "S"},
 {Anything, "TI", "A", "S"},
 {Anything, "TIEN", Anything, "S@n"},
 {Anything, "TUR", "#", "tS3"},
 {Anything, "TU", "A", "tSu"},
 {Nothing, "TWO", Anything, "tu"},
 {Anything, "T", Anything, "t"},
 {Anything, 0, Anything, Silent},
};

static Rule U_rules[] =
{
 {Nothing, "UN", "I", "jun"},
 {Nothing, "UN", Anything, "Vn"},
 {Nothing, "UPON", Anything, "@pOn"},
 {"T", "UR", "#", "Ur"},
 {"S", "UR", "#", "Ur"},
 {"R", "UR", "#", "Ur"},
 {"D", "UR", "#", "Ur"},
 {"L", "UR", "#", "Ur"},
 {"Z", "UR", "#", "Ur"},
 {"N", "UR", "#", "Ur"},
 {"J", "UR", "#", "Ur"},
 {"TH", "UR", "#", "Ur"},
 {"CH", "UR", "#", "Ur"},
 {"SH", "UR", "#", "Ur"},
 {Anything, "UR", "#", "jUr"},
 {Anything, "UR", Anything, "3"},
 {Anything, "U", "^ ", "V"},
 {Anything, "U", "^^", "V"},
 {Anything, "UY", Anything, "aI"},
 {" G", "U", "#", ""},
 {"G", "U", "%", ""},
 {"G", "U", "#", "w"},
 {"#N", "U", Anything, "ju"},
 {"T", "U", Anything, "u"},
 {"S", "U", Anything, "u"},
 {"R", "U", Anything, "u"},
 {"D", "U", Anything, "u"},
 {"L", "U", Anything, "u"},
 {"Z", "U", Anything, "u"},
 {"N", "U", Anything, "u"},
 {"J", "U", Anything, "u"},
 {"TH", "U", Anything, "u"},
 {"CH", "U", Anything, "u"},
 {"SH", "U", Anything, "u"},
 {Anything, "U", Anything, "ju"},
 {Anything, 0, Anything, Silent},
};

static Rule V_rules[] =
{
 {Anything, "VIEW", Anything, "vju"},
 {Anything, "V", Anything, "v"},
 {Anything, 0, Anything, Silent},
};

static Rule W_rules[] =
{
 {Nothing, "WERE", Anything, "w3"},
 {Anything, "WA", "S", "w0"},
 {Anything, "WA", "T", "w0"},
 {Anything, "WHERE", Anything, "hwer"},
 {Anything, "WHAT", Anything, "hw0t"},
 {Anything, "WHOL", Anything, "h@Ul"},
 {Anything, "WHO", Anything, "hu"},
 {Anything, "WH", Anything, "hw"},
 {Anything, "WAR", Anything, "wOr"},
 {Anything, "WOR", "^", "w3"},
 {Anything, "WR", Anything, "r"},
 {Anything, "W", Anything, "w"},
 {Anything, 0, Anything, Silent},
};

static Rule X_rules[] =
{
 {Anything, "X", Anything, "ks"},
 {Anything, 0, Anything, Silent},
};

static Rule Y_rules[] =
{
 {Anything, "YOUNG", Anything, "jVN"},
 {Nothing, "YOU", Anything, "ju"},
 {Nothing, "YES", Anything, "jes"},
 {Nothing, "Y", Anything, "j"},
 {"#:^", "Y", Nothing, "i"},
 {"#:^", "Y", "I", "i"},
 {" :", "Y", Nothing, "aI"},
 {" :", "Y", "#", "aI"},
 {" :", "Y", "^+:#", "I"},
 {" :", "Y", "^#", "aI"},
 {Anything, "Y", Anything, "I"},
 {Anything, 0, Anything, Silent},
};

static Rule Z_rules[] =
{
 {Anything, "Z", Anything, "z"},
 {Anything, 0, Anything, Silent},
};

Rule *Rules[] =
{
 punct_rules,
 A_rules, B_rules, C_rules, D_rules, E_rules, F_rules, G_rules,
 H_rules, I_rules, J_rules, K_rules, L_rules, M_rules, N_rules,
 O_rules, P_rules, Q_rules, R_rules, S_rules, T_rules, U_rules,
 V_rules, W_rules, X_rules, Y_rules, Z_rules
};
