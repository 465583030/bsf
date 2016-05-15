/* A Bison parser, made by GNU Bison 2.7.  */

/* Skeleton interface for Bison GLR parsers in C
   
      Copyright (C) 2002-2012 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_BSPARSERFX_H_INCLUDED
# define YY_YY_BSPARSERFX_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
/* Line 2579 of glr.c  */
#line 9 "../../../Source/BansheeSL/BsParserFX.y"

#include "BsMMAlloc.h"
#include "BsASTFX.h"
#include "BsIncludeHandler.h"

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
	typedef void* yyscan_t;
#endif

typedef struct YYLTYPE {
	int first_line;
	int first_column;
	int last_line;
	int last_column;
	char *filename;
} YYLTYPE;
#define YYLTYPE_IS_DECLARED 1

#define YYLLOC_DEFAULT(Current, Rhs, N)																\
	do																								\
		if (N)																						\
		{																							\
			(Current).first_line = YYRHSLOC (Rhs, 1).first_line;									\
			(Current).first_column = YYRHSLOC (Rhs, 1).first_column;								\
			(Current).last_line = YYRHSLOC (Rhs, N).last_line;										\
			(Current).last_column = YYRHSLOC (Rhs, N).last_column;									\
			(Current).filename = YYRHSLOC (Rhs, 1).filename;										\
		}																							\
		else																						\
		{																							\
			(Current).first_line = (Current).last_line = YYRHSLOC (Rhs, 0).last_line;				\
			(Current).first_column = (Current).last_column = YYRHSLOC (Rhs, 0).last_column;			\
			(Current).filename = NULL;																\
		}																							\
	while (0)

#define ADD_PARAMETER(OUTPUT, TYPE, NAME)															\
			OUTPUT = nodeCreate(parse_state->memContext, NT_Parameter);								\
			nodePush(parse_state, OUTPUT);															\
																									\
			NodeOption paramType;																	\
			paramType.type = OT_ParamType;															\
			paramType.value.intValue = TYPE;														\
																									\
			NodeOption paramName;																	\
			paramName.type = OT_Identifier;															\
			paramName.value.strValue = NAME;														\
																									\
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &paramType);		\
			nodeOptionsAdd(parse_state->memContext, parse_state->topNode->options, &paramName);		\



/* Line 2579 of glr.c  */
#line 101 "BsParserFX.h"

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TOKEN_INTEGER = 258,
     TOKEN_FLOAT = 259,
     TOKEN_BOOLEAN = 260,
     TOKEN_STRING = 261,
     TOKEN_IDENTIFIER = 262,
     TOKEN_FILLMODEVALUE = 263,
     TOKEN_CULLMODEVALUE = 264,
     TOKEN_COMPFUNCVALUE = 265,
     TOKEN_OPVALUE = 266,
     TOKEN_COLORMASK = 267,
     TOKEN_ADDRMODEVALUE = 268,
     TOKEN_FILTERVALUE = 269,
     TOKEN_BLENDOPVALUE = 270,
     TOKEN_BUFFERUSAGE = 271,
     TOKEN_QUEUETYPE = 272,
     TOKEN_FLOATTYPE = 273,
     TOKEN_FLOAT2TYPE = 274,
     TOKEN_FLOAT3TYPE = 275,
     TOKEN_FLOAT4TYPE = 276,
     TOKEN_INTTYPE = 277,
     TOKEN_INT2TYPE = 278,
     TOKEN_INT3TYPE = 279,
     TOKEN_INT4TYPE = 280,
     TOKEN_COLORTYPE = 281,
     TOKEN_MAT2x2TYPE = 282,
     TOKEN_MAT2x3TYPE = 283,
     TOKEN_MAT2x4TYPE = 284,
     TOKEN_MAT3x2TYPE = 285,
     TOKEN_MAT3x3TYPE = 286,
     TOKEN_MAT3x4TYPE = 287,
     TOKEN_MAT4x2TYPE = 288,
     TOKEN_MAT4x3TYPE = 289,
     TOKEN_MAT4x4TYPE = 290,
     TOKEN_SAMPLER1D = 291,
     TOKEN_SAMPLER2D = 292,
     TOKEN_SAMPLER3D = 293,
     TOKEN_SAMPLERCUBE = 294,
     TOKEN_SAMPLER2DMS = 295,
     TOKEN_TEXTURE1D = 296,
     TOKEN_TEXTURE2D = 297,
     TOKEN_TEXTURE3D = 298,
     TOKEN_TEXTURECUBE = 299,
     TOKEN_TEXTURE2DMS = 300,
     TOKEN_RWTEXTURE1D = 301,
     TOKEN_RWTEXTURE2D = 302,
     TOKEN_RWTEXTURE3D = 303,
     TOKEN_RWTEXTURE2DMS = 304,
     TOKEN_BYTEBUFFER = 305,
     TOKEN_STRUCTBUFFER = 306,
     TOKEN_RWTYPEDBUFFER = 307,
     TOKEN_RWBYTEBUFFER = 308,
     TOKEN_RWSTRUCTBUFFER = 309,
     TOKEN_RWAPPENDBUFFER = 310,
     TOKEN_RWCONSUMEBUFFER = 311,
     TOKEN_PARAMSBLOCK = 312,
     TOKEN_AUTO = 313,
     TOKEN_ALIAS = 314,
     TOKEN_SHARED = 315,
     TOKEN_USAGE = 316,
     TOKEN_SEPARABLE = 317,
     TOKEN_SORT = 318,
     TOKEN_PRIORITY = 319,
     TOKEN_TRANSPARENT = 320,
     TOKEN_PARAMETERS = 321,
     TOKEN_BLOCKS = 322,
     TOKEN_TECHNIQUE = 323,
     TOKEN_RENDERER = 324,
     TOKEN_LANGUAGE = 325,
     TOKEN_PASS = 326,
     TOKEN_VERTEX = 327,
     TOKEN_FRAGMENT = 328,
     TOKEN_GEOMETRY = 329,
     TOKEN_HULL = 330,
     TOKEN_DOMAIN = 331,
     TOKEN_COMPUTE = 332,
     TOKEN_COMMON = 333,
     TOKEN_STENCILREF = 334,
     TOKEN_FILLMODE = 335,
     TOKEN_CULLMODE = 336,
     TOKEN_DEPTHBIAS = 337,
     TOKEN_SDEPTHBIAS = 338,
     TOKEN_DEPTHCLIP = 339,
     TOKEN_SCISSOR = 340,
     TOKEN_MULTISAMPLE = 341,
     TOKEN_AALINE = 342,
     TOKEN_DEPTHREAD = 343,
     TOKEN_DEPTHWRITE = 344,
     TOKEN_COMPAREFUNC = 345,
     TOKEN_STENCIL = 346,
     TOKEN_STENCILREADMASK = 347,
     TOKEN_STENCILWRITEMASK = 348,
     TOKEN_STENCILOPFRONT = 349,
     TOKEN_STENCILOPBACK = 350,
     TOKEN_FAIL = 351,
     TOKEN_ZFAIL = 352,
     TOKEN_ALPHATOCOVERAGE = 353,
     TOKEN_INDEPENDANTBLEND = 354,
     TOKEN_TARGET = 355,
     TOKEN_INDEX = 356,
     TOKEN_BLEND = 357,
     TOKEN_COLOR = 358,
     TOKEN_ALPHA = 359,
     TOKEN_WRITEMASK = 360,
     TOKEN_SOURCE = 361,
     TOKEN_DEST = 362,
     TOKEN_OP = 363,
     TOKEN_ADDRMODE = 364,
     TOKEN_MINFILTER = 365,
     TOKEN_MAGFILTER = 366,
     TOKEN_MIPFILTER = 367,
     TOKEN_MAXANISO = 368,
     TOKEN_MIPBIAS = 369,
     TOKEN_MIPMIN = 370,
     TOKEN_MIPMAX = 371,
     TOKEN_BORDERCOLOR = 372,
     TOKEN_U = 373,
     TOKEN_V = 374,
     TOKEN_W = 375
   };
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 2579 of glr.c  */
#line 73 "../../../Source/BansheeSL/BsParserFX.y"

	int intValue;
	float floatValue;
	float matrixValue[16];
	int intVectorValue[4];
	const char* strValue;
	ASTFXNode* nodePtr;
	NodeOption nodeOption;


/* Line 2579 of glr.c  */
#line 246 "BsParserFX.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


int yyparse (ParseState* parse_state, yyscan_t scanner);

#endif /* !YY_YY_BSPARSERFX_H_INCLUDED  */
